/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes

//Declarations

void DelayminuteT0();
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );

    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
        
        GPIO_PA23_Set();
        GPIO_PA25_Set();
        GPIO_PA24_Clear();
        
        DelayminuteT0();
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


//1 minute delays used for sensors to gather readings every minute
void DelayminuteT0(){
    int seconds = 0;
    while(seconds < 60){
        TC0_TimerStart();
        while(!TC0_TimerPeriodHasExpired());
        seconds++;
    }
    return;
}

/*******************************************************************************
 End of File
*/

