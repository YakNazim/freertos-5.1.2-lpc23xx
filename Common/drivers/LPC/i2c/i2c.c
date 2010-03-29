/*
 *   I2C handling suite
 *   Jeremy Booth
 *   Portland State Aerospace Society
 *
 *   http://psas.pdx.edu
 *   
 *   FreeRTOS is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *   
 *   FreeRTOS is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with FreeRTOS; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *   
 *   A special exception to the GPL can be applied should you wish to distribute
 *   a combined work that includes FreeRTOS, without being obliged to provide
 *   the source code for any proprietary components.  See the licensing section
 *   of http://www.FreeRTOS.org for full details of how and when the exception
 *   can be applied.
 *   
 *   ***************************************************************************
 *   See http://www.FreeRTOS.org for documentation, latest information, license
 *   and contact details.  Please ensure to read the configuration and relevant
 *   port sections of the online documentation.
 *   ***************************************************************************
 */

#include <stdint.h>
#include <stdio.h>

#include "lpc23xx.h"
#include "printf/printf2.h"
#include "FreeRTOSConfig.h"

#include "i2c.h"

/*
 * I2CInit_State
 */
void I2CInit_State(i2c_master_t* s) {
    int i = 0;

    s->I2Cstate             = I2C_IDLE;
    for(i=0, i<I2C_MAX_BUFFER; i++) {
        s.I2Cbuffer[i] = 0
    }
    s->I2Cext_slave_address = 0x0;
    s->write_length         = 0x0;
    s->read_length          = 0x0;
}

/*
 * I2Cinit
 *
 * 0. Init xaction structure.
 * 1. Turn on the power to the channel
 * 2. Configure the clock for the channel
 * 3. Set I/O pins to correct mode
 * 4. Configure interrupt in VIC
 * 5. Continue initializing...tbd
 */
void I2Cinit(i2c_iface channel) {
    { 
        portENTER_CRITICAL();

        // Sat 27 March 2010 11:02:36 (PDT)
        // only create one semaphore for now, maybe one for each
        // channel later.
        vSemaphoreCreateBinary( i2cSemaphore_g );
        if( i2cSemaphore_g == NULL ) {
            vSerialPutString(0, "*** I2C-ERROR ***: Failed to create i2cSemaphore_g. I2CInit Failed!\n\r", 50);
        } else {
            switch(channel) {
                case I2C0: 
                    // structure
                    I2CInit_State( &i2c0_s_g );

                    // power
                    SET_BIT(PCONP, PCI2C0);

                    // Enable
                    I2C0CONCLR = 0x7C;
                    SET_BIT(I2C0CONSET, I2EN ); // master mode
                    ZERO_BIT(I2C0CONSET, AA );
                    printf2("I20Conset is 0x%X\n",I2C0CONSET);

                    // I2C clock
                    I2C0SCLL = I2SCLLOW;
                    I2C0SCLH = I2SCLHIGH;

                    // 2368 is 100pin package use table 107 p158 lpc23xx usermanual
                    PINSEL1 &= SDA0MASK;
                    PINSEL1 |= SDA0;
                    PINSEL1 &= SCL0MASK;
                    PINSEL1 |= SCL0;

                    // pinmode:   I2C0 pins permanent open drain (pullup)
                    // reference: lpc23xx usermanual p158 table 107 footnote 2

                    // vic
                    // set up VIC p93 table 86 lpc23xx user manual
                    SET_BIT(VICIntEnable, VICI2C0EN);
                    VICVectAddr9 = (unsigned) i2c0_isr;

                    I2C0CONCLR = 0x1<<SI;
                    break;

                case I2C1: 
                    // structure
                    I2CInit_State( &i2c1_s_g );


                    SET_BIT(PCONP, PCI2C1);

                    SET_BIT(I2C1CONSET, I2EN );
                    ZERO_BIT(I2C1CONSET, AA );

                    I2C1SCLL = I2SCLLOW;
                    I2C1SCLH = I2SCLHIGH;

                    PINSEL1 &= SDA1MASK;
                    PINSEL1 |= SDA1;
                    PINSEL1 &= SCL1MASK;
                    PINSEL1 |= SCL1;

                    PINMODE1 &= SDA1MASK;
                    PINMODE1 |= PULLUP;
                    PINMODE1 &= SCL1MASK;
                    PINMODE1 |= PULLUP;

                    SET_BIT(VICIntEnable, VICI2C1EN);
                    VICVectAddr19 = (unsigned) i2c1_isr;
                    break;

                case I2C2: 
                    // structure
                    I2CInit_State( &i2c2_s_g );


                    SET_BIT(PCONP, PCI2C2);

                    SET_BIT(I2C2CONSET, I2EN );
                    ZERO_BIT(I2C2CONSET, AA );

                    I2C2SCLL = I2SCLLOW;
                    I2C2SCLH = I2SCLHIGH;

                    PINSEL0 &= SDA2MASK;
                    PINSEL0 |= SDA2;
                    PINSEL0 &= SCL2MASK;
                    PINSEL0 |= SCL2;

                    PINMODE2 &= SDA2MASK;
                    PINMODE2 |= PULLUP;
                    PINMODE2 &= SCL2MASK;
                    PINMODE2 |= PULLUP;

                    SET_BIT(VICIntEnable, VICI2C2EN);
                    VICVectAddr30 = (unsigned) i2c2_isr;
                    break;

                default:
                    //  error         ???
                    break;
            }
        }
    }
    portEXIT_CRITICAL();
}

/*
 * i2c0_isr
 */
void i2c0_isr(void) {

    portSAVE_CONTEXT();

    uint32_t      status;
    uint8_t       holdbyte;
    uint8_t       give_binsem;
    static signed portBASE_TYPE xHigherPriorityTaskWoken; 

    xHigherPriorityTaskWoken = pdFALSE;
    give_binsem              = 0;

    status = I2C0STAT;

#if DEBUG_ISR==1
    // status (state) register observability to io pins
#include "i2c_debug_isr_status.c"
#endif
    //Read the I2C state from the correct I2STA register and then branch to
    //the corresponding state routine.
    switch(status) {
        //State 0x00 - Bus Error
        case 0x00:
            //write 0x14 to I2CONSET to set the STO and AA flags.
            SET_BIT(I2C0CONSET, STO);
            give_binsem = 1;
            SET_BIT(I2C0CONSET, AA);

            I2C0CONCLR = 0x1<<SI;
            break;

            // State 0X08 - A start condition has been transmitted, The Slave Address 
            // and Read or Write bit will be transmitted.  An ACK bit will be received.
        case 0x08:
            //write the Slave Address with R/W bit to I2DAT
            I2C0DAT = I2C0ExtSlaveAddress;

            //write 0x04 to I2CONSET to set the AA bit
            SET_BIT(I2C0CONSET,AA);

            //write 0x08 to I2CONCLR to clear the SI flag
            //                ZERO_BIT(I2C0CONSET, STA);
            I2C0CONCLR = 0x1<<SI;
            break;

            //State 0x10 - A repeated start condition has been transmitted.  The Slave
            //Address and Read or Write bit will be transmitted.  An ACK bit will be 
            //received
        case 0x10:
            //write the Slave Address with R/W bit to I2DAT
            I2C0DAT = I2C0ExtSlaveAddress;

            //write 0x08 to I2CONCLR to clear the SI flag
            ZERO_BIT(I2C0CONSET, STA);
            I2C0CONCLR = 0x1<<SI;
            break;

            //State 0x18 - Previous state was 0x08 or 0x10.  Slave Address and Read or 
            //Write has been transmitted.  An ACK has been received. The first data byte 
            //will be transmitted, an ACK bit will be received.
        case 0x18:

            FIO0SET = (1<<6);//turn on p0.6on olimex 2378 Sdev board
            if(I2C0DataCounter < I2C0DataLength) {

                FIO0SET = (1<<6);//turn on p0.6on olimex 2378 Sdev board

                I2C0DAT = I2C0TransmitData[I2C0DataCounter];
                //                    I2C0DAT = 0x55;
                SET_BIT(I2C0CONSET,AA);
                I2C0CONCLR = 0x20;
                I2C0CONCLR = 0x1<<SI;
                //    ZERO_BIT(I2C0CONSET, STO);
                // ZERO_BIT(I2C0CONSET, STA);
                I2C0DataCounter++;
            }
            break;

            //State 0x20 - Slave Address + Write has been transmitted.  NOT ACK has been 
            //received. A Stop condition will be transmitted.
        case 0x20:

            SET_BIT(I2C0CONSET, STO);
            give_binsem = 1;
            SET_BIT(I2C0CONSET, AA);
            I2C0CONCLR = 0x1<<SI;
            break;

            //State 0x28 - Data has been transmitted, ACK has been received. If the 
            //transmitted data was the last data byte then transmit a Stop condition, 
            //otherwise transmit the next data byte.
        case 0x28:
            if(I2C0DataCounter == I2C0DataLength) {
                // test condition for repeated start. Don't send STO bit...
                if(i2c_repeat_start_g != 1) {
                    SET_BIT(I2C0CONSET, STO);
                    I2C0CONCLR = 0x20;
                } else {
                    I2C0CONCLR = 0x20;
                   // SET_BIT(I2C0CONSET, STA);
                }
                give_binsem = 1;
                SET_BIT(I2C0CONSET, AA);
            } else if(I2C0DataCounter < I2C0DataLength) {
                I2C0DAT = I2C0TransmitData[I2C0DataCounter];
                I2C0CONCLR = 0x20;
                I2C0DataCounter++;
            }
            I2C0CONCLR = 0x1<<SI;
            break;

            //State 0x30 - Data has been transmitted, NOT ACK received. A Stop condition 
            //will be transmitted.
        case 0x30:
            SET_BIT(I2C0CONSET, STO);
            give_binsem = 1;
            SET_BIT(I2C0CONSET, AA);
            I2C0CONCLR = 0x1<<SI;
            break;

            //State 0x38 - Arbitration has been lost during Slave Address + Write or data. 
            //The bus has been released and not addressed Slave mode is entered. A new Start 
            //condition will be transmitted when the bus is free again.
        case 0x38:
            SET_BIT(I2C0CONSET, STA);
            SET_BIT(I2C0CONSET, AA);
            I2C0CONCLR = 0x1<<SI;
            break;

            // Data has been received, ACK has been returned. Data will be read from I2DAT. Additional
            // data will be received. If this is the last data byte then NOT ACK will be returned, otherwise
            // ACK will be returned.
            //  1. Read data byte from I2DAT into Master Receive buffer.
            //  2. Decrement the Master data counter, skip to step 5 if not the last data byte.
            //  3. Write 0x0C to I2CONCLR to clear the SI flag and the AA bit.
            //  4. Exit
            //  5. Write 0x04 to I2CONSET to set the AA bit.
            //  6. Write 0x08 to I2CONCLR to clear the SI flag.
            //  7. Increment Master Receive buffer pointer
            //  8. Exit

        case 0x50:
            I2C0ReceiveData[I2C0DataCounter] = I2C0DAT;
            I2C0DataCounter++;
            //////////////////////////////////////////////////////////////
            //////////////  HACK ALERT ///////////////////////////////////
            //  // drop through to next case(0x50->0x48), no 'break' // // 
            //////////////////////////////////////////////////////////////

            //  Previous state was State 0x08 or State 0x10 or state 0x50.
        case 0x40:
            if(I2C0DataCounter < I2C0DataLength -1 ) {
                I2C0CONSET = 0x1 << AA;  // order important! this first then clear start
                I2C0CONCLR = 0x20;       //     not in documentation!
            } else {
                I2C0CONCLR = 0x20 | 0x1<<AA;   // not in documentation!
            }
            I2C0CONCLR = 0x1<<SI;
            break;

            //State 0x48 - Slave Address + Read has been transmitted, NOT ACK has been received. 
            //A Stop condition will be transmitted.
        case 0x48:
            SET_BIT(I2C0CONSET, STO);
            give_binsem = 1;
            SET_BIT(I2C0CONSET, AA);
            I2C0CONCLR = 0x1<<SI;
            break;
            //State: 0x58 - Data has been received, NOT ACK has been returned. Data will be read 
            //from I2DAT. A Stop condition will be transmitted.
        case 0x58:
            I2C0ReceiveData[I2C0DataCounter] = I2C0DAT;
            I2C0DataCounter++;
            SET_BIT(I2C0CONSET, STO);
            give_binsem = 1;
            SET_BIT(I2C0CONSET, AA);
            I2C0CONCLR = 0x1<<SI;
            break;
        default:
            // treat like state 0x0, bus error
            SET_BIT(I2C0CONSET, STO);
            give_binsem = 1;
            SET_BIT(I2C0CONSET, AA);
            I2C0CONCLR = 0x1<<SI;
            break;
    }

    VICVectAddr = 0x0;   // clear VIC address
    if(give_binsem == 1) {  // STOP Bit has been set
        xSemaphoreGiveFromISR( i2cSemaphore_g, &xHigherPriorityTaskWoken );
    }

  /* If xHigherPriorityTaskWoken was set to true you
    we should yield.  The actual macro used here is 
    port specific. */
//  Sat 27 March 2010 12:46:21 (PDT):Only needed if context switch in i2c_isr?
//     portYIELD_FROM_ISR( xHigherPriorityTaskWoken );

    portRESTORE_CONTEXT();
}


/*
 * i2c1_isr
 */
void i2c1_isr(void) {
    // not implemented
}

/*
 * i2c2_isr
 */
void i2c2_isr(void) {
    // not implemented
}


/*
 * I2C_master_xact
 * Input: Pointer to i2c_master_t structure with xaction data
 */
void I2C0_master_xact(i2c_master_t* s) {

    uint8_t i;

    if( i2cSemaphore_g != NULL ) { 
        // See if we can obtain the semaphore. If the semaphore is not available 
        // wait I2C_BINSEM_WAIT msecs to see if it becomes free. 
        if( xSemaphoreTake( i2cSemaphore_g, I2C_BINSEM_WAIT ) == pdTRUE ) { 
            for(i=0; i<I2C_MAX_BUFFER; i++){
               i2c0_s_g.I2Cbuffer[i] = s->I2Cbuffer[i];
            }
            i2c0_s_g.I2Cext_slave_address = s->I2Cext_slave_address;
            i2c0_s_g.write_length         = s->write_length;
            i2c0_s_g.read_length          = s->read_length;

            //write 0x20 to I2CONSET to set the STA bit
            SET_BIT(I2C0CONSET, STA);

        } else { 
            vSerialPutString(0, "*** I2C-ERROR ***: Timed out waiting for i2cSemaphore_g. Skipping Request.\n\r", 50);
        } 
    } else {
        vSerialPutString(0, "*** I2C-ERROR ***: i2cSemaphore_g is NULL in I2C0MasterTX. Did you run I2CInit?\n\r", 50);
    } 
} 


/*
 * I2CGeneral_Call
 * Issue a software reset using the general call address
 * Philips i2c user manual p19 UM10204.pdf section 3.14 "Software Reset"
 * 0x6 as second byte following general call (0x0)
 * *** IF ANY SLAVE/BUS DEVICE IS HOLDING DOWN (LOW) SCL OR SDA THEN THIS WONT WORK ***
 */
/*
void I2CGeneral_Call(i2c_iface channel) {
    { portENTER_CRITICAL();
        uint8_t num_bytes = 0x1;
        uint8_t myDataToSend[num_bytes];
        myDataToSend[0]     =   0x6;
        switch(channel) {
            case I2C0: 
                I2C0MasterTX(0x0, myDataToSend, num_bytes, 0); 
                break;

            case I2C1: 
                //               I2C1MasterTX(0x0, myDataToSend, num_bytes); 
                break;

            case I2C2: 
                //                I2C2MasterTX(0x0, myDataToSend, num_bytes); 
                break;
        }
        portEXIT_CRITICAL();
    }
}
*/


