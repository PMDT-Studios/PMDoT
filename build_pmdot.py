#!/usr/bin/env python3
"""
PMDoT Engine Build Automation Script
Builds the custom Godot 4.3-stable engine with pmdot_aseprite module.
Supports incremental segmented builds and per-segment changelog tracking.
"""

import os
import sys

def main():
    root_dir = os.path.abspath(os.path.dirname(__file__))
    if root_dir not in sys.path:
        sys.path.insert(0, root_dir)

    try:
        from pmdot import cmd_build
        cmd_build(sys.argv[1:])
    except ImportError as e:
        print(f"[PMDoT Build] Error importing pmdot: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
