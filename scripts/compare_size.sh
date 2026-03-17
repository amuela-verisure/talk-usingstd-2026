#!/bin/bash
# Compare binary sizes between C++17 and C++20 firmware builds.
# Run from the project root after building the arm-cortexm4 preset.
set -euo pipefail

BUILD_DIR="${1:-build/arm}"

echo "=== C++17 Firmware ==="
arm-none-eabi-size "${BUILD_DIR}/firmware_cpp17"
echo ""

echo "=== C++20 Firmware ==="
arm-none-eabi-size "${BUILD_DIR}/firmware_cpp20"
echo ""

echo "=== Section-by-section comparison ==="
echo "Section | C++17 | C++20 | Delta"
echo "--------|-------|-------|------"

# Parse arm-none-eabi-size output (Berkeley format).
read_sizes() {
    arm-none-eabi-size "$1" | tail -1 | awk '{print $1, $2, $3}'
}

sizes17=($(read_sizes "${BUILD_DIR}/firmware_cpp17"))
sizes20=($(read_sizes "${BUILD_DIR}/firmware_cpp20"))

sections=(".text" ".data" ".bss")
for i in 0 1 2; do
    delta=$((${sizes20[$i]} - ${sizes17[$i]}))
    printf "%-7s | %5d | %5d | %+d\n" "${sections[$i]}" "${sizes17[$i]}" "${sizes20[$i]}" "$delta"
done
