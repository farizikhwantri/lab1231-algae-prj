/* Include FreeRTOS */
#include "FreeRTOS_AVR.h"
#include <Arduino.h>
#include "basic_io_avr.h"
#include <SoftwareSerial.h>
#include <cozir.h>
#include <stdio.h>
#include <DS3231.h>
//#include <time.h>

SoftwareSerial sws(13,12);
COZIR czr(&sws);

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);
Time w;
#define t 1
#define f 0
#define ANALOG_SENSOR_PIN A0
#define DIGITAL_SENSOR_PIN 4

static int stateDigital;
static int lightAnalogValue;

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
static char *old_time = "";
static char *taimu = "";
TaskHandle_t temp, co2, hum, light, print, clock;

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


static void vLightReader ( void *pvParameter){
  for( ;; ){
    vTaskSuspendAll();
    {
      lightAnalogValue = analogRead(ANALOG_SENSOR_PIN);
    }
    xTaskResumeAll();
  }
}

static void vPrintTask ( void *pvParameter ){
  //Serial.print(F("Unused Stack Print: "));
  //Serial.println(uxTaskGetStackHighWaterMark(print));
  //Serial.print(F("Free Heap: "));
  //Serial.println(freeHeap());
  for( ;; ){
      vTaskSuspendAll();
    	{
        if(resCO2.ucStatus==t && resTemp.ucStatus==t && resHum.ucStatus==t){

            w =rtc.getTime();
            Serial.print(resCO2.ulCO2Val);
            Serial.print(F(","));
            Serial.print(resTemp.usVal);
            Serial.print(F(","));
            Serial.print(resHum.usVal);
            Serial.print(F(","));
            Serial.print(lightAnalogValue);
            Serial.print(F(","));
            Serial.print(rtc.getDateStr());
            Serial.print(F(","));
            Serial.print(rtc.getTimeStr());
            Serial.print(F(","));
            Serial.println(w.sec);

          //Serial.println(old_time);
          //Serial.println(taimu);
          //Serial.println("Keluar");
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
/*
static void vClockRTC ( void *pvParameter ){
  for( ;; ){
      vTaskSuspendAll();
    	{
        if(resCO2.ucStatus==t && resTemp.ucStatus==t && resHum.ucStatus==t){

        }
        else{
        }
    	}
    	xTaskResumeAll();
    }

}*/

void setup( void ){
  sws.begin(9600);
  Serial.begin(9600);
  rtc.begin();
  //czr.init();

  xTaskCreate( vTempReader, "TempReader", configMINIMAL_STACK_SIZE + 250, NULL, 1, &temp);
  xTaskCreate( vCO2Reader, "CO2Reader", 500, NULL, 1, &co2);
  xTaskCreate( vHumidityReader, "HumidityReader", configMINIMAL_STACK_SIZE + 100, NULL, 1, &hum);
  xTaskCreate( vLightReader, "LightReader", 400, NULL, 1, &light);
  xTaskCreate( vPrintTask, "PrintTask", 400, NULL, 1, &print);
  //xTaskCreate( vClockRTC, "PrintTask", 400, NULL, 1, &clock);

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
