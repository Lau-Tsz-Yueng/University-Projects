#include "extra.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTCC26XX.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>
#include <ti/drivers/PWM.h>

PWM_Handle pwm1;
UART_Handle uart;

Semaphore_Handle uart_sem = NULL;

void UART_printf(UART_Handle uart, const char *fmt, ...)
{
    BIOS_ThreadType tt = BIOS_getThreadType();
    if (tt != BIOS_ThreadType_Main)
    {
        //Create UART semaphore if it doesn't already exist.
        if (uart_sem == NULL)
        {
            Semaphore_Params sp;
            Semaphore_Params_init(&sp);
            sp.mode = Semaphore_Mode_BINARY;
            uart_sem = Semaphore_create(1, &sp, NULL);
        }
        //Grab UART semaphore.
        Semaphore_pend(uart_sem, BIOS_WAIT_FOREVER);
    }
    //Create buffer for UART_write to act on.
    char *msgbuf;
    int n = 100 + strlen(fmt);
    msgbuf = calloc(n, sizeof(char));
    //Fill buffer with printf data.
    va_list va;
    va_start(va, fmt);
    int m = vsnprintf(msgbuf, n, fmt, va);
    //Reallocate as needed to support larger messages.
    while (m >= n)
    {
        realloc(msgbuf, m + 1);
        n = m + 1;
        va_end(va);
        va_start(va, fmt);
        m = vsnprintf(msgbuf, n, fmt, va);
    }
    //Write buffer out over UART.  This may induce Task sleeping, hence the Semaphore requirements.
    UART_write(uart, msgbuf, m);
    //Free buffer
    free(msgbuf);
    va_end(va);
    //Drop Semaphore
    if (tt != BIOS_ThreadType_Main)
    {
        Semaphore_post(uart_sem);
    }
}

void UART_readFlush(UART_Handle uart)
{
    //Create UART semaphore if it doesn't already exist.
    if (uart_sem == NULL)
    {
        Semaphore_Params sp;
        Semaphore_Params_init(&sp);
        sp.mode = Semaphore_Mode_BINARY;
        uart_sem = Semaphore_create(1, &sp, NULL);
    }
    //Grab UART semaphore.
    Semaphore_pend(uart_sem, BIOS_WAIT_FOREVER);
    UART_control(uart, UARTCC26XX_CMD_RX_FIFO_FLUSH, NULL);
    //Drop Semaphore
    Semaphore_post(uart_sem);
}
