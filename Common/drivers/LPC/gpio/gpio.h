/*
	GPIO handling suite
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


#ifndef GPIO_H_
#define GPIO_H_

#include "FreeRTOS.h"

enum PortNumber {PORT0=0, PORT1=1, PORT2=2, PORT3=3, PORT4=4};

//this should go somewhere else, but I'm not sure where that is
//enum BOOL {FALSE=0, TRUE=1};

void initFGPIOPin(enum PortNumber , portLONG , enum BOOL );

#endif




