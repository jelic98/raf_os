#!/bin/bash
clear
script/make_tool.sh keyset
script/make_tool.sh keyclear
script/make_tool.sh keygen
script/make_tool.sh encr
script/make_tool.sh decr
script/make_app.sh main
script/start_emu.sh
