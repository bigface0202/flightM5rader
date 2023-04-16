#define LGFX_AUTODETECT

#include <Arduino.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

static LGFX lcd;
static LGFX_Sprite sprite(&lcd);

String apiAddress = "http://data-live.flightradar24.com/zones/fcgi/feed.js?bounds=35.6,35.5,139.7,139.8&adsb=1&mlat=1&faa=1&flarm=1&estimated=1&air=1&gnd=1&vehicles=1&gliders=1&array=1";
HTTPClient http;

String unix2datetime(time_t unixTime) {
  time_t unixTimeJTC = unixTime + 32400;
  struct tm *tmTime = localtime(&unixTimeJTC);
  char dateTimeString[20];
  strftime(dateTimeString, sizeof(dateTimeString), "%Y-%m-%d %H:%M:%S", tmTime);

  return dateTimeString;
}

int width = 160;
int height = 120;

void setup()
{
  // 初期化処理
  lcd.init();
  lcd.clear(TFT_BLACK);
  SD.begin(TFCARD_CS_PIN, SPI, 20000000);
  M5.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // ディスプレイの設定
  int _cursorX = 0;
  M5.Lcd.setTextFont(4);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(0, 0);
  // WiFiに接続
  M5.Lcd.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    M5.Lcd.setCursor(0 + 5 * _cursorX, 30);
    M5.Lcd.print(".");
    delay(300);
    _cursorX++;
    if (_cursorX > 320) {
      _cursorX = 0;
    }
  }
  // 画面の初期化
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  // 接続先IPアドレスの表示
  M5.Lcd.print("Connected with IP:");
  M5.Lcd.print(WiFi.localIP());
  delay(1000);
  M5.Lcd.fillScreen(BLACK);

  sprite.createSprite(width, height);
  sprite.drawPngFile(SD, "/img/airplane.png", 0, 0);
}

void loop()
{
  for (int i = 0; i < 10; i ++) {
    sprite.pushRotateZoom(width, height, 0, i, i);
    delay(100);
  }
  M5.Lcd.fillScreen(BLACK);
  String payload;
  http.begin(apiAddress);
  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      DynamicJsonDocument doc(4096);
      payload = http.getString();
      deserializeJson(doc, payload);
      int len = doc["aircraft"].size();

      for (int i = 0; i < len; i++) {
        String flightName = doc["aircraft"][i][10];
        time_t unixTimeUTC = doc["aircraft"][i][11];
        String departure = doc["aircraft"][i][12];
        String arrival = doc["aircraft"][i][13];

        // Unix timeから日本時間へ変換
        String dateTimeString = unix2datetime(unixTimeUTC);

        M5.Lcd.setCursor(0, 0);
        M5.Lcd.print("Flight Name: ");
        M5.Lcd.println(flightName);
        M5.Lcd.print("Time: ");
        M5.Lcd.println(dateTimeString);
        M5.Lcd.print("Departure: ");
        M5.Lcd.println(departure);
        M5.Lcd.print("arrival: ");
        M5.Lcd.println(arrival);
        delay(3000);
        M5.Lcd.fillScreen(BLACK);
      }
    } else {
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.print("HTTP GET ERROR: ");
      M5.Lcd.println(httpCode);
      delay(5000);
    }
  }
  http.end();
}