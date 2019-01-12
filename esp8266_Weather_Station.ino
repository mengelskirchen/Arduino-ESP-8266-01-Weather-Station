#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
const char* ssid = ""; //WLAN-Name
const char* password = ""; //WLAN-Passwort
const char* host = "api.openweathermap.org";
const char* location = "Köln";
const char* apikey = ""; // API-Key vom openweathermap.org Account
const int httpsPort = 443;
// Use web browser to view and copy
// SHA1 fingerprint of the certificate
const char* fingerprint = "";

byte degree[8] = { // Grad-Zeichen °
  B00010,
  B00101,
  B00010,
  B00000,
  B00000,
  B00000,
  B00000,
};

void write(LiquidCrystal_I2C lcd, String s) {
  write(lcd, s.c_str());
}

void writeLn(LiquidCrystal_I2C lcd, String s, int ln) {
  writeLn(lcd, s.c_str(), ln);
}

void writeLn(LiquidCrystal_I2C lcd, const char* s, int ln) {
  int i = 0;
  while (*s != '\0' && i < 20) {
    delay(10);
    lcd.setCursor(i, ln);
    lcd.write(*s);
    ++s;
    yield();
    i++;
  }
}

void writeLnRest(LiquidCrystal_I2C lcd, String s, int ln, int st) {
  writeLnRest(lcd, s.c_str(),ln,st);
}

void writeLnRest(LiquidCrystal_I2C lcd, const char* s, int ln, int st) {
  int i = st;
  while (*s != '\0' && i < 20) {
    delay(10);
    lcd.setCursor(i, ln);
      lcd.write(*s);

    ++s;  //you can increment pointers without assigning an address to them
    yield();
    i++;
  }
}

void write(LiquidCrystal_I2C lcd, const char* s) {
  int i = 0;
  int j = 0;
  while (*s != '\0') {
    delay(10);
    lcd.setCursor(i, j);
    lcd.write(*s);
    ++s;  //you can increment pointers without assigning an address to them
    yield();
    i++;
    if (i > 19) {
      j++;
      i = 0;
      if (j > 3)
        j = 0;
    }
  }
}

void setup() {
  lcd.begin(0, 2);
  lcd.backlight();
  lcd.createChar(1, degree);
  Serial.begin(9600);
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  writeLn(lcd, "connecting to:", 0);
  writeLn(lcd, String(ssid), 1);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  lcd.clear();
  writeLn(lcd, "WiFi connected.", 0);
  writeLn(lcd, "IP addresse: ", 1);
  IPAddress ip = WiFi.localIP();
  writeLn(lcd, String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]), 2);
  delay(100);
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Use WiFiClientSecure class to create TLS connection
  WiFiClientSecure client;
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  if (client.verify(fingerprint, host)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }

  String url = "/data/2.5/weather?q="+location+"&APPID="+apikey+"&units=metric&lang=de";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  yield();
  StaticJsonBuffer<1024> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(String(line));
  write(lcd, String(line).c_str());
  yield();
  lcd.clear();
  String temp = root["main"]["temp"].as<String>();
  String pressure = root["main"]["pressure"].as<String>();
  String wind = root["wind"]["speed"].as<String>();
  String weather = root["weather"][0]["description"].as<String>();
  Serial.println(root["main"]["pressure"].as<float>());
  Serial.println(temp);
  writeLn(lcd, "Temp: " + temp + "\1C", 0);
  writeLn(lcd, "Druck: " + pressure, 1);
  writeLn(lcd, "Wind: " + wind, 2);
  writeLn(lcd, "Wetter: ", 3);
  yield();
  Serial.println("==========");
  Serial.println("closing connection");
  int c=0;
  int p=0;
  while(c<60000) {
    String weather = root["weather"][p++]["description"].as<String>();
    if(weather=="") {
        p=0;
        weather = root["weather"][p++]["description"].as<String>();
      }
      lcd.setCursor(8,3);
      writeLnRest(lcd,"            ",3,8);
      lcd.setCursor(8,3);
      
      writeLnRest(lcd,weather,3,8);
      delay(5000);
      c+=5000;
    }
}
