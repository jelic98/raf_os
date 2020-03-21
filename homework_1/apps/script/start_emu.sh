#!/bin/bash
echo '[SUCCESS] Emulator started'
qemu-system-i386 -hdb ../hd_oldlinux.img -fda ../Image -boot a
