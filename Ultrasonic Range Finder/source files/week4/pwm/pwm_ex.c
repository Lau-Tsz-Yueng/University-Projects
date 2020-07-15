#include "extra.h"
#include <stdint.h>
#include <stddef.h>
#include <xdc/cfg/global.h>
#include <xdc/runtime/Types.h>
#include "Board.h"
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/PWM.h>

void pwmLoop() 
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
