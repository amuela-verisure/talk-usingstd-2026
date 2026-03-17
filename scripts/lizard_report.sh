#!/bin/bash
# Compare cyclomatic complexity between C++17 and C++20 code.
# Requires: pip install lizard
set -euo pipefail

echo "=== C++17 Complexity ==="
lizard \
    demo1_concepts/cpp17/ \
    demo2_ranges/cpp17/ \
    demo3_expected/cpp17/ \
    demo4_consteval/cpp17/ \
    --languages cpp \
    --sort cyclomatic_complexity \
    2>/dev/null || echo "(lizard not installed: pip install lizard)"

echo ""
echo "=== C++20 Complexity ==="
lizard \
    demo1_concepts/cpp20/ \
    demo2_ranges/cpp20/ \
    demo3_expected/cpp20/ \
    demo4_consteval/cpp20/ \
    --languages cpp \
    --sort cyclomatic_complexity \
    2>/dev/null || echo "(lizard not installed: pip install lizard)"
