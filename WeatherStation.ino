/* Program to get current temperature and humidity using sensor DHT-11 KY-015 and post it to a server
    approximately every 30 seconds using ESP8266 ESP-01 Wi-Fi module.
    The program is designed to be used with Arduino UNO R3 ATmega328P microcontroller but should work
    with variety of other microcontrollers also.
*/

#include <SoftwareSerial.h>

#define RX 11
#define TX 10
#define DHTPin 7

// Wi-Fi credentials.

const String AP = "Guo";
const String PASS = "etnoastro";

// Server credentials.

const String API = "8KNDK9GKX7R97R02";
const String HOST = "api.thingspeak.com";
const String PORT = "80";
const String temperatureField = "field1";
const String humidityField = "field2";

const int postEverySeconds = 30;

/* Data from sensor is saved to this array.
    data[0] = humidity (integer)
    data[1] = humidity (floating point)
    data[2] = temperature (integer)
    data[3] = temperature (floating point)
*/

byte data[4];

int countTrueCommand;
int countTimeCommand;
boolean found = false;

SoftwareSerial esp8266(RX, TX);

void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
  pinMode(DHTPin, OUTPUT);

  //  Setup Wi-Fi module.

  sendCommand("AT", 5, "OK");
  sendCommand("AT+CWMODE=1", 5, "OK");
  sendCommand("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"", 20, "OK");
}

void loop() {

  // Load data from the sensor and store it in the global array data.

  loadData();

  // Build GET request.

  String request = "GET /update?api_key=" + API + "&" + temperatureField + "=" + String(data[2]) + "&" + humidityField + "=" + String(data[0]);

  // Send the request.

  sendCommand("AT+CIPMUX=1", 5, "OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT, 15, "OK");
  sendCommand("AT+CIPSEND=0," + String(request.length() + 4), 4, ">");

  esp8266.println(request);
  delay(postEverySeconds * 1000);
  countTrueCommand++;

  // Close connection.

  sendCommand("AT+CIPCLOSE=0", 5, "OK");
}

void loadData() {
  digitalWrite(DHTPin, LOW);

  delay(30);

  digitalWrite(DHTPin, HIGH);

  delayMicroseconds(40);

  pinMode(DHTPin, INPUT);

  while (digitalRead(DHTPin) == HIGH); delayMicroseconds(80);

  if (digitalRead(DHTPin) == LOW); delayMicroseconds(80);

  for (int i = 0; i < 4; i++) data[i] = readData();

  pinMode(DHTPin, OUTPUT);

  digitalWrite(DHTPin, HIGH);
}

byte readData() {
  byte data;

  for (int i = 0; i < 8; i++) {
    if (digitalRead(DHTPin) == LOW) {
      while (digitalRead(DHTPin) == LOW); delayMicroseconds(30);

      if (digitalRead(DHTPin) == HIGH) data |= (1 << (7 - i));

      while (digitalRead(DHTPin) == HIGH);
    }
  }

  return data;
}

// sendCommand() is used to communicate with Wi-Fi module using software serial.

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");

  while (countTimeCommand < (maxTime * 1)) {
    esp8266.println(command);

    if (esp8266.find(readReplay)) {
      found = true;

      break;
    }

    countTimeCommand++;
  }

  if (found == true) {
    Serial.println("Success");

    countTrueCommand++;
    countTimeCommand = 0;
  } else {
    Serial.println("Fail");

    countTrueCommand = 0;
    countTimeCommand = 0;
  }

  found = false;
}
