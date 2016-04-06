#include <Arduino.h>
#include <EEPROM.h>

uint8_t addr_arr[2];

#define EEPROM_ADDR_NODE_UPPER 0xfffe
#define EEPROM_ADDR_NODE_LOWER 0xffff

void setup() {
  Serial.begin(9600);
  Serial.print("BAKAAAR ALAMAT KE EEPROM\n\r");
  Serial.print("MASUKKAN ALAMAT (4 digit hex, lowercase)\n\r");
  Serial.print("address = ");
  char ctr = 0;
  uint16_t addr;
  for (char z = 1; z <= 4; z++)
  {
    char sc;
    while (!Serial.available()) // tunggu sampai ada data masuk dari serial
      ;
    sc = tolower(Serial.read());
    Serial.write(sc);
    if (sc < 59) // ASCII '0'- '9'
      addr |= sc - '0';
    else // asumsi inputan 'a'-'f', konversi ke hex
      addr |= sc - 'a' + 0xa;
    if (z < 4)
      addr <<= 4;
  }
  Serial.print("address = ");
  Serial.print(addr, HEX);
  Serial.print("\r\n");
  uint8_t* p = (uint8_t*) &addr;
  addr_arr[1] = *(p + 1);
  addr_arr[0] = *p;
  Serial.print("upper addr = "); Serial.print(addr_arr[1], HEX); Serial.print("\r\n");
  Serial.print("lower addr = "); Serial.print(addr_arr[0], HEX); Serial.print("\r\n");
  
  Serial.print("\r\nmulai bakar ke EEPROM (y/t)?\r\n");
  while(tolower(Serial.read()) != 'y')
    ;
  
  Serial.print("MULAI TULIS KE EEPROM\r\n");
  EEPROM.write(EEPROM_ADDR_NODE_UPPER, addr_arr[1]);
  EEPROM.write(EEPROM_ADDR_NODE_LOWER, addr_arr[0]);
  Serial.write("HASIL BACA EEPROM = ");
  uint8_t a1, a2;
  Serial.write(a1=EEPROM.read(EEPROM_ADDR_NODE_UPPER), HEX);
  Serial.write(a2=EEPROM.read(EEPROM_ADDR_NODE_LOWER), HEX);
  if(a1 == addr_arr[1] && a2 == addr_arr[0])
    Serial.write("\n\rPEMBAKARAN SUKSES\n\r");
  else
    Serial.write("\n\rTERJADI KESALAHAN\n\r");
}

void loop() {
  
}
