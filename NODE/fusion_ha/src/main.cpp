
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

// library udah dipindahin, include di sini
#include "constants.h"
#include "paket.h"

//#include <time.h>

// pengganti printf.h
int serial_putc( char c, FILE * )
{
    Serial.write( c );
    return c;
}

void printf_begin(void)
{
    fdevopen( &serial_putc, 0 );
}
// ~pengganti

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
  // the xaxaxa of C/C++, point everything directly!
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
  do { char* z = (char*)paket; while(z < (char*)paket + PANJANG_PAKET) printf("%c", *(z++)); printf("\n\r"); } while(0);

/**
   fungsi untuk mengirim paket via radio
   @param paket* memory addr paket
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
  delay(x);
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
uint16_t addr_saya;

// handle untuk task RTOS
TaskHandle_t task_route, task_svc;

// mutex-mutex
SemaphoreHandle_t sem_pckt;

static void vTaskRoute(void* pvParam)
{ while (1) {
    if (node_type == NODE_SENSOR)
    {
      // apa ada paket yang mau masuk?
      if (!radio.available())
      {
        // kalau nggak, kirim paketnya sekarang
        // TODO: ini critical section, rawan chaos. tambahkan mutex di sini
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
          continue;
        }
        else // paket ini bukan dari saya dan bukan buat saya, forward ke node berikutnya
        {
          printf("ada paket yang diterusin "); cetakpaket(buf);
          radio.write(&buf, PANJANG_PAKET);
        }
      }
      delay_rand();
    }
    else
    {
      bool done = false;
      uint8_t arr[PANJANG_PAKET];
      if (radio.available())
      {
        while (!done)
          done = radio.read(&arr, PANJANG_PAKET);
        printf("yang ditangkep: "); cetakpaket(arr);
      }
    }
  }
}

static void vTaskSvc(void* param)
{ while (1) {
    // task untuk menjalankan service (lapor ke server, monitor board,
    // execute command, dll)
    vTaskDelay(100);
  }
}

void setup( void ){
  sws.begin(9600);
  Serial.begin(9600);
  //czr.init();

  // dummy packet
  // pckt = { .header    = 'h', // uint8_t header
  //          .timestamp = 'k', // uint64_t timestamp
  //          .addr_s    = 's' << 8 | 'z', // float_t addr_s
  //          .addr_t    = 't' << 8 | 'g', // uint16_t addr_t
  //          .ts_c      = '1' << 8 | '2', // float ts_c
  //          .co2       = '9' << 8 | 'A', // float co2
  //          .hu        = 0x42434445LL, // float hu
  //          .li        = 'F' << 8 | '0', // uint16_t li
  //        };

  Serial.begin(9600);

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

  printf("TEKAN 'T' UNTUK MASUK KE SINK MODE, S UNTUK MASUK KE SENSOR MODE\n\r");
  char typ = 0;
  while (typ != 'T' && typ != 'S')
  {
    while (!Serial.available())
      ;
    typ = toUpperCase(Serial.read());
  }
  printf("%c\n\r", t);
  if (typ == 'T')
  {
    printf("SAYA ADALAH SINK\n\r");
    node_type = NODE_SINK;
    addr_saya = ADDR_SINK;
  }
  else if (typ == 'S')
  {
    printf("SAYA ADALAH SOURCE\n\r");
    node_type = NODE_SENSOR;
    // minta addr node ini
    printf("MASUKKAN ADDR NODE INI: ");
    for (char z = 1; z <= 4; z++)
    {
      char sc;
      while (!Serial.available()) // tunggu sampai ada data masuk dari serial
        ;
      sc = Serial.read();
      printf("%c", sc);
      if (sc < 59) // ASCII '0'- '9'
        addr_saya |= sc - '0';
      else // asumsi inputan 'a'-'f', konversi ke hex
        addr_saya |= sc - 'a' + 0xa;
      if (z < 4)
        addr_saya <<= 4;
    }
  }
  // init fungsi random() untuk random delay
  randomSeed(analogRead(0));

  printf("\n\rADDR: %4x\n\r", addr_saya);

  // init mutex buat pckt
  sem_pckt = xSemaphoreCreateBinary();
  if(sem_pckt == NULL)
  {
    Serial.println("sem_init gagal\n\r");
    while(1) ;
  }

  // init RTOS
  xTaskCreate(vTaskRoute, "routing", configMINIMAL_STACK_SIZE + 250, NULL, 1, &task_route);
  xTaskCreate(vTaskSvc, "node_svc", configMINIMAL_STACK_SIZE, NULL, 1, &task_svc);


  xTaskCreate( vTempReader, "TempReader", configMINIMAL_STACK_SIZE + 250, NULL, 1, &temp);
  xTaskCreate( vCO2Reader, "CO2Reader", 500, NULL, 1, &co2);
  xTaskCreate( vHumidityReader, "HumidityReader", configMINIMAL_STACK_SIZE + 100, NULL, 1, &hum);
  // xTaskCreate( vLightReader, "LightReader", 200, NULL, 2, &light);
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
  while(1)
    ;

}
//------------------------------------------------------------------------------
void loop() {}
