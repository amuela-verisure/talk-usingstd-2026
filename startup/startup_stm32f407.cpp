// Minimal startup code for STM32F407VG (Cortex-M4).
// Provides vector table and Reset_Handler.

#include <cstdint>

extern "C" {

// Symbols from linker script.
extern uint32_t _etext;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
extern uint32_t _estack;

// Main entry point (defined in app/).
int main();

void Reset_Handler() {
    // Copy .data from Flash to RAM.
    uint32_t* src = &_etext;
    uint32_t* dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    // Zero .bss.
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    main();

    // Hang if main returns.
    while (true) {}
}

void Default_Handler() {
    while (true) {}
}

// Weak aliases for all ISR handlers.
void NMI_Handler()        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler()  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler()  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler()   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler() __attribute__((weak, alias("Default_Handler")));
void SVC_Handler()        __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler()     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler()    __attribute__((weak, alias("Default_Handler")));

// Vector table placed at start of Flash.
__attribute__((section(".isr_vector")))
const uint32_t vector_table[] = {
    reinterpret_cast<uint32_t>(&_estack),
    reinterpret_cast<uint32_t>(Reset_Handler),
    reinterpret_cast<uint32_t>(NMI_Handler),
    reinterpret_cast<uint32_t>(HardFault_Handler),
    reinterpret_cast<uint32_t>(MemManage_Handler),
    reinterpret_cast<uint32_t>(BusFault_Handler),
    reinterpret_cast<uint32_t>(UsageFault_Handler),
    0, 0, 0, 0, // Reserved
    reinterpret_cast<uint32_t>(SVC_Handler),
    0, 0, // Reserved
    reinterpret_cast<uint32_t>(PendSV_Handler),
    reinterpret_cast<uint32_t>(SysTick_Handler),
};

} // extern "C"
