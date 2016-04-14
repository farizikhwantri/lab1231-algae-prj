#include "paket.h"

/*****************************************************************************/
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

/*****************************************************************************/

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
/*****************************************************************************/
