#!/bin/bash
if [ $# -eq 0 ]; then
echo "[FAIL] No arguments supplied"
else
clear
if script/make_app.sh $1; then
script/start_emu.sh
else
echo "[FAIL] Emulator cannot start"
fi
fi
