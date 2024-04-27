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
#include <limits.h>

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
char temperatures[8];//array for room temps read in
int temps[4]; //holds most recent temperatures
char humidities[8];//array for room humidities read in
int humids[4]; //holds most recent humidities read
char pressures[12];//array for room pressures read in
int pressured[4]; //holds most recent pressures read
int tempavgs[240];//temperatures for a whole hour to avg together and store
int humidavgs[240];//humidities for a whole hour to avg together and store
int pressureavgs[240];//pressures for a whole hour to avg together and store

// get temperature for each sensor from Arduino
void readTemp(); 

// gets humidity for each sensor from Arduino
void readHumidity();

// gets pressure for each sensor from Arduino
void readPressure(); 

//average temperatures for storage
void avgSensors();

//avgerage humidities for storage
int avgHumid();

//convert read in chars to integers
void asciiToInt();

//create a queue
struct Queue* createQueue(int capacity);

//check if queue is full
int isFull(struct Queue* queue);

//check if queue is empty
int isEmpty(struct Queue* queue);

//add item to end of queue
void enqueue(struct Queue* queue, int item);

//remove item from front of queue
void dequeue(struct Queue* queue);

//get item in front of queue
int front(struct Queue* queue);

//get item at end of queue
int rear(struct Queue* queue);

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{ 
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    struct Queue* tempavgdata = createQueue(2975);
    struct Queue* humidityavgdata = createQueue(2975);
    struct Queue* pressureavgdata = createQueue(2975);
    
    
    
    while ( true ){
//        for(int i = 0; i < 4; i++){
//        temperatures[i] = 0;
//        }
//        for(int i = 0; i < 4; i++){
//            humidities[i] = 0;
//        }
//        for(int i = 0; i < 6; i++){
//            pressures[i] = 0;
//        }
//        readTemp();
//        //readHumidity();
//        //readPressure();
//        asciiToInt();
//        DelaySec(5);
        
        //for pssc testing
        int seconds = 0;
        int minutes = 0;
        while(minutes < 5){
            while(seconds <60){
                TC0_TimerStart();
                while(!TC0_TimerPeriodHasExpired());
                seconds++;
            }
            for(int i = 0; i < 4; i++){
            temperatures[i] = 0;
            }
            for(int i = 0; i < 4; i++){
                humidities[i] = 0;
            }
            for(int i = 0; i < 6; i++){
                pressures[i] = 0;
            }
            minutes++;
            seconds = 0;
            readTemp();
            asciiToInt();
            adjustDampersPSSC();
        }
        
        int avg = 0;
        avg = avgTemp();
        if(isFull(tempavgdata)){
            dequeue(tempavgdata);
            enqueue(tempavgdata, avg);
        }else{
            enqueue(tempavgdata, avg);
        }
        minutes = 0;
        
//        while(minutes < 60){
//            while(seconds < 60){
//                TC0_TimerStart();
//                while(!TC0_TimerPeriodHasExpired());
//                seconds++;
//            }
//            minutes++;
//            seconds = 0;
//            readTemp();
//            if(minutes%5 == 0){
//                adjustDampers();
//            }
//        }
//        int avg = 0;
//        avg = avgTemp();
//        if(isFull(tempavgdata)){
//            dequeue(tempavgdata);
//            enqueue(tempavgdata, avg);
//        }else{
//            enqueue(tempavgdata, avg);
//        }
//        avg = avgHumid();
//        enqueue(humidityavgdata, avg);
//        avg = avgPress();
//        enqueue(pressureavgdata, avg);
//        minutes = 0;
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}

//read temperature of sensors
void readTemp(void){
    //send command to get temperatures
    for(int i = 0; i < 4; i++){
        temperatures[i] = 0;
    }
    SERCOM1_USART_Write("0",1);
    while(SERCOM1_USART_WriteIsBusy()){
    }
    SERCOM1_USART_ReceiverEnable();
    SERCOM1_USART_Read(&temperatures,8);
    DelaySec(2);
    
    return ;
}

void asciiToInt(){
    int temp;
//    temp = (int)(temperatures[0])+0;
//    temp = temp - 0x30;
//    temp = temp*10;
//    temp += ((int)(temperatures[1])+0) - 0x30;
//    temps[0] = temp;
//    
//    temp = (int)(temperatures[2])+0;
//    temp = temp - 0x30;
//    temp = temp*10;
//    temp += ((int)(temperatures[3])+0) - 0x30;
//    temps[1] = temp;
//    
//    temp = (int)(temperatures[4])+0;
//    temp = temp - 0x30;
//    temp = temp*10;
//    temp += ((int)(temperatures[5])+0) - 0x30;
//    temps[2] = temp;
//    
//    temp = (int)(temperatures[6])+0;
//    temp = temp - 0x30;
//    temp = temp*10;
//    temp += ((int)(temperatures[7])+0) - 0x30;
//    temps[3] = temp;
    
    for(int i = 0; i < 8; i = i+2){
    temp = (int)(temperatures[i])+0;
    temp = temp - 0x30;
    temp = temp*10;
    temp += ((int)(temperatures[i+1])+0) - 0x30;
    if(i == 0)temps[0] = temp;
    if(i == 2)temps[1] = temp;
    if(i == 4)temps[2] = temp;
    if(i == 6)temps[3] = temp;
    }
    
}

//read humidity of sensors
void readHumidity(void){
    int humid;
    for(int i = 0; i < 8; i = i+2){
    humid = (int)(temperatures[i])+0;
    humid = humid - 0x30;
    humid = humid*10;
    humid += ((int)(temperatures[i+1])+0) - 0x30;
    if(i == 0)temps[0] = humid;
    if(i == 2)temps[1] = humid;
    if(i == 4)temps[2] = humid;
    if(i == 6)temps[3] = humid;
    }
    return;
}


//read pressure of sensors
void readPressure(){
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


//open or close damper based on most recent temperature in the room
void adjustDampersPSSC(){
    //check room 1
    if(((temps[0] - temps[1]) > 15)||((temps[1] - temps[0]) > 15)){
        Damper1_Set();
    }else{
        Damper1_Clear();
    }
    return;
}

//open or close damper based on most recent temperature in the room
void adjustDampers(){
    //check room 1
    if((temps[0] || temps[1]) > 70){
        
    }else{
        
    }
    //check room 2
    if(temps[2] > 70){
        
    }else{
        
    }
    //check room 3
    if(temps[3] > 70){
        
    }else{
        
    }
    return;
}

//averages sensor readings for storage
int avgTemp(){
    float avg;
    for(int i = 0; i < 240; i++){
        avg += tempavgs[i];
    }
    avg = avg/240;
    
    //empty temp array to make ready for new temperatures
    for(int i = 0; i < 240; i++){
        tempavgs[i] = 0;
    }
    return avg;
}
int avgHumid(){
    float avg;
    for(int i = 0; i < 240; i++){
        avg += humidavgs[i];
    }
    avg = avg/240;
    
    //empty temp array to make ready for new temperatures
    for(int i = 0; i < 240; i++){
        humidavgs[i] = 0;
    }
    return avg;
}
int avgPress(){
    float avg;
    for(int i = 0; i < 240; i++){
        avg += pressureavgs[i];
    }
    avg = avg/240;
    
    //empty temp array to make ready for new temperatures
    for(int i = 0; i < 240; i++){
        pressureavgs[i] = 0;
    }
    return avg;
}

//structure to represent a queue
struct Queue{
    int front, rear, size;
    int capacity;
    int* array;
};

//creates queue
struct Queue* createQueue(int capacity){
    struct Queue* queue = (struct Queue*)malloc(
            sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    
    queue->rear = capacity - 1;
    queue->array = (int*)malloc(
            queue->capacity* sizeof(int));
    return queue;
}

//queue is full when size = capacity
int isFull(struct Queue* queue){
    return (queue->size == queue->capacity);
}

//queue is empty when size = 0
int isEmpty(struct Queue* queue){
    return (queue->size == 0);
}

//add item to end of queue
void enqueue(struct Queue* queue, int item){
    if(isFull(queue))return;
    queue->rear = (queue->rear+1) % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    return;
}

//remove item from front of queue
void dequeue(struct Queue* queue){
    if(isEmpty(queue)) return;
    int item = queue->array[queue->front];
    queue->front = (queue->front+1) % queue->capacity;
    queue->size = queue->size - 1;
}

//get front of queue
int front(struct Queue* queue){
    if(isEmpty(queue))return INT_MIN;
    return queue->array[queue->front];
}

//get rear of queue
int rear(struct Queue* queue){
    if(isEmpty(queue))return INT_MIN;
    return queue->array[queue->rear];
}

/*******************************************************************************
 End of File
*/
