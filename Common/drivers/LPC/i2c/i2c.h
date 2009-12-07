
/*
	i2c handling suite
	Jeremy Booth
	Portland State Aeronautical Society
	http://psas.pdx.edu

	
	FreeRTOS is free software; you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 2 of the License, or
        (at your option) any later version.

        FreeRTOS is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with FreeRTOS; if not, write to the Free Software
        Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

        A special exception to the GPL can be applied should you wish to distribute
        a combined work that includes FreeRTOS, without being obliged to provide
        the source code for any proprietary components.  See the licensing section
        of http://www.FreeRTOS.org for full details of how and when the exception
        can be applied.

        ***************************************************************************
        See http://www.FreeRTOS.org for documentation, latest information, license
        and contact details.  Please ensure to read the configuration and relevant
        port sections of the online documentation.
        ***************************************************************************
*/

#ifndef _I2C_H
#define _I2C_H

// MAX values
#define I2C_MAX_BUFFER   16

// VIC table page 94 lpc23xx manual
#define VICI2C0EN        9
#define VICI2C1EN        19
#define VICI2C2EN        30

    // I2CnCONSET
#define AA               2
#define SI               3
#define STO              4
#define STA              5
#define I2EN             6

    // PCONP
#define PCI2C0           7
#define PCI2C1           19
#define PCI2C2           26

    // I2C clock
    // Table 435 p496 lpc23xx
#define I2SCLHIGH        200
#define I2SCLLOW         200

    // PINSEL0
#define SDA2             (0x2<<20)
#define SDA2MASK         ~(0x3<<20)

#define SCL2             0x2<<20
#define SCL2MASK         ~(0x3<<22)

#define PULLUP           0x0

    // PINSEL1
#define SDA1             (0x3<<6)
#define SDA1MASK         ~(0x3<<6)

#define SCL1             0x3<<8
#define SCL1MASK         ~(0x3<<8)

#define SDA0             0x1<<22
#define SDA0MASK         ~(0x3<<22)

#define SCL0             0x1<<24
#define SCL0MASK         ~(0x3<<24)

// Control
#define READMASK         (0x1<<8) 
// Thu 12 November 2009 10:44:04 (PST)
// #define WRITEMASK        ~(0x1<<8)
#define WRITEMASK ~(0x1)

// I2C bus control characters
#define SEND_I2C_STOPSTART	0x3000
#define SEND_I2C_REPEATEDSTART	0x2000
#define SEND_I2C_STOP		0x1000


typedef enum { I2C0=0, I2C1, I2C2} i2c_iface;

/*
void i2c0_isr(void) __attribute__ ((interrupt("IRQ")));
void i2c1_isr(void) __attribute__ ((interrupt("IRQ")));
void i2c2_isr(void) __attribute__ ((interrupt("IRQ")));
*/

void i2c0_isr(void) __attribute__ ((naked));
void i2c1_isr(void) __attribute__ ((naked));
void i2c2_isr(void) __attribute__ ((naked));

void i2cinit(i2c_iface channel) ;
void I2C0MasterRX(int deviceAddr, int *myDataToSend, int dataLength) ;
void I2C0MasterTX(int deviceAddr, uint32_t *myDataToSend, int dataLength) ;

#endif

