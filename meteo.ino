#define PERIOD 5  // seconds

#include <SoftwareSerial.h>

SoftwareSerial mhz19b(4, 5);  // RX, TX

const byte set5000ppm[9] = {0xff, 0x01, 0x99, 0x00, 0x00, 0x00, 0x13, 0x88, 0xcb};
const byte set2000ppm[9] = {0xff, 0x01, 0x99, 0x00, 0x00, 0x00, 0x07, 0xd0, 0x8f};
const byte measureCO2[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};

int touch_mhz19b(byte response[9], const byte request[9]) {
  byte acc = 0;
  mhz19b.write(request, 9);
  mhz19b.readBytes(response, 9);
  for (int i = 1; i < 8; ++i) acc -= response[i];
  return (response[0] != 0xff) + (response[1] != request[2]) * 2 + (response[8] != acc) * 4;
}

void setup() {
  Serial.begin(115200);
  mhz19b.begin(9600);
  delay(1000);

  byte response[9];
  if (touch_mhz19b(response, set5000ppm)) panic();
}

void loop() {
  byte response[9];
  if (touch_mhz19b(response, measureCO2)) {
    Serial.println("error");
    return;
  }

  int ppm = response[2] * 256 + response[3];
  Serial.println(ppm);

  delay(PERIOD * 1000);
}

void panic() {
  while (1) {
    PORTB ^= 1 << 5;
    delay(500);
  }
}
