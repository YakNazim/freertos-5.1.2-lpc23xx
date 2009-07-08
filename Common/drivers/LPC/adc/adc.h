/*
	ADC handling suite
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


//should put a conditional here to ensure enum only allows the correct set 
// of ports per processor type

#ifndef ADC_H
#define ADC_H

#include "FreeRTOS.h"

enum ADCPort {AD0_0, AD0_1, AD0_2, AD0_3, AD0_4, AD0_5, AD0_6, AD0_7};

void configureADCPort(enum ADCPort port );
portBASE_TYPE getChannel(enum ADCPort port);
portBASE_TYPE getPort(enum ADCPort port);

/* Checks if the uxADC_Channel on uxADC_Port is done with a conversion */
portBASE_TYPE uxADCDone(enum ADCPort port);

/* Returns the conversion result of the channel and port specified */
portLONG cADC_Result( enum ADCPort port );

#endif
