#include "extra.h"

#include <stdint.h>
#include <stddef.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/Types.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <xdc/runtime/Memory.h>
#include <xdc/runtime/IHeap.h>

#include <ti/sysbios/knl/Task.h>

#include <ti/drivers/ADCBuf.h>

#include "Board.h"

#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTCC26XX.h>

#include <xdc/runtime/Timestamp.h>

#include <ti/sysbios/BIOS.h>
#include <ti/drivers/dpl/HwiP.h>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

void mainThread(UArg a1, UArg a2)
{
    GPIO_init();
    UART_init();

    UART_Params uartParams;
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;

    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = 115200;
    uart = UART_open(Board_UART0, &uartParams);
    if (uart == NULL)
    {
        //Open failed
        while (1)
            ;
    }

    UART_printf(uart, "Starting the ADC example program\r\n");

    //==========================================================

    UART_printf(uart, "Initialising the ADCBuf Semaphore...");
    Semaphore_Params sp;
    Semaphore_Params_init(&sp);
    sp.mode = Semaphore_Mode_BINARY;
    ADCBufSem = Semaphore_create(0, &sp, NULL);

    if (ADCBufSem == NULL)
    {
        UART_printf(uart, "Error initialising the ADCBuf Semaphore\r\n");
        while (1)
            ;
    }
    UART_printf(uart, "\tDone.\r\n");

    //========================================================

    UART_printf(uart, "Initialising the ADCBuf...");
    ADCBuf_init();
    /* Set up an ADCBuf peripheral in ADCBuf_RECURRENCE_MODE_CONTINUOUS */
    ADCBuf_Params adcBufParams;
    ADCBuf_Params_init(&adcBufParams);
    adcBufParams.callbackFxn = adcBufCallback;
    adcBufParams.recurrenceMode = ADCBuf_RECURRENCE_MODE_ONE_SHOT;
    adcBufParams.returnMode = ADCBuf_RETURN_MODE_CALLBACK;
    adcBufParams.samplingFrequency = 20000;
    adcBuf = ADCBuf_open(Board_ADCBUF0, &adcBufParams);

    /* Configure the conversion struct */
    ADCBuf_Conversion singleConversion;
    singleConversion.arg = NULL;
    singleConversion.adcChannel = Board_ADCBUF0CHANNEL1;
    singleConversion.sampleBuffer = sampleBufferOne;
    singleConversion.samplesRequestedCount = ADCBUFFERSIZE;

    if (!adcBuf)
    {
        /* AdcBuf did not open correctly. */
        UART_printf(uart, "Error initialising the ADCBuf\r\n");
        while (1)
            ;
    }

    UART_printf(uart, "\tDone.\r\n");

    //========================================================

    /* Start converting. */
    if (ADCBuf_convert(adcBuf, &singleConversion, 1) !=
    ADCBuf_STATUS_SUCCESS)
    {
        /* Did not start conversion process correctly. */
        UART_printf(uart, "Error starting the ADCBuf\r\n");
        while (1)
            ;
    }

    Semaphore_pend(ADCBufSem, BIOS_WAIT_FOREVER);

    UART_printf(uart, "Buffer Ready\r\n");

    //TODO: Do something with microVoltBuffer

    //Wait forever.
    while (1)
        ;

    return;
}
