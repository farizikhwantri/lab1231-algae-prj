#ifndef CONSTANTS_H
#define CONSTANTS_H
#endif

/** role dari node radio saat ini */
typedef enum { NODE_SENSOR, NODE_SINK } node_t;

/** alamat radio device yang bertindak sebagai sink */
#define ADDR_SINK 0x7fff

/**
  paket ini adalah paket data
*/
#define FLAG_DATA     0b00001000
/**
  paket ini adalah paket informasi routing
*/
#define FLAG_INFO     0b00000000
/**
  paket ini adalah perintah dari server
*/
#define FLAG_CMD      0b00001100
/***************************************/
/**
  minta ack balik dari penerima paket
*/
#define FLAG_REQ_ACK  0b10000000
/**************************************/
