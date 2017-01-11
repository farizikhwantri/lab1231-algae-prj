
/* Include FreeRTOS */
#include "FreeRTOS_AVR.h"
#include <Arduino.h>
#include "basic_io_avr.h"
#include <SoftwareSerial.h>
#include <cozir.h>
#include <stdio.h>
// #include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>
#include "printf.h"
#include <utility/task.h>
#include <AodvNetwork.h>
#include <MirfHardwareSpiDriver.h>
#include <Mirf.h>

//#include <time.h>

// objek device
SoftwareSerial sws(13,12);
COZIR czr(&sws);
RF24 radio(9, 10);

// COZIR init
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

// dummy sink addr
#define ADDR_SINK 0x7fff

// <p> paket radio
# define PANJANG_PAKET 184/8

// role dari node ini
// TODO: masukkan ke file .h
typedef enum { NODE_SENSOR, NODE_SINK } node_t;

/* deskripsi paket */
// TODO: masukkan ke file .h
typedef struct
{
  uint8_t   header;
  uint64_t   timestamp;
  uint16_t  addr_s;
  uint16_t  addr_t;
  // payload, data sensor
  // warnung: konversi float* ke 4 x uint8*
  float  ts_c;
  float  co2;
  float  hu;
  // data yang dikirimkan dari light sensor adalah data mentah 16 bit
  uint16_t  li;
  // ~payload, data sensor
  // crc 16 bit, data dihitung berdasarkan timestamp dan payload
} paket;

typedef paket SensorValuePayload;

/**
Konversi isi paket ke array. Dibutuhkan saat paket akan dikirim
Array yang dihasilkan berpola big endian
@param paket pc yang akan dikonversi ke array
@param array arr yang akan diisi dari paket
@return void
*/
void toarray(paket* pc, uint8_t paket_yang_dibalikin[PANJANG_PAKET])
{
  // the huehuehue of C/C++: point everything directly!
  // cast pointer untuk menunjuk paket this
  uint8_t* ptrc = (uint8_t*) pc;
  uint8_t ctr = PANJANG_PAKET;
  // lalu copy isi paket yang ditunjuk pointer ke array
  while (ctr)
  {
    paket_yang_dibalikin[--ctr] = *(ptrc + ctr);
  }
}

/**
Parse array yang diterima dari radio jadi paket ke this
Array yang diterima sesuai urutan paket dalam pola big endian
@param array yang akan dijadikan paket
@param paket hasil keluaran
@return void
*/
void parse(uint8_t paket_diterima[PANJANG_PAKET], paket* pc)
{
  // cast pointer untuk menunjuk paket
  uint8_t* ptrc = (uint8_t*) pc;
  uint8_t ctr = PANJANG_PAKET;
  // lalu copy isi array yang ditunjuk pointer ke paket
  while (ctr)
  {
    *(ptrc + --ctr) = paket_diterima[ctr];
  }
}
// </p> paket radio

// macro debug
#define cetakpaket(paket)\
char* z = (char*)paket; while(z < (char*)paket + PANJANG_PAKET) printf("%c", *(z++)); printf("\n\r");

/**
fungsi untuk mengirim paket via radio
@param paket* alamat ke paket
@return ok bernilai true jika pengiriman berhasil, false jika sebaliknya
*/
bool kirim_paket(paket* paket_dikirim)
{
  uint8_t yang_dikirim[PANJANG_PAKET];
  toarray(paket_dikirim, yang_dikirim);
  printf("ngirim paket gan "); cetakpaket(paket_dikirim);
  radio.stopListening();
  bool ok = radio.write(yang_dikirim, PANJANG_PAKET);
  radio.startListening();
  return ok;
}

/**
fungsi untuk random delay, bersifat blocking
*/
void delay_rand()
{
  char x = random(100); // delay antara 0 - 100 ms
  // delay(x);
}

// paket dummy
paket pckt;

// Address dari masing-masing pipa
const uint64_t pipes[] = { 0xAAAAAAAAE1LL, 0xAAAAAAAAD2LL, 0xAAAAAAD4LL, 0xAAAAAAAAD6LL, 0xAAAAAAAAD8LL };

// variabel buffer, untuk nampung paket yang masuk ketika akan ngirim
uint8_t buf[PANJANG_PAKET];

// variabel tipe node ini
node_t node_type;

// address node ini
// addr node 1 = 0xfa01 -> mega, node 2 = 0xfa03 -> uno, node sink = ADDR_SINK -> mega ADK, hard code manual ya huehuehue
uint16_t addr_saya;

// handle untuk task RTOS
TaskHandle_t task_route, task_svc;

// mutex-mutex
SemaphoreHandle_t sem_pckt;

BroadcastNetwork network(0);
static void vTaskRoute(void* pvParam)
{
  while (1)
  {
    AodvPacket pckt;
    // vTaskSuspendAll();

    // apa ada paket yang mau masuk?
    // vTaskSuspendAll();
    if(network.getPacket(pckt))
    {
      SensorValuePayload payload;
      pckt.readPayload((byte*)&payload, sizeof(SensorValuePayload));
    }
    // xTaskResumeAll();
  }
}

static void vTaskSvc(void* param)
{ while (1) {
  // task untuk menjalankan service (lapor ke server, monitor board,
  // execute command, dll)
  printf("millis: %ld\n", millis());
  vTaskDelay(100);
}
}

TaskHandle_t route, svc;

void setup( void ){
  sws.begin(9600);
  Serial.begin(9600);
  printf_begin();
  radio.begin();
  //czr.init();
  node_type = NODE_SINK;

  xTaskCreate( vTempReader, "TempReader", configMINIMAL_STACK_SIZE + 250, NULL, 1, &temp);
  xTaskCreate( vCO2Reader, "CO2Reader", 500, NULL, 1, &co2);
  xTaskCreate( vHumidityReader, "HumidityReader", configMINIMAL_STACK_SIZE + 100, NULL, 1, &hum);
  //xTaskCreate( vLightReader, "LightReader", 200, NULL, 2, &light);
  xTaskCreate( vPrintTask, "PrintTask", 400, NULL, 1, &print);
  xTaskCreate( vTaskRoute, "RoutingTask", 400, NULL, 1, &route);
  xTaskCreate( vTaskSvc, "ServiceTask", configMINIMAL_STACK_SIZE, NULL, 1, &svc);

  /*
  * UNCOMMENT TO VIEW AMOUNT OF FREE HEAP LEFT
  */
  Serial.print(F("Free Heap: "));
  Serial.print(freeHeap());

  vTaskStartScheduler();
  //Serial.print(czr.ReadAutoCalibration());

  //czr.CalibrateFreshAir();
  //czr.CalibrateKnownGas(1416);
  while(1)
  ;

}
//------------------------------------------------------------------------------
void loop() {}
