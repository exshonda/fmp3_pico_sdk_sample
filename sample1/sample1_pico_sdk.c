#include <stdio.h>
#include "pico/stdlib.h"
#include "kernel.h"

int main()
{
    stdio_init_all();

#if 0
    while(!stdio_usb_connected())
        Asm("WFI");;
#endif

    Asm("cpsid f":::"memory");

    sta_ker();

    return 0;
}
