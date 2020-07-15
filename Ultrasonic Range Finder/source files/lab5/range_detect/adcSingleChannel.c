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

#include <ti/drivers/timer/GPTimerCC26XX.h>
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


#define SENSITIVITY (0x80000)

void PWM_pulsed()
{

    //Forever loop
    while (1)
    {

        //Loop to find next edge
        //Number of buffers to try before concluding lost (wait 1 second = 150 meters)
        int max_wait_buffers = (SAMPLE_RATE / ADCBUFFERSIZE);

        //Make sure we're at the start of a sample period,else we'll run out of time on the first buffer we receive.
        //We do this by first clearing the semaphore, then pending on it to synchronise with the start of the next
        //sample period.
        Semaphore_reset(ADCBufSem, 0); //Clear the semaphore.
        Semaphore_pend(ADCBufSem, BIOS_WAIT_FOREVER); //Wait for next sample period.

        //Set the PWM duty to half the period (50% duty)
        PWM_setDuty(pwm1, PWM_PER_US / 2);
        uint32_t start = Timestamp_get32();

        //buffer counter - so we can cause the pulse read to expire after a certian amount of time.
        int bufcount = 0;
        //Wait for 1 second's worth of buffers (max theoretical distance 150m).
        while (bufcount++ < max_wait_buffers)
        {
            //Wait for next set of samples
            Semaphore_pend(ADCBufSem, BIOS_WAIT_FOREVER);

            //Loop through the buffer to find the position at which the return signal starts.
            //TODO: complete.

            //Test logic of is_it_here().
            uint32_t test_buffer[32] = {0,0,0,0,0,0,0,0, //All 0s, FALSE
                                        3000000,3000000,3000000,3000000,3000000,3000000,3000000,3000000, //DC, FALSE
                                        3000000,0000000,3000000,0000000,3000000,0000000,3000000,0000000, //80kHz + DC, FALSE
                                        3000000,2000000,1000000,2000000,3000000,2000000,1000000,2000000 //40kHz + DC, TRUE
            };
            if (is_it_here(test_buffer, 0))
            {
                UART_printf(uart, "This should never happen (all 0s)\r\n");
            }
            if (is_it_here(test_buffer, 8))
            {
                UART_printf(uart, "This should never happen (DC)\r\n");
            }
            if (is_it_here(test_buffer, 16))
            {
                UART_printf(uart, "This should never happen (Different Freq)\r\n");
            }
            if (is_it_here(test_buffer, 24))
            {
                UART_printf(uart, "This should always happen\r\n");
            }
            int i = 0;
            //Check we didn't get all the way through the loop.
            if (i == ADCBUFFERSIZE - FILTER_COEFFS)
            {
                //If we did, go to the next buffer. (next iteration of "while(bufcount++ < max_wait_buffers)")
                continue;
            }
            else
            {
                //We received the return signal.  Print some info about it.
                print_response(i, start, bufcount);
                break;
            }
        } //End loop to find next edge (Semaphore_pend(ADCBufSem))
        if (max_wait_buffers < bufcount)
        {
            UART_printf(uart, "Timed out waiting for response\r\n");
        }
        PWM_setDuty(pwm1, 0);
        //Wait for any echoes to disappear (50ms)
        Task_sleep(SLEEP_1_SEC / 5);
        //Get rid of any pending samples
        Semaphore_pend(ADCBufSem, BIOS_WAIT_FOREVER);
    } //End forever loop

}

/**
* Autocorrelation function for a 40kHz sinusoid.
* @param buffer Buffer to autocorrelate with.
* @param i Position in buffer to start autocorrelation.
* @return the real and imaginary parts of the autocorrelation response.
*/
Complex32 autocorrelate(volatile uint32_t *buffer, int i)
{
 Complex32 response = { 0, 0 };
 int j;
 for (j = 0; j < FILTER_COEFFS; j++)
 {
     response.real += (((int32_t) buffer[i + j] >> 6) * my_filter_re[j])
             >> 18;
     response.imag += (((int32_t) buffer[i + j] >> 6) * my_filter_im[j])
             >> 18;
 }
 return response;
}

/**
* Function to find if the return signal has occurred at a specific point.
* @param buffer Buffer to look in.
* @param i position in buffer to check for response.
* @return Whether there is a significant response at position i.
*/

Bool is_it_here(volatile uint32_t *buffer, int i)
{
    //TODO: Complete
}
