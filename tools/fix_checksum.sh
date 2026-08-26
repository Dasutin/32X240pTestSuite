#!/bin/sh
set -eu

ROM=${1:?usage: fix_checksum.sh ROM}

ROM_SIZE=$(wc -c < "$ROM")

if [ "$ROM_SIZE" -ne 4194304 ]
then
	echo "ROM must be exactly 4 MiB: $ROM" >&2
	exit 1
fi

printf '\000\077\377\377' |
	dd of="$ROM" bs=1 seek=420 count=4 conv=notrunc status=none

CHECKSUM=$(od -An -v -t u2 --endian=big -j 512 "$ROM" |
	awk '{ for (i = 1; i <= NF; ++i) sum = (sum + $i) % 65536 }
		END { printf "%d", sum }')

HIGH_BYTE=$((CHECKSUM / 256))
LOW_BYTE=$((CHECKSUM % 256))

awk -v high="$HIGH_BYTE" -v low="$LOW_BYTE" \
	'BEGIN { printf "%c%c", high, low }' |
	dd of="$ROM" bs=1 seek=398 count=2 conv=notrunc status=none

STORED_CHECKSUM=$(od -An -N 2 -j 398 -t u2 --endian=big "$ROM" | tr -d ' ')

if [ "$STORED_CHECKSUM" -ne "$CHECKSUM" ]
then
	echo "Failed to write ROM checksum" >&2
	exit 1
fi
