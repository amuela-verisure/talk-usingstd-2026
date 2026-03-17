# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Conference talk companion repo for **"C++20 and beyond: improving embedded systems performance"** (using std::cpp 2026). Four demos compare C++17 and C++20 implementations of a synthetic Cortex-M4 sensor node. Attendees clone, build, and reproduce every measurement.

## Build Commands

Build uses **Ninja** (generator configured in `CMakePresets.json`).

```bash
# Configure + build + test (host)
cmake --preset host-debug
cmake --build --preset host-debug
ctest --preset host-debug

# ARM cross-compilation (requires arm-none-eabi-gcc 13.2)
cmake --preset arm-cortexm4
cmake --build --preset arm-cortexm4

# Single test
cmake --build --preset host-debug --target test_demo3
./build/host-debug/test_demo3

# Clean
rm -rf build/

# Compilation time comparison
python3 scripts/compile_time.py                     # 10 iterations, summary table
python3 scripts/compile_time.py --ftime-report      # + GCC phase breakdown
python3 scripts/compile_time.py --csv out.csv       # export to CSV
```

Three CMake presets: `host-debug` (testing ON), `host-release` (-Os -flto), `arm-cortexm4` (cross-compile, testing OFF).

## Architecture

Each demo has mirrored `cpp17/` and `cpp20/` directories with matching functional interfaces. Tests prove equivalence.

```
demo<N>/
  cpp17/include/demo<N>/cpp17/  -- C++17 headers
  cpp17/src/                    -- C++17 sources (if not header-only)
  cpp20/include/demo<N>/cpp20/  -- C++20 headers
  cpp20/src/                    -- C++20 sources
  test/                         -- doctest equivalence tests + mocks
```

| Demo | C++17 Pattern | C++20 Feature | Third-party Dep |
|------|--------------|---------------|-----------------|
| 1 (Concepts) | SFINAE `enable_if` traits | `concept I2CSensor`, `consteval` validation | none |
| 2 (Ranges) | 3 loops + 2 intermediate buffers | `views::filter \| transform \| sliding` | range-v3 |
| 3 (Expected) | Nested-if arrow anti-pattern | `and_then`, `transform`, `or_else`, `value_or` | tl::expected |
| 4 (consteval) | Runtime `init_calibration()` in .bss | `consteval` + `bit_cast` + `constinit` + `span` + `gsl::narrow` | Microsoft GSL |

**Namespaces:** `demo1::cpp17::`, `demo1::cpp20::`, etc. Hardware abstractions live in `hw::`. Shared types (`ErrorCode`, `SensorConfig`, `SensorState`, `SteinhartCoeffs`) are in `common/include/common/` with no namespace prefix.

**CMake targets:** `common` (INTERFACE), `demo1_cpp17`/`demo1_cpp20` (INTERFACE, header-only), `demo2_cpp17`/`demo2_cpp20` through `demo4_cpp17`/`demo4_cpp20` (STATIC). ARM builds produce `firmware_cpp17` and `firmware_cpp20` executables with linker script `linker/STM32F407VG.ld`. Test executables: `test_demo1` through `test_demo4`. Compilation probes: `probe_demo1_cpp17`/`probe_demo1_cpp20` (OBJECT, EXCLUDE_FROM_ALL — built only on demand for `scripts/compile_time.py`).

## Key Constraints

- **No exceptions** (`-fno-exceptions`): errors via `ErrorCode` enum or `tl::expected<T, ErrorCode>`. The only `throw` is inside `consteval gen_cal_table()` (compile-time only, never emitted). GCC rejects `throw` in `consteval` with `-fno-exceptions`, so the code uses conditional compilation: `throw` when exceptions are enabled, a non-constexpr function call (triggering a compile error) otherwise.
- **No RTTI** (`-fno-rtti`).
- **32-bit IEEE 754 floats** enforced by `static_assert(sizeof(float) == 4)`.
- **No heap allocation** in demo hot paths.
- `gsl::narrow` calls `std::terminate()` on overflow (not exceptions) under `-fno-exceptions`.
- ARM toolchain flags: `-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -Os -flto`.

## Dependencies

All auto-fetched via CMake FetchContent (`cmake/FetchDeps.cmake`). No manual install needed:

- tl::expected v1.1.0 -- `#include <tl/expected.hpp>`
- range-v3 0.12.0 -- `#include <range/v3/view/filter.hpp>` etc.
- Microsoft GSL v4.0.0 -- `#include <gsl/gsl>`
- doctest v2.4.11 (host builds only) -- `#include <doctest/doctest.h>`

## Tooling

- **`compile_commands.json`** — Symlinked from project root to `build/host-debug/compile_commands.json` on configure (host builds only). Provides LSP support (clangd, etc.).
- **`.clang-format`** — LLVM-based, 4-space indent, 100 column limit. Format with `clang-format -i <file>`.
- **`.clang-tidy`** — Enforces naming conventions via `readability-identifier-naming`: `snake_case` functions/variables, `CamelCase` types, `UPPER_CASE` constants.

## Important Design Notes

- **Demo 2 materialization:** The C++20 pipeline materializes filtered+calibrated values into one `std::array` before `views::sliding`, because range-v3's `views::sliding` requires a forward range and `views::filter` produces an input range. The PPTX slide shows a purely lazy pipeline; the repo code is the correct implementation.
- **Intentional duplication:** C++17 and C++20 driver bodies are identical by design -- proves only the constraint syntax changes. Do not refactor into shared code.
- **Status register bits:** Named constants `hw::STATUS_CAL_VALID`, `STATUS_OFFSET_VALID`, `STATUS_VALIDATED`, `STATUS_SAFE_MODE` in `hw_registers.hpp`.
- Warning flags applied via `set_project_warnings()` in `cmake/CompilerWarnings.cmake`.

## Docker

A `Dockerfile` provides a reproducible build environment with all toolchains pre-installed.

```bash
# Build the image
docker build -t using-2026 .

# Run the full pipeline (host build + test + ARM build + size comparison + lizard)
docker run --rm using-2026

# Interactive shell
docker run --rm -it using-2026 bash
```

## Reference Docs

- `bench/` -- compilation-time probe sources for header-only Demo 1 (used by `scripts/compile_time.py`)
