# Godbolt Compiler Explorer Links

All sessions use **arm-none-eabi-gcc 13.2** with `-Os -DNDEBUG -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -fno-exceptions -fno-rtti`.
Self-contained source files are in `scripts/godbolt/` if you need to recreate the sessions.

## Demo 1: Concept diagnostic clarity

Both panes deliberately fail to compile by instantiating a driver with `BadSensor` (wrong return type, missing `device_address`). Compare the error output:

- **C++17:** `"no type named 'type' in 'struct std::enable_if<false, void>'"` — tells you nothing about *which* constraint failed or why.
- **C++20:** `"read_register does not satisfy return-type-requirement"` and `"the required expression 'T::device_address' is invalid"` — pinpoints exactly what's wrong.

**Evidence in compiler output:**
- C++17: look for `enable_if` and `no type named 'type'` — the only clue is that some boolean condition was `false`
- C++20: look for `constraints not satisfied`, then two specific notes naming the exact requirements that failed and why

**Conclusion:** Concepts don't change the binary — they change the developer experience. The C++20 diagnostic tells you *what* to fix. The C++17 diagnostic tells you *something* failed. For embedded teams maintaining long-lived codebases, this reduces debugging time on template constraint errors from minutes to seconds.

**Godbolt link:** https://using26-demo1.godbolt.org/z/er9nvn3Eh

## Demo 2: Ranges-v3 pipeline vs loops

**Evidence in assembly:**

Stack allocation (search for `sub sp`):
- C++17: `sub sp, sp, #516` — two `std::array<float, 64>` buffers (~512 bytes + overhead)
- C++20: `sub sp, sp, #308` — one buffer (~256 bytes + overhead)

Sliding window inner loop (search for `vadd.f32` inside a tight loop):
- C++17 (labels `.L12`/`.L13`): `vldmia.32` → `vadd.f32` → loop, then `vmul.f32` (÷8) → `vstmia.32`
- C++20 (labels `.L26`/`.L27`): `vldmia.32` → `vadd.f32` → loop, then `vmul.f32` (÷8) → `vstmia.32`
- **Identical instruction sequences** — same math, same FPU operations

Filter+calibrate stage:
- C++17: `vfma.f32` inlined directly in the filter loop — compact, no function calls
- C++20: `blx r3` — indirect call through range-v3's stored function pointer. `cache_begin()` and `satisfy_forward()` calls from range-v3's iterator adapter infrastructure are present

**Conclusion — the honest result:** The sliding window hot loop produces identical ARM instructions. Stack usage drops from 516 to 308 bytes (one buffer vs two). However, range-v3's abstraction is **not free** — the filter+calibrate stage uses indirect function calls and iterator adapter machinery (`cache_begin`, `satisfy_forward`) that the C++17 version inlines directly. The source code is cleaner and the buffer count is lower, but there is a code size overhead from range-v3's template instantiation. This is the real engineering tradeoff: readability and reduced scratch memory vs slightly larger `.text`. At `-Os` on a Cortex-M4, the optimizer cannot fully collapse range-v3's view machinery the way it can collapse hand-written loops.

**Godbolt link:** https://using26-demo2.godbolt.org/z/4Pzjq74zv

## Demo 4: consteval calibration table

**Evidence in assembly:**

Init function:
- C++17: search for `init_calibration` — ~50 instructions visible: loop counter (`cmp r5, #256`), `bl logf` call, Steinhart-Hart multiply-accumulate (`vfma.f32`), NaN bit check (`and r3, r3, #2139095040`), store to table (`vstmia.32`)
- C++20: search for `gen_cal_table` — **not found anywhere in the assembly**. The function was evaluated entirely at compile time and emitted zero instructions.

Table storage:
- C++17: search for `cal_table:` → `.space 1024` — 1024 bytes of zeroed `.bss` (RAM), waiting to be filled at runtime by `init_calibration`
- C++20: search for `flash_cal_table:` → 256 `.word` entries — the pre-computed IEEE 754 float values baked directly into the binary as `.rodata` (flash). Zero RAM cost, zero runtime computation.

Lookup function:
- C++17: `lookup_temperature` — 4 instructions, unchecked `uint8_t` cast
- C++20: `lookup_temperature` — similar, plus a bounds check (`cmp r2, #4096` → `bl abort` on overflow)

**Conclusion:** This is the strongest demo. The C++20 version eliminates the init function entirely — `consteval` forces the compiler to compute all 256 Steinhart-Hart temperatures during compilation. The table moves from `.bss` (1024 bytes of RAM, filled by ~50 instructions of runtime code including a `logf` call per entry) to `.rodata` (1024 bytes of flash, pre-filled, zero runtime cost). NaN detection moves from a runtime `memcpy` + bit check to a compile-time error. On a memory-constrained Cortex-M4 with 64KB RAM, freeing 1024 bytes of `.bss` and removing a startup function is a measurable win.

|                      | C++17                       | C++20                        |
|----------------------|-----------------------------|------------------------------|
| Init function        | ~50 instructions in `.text` | **Gone** (consteval)         |
| Table storage        | `.bss` — 1024 bytes RAM     | `.rodata` — 1024 bytes flash |
| Runtime startup cost | `logf` × 256 iterations     | **Zero**                     |
| NaN protection       | Runtime `memcpy` bit check  | **Compile-time error**       |

**Godbolt link:** https://using26-demo4.godbolt.org/z/Pvbsqoshx

