#!/usr/bin/env python3
"""The HomeVoice proof, for the fork's GitHub workflow.

    python .github/homevoice/prove.py tasmota32c3 --out dist

A COPY of the hub's own proof (esp_firmware/build_firmware.py:
REQUIRED_MARKERS, DROPPED_BY_TASMOTA, expected_markers, prove, and where
Tasmota leaves its images). It is a copy because the fork's workflow cannot
import the hub; it is the SAME logic because the hub's tests
(tests/test_custom_build.py) load this file and fail the moment the two
disagree. Change the hub's, then copy this file over again.

What it does for one env, after `pio run -e <env>`:
  1. finds the map and both images: build_output/ first (where Tasmota's
     pio-tools/name-firmware.py puts them, moving the map out of .pio),
     then .pio/build/<env>/ -- and says which it used;
  2. proves the map: every driver the override asks for has its entry
     function in the KEPT part of the map (minus what Tasmota itself drops
     for this override, each drop explained);
  3. copies the two images into --out under the names the hub installs:
     <env>.factory.bin and <env>.bin.
Exit 0 = proven and copied; 1 = anything else, with the reason.
Standard library only.
"""
import argparse
import os
import re
import shutil
import sys
from pathlib import Path

# ── the proof: keep in step with esp_firmware/build_firmware.py ────────

#: The entry function is `bool XsnsNN(uint32_t)`. Its mangled name ends in
#: the letter for uint32_t, and that letter changed: `j` (unsigned int) on
#: the old toolchain, `m` (unsigned long) since ESP-IDF 5 on EVERY family.
#: Found 24 Sep 2026: with only `j`, the first all-families run "missed"
#: all five drivers on S2 and S3 -- builds that were fine. Both accepted.
REQUIRED_MARKERS = {
    "RC522 card reader (Xsns80)": r"_z6xsns80[jm]|xsns80\(",
    "Display core (Xdrv13)": r"_z6xdrv13[jm]|xdrv13\(",
    "Character LCD (Xdsp01)": r"_z6xdsp01[jm]|xdsp01\(",
    "TM1637 seven-segment (Xdsp15)": r"_z6xdsp15[jm]|xdsp15\(",
    "MAX7219 dot matrix (Xdsp19)": r"_z6xdsp19[jm]|xdsp19\(",
    "MPU6050 IMU (Xsns32)": r"_z6xsns32[jm]|xsns32\(",
}

DROPPED_BY_TASMOTA = {
    "USE_DISPLAY_MAX7219_MATRIX": (
        ("TM1637 seven-segment (Xdsp15)",),
        "Tasmota's template header undefines USE_DISPLAY_TM1637 (and "
        "USE_DISPLAY_MAX7219) whenever USE_DISPLAY_MAX7219_MATRIX is set: "
        "the dot-matrix and the seven-segment drivers share the MAX7219 "
        "pin names and only one can be in an image. This build carries the "
        "MATRIX. To have TM1637 instead, remove USE_DISPLAY_MAX7219_MATRIX "
        "from the override and build again."),
}


def defines_in(override_text):
    """The USE_... names an override defines (commented lines ignored)."""
    out = set()
    for line in override_text.splitlines():
        line = line.split("//", 1)[0]
        m = re.match(r"\s*#\s*define\s+(USE_[A-Z0-9_]+)", line)
        if m:
            out.add(m.group(1))
    return out


def expected_markers(override_text=""):
    """(markers to prove, notes to say)."""
    wanted = dict(REQUIRED_MARKERS)
    notes = []
    present = defines_in(override_text)
    for define, (dropped, why) in DROPPED_BY_TASMOTA.items():
        if define in present:
            for label in dropped:
                if label in wanted:
                    del wanted[label]
            notes.append(why)
    return wanted, notes


def prove(map_path, drivers, override_text):
    """Which requested drivers are MISSING from the kept part of the map.
    `drivers`: [(path, define, entry)] as the hub passes them; the fork's
    build has none (custom drivers are built by the hub)."""
    try:
        text = Path(map_path).read_text("utf-8", errors="replace").lower()
    except OSError:
        return ["the linker map itself (no file at "
                f"{map_path} -- did the build finish?)"]
    kept = text.find("linker script and memory map")
    if kept > 0:
        text = text[kept:]
    missing = []
    wanted, _notes = expected_markers(override_text)
    for path, _define, entry in drivers:
        wanted[f"custom driver {Path(path).stem} ({entry})"] = \
            rf"_z\d+{re.escape(entry)}[jm]|{re.escape(entry)}\("
    for label, pattern in wanted.items():
        if not re.search(pattern, text):
            missing.append(label)
    return missing


def artifact_candidates(source, env):
    """Where each output of `pio run -e <env>` may be, best first."""
    out = Path(source) / "build_output"
    build = Path(source) / ".pio" / "build" / env
    return {
        "factory": [out / "firmware" / f"{env}.factory.bin",
                    build / "firmware.factory.bin", build / f"{env}.factory.bin"],
        "app": [out / "firmware" / f"{env}.bin",
                build / "firmware.bin", build / f"{env}.bin"],
        "map": [out / "map" / f"{env}.map",
                build / "firmware.map", build / f"{env}.map"],
    }


def find_artifacts(source, env):
    return {kind: next((p for p in paths if p.is_file()), None)
            for kind, paths in artifact_candidates(source, env).items()}


# ── the run ────────────────────────────────────────────────────────────

def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    ap.add_argument("env", help="the PlatformIO env just built (tasmota32c3)")
    ap.add_argument("--source", default=".", help="the Tasmota tree")
    ap.add_argument("--override", default="tasmota/user_config_override.h")
    ap.add_argument("--out", default="dist",
                    help="folder for <env>.factory.bin and <env>.bin")
    args = ap.parse_args(argv)
    env = args.env
    try:
        override_text = (Path(args.source) / args.override).read_text(
            "utf-8", errors="replace")
    except OSError as exc:
        print(f"::error::no override at {args.override} ({exc})")
        return 1
    found = find_artifacts(args.source, env)
    for kind in ("map", "factory", "app"):
        print(f"  {kind:8} {found[kind] or 'NOT FOUND'}")
    for note in expected_markers(override_text)[1]:
        print(f"  note: {note}")
    if found["map"] is None:
        print(f"::error::{env}: no linker map (build_output/map/{env}.map "
              f"or .pio/build/{env}/firmware.map)")
        return 1
    missing = prove(found["map"], [], override_text)
    for label in missing:
        print(f"::error::{env}: NOT in the binary: {label}")
    wanted = expected_markers(override_text)[0]
    for label in wanted:
        print(f"  {'MISSING' if label in missing else 'in     '}  {label}")
    summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if summary:
        with open(summary, "a", encoding="utf-8") as fh:
            fh.write(f"### {env}\n\n" + "".join(
                f"- {'MISSING' if label in missing else 'in'}: {label}\n"
                for label in wanted) + "\n")
    if missing:
        # Show what the map DOES say about each missing name, so a marker
        # that stops matching (as `j` did when uint32_t became `m`) reads
        # as that in the log, instead of as five drivers gone.
        try:
            lines = Path(found["map"]).read_text("utf-8", errors="replace").splitlines()
        except OSError:
            lines = []
        for label in missing:
            name = re.search(r"\((\w+)\)$", label)
            if not name:
                continue
            hits = [ln.strip() for ln in lines if name.group(1).lower() in ln.lower()]
            print(f"  map lines naming {name.group(1)}: {len(hits)}")
            for ln in hits[:3]:
                print(f"      {ln[:160]}")
        print(f"::error::{env}: a green compile is not a proof -- "
              + ", ".join(missing) + " left no trace in the linker map. "
              "No images are published for this env.")
        return 1
    if found["factory"] is None or found["app"] is None:
        print(f"::error::{env}: proven, but the images are missing "
              f"(factory: {found['factory']}, app: {found['app']})")
        return 1
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    for kind, name in (("factory", f"{env}.factory.bin"), ("app", f"{env}.bin")):
        shutil.copy2(found[kind], out / name)
        print(f"  -> {out / name}  ({(out / name).stat().st_size:,} bytes)")
    print(f"  {env}: every requested driver is in.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
