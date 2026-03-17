/* Linker script for representative Cortex-M4: 256KB Flash, 64KB RAM. */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 256K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

/* Entry point. */
ENTRY(Reset_Handler)

SECTIONS
{
    /* Vector table and code -> Flash. */
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        *(.init)
        *(.fini)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    /* ARM exception unwind tables (kept even with -fno-exceptions by some libs). */
    .ARM.exidx :
    {
        *(.ARM.exidx*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    /* Initialized data: loaded from Flash, copied to RAM at startup. */
    .data : AT(_etext)
    {
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM

    /* Zero-initialized data. */
    .bss :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* Stack at end of RAM. */
    _estack = ORIGIN(RAM) + LENGTH(RAM);

    /* Heap between bss and stack. */
    _end = .;
    PROVIDE(end = .);
}
