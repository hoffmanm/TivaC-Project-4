#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"
#define PWM_FREQUENCY 55
int main(void)
{
    volatile uint32_t ui32Load;
    volatile uint32_t ui32PWMClock;
    volatile uint16_t ui8Adjust; // Needed a 16 bit value to handle larger values, name remained the same(need to change)
    // Allows adjusting of PWM. Default value will set to 50% duty cycle
    ui8Adjust = 500;
    // System Clock Set. 200MHz / 2 / 5 = 40MHz
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    // System PWM Clock Divider. 40MHz/64 = 625kHz
    ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_64);
    // Enable PWM1 and GPIOF Peripherals
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    // Configure PF1(RED_LED),PF2(GREEN_LED),PF3(BLUE_LED) as needed
    // PF1:RED_LED, will use PWM_GENERATOR_2, PWM_OUT_5
    ROM_GPIOPinConfigure(GPIO_PF1_M1PWM5);
    ROM_GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);
    // Configure SW1 and SW2
    // First three lines unlock SW2 as it is seen as a critical device
    // Fourth line configures them as inputs
    // Fifth line configures pull up resistors to allow a press and hold to be read
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x01;
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0;
    ROM_GPIODirModeSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_DIR_MODE_IN);
    ROM_GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4|GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    // 40MHz / 64 to set PWM Clock. This was done previously?
    ui32PWMClock = SysCtlClockGet() / 64;
    // Sets the count to be loaded. Based on clock this tick count gives us wanted Frequency (625k / 55) - 1
    ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
    // Configure PWM as a down-counter
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
    // Load the count value
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);
    // We have a 55Hz Period (18.2ms).  Dividing by 1000 gives us 18.2us.
    // To achieve the min 10%,50%(use for initial),and max 90% we need the following min,median,max
    // min: 18.2us * X = 1.82ms,  X=100
    // median: 18.2us * X = 9.1ms,  X=500
    // max: 18.2us * X = 16.4ms,  X=900
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust * ui32Load / 1000);
    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_2);
    while(1)
    {
        
        // Check is SW1 is pressed
        if(ROM_GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)==0x00)
        {
            ui8Adjust--;
            if (ui8Adjust < (100))
            {
                ui8Adjust = 100;
            }
            ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust * ui32Load / 1000);
        }
        
        // Check if SW2 is pressed
        if(ROM_GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0)==0x00)
        {
            ui8Adjust++;
            if (ui8Adjust > (900))
            {
                ui8Adjust = 900;
            }
            ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, ui8Adjust * ui32Load / 1000);
        }
        ROM_SysCtlDelay(10000);
    }
}