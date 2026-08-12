#!/usr/bin/env python3
"""
PMDoT Engine Build Automation Script
Builds the custom Godot 4.3-stable engine with pmdot_aseprite module.
"""

import os
import sys
import subprocess
import shutil

def run_command(cmd, cwd=None):
    print(f"[PMDoT Build] Running: {' '.join(cmd)}")
    result = subprocess.run(cmd, cwd=cwd)
    if result.returncode != 0:
        print(f"[PMDoT Build] Error executing command: {' '.join(cmd)}")
        sys.exit(result.returncode)

def main():
    root_dir = os.path.abspath(os.path.dirname(__file__))
    godot_dir = os.path.join(root_dir, "source-godot")

    if not os.path.exists(godot_dir):
        print("[PMDoT Build] Error: source-godot directory not found.")
        sys.exit(1)

    print("[PMDoT Build] Verifying module structure...")
    module_dir = os.path.join(godot_dir, "modules", "pmdot_aseprite")
    if not os.path.exists(module_dir):
        print("[PMDoT Build] Error: pmdot_aseprite module directory missing.")
        sys.exit(1)

    print("[PMDoT Build] Building PMDoT Engine using SCons...")
    scons_cmd = ["scons", "platform=windows", "target=editor", "vsprops=yes", "-j4"]
    
    # Dry-run check or build execution
    run_command(scons_cmd, cwd=godot_dir)
    print("[PMDoT Build] PMDoT Engine Build Complete successfully!")

if __name__ == "__main__":
    main()
