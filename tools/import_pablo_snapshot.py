"""Import the user-supplied Pablo archive without executing archive contents."""
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path, PurePosixPath
import stat
import zipfile

ROOT = Path(__file__).resolve().parents[1]
ARCHIVE = Path(r"C:\Users\mateu\Downloads\relay-halow-tree-main.zip")
SOURCE = ROOT / "source"
EXPECTED = "9713d7d51f6c2300632a39b19e59eb1c7de6abe29c6273ade39de424d28d5bd6"


def sha(data):
    return hashlib.sha256(data).hexdigest()


def main():
    if SOURCE.exists():
        raise SystemExit("Source already exists; refusing to overwrite the research base")
    if sha(ARCHIVE.read_bytes()) != EXPECTED:
        raise SystemExit("Archive identity changed")
    included, skipped, seen, planned = [], [], set(), []
    with zipfile.ZipFile(ARCHIVE) as archive:
        entries = archive.infolist()
        for entry in entries:
            path = PurePosixPath(entry.filename)
            if entry.filename.startswith(("/", "\\")) or ".." in path.parts or "\\" in entry.filename:
                raise ValueError(f"Unsafe member: {entry.filename}")
            if entry.is_dir():
                continue
            reason = None
            if stat.S_ISLNK(entry.external_attr >> 16):
                reason = "archive symlink not materialized on Windows; retained in supplied ZIP"
            elif any(part in (".git", "build", "__pycache__") or part.startswith(".waf") for part in path.parts) or path.suffix in (".pyc", ".pyo"):
                reason = "generated build/cache or repository metadata"
            elif any(any(c in '<>:"|?*' or ord(c) < 32 for c in part) or part.endswith((" ", ".")) for part in path.parts):
                reason = "legacy filename not portable to Windows; retained in supplied ZIP"
            if reason:
                skipped.append({"member": entry.filename, "reason": reason})
                continue
            key = entry.filename.casefold()
            if key in seen:
                raise ValueError(f"Case-insensitive path collision: {entry.filename}")
            seen.add(key)
            planned.append((entry, path))
        # Validate every path before creating files, so a bad late member
        # cannot leave an apparently complete import behind.
        for entry, path in planned:
            target = SOURCE.joinpath(*path.parts)
            target.parent.mkdir(parents=True, exist_ok=True)
            data = archive.read(entry)
            target.write_bytes(data)
            included.append({"member": entry.filename, "bytes": len(data), "sha256": sha(data)})
    provenance = ROOT / "provenance"
    provenance.mkdir(exist_ok=True)
    report = {"importedAtUtc": datetime.now(timezone.utc).isoformat(), "archive": str(ARCHIVE),
              "archiveSha256": EXPECTED, "archiveBytes": ARCHIVE.stat().st_size, "archiveEntries": len(entries),
              "included": included, "skipped": skipped, "contentsExecuted": False}
    (provenance / "import.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"source": str(SOURCE), "included": len(included), "skipped": len(skipped), "archiveSha256": EXPECTED}))


if __name__ == "__main__":
    main()
