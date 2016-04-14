/**
  sensor.cpp
  Kode untuk di-upload ke node sensor yang mengirim data ke client
  Copyright ___
  version xx.xx
*/
/* Include FreeRTOS */
#include "FreeRTOS_AVR.h"
#include <Arduino.h>
#include "basic_io_avr.h"
#include <SoftwareSerial.h>
#include <cozir.h>
#include <stdio.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <SPI.h>
#include <EEPROM.h>

//#include <time.h>

#include "constants.h"
#include "paket.h"
#include "printf.h"

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
      // vTaskSuspendAll();
      {
        if(resTemp.ucStatus==f){
          float cel = czr.Celsius();
          resTemp.usVal = cel;
          resTemp.ucStatus = t;
        }
      }
      // xTaskResumeAll();
    }
}

static void vCO2Reader ( void *pvParameter){
  //Serial.print(F("Unused Stack CO2: "));
  //Serial.println(uxTaskGetStackHighWaterMark(co2));
  //Serial.print(F("Free Heap: "));
  //Serial.println(freeHeap());
  for( ;; ){
    // vTaskSuspendAll();
    {
      if(resCO2.ucStatus==f){
        unsigned long co2_v = czr.CO2();
        resCO2.ulCO2Val = co2_v;
        resCO2.ucStatus = t;
      }
    }
    // xTaskResumeAll();
  }
}


static void vHumidityReader ( void *pvParameter){
  for( ;; ){
    // vTaskSuspendAll();
    {
      if(resHum.ucStatus==f){
        float cel = czr.Humidity();
        resHum.usVal = cel;
        resHum.ucStatus = t;
      }
    }
    // xTaskResumeAll();
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
      // vTaskSuspendAll();
    	{
        if(resCO2.ucStatus==t && resTemp.ucStatus==t && resHum.ucStatus==t){
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
    	// xTaskResumeAll();
    }

}

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
#define delay_rand() delay(random(100))

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

static void vTaskRoute(void* pvParam)
{ while (1) {
    // apa ada paket yang mau masuk?
    if (!radio.available())
    {
      // kalau nggak, kirim paketnya sekarang
      // rawan chaos: tambahkan mutex di sini
      // TODO: tambah flag untuk info data udah siap atau belum

      pckt.addr_s = addr_saya;
      pckt.addr_t = ADDR_SINK;

      kirim_paket(&pckt);
    }
    else
    {
      // tahan, ada paket masuk
      bool done = false;
      while (!done)
      {
        done = radio.read(&buf, PANJANG_PAKET);
      }
      // liat alamatnya, buat kita apa bukan?
      uint16_t addr_tujuan_paket = buf[5] << 8 | buf[4];
      uint16_t addr_asal_paket = buf[3] << 8 | buf[2];
      if (addr_tujuan_paket == addr_saya)
      {
        // apa yang mau dilakuin seandainya paket ini buat saya
        // misalnya ini perintah dari server atau informasi dari
        // tetangga
        Serial.println("hore kita dapet paket "); cetakpaket(buf);
        ;
      }
      else if (addr_asal_paket == addr_saya)
      {
        // ini paket yang mau saya kirim, keluar dari loop saat ini
        printf("paket ini sih punya saya "); cetakpaket(buf);
      }
      // paket ini bukan dari saya dan bukan buat saya, forward ke node berikutnya
      else
      {
        printf("ada paket yang diterusin "); cetakpaket(buf);
        radio.write(&buf, PANJANG_PAKET);
      }
    }
    vTaskDelay(20);
  }
}

static void vTaskSvc(void* param)
{ while (1) {
    // task untuk menjalankan service (lapor ke server, monitor board,
    // execute command, dll)
    if(Serial.available())
    {
      printf("TEKAN S UNTUK MELAKUKAN PENGATURAN");
      char input = 0;
      while(input != 'S')
      {
        input = toupper(Serial.read());
      }
      printf("SILAKAN MASUKKAN PENGATURAN");
    }
    vTaskDelay(50);
  }
}

void setup( void ){
  sws.begin(9600);
  Serial.begin(9600);
  //czr.init();


  // Serial.print(F("Free Heap: "));
  // Serial.print(freeHeap());

  // inisialisasi perangkat
  printf_begin();
  radio.begin();
  radio.setDataRate(RF24_1MBPS);
  radio.setRetries(15, 15);

  radio.openWritingPipe(pipes[0]);
  // listen di semua pipe
  for (int i = 0; i < 6; i++)
    radio.openReadingPipe(i + 1, pipes[i]);

  // mulai listen
  radio.startListening();
  // debug print
  radio.printDetails();

  // ambil address sekarang dari EEPROM
  EEPROM.begin();
  addr_saya = EEPROM.read(EEPROM_ADDR_NODE_UPPER) << 8 |
      EEPROM.read(EEPROM_ADDR_NODE_LOWER);
  printf("ADDR SAYA: %4x\n\r", addr_saya);
  // end inisialisasi radio

  xTaskCreate( vTempReader, "TempReader", configMINIMAL_STACK_SIZE + 250, NULL, 1, &temp);
  xTaskCreate( vCO2Reader, "CO2Reader", 500, NULL, 1, &co2);
  xTaskCreate( vHumidityReader, "HumidityReader", configMINIMAL_STACK_SIZE + 100, NULL, 1, &hum);
  //xTaskCreate( vLightReader, "LightReader", 200, NULL, 2, &light);
  xTaskCreate( vPrintTask, "PrintTask", 400, NULL, 1, &print);
  xTaskCreate(vTaskRoute, "Task routing", configMINIMAL_STACK_SIZE + 150, NULL, 1, &task_route);
  xTaskCreate(vTaskSvc, "Task service", configMINIMAL_STACK_SIZE, NULL, 1, &task_svc);

  vTaskStartScheduler();
  printf("SCHEDULER START");
  //Serial.print(czr.ReadAutoCalibration());

  //czr.CalibrateFreshAir();
  //czr.CalibrateKnownGas(1416);
  while(1)
    ;

}
//------------------------------------------------------------------------------
void loop() {

}
