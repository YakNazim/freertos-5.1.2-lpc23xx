/*
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

    ***************************************************************************
    ***************************************************************************
    *                                                                         *
    * Get the FreeRTOS eBook!  See http://www.FreeRTOS.org/Documentation      *
	*                                                                         *
	* This is a concise, step by step, 'hands on' guide that describes both   *
	* general multitasking concepts and FreeRTOS specifics. It presents and   *
	* explains numerous examples that are written using the FreeRTOS API.     *
	* Full source code for all the examples is provided in an accompanying    *
	* .zip file.                                                              *
    *                                                                         *
    ***************************************************************************
    ***************************************************************************

	Please ensure to read the configuration and relevant port sections of the
	online documentation.

	http://www.FreeRTOS.org - Documentation, latest information, license and 
	contact details.

	http://www.SafeRTOS.com - A version that is certified for use in safety 
	critical systems.

	http://www.OpenRTOS.com - Commercial support, development, porting, 
	licensing and training services.
*/

/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the standard demo application tasks.
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * "LCD" task - the LCD task is a 'gatekeeper' task.  It is the only task that
 * is permitted to access the display directly.  Other tasks wishing to write a
 * message to the LCD send the message on a queue to the LCD task instead of
 * accessing the LCD themselves.  The LCD task just blocks on the queue waiting
 * for messages - waking and displaying the messages as they arrive.
 *
 * "Check" hook -  This only executes every five seconds from the tick hook.
 * Its main function is to check that all the standard demo tasks are still 
 * operational.  Should any unexpected behaviour within a demo task be discovered 
 * the tick hook will write an error to the LCD (via the LCD task).  If all the 
 * demo tasks are executing with their expected behaviour then the check task 
 * writes PASS to the LCD (again via the LCD task), as described above.
 *
 * "uIP" task -  This is the task that handles the uIP stack.  All TCP/IP
 * processing is performed in this task.
 */

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "portmacro.h"

/* Demo app includes. */
#include "BlockQ.h"
#include "death.h"
#include "blocktim.h"
#include "flash.h"
#include "partest.h"
#include "GenQTest.h"
#include "QPeek.h"
#include "dynamic.h"


#include "debug.h"
#include "usbapi.h"
#include "usbhw_lpc.h"
#include "lpc23xx.h"
#include "printf/printf2.h"
#include "peripherals/can.h"

void readCanBus(void) {
	can_message_t msg;
	const int r = dequeueRxCAN(CAN_BUS_2, &msg);
	if (r == 1 ) {
		if (msg.isPopulated) {
			printf2("-----------------------\r\n");
			printf2("dataA=0x%X\r\n", msg.dataA);
			printf2("dataB=0x%X\r\n", msg.dataB);
		}
	} else {
		//printf2("CAN read error %d\r\n", r);
	}
}

void putchar2(const int fd, const int ch)
{
	VCOM_putchar(ch);
}

extern volatile uint32_t g_can_isr_count;

static void blinkyLightTask(void *pvParameters) {
	int x = 0;
		
	const int interval = 2000;
	
	int status = 0;
	for(;;) {
		x++;

		if (x == interval) {
			FIO1SET = (1 << 19);//turn on led on olimex 2378 dev board
			status = 1;
		} else if (x >= (interval * 2)) {
			FIO1CLR = (1 << 19);//turn off led on olimex 2378 Sdev board
			status = 0;
			x = 0;
			//printf2("CAN-a-Bus!!!!\r\n");
			static uint32_t a_number = 123456;
			a_number++;
			//transmitCAN(CAN_BUS_2, 0x102, a_number, 0x12345678, 4, false);


			can_message_t msg;
			memset(&msg, 0, sizeof(can_message_t));
			msg.dataA = a_number;
			msg.dataB = 0xAAAA5555;
			msg.dataLengthCode = 8;
			msg.rtr = 0;
			msg.id = 0x102;
			msg.isPopulated = 1;
			enqueueTxCAN(CAN_BUS_2, &msg);
			processCANTxQueue(CAN_BUS_2);
		}
		//readCanBus();


		int c = VCOM_getchar();
		if ( c != EOF ) {
			/*
			if (status) {
				FIO1CLR = (1 << 19);//turn off led on olimex 2378 Sdev board
				status = 0;
			} else {
				FIO1SET = (1 << 19);//turn on led on olimex 2378 dev board
				status = 1;
			}
			// show on console
			if ((c == 9) || (c == 10) || (c == 13) || ((c >= 32) && (c <= 126))) {
				DBG("%c", c);
			}
			else {
				DBG(".");
			}
			VCOM_putchar(c);
			*/

			if( c == '?' ) {
				printf2("CAN-a-Bus!\r\n");
				const uint32_t gsr = CAN2GSR;
				printf2("CAN2GSR = 0x%X\r\n", gsr);
				printf2("CAN2GSR_RS = %d\r\n", ((gsr >> 4) & 0x1));
				printf2("CAN2GSR_RXERR = %d\r\n", ((gsr >> 16) & 0xFF));
				printf2("CAN2GSR_TXERR = %d\r\n", ((gsr >> 24) & 0xFF));
				printf2("g_can_isr_count = %u\r\n", g_can_isr_count);
				printf2("CAN2IER = 0x%X\r\n", CAN2IER);

				printf2("VICIntEnable = 0x%X\r\n", VICIntEnable);
				printf2("VICIntSelect = 0x%X\r\n", VICIntSelect);
				printf2("VICVectAddr23 = 0x%X\r\n", VICVectAddr23);
				printf2("VICRawIntr = 0x%X\r\n", VICRawIntr);
				printf2("VICSWPrioMask = 0x%X\r\n", VICSWPrioMask);


			} else if( c == 'r' ) {
				printf2("reseting can2 bus\r\n");
				// Set CAN into reset mode, allows modification of CAN configuration registers
				disableCAN(CAN_BUS_2);
				CAN2GSR = 0;
				reEnableCAN(CAN_BUS_2);
			}
		}


		//vTaskDelay(1);

	}

}


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


#define mainTX_ENABLE	( ( unsigned portLONG ) (1<<4) )
#define mainRX_ENABLE	( ( unsigned portLONG ) (1<<6) )
void enableSerial0( void ) {
	PCLKSEL0 = (PCLKSEL0 & ~(3<<6)) | (1<<6);//set uart to run at CCLK speed, UART0
	PINSEL0 |= mainTX_ENABLE;
	PINSEL0 |= mainRX_ENABLE;
}

/* Configure the hardware as required by the demo. */
static void prvSetupHardware( void );


/*-----------------------------------------------------------*/

int main( void )
{
	SCS |= 1;
	FIO1DIR |= (1<<19);


	int i = 1;
	int j = 0;
	while(j ) {
		i++;
	}

	prvSetupHardware();

	usbInit();


	enableSerial0();
	xSerialPortInitMinimal(0, 115200, 250 );
	vSerialPutString(0, "Starting up LPC23xx with FreeRTOS\n", 50);
	
	/*
	                      CANPCLK
	cAnBPS =      -------------------------
	          ((1 + (TSEG1 + 1) + (TSEG2 + 1)) * (brp+1))


	          16
	          5
	*/


#if 0
	//1mbit CAN settings, tested
	const uint8_t brp = 1;
	const uint8_t sjw = 1;
	const uint8_t tseg1 = 15;
	const uint8_t tseg2 = 5;
	const bool sam = false;
#else
	//250kbit CAN settings
	const uint8_t brp = 7;
	const uint8_t sjw = 1;
	const uint8_t tseg1 = 15;
	const uint8_t tseg2 = 5;
	const bool sam = false;
#endif
	initCANQueues();

	PINMODE0 = (PINMODE0 | (1<<9) | (1<<11)) & ~((1<<8) | (1<<10));//disable pullup and pulldown resitors
	PINSEL0  = (PINSEL0  | (1<<9) | (1<<11)) & ~((1<<8) | (1<<10));//Set P0.4 and P0.5 into RD2 and TD2 mode

	initializeCAN(CAN_BUS_2, brp, sjw, tseg1, tseg2, sam);
	/*
	VICIntEnClr = CAN_VIC_INTERUPT_BITMASK;
	VICIntSelect &= ~CAN_VIC_INTERUPT_BITMASK;//Set to IRQ mode
	VICVectAddr23 = (uint32_t) canISR;
	//VICVectCntl23 = 0x04;
	VICIntEnable |= CAN_VIC_INTERUPT_BITMASK;

	CAN2MOD |= CANxMOD_RM;
	CAN2IER |=  CANxIER_RIE;
	CAN2MOD &= ~CANxMOD_RM;
	*/


	xTaskCreate( blinkyLightTask, ( signed portCHAR * ) "usbCounter", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY - 1, NULL );

	/* Start the scheduler. */
	vTaskStartScheduler();

    /* Will only get here if there was insufficient memory to create the idle
    task. */
	return 0; 
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{

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
	
	/* Setup the led's on the MCB2300 board */
	vParTestInitialise();
}







