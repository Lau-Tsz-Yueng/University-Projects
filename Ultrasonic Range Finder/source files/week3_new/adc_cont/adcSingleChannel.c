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

Types_FreqHz freq;

void Buffered_ADC_Print()
{
    int j = 0;
    while (1)
    {
        Semaphore_pend(ADCBufSem, BIOS_WAIT_FOREVER);
        uint32_t start, end, diff;
        float tdiff;
        start = Timestamp_get32();
        int i;
        uint32_t min = UINT32_MAX, max = 0;
        uint64_t sum = 0;
        for (i = 0; i < ADCBUFFERSIZE; i++)
        {
            sum += microVoltBuffer[i];

			// If current value < min, update min with current value, else keep min the same.
            min = (microVoltBuffer[i] < min)?microVoltBuffer[i]:min;
			/* 	
			as an example for the ? operator, the code below
				if (a + b < 4) {
					result = 'A';
				} else {
					result = 'B';
				}
			is equlivent to
				result = (a + b < 4) ? 'A' : 'B';
			*/
            max = 0; //Complete this to get the maximum value
        }
        float mean = sum / (float) ADCBUFFERSIZE;
        end = Timestamp_get32();
        diff = end - start;
        tdiff = diff / (float) freq.lo;
        if ((j++ & 0xF) == 0)
        {
            UART_printf(
                    uart,
                    "mean: %f, min: %u, max: %u, tdiff: %f\r\n",
                    mean, min, max, tdiff);
        }
    }
}

void mainThread(UArg a1, UArg a2)
{
    Timestamp_getFreq(&freq);
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

    UART_printf(uart, "Initialising the ADCBuf...");
    ADCBuf_init();
    /* Set up an ADCBuf peripheral in ADCBuf_RECURRENCE_MODE_CONTINUOUS */
    ADCBuf_Params adcBufParams;
    ADCBuf_Params_init(&adcBufParams);
    adcBufParams.callbackFxn = adcBufCallback;
    adcBufParams.recurrenceMode = ADCBuf_RECURRENCE_MODE_CONTINUOUS;
    adcBufParams.returnMode = ADCBuf_RETURN_MODE_CALLBACK;
    adcBufParams.samplingFrequency = SAMPLE_RATE;
    adcBuf = ADCBuf_open(Board_ADCBUF0, &adcBufParams);

    /* Configure the conversion struct */
    ADCBuf_Conversion continuousConversion;
    continuousConversion.arg = NULL;
    continuousConversion.adcChannel = Board_ADCBUF0CHANNEL1;
    continuousConversion.sampleBuffer = sampleBufferOne;
    continuousConversion.sampleBufferTwo = sampleBufferTwo;
    continuousConversion.samplesRequestedCount = ADCBUFFERSIZE;

    if (!adcBuf)
    {
        /* AdcBuf did not open correctly. */
        UART_printf(uart, "Error initialising the ADCBuf\r\n");
        while (1)
            ;
    }

    /* Start converting. */
    if (ADCBuf_convert(adcBuf, &continuousConversion, 1) !=
    ADCBuf_STATUS_SUCCESS)
    {
        /* Did not start conversion process correctly. */
        UART_printf(uart, "Error starting the ADCBuf\r\n");
        while (1)
            ;
    }

    UART_printf(uart, "\tDone.\r\n");

    Memory_Stats stats;
    Memory_getStats((IHeap_Handle)heap0, &stats);
    UART_printf(uart, "Space available on heap: %x lf, %x tot\r\n",
                stats.largestFreeSize, stats.totalFreeSize);

    //PWM_pulsed();

    Buffered_ADC_Print();

    return;
}
