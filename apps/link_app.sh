#!/bin/bash
if [ $# -eq 0 ]; then
echo "[FAIL] No arguments supplied"
else
clear
gcc -m32 -o $1.bin trougao.c kvadrat.c krug.c oblici.c -nostdlib -nostdinc -e main -Ttext=100 -static -fno-builtin ../lib/lib.a -I../include
cd ..
mkdir tmp_hd
sudo mount -o loop,offset=10240 hd_oldlinux.img tmp_hd
sudo cp apps/$1.bin tmp_hd/root
sudo umount tmp_hd
rmdir tmp_hd
qemu-system-i386 -hdb hd_oldlinux.img -fda Image -boot a
fi
