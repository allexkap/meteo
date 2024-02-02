#define BME280_ADDR 0x76
#define MHZ19B_RX 13
#define MHZ19B_TX 15

#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

#include "keys.h"

Adafruit_BME280 bme280;
SoftwareSerial mhz19b(MHZ19B_RX, MHZ19B_TX);
WiFiServer server(80);

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
    panic_blink();
  }

  byte response[9];
  if (touch_mhz19b(response, set5000ppm)) {
    Serial.println("MH-Z19B Error");
    panic_blink();
  }

  WiFi.begin(WIFI_SSID, WIFI_PSWD);
  Serial.println("Connecting to " WIFI_SSID);
  for (int i = 0; WiFi.status() != WL_CONNECTED; ++i) {
    Serial.print('.');
    delay(1000);
  }
  Serial.print("\nIP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  Serial.print("Client connected ");
  Serial.print(client.remoteIP());
  Serial.print(':');
  Serial.println(client.remotePort());

  float data[4];
  while (client.connected()) {
    wait_client(client);
    poll_sensors(data);
    client.printf("%f %f %f %f", data[0], data[1], data[2], data[3]);
  }

  client.stop();
  Serial.println("Client disconnected");
}

void poll_sensors(float data[4]) {
  data[0] = bme280.readTemperature();
  data[1] = bme280.readHumidity();
  data[2] = bme280.readPressure();

  byte response[9];
  if (!touch_mhz19b(response, measureCO2))
    data[3] = response[2] * 256 + response[3];
  else
    data[3] = NAN;
}

void wait_client(WiFiClient client) {
  while (!client.available());
  while (client.available()) client.read();
}

void panic_blink() {
  pinMode(LED_BUILTIN, OUTPUT);
  for (;;) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(500);
  }
}
