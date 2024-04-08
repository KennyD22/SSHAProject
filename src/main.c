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

uint8_t resetbme = 0xB6;

//Declarations
signed long int t_fine; // global variable 

float temperature, humidity, pressure; //variables for each data. 
float temperatures[240];//array for all temps stored in an hour

uint8_t deviceID;

//Forces a sample of the BME280. Also sets oversampling for humidity, temp and press = 1.
//Consult the BME280 Datasheet to change these options. 
void BME280_init();

//Write a byte to a register via SPI 
void writeSPI(char, char);

// return a unsigned 16-bit value 
unsigned int readSPI16bit(char);

// return a unsigned 8-bit value
unsigned char readSPI8bit(char);
// returns a unsigned 16-bit (little endian) 
unsigned int readSPI16bit_u_LE(char);

// returns a unsigned 16-bit (little endian)
signed int readSPI16bit_s_LE(char); 

// get temperature and returns it in Celsius
float readTemp(); 

// gets RH humidity and returns it as a percentage
float readHumidity();

// gets pressure and returns it in kPa.
float readPressure(); 
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{ 
    char *formattedTemp;
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    deviceID = 0x00;
    
    CS0_Set();
    
    CS0_Clear();
    BME280_init(); // starts a new sample 
    CS0_Set();
//    while(true){
//        Damper1_Clear();
//    }
    
    while ( true ){
        CS0_Clear();
        DelaySec(2);
        writeSPI(reset_REG, resetbme); // reset the BME280
        CS0_Set();
        
        CS0_Clear();
        DelaySec(2);
        BME280_init(); // starts a new sample 
        CS0_Set();
        
        deviceID = 0x00;
        CS0_Clear();
        deviceID = readSPI8bit(BME280_REG_CHIP_ID); // reads device ID
        CS0_Set();
        printf("ID = %X\n",deviceID);
        
        CS0_Clear();
        DelaySec(2);
        temperature = readTemp(); // reads temp sample 
        CS0_Set();
        printf("Celsius Temp: %f\n",temperature);
        
        
        temperature = temperature*9/5 + 32; // convert to Fahrenheit
          
        printf("F temp: %f\n",temperature);
        
        CS0_Clear();
        humidity = readHumidity(); // reads temp sample 
        CS0_Set();
        DelaySec(10);
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}

unsigned char readSPI8bit(char reg){
    uint8_t regData;
    SERCOM3_SPI_Write(&reg,1);
    while(SERCOM3_SPI_IsBusy()){
        WDT_Clear();
    } 
    SERCOM3_SPI_Read(&regData,1);
    while(SERCOM3_SPI_IsBusy()){
        WDT_Clear();
    }
    return regData; 
}

unsigned int readSPI16bit(char reg){
    unsigned int val;
    val = readSPI8bit(reg); // shift in MSB
    val = val << 8 | readSPI8bit(reg+1); // shift in LSB
    return val;
}

signed int readSPI16bit_s(char reg){
return (signed int) readSPI16bit(reg);
}

unsigned int readSPI16bit_u_LE(char reg){ // read 16-bits unsigned little endian
    unsigned int val;
    val = readSPI16bit(reg); 
    return (val >> 8) | (val << 8); // swap upper and lower regs
}

signed int readSPI16bit_s_LE(char reg) { // read 16-bit signed little endian
    return (signed int) readSPI16bit_u_LE(reg);
}

void writeSPI(char reg, char data){
    reg = reg & 0x7F; //mask off first bit to specify a write. 
    
    SERCOM3_SPI_Write(&reg,1);
    while(SERCOM3_SPI_IsBusy()){
        WDT_Clear();
    } 
    SERCOM3_SPI_Write(&data,1);
    while(SERCOM3_SPI_IsBusy()){
        WDT_Clear();
    } 
    return;
}

void setReg(char reg, char data){
    reg = reg & 0x7F;
    uint8_t txregs[] = {reg, data};
    SERCOM3_SPI_Write(&txregs,2);
    while(SERCOM3_SPI_IsBusy()){
        WDT_Clear();
    } 
    return;
}

void BME280_init(){
    uint8_t setuparr[4] = {0x00, 0x01, 0x25};
    setReg(BME280_REG_CTRL_MEAS, setuparr[0]);
    setReg(BME280_REG_CTRL_HUMIDITY, setuparr[1]); // 1Forced, 3normal mode, Humidity oversampling = 1
    setReg(BME280_REG_CONFIG, setuparr[0]);//write to config register
    setReg(BME280_REG_CTRL_MEAS, setuparr[2]); // 25,B7Forced mode,27normal mode, Temp/Press oversampling = 1
}

float readTemp(void){
    // Calibration Coefficients:
    unsigned long int dig_T1 = readSPI16bit_u_LE(dig_T1_REG); 
    signed long int dig_T2 = readSPI16bit_s_LE(dig_T2_REG);
    signed long int dig_T3 = readSPI16bit_s_LE(dig_T3_REG);
    
    CS0_Set();
    DelaySec(2);
    CS0_Clear();
    DelaySec(2);
    
    // Temperature Raw ADC:
    unsigned long int adc_T = 0;
    adc_T = readSPI16bit_u_LE(TEMP_REG);
    adc_T <<= 8; // move in XLSB register
    adc_T |= readSPI8bit(TEMP_REG + 2);
    adc_T >>= 4; // Only uses top 4 bits of XLSB register 

    // From BME280 data sheet: 
    signed long int var1  = ((((adc_T>>3) - (dig_T1 <<1))) *
	   (dig_T2)) >> 11;
  
    signed long int var2  = (((((adc_T>>4) - (dig_T1)) *
	     ((adc_T>>4) - (dig_T1))) >> 12) *
	     (dig_T3)) >> 14;

    t_fine = var1 + var2;
 
    float T = (t_fine * 5 + 128) >> 8;
    return T/100;
}

float readHumidity(void){
    // Calibration Coefficients
    unsigned int dig_H1 = readSPI8bit(dig_H1_REG);
    signed long int dig_H2 = readSPI16bit_s_LE(dig_H2_REG);
    unsigned int dig_H3 = readSPI8bit(dig_H3_REG);
    signed long int dig_H4 = (readSPI8bit(dig_H4_REG)<<4)|(readSPI8bit(dig_H4_REG+1) & 0xF);
    signed long int dig_H5 = (readSPI8bit(dig_H5_REG+1)<<4)|(readSPI8bit(dig_H5_REG)>>4);
    signed int dig_H6 = (signed int) readSPI8bit(dig_H6_REG);
    
    //Humidity raw ADC
    unsigned long int adc_H = readSPI16bit(HUM_REG);
    
    //compensate
    unsigned long int v_x1_u32r;
    v_x1_u32r = t_fine - 76800;
    
    v_x1_u32r = (((((adc_H << 14) - ((dig_H4) << 20) - ((dig_H5) * v_x1_u32r))
            + (16384)) >> 15) * (((((((v_x1_u32r * (dig_H6)) >> 10) *
		    (((v_x1_u32r * (dig_H3)) >> 11) + (32768))) >> 10) +
            (2097152)) * (dig_H2) + 8192) >> 14));
    
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
			    (dig_H1)) >> 4));
    
    v_x1_u32r = (v_x1_u32r <0 )? 0: v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400)?419430400:v_x1_u32r;
    float humidity = (v_x1_u32r>>12);
    return humidity/1024.0;
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
