/*Include libraries*/
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"


#include "can0_init.h"


/* Semaphores Creations */
SemaphoreHandle_t xSemaphore;
SemaphoreHandle_t xSemaphore2;

uint8_t state;

/**Function: initTask
  *Description: initTask task that iniatilize the initial definitions, semaphores, and then suspended
  *
  *@para  : void
  *
  *@return: void
  *
*/
void initTask(void)
{
    state = 0;      /* 0 means startup state */

    /* Create 2 semaphores, they will act as guard conditions for the 3 tasks */
    xSemaphore = xSemaphoreCreateBinary();
    xSemaphore2 = xSemaphoreCreateCounting( 2, 0 );

    /* Led Initialization*/
    SYSCTL_RCGCGPIO_R |= 0x20;
    while ((SYSCTL_PRGPIO_R & 0x00000020) == 0);

    GPIO_PORTF_LOCK_R = 0x4C4F434B;
    GPIO_PORTF_CR_R = 0x1F;
    GPIO_PORTF_DIR_R = 0x0E;
    GPIO_PORTF_PUR_R = 0x11;
    GPIO_PORTF_DEN_R = 0x1F;

    can0_init() ;


    vTaskSuspend( NULL);

}

/**Function: receiveTask
  *Description: receiveTask task that receive the incoming data and check it to give semaphore
  *
  *@para  : void
  *
  *@return: void
  *
*/
void receiveTask(void)
{
    /*Receive initializations definitions*/
    tCANMsgObject sCANMessage;
    uint8_t pui8MsgData;
    sCANMessage.ui32MsgID = 0;
    sCANMessage.ui32MsgIDMask = 0;
    sCANMessage.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;
    sCANMessage.ui32MsgLen = 8;

    CANMessageSet(CAN0_BASE, 1, &sCANMessage, MSG_OBJ_TYPE_RX);

    sCANMessage.pui8MsgData = &pui8MsgData;

    while (1)
    {
        if (uxSemaphoreGetCount( xSemaphore2 ) == 2)
        {
            state = 1;  /* 1 means normal state */

            CANMessageGet(CAN0_BASE, 1, &sCANMessage, 0);
        if (pui8MsgData == 2 )      /* if data received correctly give the semaphore */
            {

                pui8MsgData = 0 ;
                xSemaphoreGive(xSemaphore);
            }

        }
        vTaskDelay(10);
    }
}

/**Function: ledBlinkingTask
  *Description: ledBlinkingTask task that take semaphore from receive task and blink led for 1 sec
  *
  *@para  : void
  *
  *@return: void
  *
*/
void ledBlinkingTask(void)
{
    while (1)
    {
        if (xSemaphoreTake(xSemaphore, 0) == pdTRUE)    /* if there is a semaphore, take it and blink the led*/
        {
            GPIO_PORTF_DATA_R = 0x0E;
            SysCtlDelay(16000000 / 3);
            GPIO_PORTF_DATA_R=00;
            SysCtlDelay(16000000 / 3);

           xSemaphoreGive(xSemaphore);      /* give a semaphore */
           xSemaphoreGive(xSemaphore2);
        }
        vTaskDelay(30);
    }
}

/**Function: sentTask
  *Description: sentTask task that send a token after blink the led
  *
  *@para  : void
  *
  *@return: void
  *
*/
void sentTask(void)
{
    /*Send initializations definitions*/
    tCANMsgObject sCANMessage;
    uint32_t ui32MsgData;
    uint8_t *pui8MsgData;
    pui8MsgData = (uint8_t *) &ui32MsgData;

    ui32MsgData = 3;
    sCANMessage.ui32MsgID = 1;
    sCANMessage.ui32MsgIDMask = 0;
    sCANMessage.ui32Flags = MSG_OBJ_TX_INT_ENABLE;
    sCANMessage.ui32MsgLen = sizeof(pui8MsgData);
    sCANMessage.pui8MsgData = pui8MsgData;

    while (1)
    {
        if (xSemaphoreTake(xSemaphore, 0) == pdTRUE)        /* if there is a semaphore, take it and send the message */
        {
            CANMessageSet(CAN0_BASE, 1, &sCANMessage, MSG_OBJ_TYPE_TX);
            xSemaphoreGive(xSemaphore2);
        }
        vTaskDelay(30);
    }
}

/* main function to call the tasks */
int main(void)
{
    /*Initialize the system clock*/
    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
    SYSCTL_XTAL_16MHZ);
    /*Creation of tasks*/
    xTaskCreate((TaskFunction_t) initTask, NULL, 85, NULL, 3, NULL);
    xTaskCreate((TaskFunction_t) receiveTask, NULL, 85, NULL, 2, NULL);
    xTaskCreate((TaskFunction_t) ledBlinkingTask, NULL, 85, NULL, 1, NULL);
    xTaskCreate((TaskFunction_t) sentTask, NULL, 85, NULL, 0, NULL);

    /*Scheduling the tasks*/
    vTaskStartScheduler();

    while (1)
    {

    }
}
