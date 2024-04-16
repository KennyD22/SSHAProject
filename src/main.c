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
//#include "bme280def.h"                  //bme280 defs
#include <stdio.h>
#include <xc.h>
#include <string.h>
#include <math.h>

//definitions
#define BME280_REG_CHIP_ID             0xD0
#define BME280_REG_RESET               0xE0
#define BME280_REG_STATUS              0xF3
#define BME280_REG_CTRL_MEAS           0xF4
#define BME280_REG_CTRL_HUMIDITY       0xF2
#define BME280_REG_CONFIG              0xF5


#define CTRL_HUM 0xF2  // BME280 humidity register settings
#define CONTROL 0xF4 //BME280 control register
#define TEMP_REG 0xFA // BME280 temperature reg
#define HUM_REG 0xFD // BME280 humidity reg
#define PRESS_REG 0xF7 // BME280 pressure reg
#define reset_REG 0xE0 // BME280 reset register, write 0xB6 to it
#define dig_T1_REG 0x88 // BME280 temp calibration coefficients...
#define dig_T2_REG 0x8A
#define dig_T3_REG 0x8C
#define dig_H1_REG 0xA1 // BME280 humidity calibration coefficients...
#define dig_H2_REG 0xE1
#define dig_H3_REG 0xE3
#define dig_H4_REG 0xE4
#define dig_H5_REG 0xE5
#define dig_H6_REG 0xE7
#define dig_P1_REG 0x8E // BME280 pressure calibration coefficients...
#define dig_P2_REG 0x90
#define dig_P3_REG 0x92
#define dig_P4_REG 0x94
#define dig_P5_REG 0x96
#define dig_P6_REG 0x98
#define dig_P7_REG 0x9A
#define dig_P8_REG 0x9C
#define dig_P9_REG 0x9E

//Declarations
signed long int t_fine; // global variable 

float temperature, humidity, pressure; //variables for each data. 
char temperatures[8];//array for room temps used in damper adjustment
char humidities[8];//array for room humidities
char pressures[8];//array for room pressures
float tempavgs[240];//temperatures for a whole hour to avg together and store
float humidavgs[240];//humidities for a whole hour to avg together and store
float pressureavgs[240];//pressures for a whole hour to avg together and store

// get temperature for each sensor from arduino
void readTemp(); 

// gets humidity for each sensor from arduino
void readHumidity();

// gets pressure for each sensor from arduino
void readPressure(); 

void convertfromascii();

//averages temperature for storing
void avgSensors();
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{ 
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    while ( true ){
        readTemp();
        readHumidity();
        readPressure();
        DelaySec(5);
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}

//read temperature of sensors
void readTemp(void){
    //send command to get temperatures
    SERCOM1_USART_Write("0",1);
    while(SERCOM1_USART_WriteIsBusy()){
    }
    SERCOM1_USART_Read(temperatures,8);
    
    return ;
}

//read humidity of sensors
void readHumidity(void){
    SERCOM1_USART_Write("1",1);
    while(SERCOM1_USART_WriteIsBusy()){
    }
    SERCOM1_USART_Read(humidities,8);
    return;
}


//read pressure of sensors
void readPressure(){
    SERCOM1_USART_Write("2",1);
    while(SERCOM1_USART_WriteIsBusy()){
    }
    SERCOM1_USART_Read(pressures,8);
    return;
}

//convert from ascii to integers
void convertfromascii(){
    temperature = atoi(temperatures);
    humidity = atoi(humidities);
    pressure = atoi(pressures);
    return;
}

// second delay user chooses how many seconds
void DelaySec(uint8_t time){
    int seconds = 0;
    while(seconds < time){
        TC0_TimerStart();
        while(!TC0_TimerPeriodHasExpired());
        WDT_Clear();
        seconds++;
    }
    return;
}



//1 minute delays used for sensors to gather readings every minute
void DelayminuteT0(){
    int seconds = 0;
    while(seconds <60){
        TC0_TimerStart();
        while(!TC0_TimerPeriodHasExpired()){
            WDT_Clear();
        };
        WDT_Clear();
        seconds++;
    }
    return;
}

//main time function to call function on designated time frames
void onTimeT0(){
    while(true){
        int seconds = 0;
        int minutes = 0;
        while(minutes < 60){
            while(seconds < 60){
                TC0_TimerStart();
                while(!TC0_TimerPeriodHasExpired());
                seconds++;
            }
            minutes++;
            seconds = 0;
            readTemp();
            if(minutes%5 == 0){
                adjustDampers();
            }
        }
        avgSensors();
    }
    return;
}



//open or close damper based on most recent temperature in the room
void adjustDampers(){
    //check room 1
    if((temperatures[0] || temperatures[1]) > 85){
        
    }else{
        
    }
    //check room 2
    if(temperatures[2] > 85){
        
    }else{
        
    }
    //check room 3
    if(temperatures[3] > 85){
        
    }else{
        
    }
    return;
}

//averages sensor readings for storage
void avgSensors(){
    float avg;
    for(int i = 0; i < 240; i++){
        avg += temperatures[i];
    }
    avg = avg/240;
    
    //empty temp array to make ready for new temperatures
    for(int i = 0; i < 240; i++){
        temperatures[i] = 0;
    }
    return;
}


/*******************************************************************************
 End of File
*/
