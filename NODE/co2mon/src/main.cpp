
/* Include FreeRTOS */
#include "FreeRTOS_AVR.h"
#include <Arduino.h>
#include "basic_io_avr.h"
#include <SoftwareSerial.h>
#include <cozir.h>
#include <stdio.h>
//#include <time.h>

SoftwareSerial sws(13,12);
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

static void vTempReader ( void *pvParameter){
  //Serial.print(F("Unused Stack Temp: "));
  //Serial.println(uxTaskGetStackHighWaterMark(temp));
    for( ;; ){
      vTaskSuspendAll();
      {
        if(resTemp.ucStatus==f){
          float cel = czr.Celsius();
          resTemp.usVal = cel;
          resTemp.ucStatus = t;
        }
      }
      xTaskResumeAll();
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
      if(resCO2.ucStatus==f){
        unsigned long co2_v = czr.CO2();
        resCO2.ulCO2Val = co2_v;
        resCO2.ucStatus = t;
      }
    }
    xTaskResumeAll();
  }
}


static void vHumidityReader ( void *pvParameter){
  //Serial.print(F("Unused Stack Hum: "));
  //Serial.println(uxTaskGetStackHighWaterMark(hum));
  for( ;; ){
    vTaskSuspendAll();
    {
      if(resHum.ucStatus==f){
        float cel = czr.Humidity();
        resHum.usVal = cel;
        resHum.ucStatus = t;
      }
    }
    xTaskResumeAll();
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
        if(resCO2.ucStatus==t && resTemp.ucStatus==t && resHum.ucStatus==t){
          /*
          Serial.print(F("Temp: "));
          Serial.print(resTemp.usVal);
          Serial.print(F(" "));
          Serial.print(F("CO2: "));
          Serial.print(resCO2.ulCO2Val);
          Serial.print(F(" "));
          Serial.print("Hum: ");
          Serial.println(resHum.usVal);
          */

          Serial.print(resCO2.ulCO2Val);
          Serial.print(F(","));
          Serial.print(resTemp.usVal);
          Serial.print(F(","));
          Serial.println(resHum.usVal);

          resTemp.ucStatus = f;
          resCO2.ucStatus = f;
          resHum.ucStatus = f;
        }
        else{
        }
    	}
    	xTaskResumeAll();
    }

}

void setup( void ){
  sws.begin(9600);
  Serial.begin(9600);
  //czr.init();

  xTaskCreate( vTempReader, "TempReader", configMINIMAL_STACK_SIZE + 250, NULL, 1, &temp);
  xTaskCreate( vCO2Reader, "CO2Reader", 500, NULL, 1, &co2);
  xTaskCreate( vHumidityReader, "HumidityReader", configMINIMAL_STACK_SIZE + 100, NULL, 1, &hum);
  //xTaskCreate( vLightReader, "LightReader", 200, NULL, 2, &light);
  xTaskCreate( vPrintTask, "PrintTask", 400, NULL, 1, &print);

  /*
  * UNCOMMENT TO VIEW AMOUNT OF FREE HEAP LEFT
  Serial.print(F("Free Heap: "));
  Serial.print(freeHeap());
  */
  vTaskStartScheduler();
  //Serial.print(czr.ReadAutoCalibration());

  //czr.CalibrateFreshAir();
  //czr.CalibrateKnownGas(1416);
  for ( ;; );

}
//------------------------------------------------------------------------------
void loop() {}
