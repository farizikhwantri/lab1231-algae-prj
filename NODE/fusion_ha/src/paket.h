#ifndef PAKET
#define PAKET
#endif

#include <Arduino.h>

/** panjang paket keseluruhan, dalam byte */
# define PANJANG_PAKET 184/8

/* deskripsi paket */
typedef struct
{
  uint8_t   header;
  uint64_t  timestamp;
  uint16_t  addr_s;
  uint16_t  addr_hop;
  uint16_t  addr_t;
  // payload, data sensor
  float  ts_c;
  float  co2;
  float  hu;
  // data yang dikirimkan dari light sensor adalah data mentah 16 bit
  uint16_t  li;
  // ~payload, data sensor
} paket;
