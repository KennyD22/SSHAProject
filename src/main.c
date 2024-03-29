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

//definitions
#define BME280_REG_CHIP_ID             0xD0
#define BME280_REG_RESET               0xE0
#define BME280_REG_STATUS              0xF3
#define BME280_REG_CTRL_MEAS           0xF4
#define BME280_REG_CTRL_HUMIDITY       0xF2
#define BME280_REG_CONFIG              0xF5
#define BME280_REG_PRESSURE_MSB        0xF7
#define BME280_REG_PRESSURE_LSB        0xF8
#define BME280_REG_PRESSURE_XLSB       0xF9
#define BME280_REG_TEMPERATURE_MSB     0xFA
#define BME280_REG_TEMPERATURE_LSB     0xFB
#define BME280_REG_TEMPERATURE_XLSB    0xFC
#define BME280_REG_HUMIDITY_MSB        0xFD
#define BME280_REG_HUMIDITY_LSB        0xFE



//Declarations
//checker for SPI fail
bool spierr;
//stores data from one sensor
uint8_t sensordata[] = "";
//rxbuff
uint8_t rxbuff[1] = {0x00};
//address for Sensor ID
uint8_t txID[1] = {0xD0};
//address for temp setup
uint8_t tx_tempconfig[1] = {0xF4};
//holds all sensor IDs
uint8_t sensorID[12];
//stores temperature for one sensor
uint8_t tempuratures[3] = {0xD0, 0x3C, 0x4B};
//stores temperature averages based on room
int32_t roomTemps[4];
//stores averages of temperature for storage
int32_t tempAvgs[];
//stores humidity for one sensor
int32_t humidities[];
//pressure is 64 bit so it will take two spots instead of just 1
int32_t pressures[];

uint8_t tempReg[1] = {0xFA};

uint8_t regcaltempT1[2];
uint8_t regcaltempT2[2];
uint8_t regcaltempT3[2];

uint16_t dig_T1;
int16_t dig_T2;
int16_t dig_T3; 

int32_t t_fine;
int32_t tempActual;
int32_t adc_t;

uint8_t rawhex[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
uint8_t ascii[] = {0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x41,0x42,0x43,0x44,0x45,0x46};

char tempTextArray[];

int32_t tempCompensate(int32_t adc_T);

bool setupSensors(uint8_t reg, uint8_t value);
//wait one minute
void DelayminuteT0();
//wait 5 seconds
void Delay5Sec();
//calls our reads and averages at correct time intervals
//as well as adjusting dampers and transfer fans every 5 minutes
//based on sensor readings
void onTimeT0();
//reads sensor data
void readTemperatures();
//averages room temperature
void roomTempAvg();
//averages sensor data and stores it
void avgSensors();
//adjust dampers based on sensor data
void adjustDampers();

void readSensorID();
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    CS0_Set();
    
    bool sensinit;

    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
       // WDT_Enable();
        WDT_Clear();
        readSensorID();
        checkrxbuff();
        
        sensinit = setupSensors(tx_tempconfig[0], 0x93);  //x8 oversampling temp
//        if(sensinit == false){
//            SERCOM1_USART_Write("temp failed setup\n", sizeof("temp failed setup\n"));
//            while(!SERCOM1_USART_TransmitComplete()){
//            WDT_Clear();}
//        }
//        
//        else{
//            SERCOM1_USART_Write("temp passed setup\n", sizeof("temp passed setup\n"));
//            while(!SERCOM1_USART_TransmitComplete()){
//            WDT_Clear();}
//        }
        
        CS0_Clear();
        readTemperatures();
        
        readTempCalib();
        CS0_Set();
  
        //SERCOM1_USART_Write("temps Read\n", sizeof("temps Read\n"));
        //while(!SERCOM1_USART_TransmitComplete()){
        //    WDT_Clear();}

        
//    temperatureForm = temperatureForm | tempuratures[0] << 16;
//    temperatureForm = temperatureForm | tempuratures[1] << 8;
//    temperatureForm = temperatureForm | tempuratures[2];
        
//    SERCOM1_USART_Write("temps formatted\n", sizeof("temps formatted\n"));
//    while(!SERCOM1_USART_TransmitComplete()){
//            WDT_Clear();}
    
   // SERCOM1_USART_Write(&ascii, 16);
   //     while(!SERCOM1_USART_TransmitComplete()){
   //         WDT_Clear();}
    
     uint32_t actualtemp = tempCompensate(adc_t);
    
    sprintf(tempTextArray,"%d", actualtemp); 
     
     SERCOM1_USART_Write(&tempTextArray, 10);
        while(!SERCOM1_USART_TransmitComplete()){
            WDT_Clear();}
    
        while(1){
        CS0_Clear();
        readSensorID();    
        checkrxbuff();
        readTemperatures();
        CS0_Set();
//        
//        CS0_Clear();
//        SERCOM3_SPI_Write(&tempuratures,3);
//        while(SERCOM3_SPI_IsBusy()){
//        WDT_Clear();
//        }
//        CS0_Set();
        
        
        
        }
        
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}

int32_t tempCompensate(int32_t adc_T){
    int32_t var1, var2, T; 
    
    var1 = ((((adc_T>>3)-(dig_T1<<1)))*(dig_T2))>>11; 
    var2 = (((((adc_T>>4)-(dig_T1))*((adc_T>>4)-(dig_T1)))>>12)*(dig_T3))>>14;
    
   t_fine = var1+ var2; 
    T = (t_fine*5+128)>>8;
            
    return T; 
}

void readTempCalib(){
     CS0_Clear();
    SERCOM3_SPI_Write(0x88,1);
    SERCOM3_SPI_Read(&regcaltempT1[0],1);
    while(SERCOM3_SPI_IsBusy()){
        WDT_Clear();
    }
    CS0_Set();
    
     CS0_Clear();
    SERCOM3_SPI_Write(0x89,1);
    SERCOM3_SPI_Read(&regcaltempT1[1],1);
    while(SERCOM3_SPI_IsBusy()){
        WDT_Clear();
    }
    CS0_Set();
    
     CS0_Clear();
    SERCOM3_SPI_Write(0x8A,1);
    SERCOM3_SPI_Read(&regcaltempT2[0],1);
    while(SERCOM3_SPI_IsBusy()){
        WDT_Clear();
    }
    CS0_Set();
    
     CS0_Clear();
    SERCOM3_SPI_Write(0x8B,1);
    SERCOM3_SPI_Read(&regcaltempT2[1],1);
    while(SERCOM3_SPI_IsBusy()){
        WDT_Clear();
    }
    CS0_Set();
    
     CS0_Clear();
    SERCOM3_SPI_Write(0x8C,1);
    SERCOM3_SPI_Read(&regcaltempT3[0],1);
    while(SERCOM3_SPI_IsBusy()){
        WDT_Clear();
    }
    CS0_Set();
    
     CS0_Clear();
    SERCOM3_SPI_Write(0x8D,1);
    SERCOM3_SPI_Read(&regcaltempT3[1],1);
    while(SERCOM3_SPI_IsBusy()){
        WDT_Clear();
    }
    CS0_Set();
    
    dig_T1 = regcaltempT1[0] & (regcaltempT1[1]<<8);
    dig_T2 = regcaltempT2[0] & (regcaltempT2[1]<<8);
    dig_T3 = regcaltempT3[0] & (regcaltempT3[1]<<8);
    hex2Ascii(regcaltempT1,2);
    hex2Ascii(regcaltempT2,2);
    hex2Ascii(regcaltempT3,2);
    
    return;
}

// 5 second delay
void Delay5Sec(){
    int seconds = 0;
    while(seconds <5){
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
    readTemperatures();
    if(minutes%5 == 0){
        adjustDampers();
    }
    }
    avgSensors();
    return;
}

//activates sensors and prepares them for what we need
// needs pTransmitData and txSize
bool setupSensors(uint8_t reg, uint8_t value){
    
    int err;
    
    //forces first bit to be 1 to allow writing
    uint8_t tx_values[]={ (reg & 0x7F),value};
    CS0_Clear();

    err = SERCOM3_SPI_Write(&tx_values,sizeof(tx_values));
    while(SERCOM3_SPI_IsBusy()){
        WDT_Clear();
    }
    
    CS0_Set();   
    
    if(err < 0){
        return false;
    }else{
        return true;
    }
}

//reads sensor IDs then send correct or other message to UART
void readSensorID(){
    CS0_Clear();
    SERCOM3_SPI_Write(&txID[0],1);
    SERCOM3_SPI_Read(&rxbuff[0],1);
    while(SERCOM3_SPI_IsBusy()){
        WDT_Clear();
    }
    CS0_Set();
    
    return;
    
}

//check rxbuff for sensor id
void checkrxbuff(){
    if(rxbuff[0]== 0x60){
        SERCOM1_USART_Write("SensorID is correct\n", sizeof("SensorID is correct\n"));
        while(!SERCOM1_USART_TransmitComplete()){
            WDT_Clear();
        }
    }
    else{
        SERCOM1_USART_Write("SensorID is wrong\n", sizeof("SensorID is wrong\n"));
        while(!SERCOM1_USART_TransmitComplete()){
            WDT_Clear();
        }
    }
}

//reads one sensor's data
void readTemperatures(){
    CS0_Clear();
    SERCOM3_SPI_Write(&tempReg[0],1);
    SERCOM3_SPI_Read(&tempuratures,3);
    while(SERCOM3_SPI_IsBusy()){
        WDT_Clear();
    }
    CS0_Set();
    hex2Ascii(tempuratures,3);
    return;
}

//open or close damper based on most recent temperature in the room
void adjustDampers(){
    return;
}

//averages sensor readings for storage
void avgSensors(){
    return;
}

void hex2Ascii(uint8_t uncomphex[], int sizeofarray){
    for(int j = 0; j < sizeofarray; j++){
        uint8_t temp1 = ((uncomphex[j] & 0xF0)>>4);
        uint8_t temp2 = (uncomphex[j] & 0x0F);
        for(int i = 0; i < 16; i++){
            WDT_Clear();
            if(temp1 == rawhex[i]){
                SERCOM1_USART_Write(ascii[i], sizeof(ascii[i]));
                while(!SERCOM1_USART_TransmitComplete()){
                    WDT_Clear();
                }
                break;
            }
        }
        for(int i = 0; i < 16; i++){
            WDT_Clear();
            if(temp2 == rawhex[i]){
                SERCOM1_USART_Write(ascii[i], sizeof(ascii[i]));
                while(!SERCOM1_USART_TransmitComplete()){
                    WDT_Clear();
                }
                break;
            }
        }
    }
    SERCOM1_USART_Write("\n", sizeof("\n"));
        while(!SERCOM1_USART_TransmitComplete()){
            WDT_Clear();
        }
    return;
}

/*******************************************************************************
 End of File
*/
