"""Record the human RAW definition without changing geometry or original source."""
import hashlib
import json
from pathlib import Path
from prepare_star import audit_one_raw

ROOT = Path(__file__).resolve().parents[1]
folder = ROOT / "prepared/star-2500-corner"
manifest = json.loads((folder / "manifest.json").read_text(encoding="utf-8"))
for entry in manifest:
    path = folder / entry["file"]
    if hashlib.sha256(path.read_bytes()).hexdigest() != entry["sha256"]:
        raise RuntimeError("Prepared input changed before clarification: " + entry["file"])
preflight = folder / "raw-preflight.json"
backup = ROOT / "provenance/raw-preflight-before-clarification-20261009.json"
if backup.exists():
    raise RuntimeError("Clarification already recorded; inspect before repeating")
backup.write_bytes(preflight.read_bytes())
layout = json.loads((folder / "topology.json").read_text(encoding="utf-8"))
preflight.write_text(json.dumps(audit_one_raw(layout), indent=2) + "\n", encoding="utf-8")
for entry in manifest:
    path = folder / entry["file"]
    entry["bytes"] = path.stat().st_size
    entry["sha256"] = hashlib.sha256(path.read_bytes()).hexdigest()
(folder / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
print("RAW policy recorded; station geometry and original source unchanged")
