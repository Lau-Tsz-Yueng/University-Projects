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

void PWM_pulsed()
{
    while(1)
    {
        //Sleep for 500ms
        Task_sleep(SLEEP_1MS * 500);
        //Set duty to 0
        UART_printf(uart, "PWM = 0\r\n");
        PWM_setDuty(pwm1, 0);
        //Sleep for 500ms
        Task_sleep(SLEEP_1MS * 500);
        //Set duty to 50%
        UART_printf(uart, "PWM = 50%%\r\n");
        PWM_setDuty(pwm1, PWM_INIT_DUTY);
    }

}


void init_PWM()
{
    PWM_init();
    PWM_Params pwmparams;
    PWM_Params_init(&pwmparams);
    pwmparams.dutyUnits = PWM_DUTY_US;
    //Initialise with PWM off.
    pwmparams.dutyValue = 0;
    pwmparams.periodUnits = PWM_PERIOD_US;
    pwmparams.periodValue = PWM_PER_US;
    pwm1 = PWM_open(Board_PWM2, &pwmparams);
    if (pwm1 == NULL)
    {
        /* Board_PWM0 did not open */
        UART_printf(uart, "Error initialising PWM 2\r\n");
        while (1)
            ;
    }

    PWM_start(pwm1);
    //Wait for long enough that no echoes should show up in the ADC samples
    //Sound travels at 300m/s, so a sleep of 1 sec will remove any echoes within 150m
    Task_sleep(SLEEP_1_SEC);

    //Get rid of the pending samples
    Semaphore_pend(ADCBufSem, BIOS_WAIT_FOREVER);
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


    Memory_Stats stats;
    Memory_getStats((IHeap_Handle)heap0, &stats);
    UART_printf(uart, "Space available on heap: %x lf, %x tot\r\n",
                stats.largestFreeSize, stats.totalFreeSize);


    UART_printf(uart, "Initialising PWM output on pin DIO21...");
    init_PWM();
    UART_printf(uart, "\tDone.\r\n");

    PWM_pulsed();


    return;
}
