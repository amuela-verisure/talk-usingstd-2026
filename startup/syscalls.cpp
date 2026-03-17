// Minimal syscall stubs for newlib-nano (bare-metal).
// Required to link with -specs=nano.specs.

#include <cstdint>

extern "C" {

extern uint32_t _end; // From linker script.

void* _sbrk(int incr) {
    static uint8_t* heap_end = reinterpret_cast<uint8_t*>(&_end);
    uint8_t* prev = heap_end;
    heap_end += incr;
    return prev;
}

int _write(int /*fd*/, const char* /*buf*/, int len) {
    // No-op: discard output on bare-metal.
    return len;
}

int _read(int /*fd*/, char* /*buf*/, int /*len*/) {
    return 0;
}

int _close(int /*fd*/) {
    return -1;
}

int _lseek(int /*fd*/, int /*offset*/, int /*whence*/) {
    return 0;
}

int _fstat(int /*fd*/, void* /*st*/) {
    return 0;
}

int _isatty(int /*fd*/) {
    return 1;
}

void _exit(int /*status*/) {
    while (true) {}
}

void _kill(int /*pid*/, int /*sig*/) {}

int _getpid() {
    return 1;
}

} // extern "C"
