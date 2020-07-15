#ifndef EXTRA_H_
#define EXTRA_H_

#define ADC_SS 0
#define MODE_GPIO 0

/*
 * 20kHz sample rate, max that the board can support when using ADC_convert().
 * Can push to 25kHz, but this almost maxes the CPU and causes the printing in the
 * main thread to be delayed.  1sec (20kHz) -> 1.2-1.4 secs (25kHz)
 */
#define SAMPLE_RATE (160000)

//Filter # of coefficients
#if SAMPLE_RATE == 100000
#define FILTER_COEFFS (5)
#elif SAMPLE_RATE == 160000
#define FILTER_COEFFS (8)
#endif

//ADC sample buffer size
#define ADCBUFFERSIZE (1024)

//Task_sleep has 1 tick = 10us
#define SLEEP_1_SEC (100000)

//PWM Period in us
#define PWM_PER_US (25)

//PWM initial duty
#define PWM_INIT_DUTY (0)

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <xdc/runtime/Types.h>

#include <ti/drivers/timer/GPTimerCC26XX.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/PWM.h>

#include <ti/sysbios/knl/Semaphore.h>

#include <ti/drivers/ADCBuf.h>
void adcBufCallback(ADCBuf_Handle handle, ADCBuf_Conversion *conversion,
    void *completedADCBuffer, uint32_t completedChannel);
//Made volatile as changed in ISR
extern volatile uint16_t sampleBufferOne[ADCBUFFERSIZE];
extern volatile uint16_t sampleBufferTwo[ADCBUFFERSIZE];
extern volatile uint32_t microVoltBuffer[ADCBUFFERSIZE];
extern Semaphore_Handle ADCBufSem;
extern ADCBuf_Handle adcBuf;
extern volatile uint32_t adcBufReturnTime;

//BIOS Task library
#include <ti/sysbios/knl/Task.h>
//main thread
extern void mainThread(UArg a1, UArg a2);

void UART_printf(UART_Handle uart, const char *fmt, ...);

void UART_readFlush(UART_Handle uart);

extern UART_Handle uart;
extern PWM_Handle pwm1;

#if SAMPLE_RATE == 100000
extern int32_t my_filter_im[FILTER_COEFFS];
extern int32_t my_filter_re[FILTER_COEFFS];
#endif

#if SAMPLE_RATE == 160000
extern int32_t my_filter_re[FILTER_COEFFS];
extern int32_t my_filter_im[FILTER_COEFFS];
#endif


typedef struct complex32
{
    int32_t real;
    int32_t imag;
} Complex32;

#endif /* EXTRA_H_ */
