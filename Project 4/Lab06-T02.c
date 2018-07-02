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
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"


#define PWM_FREQUENCY 55
#define DUTY10 100
#define DUTY90 900

int main(void)
{
    volatile uint32_t ui32Load;
    volatile uint32_t ui32PWMClock;
    volatile uint16_t defaultAdjust; // Needed a 16 bit value to handle larger values, name remained the same(need to change)
    volatile uint16_t r,g,b; // Used for nested for loops

    // Allows adjusting of PWM. Default value will set to 10% duty cycle
    defaultAdjust = 100;
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
    // PF2:BLUE_LED, will use PWM_GENERATOR_3, PWM_OUT_6
    ROM_GPIOPinConfigure(GPIO_PF2_M1PWM6);
    // PF3:GREEN_LED, will use PWM_GENERATOR_3, PWM_OUT_7
    ROM_GPIOPinConfigure(GPIO_PF3_M1PWM7);
    ROM_GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    // 40MHz / 64 to set PWM Clock. This was done previously?
    ui32PWMClock = SysCtlClockGet() / 64;
    // Sets the count to be loaded. Based on clock this tick count gives us wanted Frequency (625k / 55) - 1
    ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
    // Configure PWM_GEN_1 as a down-counter for PF1(RED_LED)
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    // Configure PWM_GEN_2 as a down-counter for PF2/3(BLUE_LED/GREEN_LED)
    PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    // Load the count value
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);

    // We have a 55Hz Period (18.2ms).  Dividing by 1000 gives us 18.2us.
    // To achieve the min 10%,50%(use for initial),and max 90% we need the following min,median,max
    // min: 18.2us * X = 1.82ms,  X=100
    // median: 18.2us * X = 9.1ms,  X=500
    // max: 18.2us * X = 16.4ms,  X=900
    // start all three LEDs at 10%
    // RED_LED
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, defaultAdjust * ui32Load / 1000);
    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_2);
    // BLUE_LED
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, defaultAdjust * ui32Load / 1000);
    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);
    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_3);
    // GREEN_LED
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, defaultAdjust * ui32Load / 1000);
    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_3);

    while(1)
    {

        //FOR TESTING PURPOSES
        //SEE EACH LED CYCLE THE DUTY CYCLES
//        for (b=DUTY10;b<DUTY90;b++)
//        {
//            ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, b * ui32Load / 1000);
//            ROM_SysCtlDelay(100000);
//        } // end b++ loop
//        for (b=DUTY90;b>=DUTY10;b--)
//        {
//            ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, b * ui32Load / 1000);
//            ROM_SysCtlDelay(100000);
//        } // end b-- loop
//        for (g=DUTY10;g<DUTY90;g++)
//        {
//            ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, g * ui32Load / 1000);
//            ROM_SysCtlDelay(100000);
//        } // end r loop
//        for (g=DUTY90;g>=DUTY10;g--)
//        {
//            ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, g * ui32Load / 1000);
//            ROM_SysCtlDelay(100000);
//        } // end g loop

        // Nested For-Loops per instructions
        for (r=DUTY10;r<DUTY90;r++)
        {
            for(b=DUTY10;b<DUTY90;b++)
            {
                for(g=DUTY10;g<DUTY90;g++)
                {
                    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, r * ui32Load / 1000);
                    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, b * ui32Load / 1000);
                    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, g * ui32Load / 1000);
                    ROM_SysCtlDelay(100000);
                } // end g for loop
            } // end b for loop
        } // end r for loop

        for (r=DUTY90;r>=DUTY10;r--)
        {
            for(b=DUTY90;b>=DUTY10;b--)
            {
                for(g=DUTY90;g>=DUTY10;g--)
                {
                    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, r * ui32Load / 1000);
                    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, b * ui32Load / 1000);
                    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, g * ui32Load / 1000);
                    ROM_SysCtlDelay(100000);
                } // end g for loop
            } // end b for loop
        } // end r for loop

    } // end while
} // end main