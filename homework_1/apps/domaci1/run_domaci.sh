#!/bin/bash
clear
if ./make_domaci.sh; then
./start_emu.sh
else
echo "[FAIL] Emulator cannot start"
fi
