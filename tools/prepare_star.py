"""Prepare the confirmed 2500-STA star geometry and audit RAW representability."""
from __future__ import annotations
import argparse
from collections import defaultdict
import hashlib
import json
import math
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BASE_HASH = "9713d7d51f6c2300632a39b19e59eb1c7de6abe29c6273ade39de424d28d5bd6"


def make_layout():
    # The professor allowed the gateway at the edge. Leave x=0 for the AP;
    # stations remain a full 50x50 lattice, including the right boundary.
    stations = []
    for row in range(50):
        for column in range(50):
            aid = len(stations) + 1  # Official star installs STA devices first.
            x, y = (column + 1) * 100, row * 100
            stations.append({"station": aid, "role": "STA", "xMeters": x, "yMeters": y,
                             "zMeters": 0, "distanceToGatewayMeters": math.hypot(x, y),
                             "expectedAidIfOfficialInstallOrder": aid,
                             "expectedAidPage": aid >> 11, "expectedPageLocalAid": aid & 2047})
    return {"schemaVersion": 1, "sourceArchiveSha256": BASE_HASH,
            "starReference": "scratch/test/s1g-test-tim-raw.cc", "nStations": 2500,
            "nGateways": 1, "nRelays": 0, "nTotalNodes": 2501,
            "domainMeters": {"width": 5000, "height": 5000},
            "lattice": {"rows": 50, "columns": 50, "spacingMeters": 100,
                        "xFirst": 100, "xLast": 5000, "yFirst": 0, "yLast": 4900},
            "gateway": {"role": "AP", "xMeters": 0, "yMeters": 0, "zMeters": 0,
                        "placement": "domain-corner"},
            "aidMappingStatus": "planned-official-install-order; confirm by runtime trace",
            "stations": stations}


def validate_layout(layout):
    points = [(layout["gateway"]["xMeters"], layout["gateway"]["yMeters"])]
    points += [(s["xMeters"], s["yMeters"]) for s in layout["stations"]]
    if len(points) != 2501 or len(set(points)) != len(points):
        raise ValueError("Incorrect population or overlapping positions")
    if any(not (0 <= x <= 5000 and 0 <= y <= 5000) for x, y in points):
        raise ValueError("Position outside the stated domain")
    # Check all potentially violating pairs in spatial bins, rather than
    # asserting spacing merely because the generator used a grid formula.
    bins = defaultdict(list)
    minimum_squared = math.inf
    for x, y in points:
        bx, by = math.floor(x / 100), math.floor(y / 100)
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1):
                for px, py in bins[(bx + dx, by + dy)]:
                    distance = (x - px) ** 2 + (y - py) ** 2
                    minimum_squared = min(minimum_squared, distance)
                    if distance < 100 ** 2:
                        raise ValueError("A pair violates the 100m separation rule")
        bins[(bx, by)].append((x, y))
    return {"passed": True, "minimumSeparationIncludingGatewayMeters": math.sqrt(minimum_squared),
            "nearestStationMeters": min(s["distanceToGatewayMeters"] for s in layout["stations"]),
            "farthestStationMeters": max(s["distanceToGatewayMeters"] for s in layout["stations"]),
            "violatingPairs": 0, "topologyExecutedInNs3": False}


def audit_one_raw(layout):
    counts = defaultdict(int)
    for station in layout["stations"]:
        counts[station["expectedAidPage"]] += 1
    return {"status": "blocked_global_raw_implementation_and_model_validation", "nPhysicalStations": 2500,
            "confirmedPolicy": "one RAW group for all stations; no page rotation",
            "onePageRangeDescriptorRepresentsAllStations": False,
            "pagePopulationExpected": dict(counts), "aidEnd2500EncodedIn11Bits": 2500 & 2047,
            "pageRangeDescriptorsForAllAssignedAids": 2,
            "pageDescriptors": [{"page": 0, "localAidStart": 1, "localAidEnd": 2047},
                                {"page": 1, "localAidStart": 0, "localAidEnd": 452}],
            "policyDecisionPending": False,
            "pageRangesAreChosenPolicy": False,
            "globalAllStationsSupportValidated": False,
            "knownModelWork": ["Validate all-stations RAW representation and AP/STA membership without page rotation",
                               "AP must reconstruct global AID from RAW page/local fields for page-scoped groups",
                               "TIM/page selection must align with the advertised RAW page",
                               "partial association must not dereference absent AID entries",
                               "RPS copies must own their active storage",
                               "traffic must not wait for all geographically unreachable STAs"],
            "researchEligible": False, "simulationStarted": False}


def drawing(layout):
    left, top, scale = 60, 50, .12
    pieces = ['<svg xmlns="http://www.w3.org/2000/svg" width="760" height="750" viewBox="0 0 760 750">',
              '<rect width="760" height="750" fill="white"/>',
              '<text x="60" y="24" font-family="Arial" font-size="18">Pablo: 2500 STAs + 1 AP — planned star geometry</text>',
              '<rect x="60" y="50" width="600" height="600" fill="#f8fafc" stroke="#334155"/>']
    for station in layout["stations"]:
        x = left + station["xMeters"] * scale
        y = top + (5000 - station["yMeters"]) * scale
        color = "#2563eb" if station["expectedAidPage"] == 0 else "#ea580c"
        pieces.append(f'<circle cx="{x:g}" cy="{y:g}" r="1.7" fill="{color}"/>')
    pieces.extend(['<circle cx="60" cy="650" r="6" fill="#b91c1c"/>',
                   '<text x="75" y="645" font-family="Arial" font-size="14">AP (0,0)</text>',
                   '<text x="300" y="680" font-family="Arial" font-size="14">x [m]: 0 … 5000</text>',
                   '<text x="15" y="390" transform="rotate(-90 15 390)" font-family="Arial" font-size="14">y [m]: 0 … 5000</text>',
                   '<text x="60" y="708" font-family="Arial" font-size="13">100 m minimum including AP; blue/orange: expected AID pages 0/1.</text>',
                   '<text x="60" y="729" font-family="Arial" font-size="13">Geometry only. No coverage, association or packet-delivery result is shown.</text>', '</svg>'])
    return "\n".join(pieces)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT / "prepared" / "star-2500-corner")
    args = parser.parse_args()
    output = args.output.resolve()
    output.relative_to(ROOT)
    output.mkdir(parents=True, exist_ok=False)
    layout = make_layout()
    verification = validate_layout(layout)
    documents = {"topology.json": layout, "geometry-verification.json": verification,
                 "raw-preflight.json": audit_one_raw(layout)}
    for name, document in documents.items():
        (output / name).write_text(json.dumps(document, indent=2) + "\n", encoding="utf-8")
    (output / "layout.svg").write_text(drawing(layout), encoding="utf-8")
    manifest = [{"file": path.name, "bytes": path.stat().st_size,
                 "sha256": hashlib.sha256(path.read_bytes()).hexdigest()} for path in sorted(output.iterdir())]
    (output / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"output": str(output), "geometry": verification,
                      "rawStatus": documents["raw-preflight.json"]["status"]}))


if __name__ == "__main__":
    main()
