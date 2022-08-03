#include <SPI.h>
#include <NTPClient.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>

#define SS_PIN D4
#define RST_PIN D3    // Configurable, see typical pin layout above
#define OLED_RESET LED_BUILTIN //4
Adafruit_SSD1306 display(OLED_RESET);
int ledPin = 15;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
#define BUZZ_PIN D8
#define GATE_PIN D0
const char* host = "script.google.com";
const int httpsPort = 443;
const char* fingerprint  = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6"; // for https


//***********Things to change*******************
String GOOGLE_SCRIPT_ID = "AKfycbxeXN2YSEeeRntnvSX6oNtPUhLxjjiLcMMp8fdUPyKqKs3G3dj4731qNdThDqBtJdfd"; // Replace by your GAS service id
const String unitName = "Technopark"; // any name without spaces and special characters
//***********Things to change*******************
uint64_t openGateMillis = 0;
WiFiClientSecure client;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.id.pool.ntp.org", 25200, 60000);

void LcdClearAndPrint(String text)
{
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
  display.print(text);
  display.display();
}

void ledclock(String text2)
{
  display.clearDisplay();
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 5);
  display.print(text2);
  display.display();
}

void Siren()
{
  for (int hz = 440; hz < 1000; hz++) {
    tone(BUZZ_PIN, hz, 50);
    delay(5);
  }

  for (int hz = 1000; hz > 440; hz--) {
    tone(BUZZ_PIN, hz, 50);
    delay(5);
  }
  digitalWrite(BUZZ_PIN, LOW);
}


void Beep()
{
  for (int i = 0; i < 1000; i++)
  {
    analogWrite(BUZZ_PIN, i);
    delayMicroseconds(50);
  }
  digitalWrite(BUZZ_PIN, LOW);
}

void Beep2()
{
  tone(BUZZ_PIN, 1000, 30);
  delay(300);
  digitalWrite(BUZZ_PIN, LOW);
}

void setup() {

  WiFiManager wm;
    wm.resetSettings();
    bool res;
    res = wm.autoConnect("Rfid System");
    if(!res) {
     Serial.println("Failed to connect");
     // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }

    WiFi.mode(WIFI_STA);


  timeClient.begin();
  
  pinMode(GATE_PIN, OUTPUT);
  pinMode(BUZZ_PIN, OUTPUT);
  digitalWrite(GATE_PIN, LOW);
  digitalWrite(BUZZ_PIN, LOW);
  

  Serial.begin(921600); 

  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
// lcd.begin(0, 2); //ESP8266-01 I2C with pin 0-SDA 2-SCL
// Turn on the blacklight and print a message.
   display.display(); 
  LcdClearAndPrint("Sedang Menyambung");+-
  

  // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  delay(4);       // Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  ledclock(timeClient.getFormattedTime());
}
byte readCard[4];

void HandleDataFromGoogle(String data)
{
  int ind = data.indexOf(":");
  String access = data.substring(0, ind);
  int nextInd = data.indexOf(":", ind + 1);
  String name = data.substring(ind + 1, nextInd);
  String text = data.substring(nextInd + 1, data.length());
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  Serial.println(name);
  LcdClearAndPrint(name);
  display.setCursor(0, 15);
  display.print(text);
  display.display();
  delay(500);

  if (access=="-1")
  { 
    display.clearDisplay();
    display.display();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.print(" " + String("denied"));
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,15);
    Siren(); 
    LcdClearAndPrint("Ready");
    display.display();
  }
  else if(access=="any")
  {
    display.clearDisplay();
    display.display();
    display.setTextColor(WHITE);   
    display.setTextSize(1);
    display.setCursor(12,10); //col=0 row=0
    display.print(" " + String("Selamat Belajar"));
    OpenGate();
    display.display();
    delay(500);
  }
  else if (access=="fridge")
  {
    display.clearDisplay();
    display.display();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(20,10); //col=0 row=0
    display.print(" " + String("Terima Kasih"));
    OpenGate();
    display.display();
  }

}

void OpenGate()
{
  openGateMillis = millis()+1000;
  digitalWrite(GATE_PIN, HIGH);
  Beep();
  delay(100);
  Beep();
}

void CloseGate()
{
  openGateMillis = 0;
  digitalWrite(GATE_PIN, LOW);
  Beep2();
  LcdClearAndPrint("Ready");
}

void loop() {
  display.clearDisplay();
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(15,10); //col=0 row=0
  timeClient.update();
  display.print(timeClient.getFormattedTime());
  display.display();
  delay(800);

  timeClient.getFormattedTime();
  if (openGateMillis > 0 && openGateMillis < millis())
  {
    CloseGate();
  }


  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  Serial.println(F("Scanned PICC's UID:"));
  String uid = "";
  for (uint8_t i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
    uid += String(readCard[i],HEX);
  }
  Serial.println("");

  Beep();
  LcdClearAndPrint("Membaca data...");
  String data = sendData("id=" + unitName + "&uid=" + uid,NULL);
  HandleDataFromGoogle(data);


  mfrc522.PICC_HaltA();
}


String sendData(String params, char* domain) {
  //google scripts requires two get requests 
  bool needRedir = false;
  if (domain == NULL)
  {
    domain=(char*)host;
    needRedir = true;
    params = "/macros/s/" + GOOGLE_SCRIPT_ID + "/exec?" + params;
  }
  
  Serial.println(*domain);
  String result = "";
  client.setInsecure();
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return "";
  }

  if (client.verify(fingerprint, domain)) {
  }

  Serial.print("requesting URL: ");
  Serial.println(params);

  client.print(String("GET ") + params + " HTTP/1.1\r\n" +
    "Host: " + domain + "\r\n" +
    "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {

    String line = client.readStringUntil('\n');
    //Serial.println(line);
    if (needRedir) {

    int ind = line.indexOf("/macros/echo?user");
    if (ind > 0)
    {
      Serial.println(line);
      line = line.substring(ind);
      ind = line.lastIndexOf("\r");
      line = line.substring(0, ind);
      Serial.println(line);
      result = line;
    }
    }

    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  while (client.available()) {
    String line = client.readStringUntil('\n');
    if(!needRedir)
    if (line.length() > 5)
      result = line;
    //Serial.println(line);
    
    }
  if (needRedir)
    return sendData(result, "script.googleusercontent.com");
  else return result;
  

}
