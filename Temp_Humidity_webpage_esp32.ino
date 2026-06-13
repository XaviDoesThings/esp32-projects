#include <WiFi.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

// Replace with your network credentials
const char* ssid = "MYWIFISSID"; //change this to your wifi
const char* password = "MYPASSWORD"; //change this to your password

// DHT11 Sensor setup
#define DHTPIN 4          // Pin where the DHT11 is connected
#define DHTTYPE DHT11     // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// Create a web server on port 80
WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Start the server
  server.begin();
}

void loop() {
  WiFiClient client = server.available(); // Listen for incoming clients

  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          // If the current line is blank, you got two newline characters in a row.
          // That's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // Read temperature and humidity
            float h = dht.readHumidity();
            float t = dht.readTemperature();

            // Create the HTML response
            String response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: text/html\r\n";
            response += "Connection: close\r\n\r\n";
            response += "<!DOCTYPE HTML><html>";
            response += "<head>";
            response += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
            response += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
            response += "<style>";
            response += "body { font-family: Arial, sans-serif; background-color: #f4f4f4; text-align: center; }";
            response += "h1 { color: #333; }";
            response += ".container { background-color: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); display: inline-block; }";
            response += ".gauge { width: 300px; height: 300px; margin: 20px auto; }";
            response += ".value { font-size: 24px; font-weight: bold; margin: 10px 0; }";
            response += ".footer { margin-top: 20px; font-size: 14px; color: #777; }";
            response += "</style>";
            response += "</head>";
            response += "<body>";
            response += "<div class='container'>";
            response += "<h1>DHT11 Sensor Data</h1>";
            response += "<div class='gauge'><canvas id='tempGauge'></canvas></div>";
            response += "<div class='value' style='color: red;'>" + String(t) + " C</div>";
            response += "<div class='gauge'><canvas id='humidityGauge'></canvas></div>";
            response += "<div class='value' style='color: blue;'>" + String(h) + " %</div>";
            response += "</div>";
            response += "<script>";
            response += "function createGauge(canvasId, value, maxValue, label, color) {";
            response += "  var ctx = document.getElementById(canvasId).getContext('2d');";
            response += "  var gauge = new Chart(ctx, {";
            response += "    type: 'doughnut',";
            response += "    data: {";
            response += "      datasets: [{";
            response += "        data: [value, maxValue - value],";
            response += "        backgroundColor: [color, '#E0E0E0'],";
            response += "        borderWidth: 0";
            response += "      }],";
            response += "      labels: [label, '']";
            response += "    },";
            response += "    options: {";
            response += "      responsive: true,";
            response += "      cutoutPercentage: 70,";
            response += "      legend: { display: false },";
            response += "      tooltips: { enabled: false }";
            response += "    }";
            response += "  });";
            response += "}";
            response += "createGauge('tempGauge', " + String(t) + ", 50, 'Temperature (C)', 'red');";
            response += "createGauge('humidityGauge', " + String(h) + ", 100, 'Humidity (%)', 'blue');";
            response += "</script>";
            response += "</body>";
            response += "</html>";

            client.println(response);
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    delay(1);
    client.stop();
  }
}
