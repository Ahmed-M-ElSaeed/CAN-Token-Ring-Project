/*
 * can0_init.c
 *
 *  Created on: Jul 4, 2019
 *      Author: AVE-LAP-064
 */

#include "can0_init.h"


void can0_init()
{

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
       GPIOPinConfigure(GPIO_PB4_CAN0RX);
       GPIOPinConfigure(GPIO_PB5_CAN0TX);
       GPIOPinTypeCAN(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5);

       SysCtlPeripheralEnable(SYSCTL_PERIPH_CAN0);
       CANInit(CAN0_BASE);

       CANBitRateSet(CAN0_BASE, SysCtlClockGet(), 500000);

       CANEnable(CAN0_BASE);

}

