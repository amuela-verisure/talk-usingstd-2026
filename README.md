# C++20 and beyond: improving embedded systems performance

Companion repo for the **using std::cpp 2026** talk:
*"The tools and techniques to build your own case."*

## What is this?

A synthetic sensor acquisition node for Cortex-M4 (256KB flash, 64KB RAM) with **four demonstrations** comparing C++17 and C++20 implementations side-by-side. Every measurement in the talk is reproducible from this repo.

## Demos

| # | Feature                                     | C++17                                | C++20                                                        | What to measure                                             |
|---|---------------------------------------------|--------------------------------------|--------------------------------------------------------------|-------------------------------------------------------------|
| 1 | **Concepts + constexpr + static_assert**    | SFINAE traits, runtime validation    | `I2CSensor` concept, `consteval` validation, `static_assert` | Diagnostic clarity (34 vs 3 lines), binary size (identical) |
| 2 | **Ranges-v3 pipeline**                      | 3 loops, 2 intermediate buffers      | `filter \| transform \| sliding \| transform`                | Assembly equivalence, .bss decrease (buffers eliminated)    |
| 3 | **tl::expected monadic chains**             | Nested ifs (arrow anti-pattern)      | `and_then`, `transform`, `or_else`, `value_or`               | Cyclomatic complexity (6 vs 1), identical binary            |
| 4 | **consteval + bit_cast + constinit + span** | Runtime `init_calibration()` in .bss | Compile-time table in .rodata/.data                          | Init function gone from binary, .bss eliminated             |

## Building

### Prerequisites

- CMake 3.21+, Ninja
- Host: g++ 12+ or clang++ 15+ with C++20 support
- ARM: arm-none-eabi-gcc 13.2+ (for cross-compilation and size comparison)
- Optional: [lizard](https://github.com/terryyin/lizard) (`pip install lizard`) for complexity reports

### Host build (tests)

```bash
cmake --preset host-debug
cmake --build --preset host-debug
ctest --preset host-debug
```

### ARM cross-compilation (size comparison)

```bash
cmake --preset arm-cortexm4
cmake --build --preset arm-cortexm4
./scripts/compare_size.sh build/arm
```

`compare_size.sh` runs `arm-none-eabi-size` on both firmware binaries and prints a section-by-section table (`.text`, `.data`, `.bss`) with deltas. This is the primary metric for the talk — it shows how C++20 features reduce flash and RAM usage compared to equivalent C++17 code.

### Complexity report

```bash
./scripts/lizard_report.sh
```

`lizard_report.sh` runs [lizard](https://github.com/terryyin/lizard) over the C++17 and C++20 source trees separately, sorted by cyclomatic complexity. Cyclomatic complexity counts the number of independent paths through a function — lower is easier to test and review. Demo 3's `init_sensor` shows the most dramatic improvement (nested ifs vs monadic chain). Requires `pip install lizard`.

### Compilation time comparison

```bash
python3 scripts/compile_time.py
```

`compile_time.py` measures per-demo compilation wall-clock time for C++17 vs C++20 source files. It extracts exact compiler commands from `compile_commands.json` and runs each file multiple times (default 10) with a warmup pass. Demo 1 is header-only, so it uses probe translation units in `bench/` that force template instantiation.

Options: `--iterations N` (default 10), `--ftime-report` (GCC phase breakdown), `--csv FILE`.

Sample results (g++ 13.3, host-debug):

| Demo          | C++17  | C++20  | Delta   | Ratio | Dominant cost               |
|---------------|--------|--------|---------|-------|-----------------------------|
| 1 (Concepts)  | 27 ms  | 29 ms  | +3 ms   | 1.1x  | — (negligible)              |
| 2 (Ranges)    | 54 ms  | 601 ms | +547 ms | 11.2x | range-v3 header parsing     |
| 3 (Expected)  | 21 ms  | 205 ms | +184 ms | 9.8x  | tl::expected header parsing |
| 4 (consteval) | 105 ms | 134 ms | +29 ms  | 1.3x  | consteval evaluation        |

The compile-time cost is a one-time build cost, not a runtime cost. The `-ftime-report` flag shows that header parsing (template-heavy supplement libraries) dominates — the actual code generation difference is small.

### Godbolt assembly inspection

Pre-configured Godbolt sessions with self-contained source code (no project headers needed):

- **Demo 1** — [Concept vs SFINAE diagnostics](https://using26-demo1.godbolt.org/z/4n4ov4adr): compare error messages when constraints fail
- **Demo 2** — [Ranges pipeline vs loops](https://using26-demo2.godbolt.org/z/vWKsq6eor): equivalent sliding window assembly, one buffer vs two
- **Demo 4** — [consteval calibration table](https://using26-demo4.godbolt.org/z/6b9Ex69qe): init function gone, table baked into `.rodata`

See `scripts/godbolt_links.md` for detailed evidence and conclusions for each demo.

### Docker (all-in-one)

```bash
docker build -t using-2026 .
docker run --rm using-2026
```

## Project structure

```
common/           Shared types (ErrorCode, SensorConfig, hw::I2CDevice)
demo1_concepts/   Demo 1: C++17 SFINAE vs C++20 Concepts
demo2_ranges/     Demo 2: C++17 loops vs C++20 ranges-v3 pipeline
demo3_expected/   Demo 3: C++17 nested-ifs vs C++20 tl::expected monadic chain
demo4_consteval/  Demo 4: C++17 runtime init vs C++20 consteval table
app/              Firmware entry points (main_cpp17.cpp, main_cpp20.cpp)
bench/            Compilation-time probes (Demo 1 header-only instantiation)
startup/          Cortex-M4 startup code and syscall stubs
linker/           Linker script for STM32F407VG (memory layout explained below)
cmake/            Build system modules (FetchContent, warnings, toolchain)
scripts/          Analysis scripts (size comparison, lizard, compile time, Godbolt links)
```

Each demo directory has `cpp17/` and `cpp20/` subdirectories with matching interfaces, plus a `test/` directory with equivalence tests.

The `linker/STM32F407VG.ld` script is named after the target MCU (an STMicroelectronics Cortex-M4 chip with 256KB flash and 64KB RAM). The project doesn't run on real hardware — the chip was chosen as a representative target to produce realistic size measurements. The script defines where code and data end up in memory. On a microcontroller there is no OS — the binary is laid out directly into flash and RAM by the linker. Understanding these sections is key to reading the size comparison results:

| Section | Location                        | What goes here                                                                                           | Why it matters                                                           |
|---------|---------------------------------|----------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------|
| `.text` | Flash (read-only)               | Code, constants, vector table. Demo 4's `constexpr flash_cal_table` lives here.                          | Size = code footprint. Larger `.text` means more flash consumed.         |
| `.data` | RAM (copied from flash at boot) | Mutable variables with non-zero initial values. Demo 4 C++20's `constinit runtime_cal_table` lands here. | Uses both flash (for the initial values) and RAM (for the runtime copy). |
| `.bss`  | RAM (zeroed at boot)            | Mutable variables initialized to zero. Demo 4 C++17's 1024-byte `cal_table` lands here.                  | Free to declare (no flash cost), but consumes scarce RAM.                |

On a desktop system the OS loads your program, sets up memory, and provides syscalls. On bare-metal there is no OS — the `startup/` directory fills that role:

- `startup_stm32f407.cpp` — Contains the **vector table** (an array of function pointers the hardware reads on power-on to find the stack and reset handler) and `Reset_Handler()`, the very first code that runs. It copies `.data` from flash to RAM and zeros `.bss` using symbols from the linker script (`_sdata`, `_edata`, `_etext`, `_sbss`, `_ebss`), then calls `main()`.
- `syscalls.cpp` — Stub implementations of POSIX functions (`_sbrk`, `_write`, `_read`, etc.) required by **newlib-nano**, the minimal C library used in ARM bare-metal builds. They are all no-ops — there is no console or filesystem — but the linker needs them to resolve symbols from the C runtime.

The C++20 demos shrink `.bss` dramatically (1348 → 8 bytes) by moving data to `.rodata` (compile-time constant → flash) or `.data` (compile-time initialized → still in RAM, but no runtime init function needed).

The `app/` entry points instantiate every demo to produce linkable ARM `.elf` binaries for the size comparison. Results are written to `volatile` sinks to prevent the optimizer from stripping unused computations. Both files end with `while (true) {}` — the standard bare-metal idle loop (no OS to return to). The C++20 version reflects the API improvements: `std::span` instead of pointer+size (Demo 2), compile-time config validation (Demo 1), and `get_flash_table()` replacing runtime `init_calibration()` (Demo 4).

## Verifying the claims

After building the ARM preset, you can reproduce every claim from the talk yourself:

**Demo 1 — Compile-time config validation:**
Change `default_cfg.sample_rate` to an invalid value (e.g., 42) in `demo1_concepts/cpp20/include/demo1/cpp20/config_validation.hpp`. The C++20 build fails at compile time (`static_assert`); the C++17 equivalent compiles and only catches the error at runtime.

**Demo 2 — Equivalent assembly:**
Use `arm-none-eabi-objdump -d` on both firmware binaries and compare the `process_readings` inner loop. Look for the same `vldr`, `vcmp`, `vmla`, `vstr` instruction patterns.

**Demo 3 — Same branch structure:**
Disassemble both versions' `init_sensor` and count conditional branch instructions (`beq`, `bne`, `cbz`, `cbnz`). Both should have the same number — the monadic chain compiles to the same branches as the nested ifs.

**Demo 4 — Init function gone:**
```bash
arm-none-eabi-nm build/arm/firmware_cpp17 | c++filt | grep cal_table   # in .bss
arm-none-eabi-nm build/arm/firmware_cpp20 | c++filt | grep cal_table   # nothing — consteval, not emitted
```

> **Note:** With `-Os -flto`, LTO may inline `init_calibration()` into `main` and constant-propagate the calibration tables, removing named symbols from the binary. The section sizes (`.bss` 1348 → 8) confirm the data placement change. To see individual symbols, build without LTO (`-O0`) or inspect with `arm-none-eabi-objdump -d`.

## Third-party dependencies

Fetched automatically via CMake FetchContent:

- [tl::expected](https://github.com/TartanLlama/expected) v1.1.0 (C++23 `std::expected` polyfill)
- [range-v3](https://github.com/ericniebler/range-v3) 0.12.0 (C++23 `std::ranges` polyfill)
- [Microsoft GSL](https://github.com/microsoft/GSL) v4.0.0 (`gsl::narrow`, `gsl::span`)
- [doctest](https://github.com/doctest/doctest) v2.4.11 (testing, host builds only)

> **Note:** tl::expected v1.1.0 ships its own Catch2-based test suite, which CMake registers automatically. Running `ctest` will show 5 tests (4 demo equivalence tests + 1 tl::expected upstream test). This is expected and benign.

All libraries work under `-fno-exceptions -fno-rtti`. Behavior without exceptions:

- `tl::expected::value()` on an unexpected calls `std::terminate` (not throw)
- `gsl::narrow` calls `std::terminate` on narrowing failure (not throw `gsl::narrowing_error`). Microsoft GSL omits `gsl::narrow` entirely when exceptions are disabled; the project provides a manual bounds check + `std::abort()` fallback under `-fno-exceptions`
- range-v3 works without exceptions; throwing utilities like `ranges::at` are unavailable but unused here

## Toolchain

```
arm-none-eabi-gcc 13.2
-std=c++20 -Os -flto -fno-exceptions -fno-rtti
-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
```

## Design notes

- **Why `std::reference_wrapper` in Demo 3?** The monadic chain needs to pass the `hw::I2CDevice&` through each step. Since `tl::expected` stores values by value, `std::reference_wrapper<hw::I2CDevice>` avoids copies and enables `transform` (which requires a non-void value type).

- **Why range-v3 instead of `std::ranges`?** `views::sliding` is a C++23 addition. GCC 13.2's `std::ranges` doesn't include it. range-v3 is the reference implementation that `std::ranges` was based on.

- **Why `__builtin_logf` in Demo 4?** `std::log` is not `constexpr` until C++26. GCC's `__builtin_logf` is evaluated at compile time in `consteval` context. This is a GCC extension — Clang may not support it, which is a known limitation.

- **C++17 code compiles as C++20.** The host test build links both C++17 and C++20 libraries into a single C++20 binary. The C++17 code avoids constructs deprecated between C++17 and C++20.

## License

See [LICENSE](LICENSE).

---

*"Don't take my word for it. Take your compiler's."*
