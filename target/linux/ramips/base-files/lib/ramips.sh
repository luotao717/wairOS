#!/bin/sh
#
# Copyright (C) 2009-2011 OpenWrt.org
#

RAMIPS_BOARD_NAME=
RAMIPS_MODEL=

ramips_get_mtd_offset_size_format() {
	local mtd="$1"
	local offset="$2"
	local size="$3"
	local format="$4"
	local dev

	dev=$(find_mtd_part $mtd)
	[ -z "$dev" ] && return

	dd if=$dev bs=1 skip=$offset count=$size 2>/dev/null | hexdump -v -e "1/1 \"$format\""
}

ramips_get_mtd_part_magic() {
	local mtd="$1"
	ramips_get_mtd_offset_size_format "$mtd" 0 4 %02x
}



ramips_board_detect() {
	local machine
	local name

	machine=$(awk 'BEGIN{FS="[ \t]+:[ \t]"} /machine/ {print $2}' /proc/cpuinfo)

	case "$machine" in
	*WIFI-ROUTER-WALLPOE*)
		name="wifi-router-wallpoe"
		;;
	*WIFI-ROUTER-POE*)
		name="wifi-router-poe"
		;;
	*WIFI-ROUTER*)
		name="wifi-router"
		;;
	esac

	[ -z "$name" ] && name="unknown"

	[ -z "$RAMIPS_BOARD_NAME" ] && RAMIPS_BOARD_NAME="$name"
	[ -z "$RAMIPS_MODEL" ] && RAMIPS_MODEL="$machine"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$RAMIPS_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$RAMIPS_MODEL" > /tmp/sysinfo/model
}

ramips_board_name() {
	local name

	[ -f /tmp/sysinfo/board_name ] && name=$(cat /tmp/sysinfo/board_name)
	[ -z "$name" ] && name="unknown"

	echo "$name"
}
