#define BME280_ADDR 0x76
#define MHZ19B_RX 4
#define MHZ19B_TX 5

#define PERIOD 5  // seconds

#include <Adafruit_BME280.h>
#include <SoftwareSerial.h>

Adafruit_BME280 bme280;
SoftwareSerial mhz19b(MHZ19B_RX, MHZ19B_TX);

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

  if (!bme280.begin(BME280_ADDR)) {
    Serial.println("BME280 Error");
    panic();
  }

  byte response[9];
  if (touch_mhz19b(response, set5000ppm)) {
    Serial.println("MH-Z19B Error");
    panic();
  }
}

void loop() {
  float temperature, humidity, pressure, co2_ppm;

  temperature = bme280.readTemperature();
  humidity = bme280.readHumidity();
  pressure = bme280.readPressure();

  byte response[9];
  if (!touch_mhz19b(response, measureCO2))
    co2_ppm = response[2] * 256 + response[3];
  else
    co2_ppm = NAN;

  Serial.println(String(temperature) + ' ' + String(humidity) + ' ' +
                 String(pressure) + ' ' + String(co2_ppm));

  delay(PERIOD * 1000);
}

void panic() {
  for (;;) {
    PORTB ^= 1 << 5;
    delay(500);
  }
}
