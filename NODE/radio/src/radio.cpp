/**
   Line  From Arduino Pin  From Grid Position  To Radio Pin
    GND   GND                 U17                   1
    3V3   3V3                 U14                   2
    CE    9                   D15                   3
    CSN   10                  D14                   4
    SCK   13                  D11                   5
    MOSI  11                  D13                   6
    MISO  12                  D12                   7
*/

#include <FreeRTOS_AVR.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"

// device radionya
RF24 radio(9, 10);

// dummy sink addr
#define ADDR_SINK 0x7fff

// <p> paket radio
# define PANJANG_PAKET 128/8

// role dari node ini
// TODO: masukkan ke file .h
typedef enum { NODE_SENSOR, NODE_SINK } node_t;

/* deskripsi paket */
// TODO: masukkan ke file .h
typedef struct
{
  uint8_t   header;
  uint8_t   timestamp;
  uint16_t  addr_s;
  uint16_t  addr_t;
  // payload, data sensor
  uint16_t  ts_c;
  uint16_t  co2;
  uint32_t  hu;
  // data yang dikirimkan dari light sensor adalah data mentah 16 bit
  uint16_t  li;
  // ~payload, data sensor
  // crc 16 bit, data dihitung berdasarkan timestamp dan payload
  uint16_t crc;
} paket;

/**
   Konversi isi paket ke array. Dibutuhkan saat paket akan dikirim
   Array yang dihasilkan dalam pola big endian
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
// addr node 1 = 0xfa01 -> mega, node 2 = 0xfa03 -> uno, node sink = ADDR_SINK -> mega ADK, hard code manual ya huehuehue
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
        // rawan chaos: tambahkan mutex di sini
        while(xSemaphoreTake(&sem_pckt, 0))
          ;
        pckt.addr_s = addr_saya;
        pckt.addr_t = ADDR_SINK;
        xSemaphoreGive(&sem_pckt);
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
        if (addr_asal_paket == addr_saya)
        {
          // ini paket yang mau saya kirim, keluar dari loop saat ini
          printf("paket ini sih punya saya "); cetakpaket(buf);
          return;
        }
        // paket ini bukan dari saya dan bukan buat saya, forward ke node berikutnya
        printf("ada paket yang diterusin "); cetakpaket(buf);
        radio.write(&buf, PANJANG_PAKET);
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

void setup() {

  // dummy packet
  pckt = { .header    = 'h', // uint8_t header
           .timestamp = 't', // uint8_t timestamp
           .addr_s    = 's' << 8 | 'z', // uint16_t addr_s
           .addr_t    = 't' << 8 | 'g', // uint16_t addr_t
           .ts_c      = '1' << 8 | '2', // uint16_t ts_c
           .co2       = '9' << 8 | 'A', // uint16_t co2
           .hu        = 0x42434445LL, // uint32_t hu
           .li        = 'F' << 8 | '0', // uint16_t li
           .crc       = 'r' // uint8_t crc
         };

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
  printf("\n\rADDR: %4x\n\r", addr_saya);

  printf("TEKAN 'T' UNTUK MASUK KE SINK MODE, S UNTUK MASUK KE SENSOR MODE\n\r");
  char t = 0;
  while (t != 'T' && t != 'S')
  {
    while (!Serial.available())
      ;
    t = toUpperCase(Serial.read());
  }
  printf("%c\n\r", t);
  if (t == 'T')
  {
    printf("SAYA ADALAH SINK\n\r");
    node_type = NODE_SINK;
  }
  else if (t == 'S')
  {
    printf("SAYA ADALAH SOURCE\n\r");
    node_type = NODE_SENSOR;
  }
  // init fungsi random() untuk random delay
  randomSeed(analogRead(0));

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
  printf("RUNNING TASK\n\r");
  vTaskStartScheduler();

  while (1)
    ;
}

void loop() {

}
