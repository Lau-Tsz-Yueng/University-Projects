#ifndef EXTRA_H_
#define EXTRA_H_

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <xdc/runtime/Types.h>
#include <ti/drivers/PWM.h>

#include <ti/drivers/UART.h>

void UART_printf(UART_Handle uart, const char *fmt, ...);

void UART_readFlush(UART_Handle uart);

//BIOS Task library
#include <ti/sysbios/knl/Task.h>
//main thread
extern void mainThread(UArg a1, UArg a2);

extern UART_Handle uart;
extern PWM_Handle pwm1;

//PWM period in microseconds
#define PWM_PER_US 25

//PWM duty cycle in microseconds
#define PWM_INIT_DUTY (PWM_PER_US / 2)

//Sleep for 1 millisecond (1 tick is 10us)
#define SLEEP_1MS 100


#endif /* EXTRA_H_ */

