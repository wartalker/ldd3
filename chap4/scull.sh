#!/bin/bash

module="scull"
device="scull"
mode="664"

/sbin/insmod ./$module.ko || exit 1
rm -f /dev/$device

major=$(awk '$2=="scull" {print $1}' /proc/devices)
echo "major="$major

mknod /dev/$device c $major 0
group="staff"
grep -q '^staff:' /etc/group || group="wheel"

chgrp $group /dev/$device
chmod $mode /dev/$device
