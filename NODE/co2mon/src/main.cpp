
/* Include FreeRTOS */
#include "FreeRTOS_AVR.h"
#include <Arduino.h>
#include "basic_io_avr.h"
#include <SoftwareSerial.h>
#include <cozir.h>

SoftwareSerial sws(11,10);
COZIR czr(&sws);

#define t 1
#define f 0

typedef struct {
  float usVal;
  unsigned char ucStatus;
} resultF;

typedef struct {
  unsigned long ulCO2Val;
  unsigned char ucStatus;
} resultCO2;

static volatile resultF resTemp = {0.0, f};
static volatile resultCO2 resCO2 = {0, f};
static volatile resultF resHum = {0.0, f};
//static volatile result resTemp = {0, f};

TaskHandle_t temp, co2, hum, light, print;

/* Declare a variable of type SemaphoreHandle_t.  This is used to reference the
mutex type semaphore that is used to ensure mutual exclusive access to stdout. */
SemaphoreHandle_t xMutex;

static void vTempReader ( void *pvParameter){
  //Serial.print(F("Unused Stack Temp: "));
  //Serial.println(uxTaskGetStackHighWaterMark(temp));
    for( ;; ){
      vTaskSuspendAll();
      {
        //Serial.println(F("Mutex diambil Temp"));
        if(resTemp.ucStatus==f){
          float cel = czr.Celsius();
          resTemp.usVal = cel;
          resTemp.ucStatus = t;
        }
      }
      xTaskResumeAll();
      //Serial.println(F("Mutex diepas Temp"));
    }
}

static void vCO2Reader ( void *pvParameter){
  //Serial.print(F("Unused Stack CO2: "));
  //Serial.println(uxTaskGetStackHighWaterMark(co2));
  //Serial.print(F("Free Heap: "));
  //Serial.println(freeHeap());
  for( ;; ){
    vTaskSuspendAll();
    {
      //Serial.println(F("Mutex diambil CO2"));
      if(resCO2.ucStatus==f){
        unsigned long co2_v = czr.CO2();
        resCO2.ulCO2Val = co2_v;
        resCO2.ucStatus = t;
      }
    }
    xTaskResumeAll();
    //Serial.println(F("Mutex dilepas CO2"));
  }
}


static void vHumidityReader ( void *pvParameter){
  //Serial.print(F("Unused Stack Hum: "));
  //Serial.println(uxTaskGetStackHighWaterMark(hum));
  for( ;; ){
    vTaskSuspendAll();
    {
    //  Serial.println(F("Mutex diambil Humidity"));
      if(resHum.ucStatus==f){
        float cel = czr.Humidity();
        resHum.usVal = cel;
        resHum.ucStatus = t;
      }
    }
    xTaskResumeAll();
  //  Serial.println(F("Mutex dilepas Humidity"));
  }
}

/*
static void vLightReader ( void *pvParameter){

}*/

static void vPrintTask ( void *pvParameter ){
  //Serial.print(F("Unused Stack Print: "));
  //Serial.println(uxTaskGetStackHighWaterMark(print));
  //Serial.print(F("Free Heap: "));
  //Serial.println(freeHeap());
  for( ;; ){
      vTaskSuspendAll();
    	{
      //  Serial.println(F("Mutex diambil Print"));
        if(resCO2.ucStatus==t && resTemp.ucStatus==t && resHum.ucStatus==t){
          Serial.print(F("Temp: "));
          Serial.print(resTemp.usVal);
          Serial.print(F(" "));
          Serial.print(F("CO2: "));
          Serial.print(resCO2.ulCO2Val);
          Serial.print(F(" "));
          Serial.print("Hum: ");
          Serial.println(resHum.usVal);
          resTemp.ucStatus = f;
          resCO2.ucStatus = f;
          resHum.ucStatus = f;
        }
        else{
        //  Serial.print(F("Print belum bisa."));
        }
    	}
    	xTaskResumeAll();
    //  Serial.println(F("Mutex dilepas Print"));
    }

}

void setup( void ){
  sws.begin(9600);
  Serial.begin(9600);

  /* Before a semaphore is used it must be explicitly created.  In this example
  a mutex type semaphore is created. */
  xMutex = xSemaphoreCreateMutex();

  if( xMutex != NULL )
  {
    xTaskCreate( vTempReader, "TempReader", configMINIMAL_STACK_SIZE + 250, NULL, 1, &temp);
    xTaskCreate( vCO2Reader, "CO2Reader", 500, NULL, 1, &co2);
    xTaskCreate( vHumidityReader, "HumidityReader", configMINIMAL_STACK_SIZE + 100, NULL, 1, &hum);
    //xTaskCreate( vLightReader, "LightReader", 200, NULL, 2, &light);
    xTaskCreate( vPrintTask, "PrintTask", 400, NULL, 1, &print);
    //Serial.print(F("Free Heap: "));
    vTaskStartScheduler();
  }

  for ( ;; );

}
//------------------------------------------------------------------------------
void loop() {}
