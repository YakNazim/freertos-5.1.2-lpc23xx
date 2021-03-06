

/*
 * Portions of this code:
 FreeRTOS.org V5.1.2 - Copyright (C) 2003-2009 Richard Barry.

 This file is part of the FreeRTOS.org distribution.

 FreeRTOS.org is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 FreeRTOS.org is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with FreeRTOS.org; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 A special exception to the GPL can be applied should you wish to distribute
 a combined work that includes FreeRTOS.org, without being obliged to provide
 the source code for any proprietary components.  See the licensing section 
 of http://www.FreeRTOS.org for full details of how and when the exception
 can be applied.
 */

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "portmacro.h"

/* Demo app includes. */
/*
#include "BlockQ.h"
#include "death.h"
#include "blocktim.h"
#include "flash.h"
#include "partest.h"
#include "GenQTest.h"
#include "QPeek.h"
#include "dynamic.h"
*/
#include <stdint.h>
#include "serial/serial.h"
#include "printf/uart0PutChar2.h"
#include "printf/printf2.h"
#include "peripherals/pwm.h"
#include "i2c/i2c.h"

#define I2CTEST_STACK_SIZE 1024

// Constants to setup I/O. 
#define mainTX_ENABLE	( ( unsigned portLONG ) (1<<4) )
#define mainRX_ENABLE	( ( unsigned portLONG ) (1<<6) )
/*
#define mainTX_ENABLE	( ( unsigned portLONG ) 0x0001 )
#define mainRX_ENABLE	( ( unsigned portLONG ) 0x0004 )
#define mainTX1_ENABLE	( ( unsigned portLONG ) 0x10000 )
#define mainRX1_ENABLE	( ( unsigned portLONG ) 0x40000 )
*/

/* Demo application definitions. */
#define mainQUEUE_SIZE						( 3 )
#define mainCHECK_DELAY						( ( portTickType ) 5000 / portTICK_RATE_MS )
#define mainBASIC_WEB_STACK_SIZE            ( configMINIMAL_STACK_SIZE * 6 )

/* Task priorities. */
#define mainQUEUE_POLL_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY				( tskIDLE_PRIORITY + 3 )
#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2 )
#define mainFLASH_PRIORITY                  ( tskIDLE_PRIORITY + 2 )
#define mainCREATOR_TASK_PRIORITY           ( tskIDLE_PRIORITY + 3 )
#define mainGEN_QUEUE_TASK_PRIORITY			( tskIDLE_PRIORITY ) 

/* Constants to setup the PLL. */
//#define mainPLL_MUL			( ( unsigned portLONG ) ( 8 - 1 ) )
//#define mainPLL_DIV			( ( unsigned portLONG ) 0x0000 )
//#define mainCPU_CLK_DIV		( ( unsigned portLONG ) 0x0003 )

//Get 48mhz for 
#define mainPLL_MUL			( ( unsigned portLONG ) ( 11 ) )
#define mainPLL_DIV			( ( unsigned portLONG ) 0x0000 )
#define mainCPU_CLK_DIV		( ( unsigned portLONG ) 0x0005 )

// #define PCLK ((12 * (mainPLL_MUL+1) * 2) / (mainPLL_DIV + 1)) / mainCPU_CLK_DIV

#define mainPLL_ENABLE		( ( unsigned portLONG ) 0x0001 )
#define mainPLL_CONNECT		( ( ( unsigned portLONG ) 0x0002 ) | mainPLL_ENABLE )
#define mainPLL_FEED_BYTE1	( ( unsigned portLONG ) 0xaa )
#define mainPLL_FEED_BYTE2	( ( unsigned portLONG ) 0x55 )
#define mainPLL_LOCK		( ( unsigned portLONG ) 0x4000000 )
#define mainPLL_CONNECTED	( ( unsigned portLONG ) 0x2000000 )
#define mainOSC_ENABLE		( ( unsigned portLONG ) 0x20 )
#define mainOSC_STAT		( ( unsigned portLONG ) 0x40 )
#define mainOSC_SELECT		( ( unsigned portLONG ) 0x01 )

/* Constants to setup the MAM. */
#define mainMAM_TIM_3		( ( unsigned portCHAR ) 0x03 )
#define mainMAM_MODE_FULL	( ( unsigned portCHAR ) 0x02 )

#define PCLK    configCPU_CLOCK_HZ

uint32_t microsecondsToCPUTicks(const uint32_t microseconds) {
    uint32_t ret = (configCPU_CLOCK_HZ / 1000000) * microseconds;
    return(ret);
}

uint32_t millisecondsToCPUTicks(const uint32_t miliseconds) {
    uint32_t ret = (configCPU_CLOCK_HZ / 1000) * miliseconds;
    return(ret);
}

#define BLINKM_ADDR 0x09

static void i2cblinkmTask(void *pvParameters) {
    int x = 0;
    signed portCHAR theChar;
    signed portBASE_TYPE status;
    const int interval = 100000;

    uint32_t blinkm_id;

    uint32_t pwmDutyCycle = 1000;

    int myDataToSend;

    printf2("VICIntEnable is: 0x%X\n\r", VICIntEnable);

    for(;;) {
        //vSerialPutString(0, "Testing...\r\n", 50);
        x++;

        if (x == interval) {
//            FIO1SET = (1<<19);//turn on led on olimex 2378 dev board

            pwmDutyCycle += 100;
            if(pwmDutyCycle > 2000 ) {
                pwmDutyCycle = 1000;
            }
            setPWMDutyCycle(PWM1_1, microsecondsToCPUTicks(pwmDutyCycle));

        } else if (x >= (interval*2)) {
            FIO1CLR = (1<<19);//turn off led on olimex 2378 Sdev board
//            vTaskDelay( 1000 );


            x = 0;

            printf2("i2c Light Task...\r\n");

            myDataToSend = 'c';
            printf2("VICRawIntr register is: 0x%X\n\r",VICRawIntr);

            printf2("sending c...\r\n");

            I2C0MasterTX(BLINKM_ADDR, &myDataToSend, 1);

            printf2("VICRawIntr register is: 0x%X\n\r",VICRawIntr);

            myDataToSend = 0xff;

            I2C0MasterTX(BLINKM_ADDR, &myDataToSend, 1);

            myDataToSend = 0xc4;

            I2C0MasterTX(BLINKM_ADDR, &myDataToSend, 1);

            status = xSerialGetChar(0, &theChar, 1);
            if( status == pdTRUE ) {
                printf2("You typed the character: '%c'\r\n", theChar);
            }
        }
    }
}


/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
#ifdef RUN_FROM_RAM
    /* Remap the interrupt vectors to RAM if we are are running from RAM. */
    SCB_MEMMAP = 2;
#endif

    /* Disable the PLL. */
    PLLCON = 0;
    PLLFEED = mainPLL_FEED_BYTE1;
    PLLFEED = mainPLL_FEED_BYTE2;

    /* Configure clock source. */
    SCS |= mainOSC_ENABLE;
    while( !( SCS & mainOSC_STAT ) );
    CLKSRCSEL = mainOSC_SELECT; 

    /* Setup the PLL to multiply the XTAL input by 4. */
    PLLCFG = ( mainPLL_MUL | mainPLL_DIV );
    PLLFEED = mainPLL_FEED_BYTE1;
    PLLFEED = mainPLL_FEED_BYTE2;

    /* Turn on and wait for the PLL to lock... */
    PLLCON = mainPLL_ENABLE;
    PLLFEED = mainPLL_FEED_BYTE1;
    PLLFEED = mainPLL_FEED_BYTE2;
    CCLKCFG = mainCPU_CLK_DIV;	
    while( !( PLLSTAT & mainPLL_LOCK ) );

    /* Connecting the clock. */
    PLLCON = mainPLL_CONNECT;
    PLLFEED = mainPLL_FEED_BYTE1;
    PLLFEED = mainPLL_FEED_BYTE2;
    while( !( PLLSTAT & mainPLL_CONNECTED ) ); 


    /* 
       This code is commented out as the MAM does not work on the original revision
       LPC2368 chips.  If using Rev B chips then you can increase the speed though
       the use of the MAM.

       Setup and turn on the MAM.  Three cycle access is used due to the fast
       PLL used.  It is possible faster overall performance could be obtained by
       tuning the MAM and PLL settings.
       MAMCR = 0;
       MAMTIM = mainMAM_TIM_3;
       MAMCR = mainMAM_MODE_FULL;
       */
       FIO1DIR |= (1<<19);

}


void enableSerial0( void ) {
    PCLKSEL0 = (PCLKSEL0 & ~(3<<6)) | (1<<6);//set uart to run at CCLK speed, UART0
    PINSEL0 |= mainTX_ENABLE;
    PINSEL0 |= mainRX_ENABLE;
}


/*-----------------------------------------------------------*/

//#define PCLK    48000000
int main( void )
{
    prvSetupHardware();

    enableSerial0();

    PWMinit (0, millisecondsToCPUTicks(30));                // 30ms period, given 48mhz CPU clock

    setupPWMChannel(PWM1_1, microsecondsToCPUTicks(1500)); // 1ms duty cycle, given 48mhz CPU clock

    xSerialPortInitMinimal(0, 115200, 250 );
    vSerialPutString(0, "Starting up LPC23xx with FreeRTOS\n\r", 50);

    SCS |= 1; //Configure FIO

    // Initialize I2C0
    I2Cinit(I2C0);


    xTaskCreate( i2cblinkmTask, 
            ( signed portCHAR * ) "i2cblinkmTask", 
            I2CTEST_STACK_SIZE, NULL, 
            mainCHECK_TASK_PRIORITY - 1, 
            NULL );

    /* Start the scheduler. */
    vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle
       task. */
    return 0; 
}

/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
    unsigned portBASE_TYPE uxColumn = 0;
    static xLCDMessage xMessage = { 0, "PASS" };
    static unsigned portLONG ulTicksSinceLastDisplay = 0;
    static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    // Called from every tick interrupt.  Have enough ticks passed to make it
    // time to perform our health status check again? 
    ulTicksSinceLastDisplay++;
    if( ulTicksSinceLastDisplay >= mainCHECK_DELAY )
    {
        ulTicksSinceLastDisplay = 0;

        /* Has an error been found in any task? */

        if( xAreBlockingQueuesStillRunning() != pdTRUE )
        {
            xMessage.pcMessage = "ERROR - BLOCKQ";
        }

        if( xAreBlockTimeTestTasksStillRunning() != pdTRUE )
        {
            xMessage.pcMessage = "ERROR - BLOCKTIM";
        }

        if( xAreGenericQueueTasksStillRunning() != pdTRUE )
        {
            xMessage.pcMessage = "ERROR - GENQ";
        }

        if( xAreQueuePeekTasksStillRunning() != pdTRUE )
        {
            xMessage.pcMessage = "ERROR - PEEKQ";
        }       

        if( xAreDynamicPriorityTasksStillRunning() != pdTRUE )
        {
            xMessage.pcMessage = "ERROR - DYNAMIC";
        }

        xMessage.xColumn++;

        /* Send the message to the LCD gatekeeper for display. */
        xHigherPriorityTaskWoken = pdFALSE;
    }
}



