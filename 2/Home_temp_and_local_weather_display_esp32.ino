#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define DHTPIN 4
#define DHTTYPE DHT11


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastSwitch = 0;
bool showInside = true;

float outsideTemp = 0;
int outsideHumidity = 0;
String weatherDesc = "";

const char* ssid = "SECRET"; // my wifi id here 
const char* password = "SECRET"; // my wifi password here 

String weatherkey = "SECRET"; // your weather key from your designated api
String location = "SECRET"; // your location

unsigned long lastWeatherUpdate = 0;

void drawHouse() {
  // House body
  display.drawRect(96, 24, 28, 30, SSD1306_WHITE);

  // Roof
  display.drawLine(96, 24, 110, 8, SSD1306_WHITE);
  display.drawLine(124, 24, 110, 8, SSD1306_WHITE);

  // Door
  display.drawRect(106, 38, 8, 16, SSD1306_WHITE);

  // Window
  display.drawRect(100, 30, 6, 6, SSD1306_WHITE);
}

void drawCloud() {
  // Cloud puffs
  display.drawCircle(102, 24, 8, SSD1306_WHITE);
  display.drawCircle(110, 18, 10, SSD1306_WHITE);
  display.drawCircle(120, 24, 8, SSD1306_WHITE);

  // Cloud body
  display.drawRoundRect(96, 24, 32, 22, 6, SSD1306_WHITE);

  // Fill the cloud
  display.fillCircle(102, 24, 8, SSD1306_WHITE);
  display.fillCircle(110, 18, 10, SSD1306_WHITE);
  display.fillCircle(120, 24, 8, SSD1306_WHITE);
  display.fillRoundRect(96, 24, 32, 22, 6, SSD1306_WHITE);

}

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(".");
  }

  Serial.println("WiFi Connected");

  getWeather();

  dht.begin();

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while (true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 0);
  display.println("Starting...");
  display.display();

  delay(2000);
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("DHT read failed");

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Sensor Error");
    display.display();

    delay(2000);
    return;
  }

  if (millis() - lastSwitch > 20000){ // 20 seconds
  showInside = !showInside;
  lastSwitch = millis();
  }

  if(millis() - lastWeatherUpdate > 600000) {  // 10 mins
  getWeather();
  lastWeatherUpdate = millis();
  }


  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print(" C  Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  display.clearDisplay();

  if (showInside) {

    drawHouse();

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print(temperature, 1);
    display.println(" C");

    display.setTextSize(2);
    display.setCursor(0, 32);
    display.print(humidity, 0);
    display.println("%");

  } else {

    drawCloud();

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print(outsideTemp, 1);
    display.println(" C");

    display.setTextSize(2);
    display.setCursor(0, 32);
    display.print(outsideHumidity, 0);
    display.println("%");
  }
  display.display();

  delay(2000);
}

void getWeather() {

  HTTPClient http;

  String url =
    "http://api.weatherstack.com/current?access_key=" +  // I personally use weatherstack as its free for basic usage!
    weatherkey +
    "&query=" +
    location;

  http.begin(url);

  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("RAW RESPONCE");
    Serial.println(payload);
  
    DynamicJsonDocument doc(4096);
    deserializeJson(doc, payload);
    Serial.println("Temp");
    Serial.println(doc["current"]["temperature"].as<float>());
    
    Serial.println("Humidity");
    Serial.println(doc["current"]["humidity"].as<int>());

    outsideTemp =
    doc["current"]["temperature"];

    outsideHumidity =
    doc["current"]["humidity"];

    weatherDesc =
    doc["current"]["weather_descriptions"][0].as<String>();
    
    Serial.println("Weather Updated");
  }
  http.end();

