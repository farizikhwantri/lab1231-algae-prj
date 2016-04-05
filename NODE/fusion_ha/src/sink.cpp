

static void vTaskRoute(void* pvParam)
{ while (1) {
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
