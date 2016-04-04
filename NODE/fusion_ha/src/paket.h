#ifndef PAKET_H
#define PAKET_H
#endif

#include <Arduino.h>

/** definisi panjang paket */
#define PANJANG_PAKET 184/4

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

/**
   Konversi isi paket ke array. Dibutuhkan saat paket akan dikirim
   Array yang dihasilkan berpola big endian
   @param paket pc yang akan dikonversi ke array
   @param array arr yang akan diisi dari paket
   @return void
*/
void toarray(paket* pc, uint8_t paket_yang_dibalikin[PANJANG_PAKET]);

/**
   Parse array yang diterima dari radio jadi paket ke this
   Array yang diterima sesuai urutan paket dalam pola big endian
   @param array yang akan dijadikan paket
   @param paket hasil keluaran
   @return void
*/
void parse(uint8_t paket_diterima[PANJANG_PAKET], paket* pc);
// </p> paket radio
