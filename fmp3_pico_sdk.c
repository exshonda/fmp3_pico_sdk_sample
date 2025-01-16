#include <stdio.h>
#include "pico/stdlib.h"
#include "t_syslog.h"
#include "log_output.h"
#include "sil.h"
#include "fmp3_pico_sdk.h"

void task1(EXINF exinf)
{
    while (1) {
        printf("[CORE%d] Hello, world!\n", (exinf >> 16) - 1);
        dly_tsk(991 * 1000);
    }
}

void task2(EXINF exinf)
{
    while (1) {
        printf("[CORE%d] Hello, world!\n", (exinf >> 16) - 1);
        dly_tsk(1009 * 1000);
    }
}

int main()
{
    stdio_init_all();

    Asm("cpsid f":::"memory");

    sta_ker();

    return 0;
}
