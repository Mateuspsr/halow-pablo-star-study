"""Record isolated checks and original-source integrity; never label as ns-3 runs."""
import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

manifest = json.loads((ROOT / "provenance/import.json").read_text(encoding="utf-8"))
changed = []
expected = set()
for entry in manifest["included"]:
    path = ROOT / "source" / entry["member"]
    expected.add(path.resolve())
    if not path.is_file() or sha(path) != entry["sha256"]:
        changed.append(entry["member"])
extra = [str(path.relative_to(ROOT / "source")) for path in
         (ROOT / "source").rglob("*") if path.is_file() and path.resolve() not in expected]
logs = [ROOT / "validation/raw-membership/run.log", ROOT / "validation/link-quality-run.log"]
inputs = [ROOT / "implementation/raw-membership.h", ROOT / "tests/raw-membership-check.cc",
          ROOT / "implementation/link-quality.h", ROOT / "implementation/link-quality.cc",
          ROOT / "tests/link-quality-check.cc", ROOT / "study/FOUR_SCENARIO_CONTRACT.json"]
report = {
    "recordedAtUtc": datetime.now(timezone.utc).isoformat(),
    "clientStudyDate": "2026-10-09",
    "sourceArchiveSha256": manifest["archiveSha256"],
    "originalSource": {"filesChecked": len(expected), "changedOrMissing": changed,
                       "extraFiles": extra, "intact": not changed and not extra},
    "scope": "standalone membership and link-quality regression components only",
    "logs": [{"path": str(path.relative_to(ROOT)), "sha256": sha(path),
              "output": path.read_text(encoding="utf-8")} for path in logs],
    "inputs": [{"path": str(path.relative_to(ROOT)), "sha256": sha(path)} for path in inputs],
    "ns3IntegrationValidated": False,
    "ns3SimulationExecuted": False,
    "researchResultsAvailable": False
}
destination = ROOT / "validation/components-20261009.json"
destination.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
print(json.dumps({"report": str(destination), "originalSource": report["originalSource"]}))
if changed or extra:
    raise SystemExit(1)
