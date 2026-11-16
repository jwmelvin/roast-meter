#!/bin/bash
# Compile (and optionally upload) the Roast Meter firmware with arduino-cli.
#
#   ./build.sh [plain|ble] [upload [PORT]]
#
# The Arduino tools want each sketch in a folder of the same name, so the
# sources in src/ are copied into build/<sketch>/ first.
#
# One-time setup:
#   brew install arduino-cli
#   arduino-cli config add board_manager.additional_urls \
#     https://raw.githubusercontent.com/sparkfun/Arduino_Apollo3/main/package_sparkfun_apollo3_index.json
#   arduino-cli core update-index
#   arduino-cli core install SparkFun:apollo3@2.2.1
#   arduino-cli lib install "SparkFun MAX3010x Pulse and Proximity Sensor Library@1.1.2" \
#     "SparkFun Micro OLED Breakout@1.3.4" ArduinoBLE
set -euo pipefail

FQBN=SparkFun:apollo3:sfe_artemis_thing_plus
ROOT="$(cd "$(dirname "$0")" && pwd)"
TARGET="${1:-plain}"

case "$TARGET" in
  plain) SKETCH=roast_meter;     cp_src() { cp "$ROOT/src/roast_meter.ino" "$ROOT/src/roast_math.h" "$1/"; } ;;
  ble)   SKETCH=roast_meter_ble; cp_src() { cp "$ROOT/src/roast_meter_ble.cpp" "$1/roast_meter_ble.ino"; } ;;
  *) echo "usage: $0 [plain|ble] [upload [PORT]]" >&2; exit 2 ;;
esac

DIR="$ROOT/build/$SKETCH"
rm -rf "$DIR" && mkdir -p "$DIR"
cp_src "$DIR"

arduino-cli compile --fqbn "$FQBN" --warnings all --output-dir "$DIR/out" "$DIR"

if [ "${2:-}" = "upload" ]; then
  PORT="${3:-$(ls /dev/cu.usbserial-* /dev/cu.wchusbserial* /dev/cu.usbmodem* 2>/dev/null | head -1 || true)}"
  if [ -z "$PORT" ]; then
    echo "No USB serial port found. Plug in the meter, or pass the port: $0 $TARGET upload /dev/cu.xxx" >&2
    exit 1
  fi
  arduino-cli upload --fqbn "$FQBN" -p "$PORT" --input-dir "$DIR/out" "$DIR"
fi
