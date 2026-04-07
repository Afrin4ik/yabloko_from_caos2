#include "../syscall.h"
#include <stdint.h>

int main() {
    const char* spell = "This is snake file. Just check of correct functionality\n";
    syscall(SYS_puts, (uint32_t)spell);
    return 0;
}
