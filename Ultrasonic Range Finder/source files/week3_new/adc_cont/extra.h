#ifndef EXTRA_H_
#define EXTRA_H_


#define SAMPLE_RATE (160000)

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

#include <ti/drivers/UART.h>
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

#endif /* EXTRA_H_ */
