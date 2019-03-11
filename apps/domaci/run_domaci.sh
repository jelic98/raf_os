#!/bin/bash
if [ $# -eq 0 ]; then
echo "[FAIL] No arguments supplied"
else
clear
if make_domaci.sh $1; then
start_emu.sh
else
echo "[FAIL] Emulator cannot start"
fi
fi
