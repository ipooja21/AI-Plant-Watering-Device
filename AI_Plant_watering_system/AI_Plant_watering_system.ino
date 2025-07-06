#include <WiFi.h>
#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11
#define SOIL_MOISTURE_PIN 34
#define RELAY_PIN 27

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Pixel6a";
const char* password = "ShikhaR1";

WiFiServer server(80);

String getSensorData() {
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  String data = String(soilMoisture) + "," + String(temp) + "," + String(humidity);
  return data;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    client.flush();

    if (request.indexOf("/WATER=ON") != -1) {
      digitalWrite(RELAY_PIN, HIGH); // Turn pump ON
    } 
    if (request.indexOf("/WATER=OFF") != -1) {
      digitalWrite(RELAY_PIN, LOW); // Turn pump OFF
    }
    if (request.indexOf("/getData") != -1) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/plain");
      client.println();
      client.println(getSensorData());
      return;
    }

    // HTML Page
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();
    client.println("<!DOCTYPE html><html lang='en'><head>");
    client.println("<meta charset='UTF-8'>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
    client.println("<title>Plant Watering System</title>");
    client.println("<style>");
    client.println("body { font-family: Arial, sans-serif; text-align: center; background: #f4f4f4; padding: 20px; }");
    client.println(".container { max-width: 400px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }");
    client.println("h1 { color: #333; }");
    client.println("p { font-size: 18px; }");
    client.println(".button { padding: 10px 20px; font-size: 16px; margin: 5px; border: none; cursor: pointer; border-radius: 5px; }");
    client.println(".on { background-color: #28a745; color: white; }");
    client.println(".off { background-color: #dc3545; color: white; }");
    client.println("</style></head><body>");
    client.println("<div class='container'>");
    client.println("<h1>Plant Watering System</h1>");
    client.println("<p><strong>Soil Moisture:</strong> <span id='soilMoisture'>Loading...</span></p>");
    client.println("<p><strong>Temperature:</strong> <span id='temperature'>Loading...</span> °C</p>");
    client.println("<p><strong>Humidity:</strong> <span id='humidity'>Loading...</span> %</p>");
    client.println("<a href='/WATER=ON'><button class='button on'>Water ON</button></a>");
    client.println("<a href='/WATER=OFF'><button class='button off'>Water OFF</button></a>");
    client.println("</div>");

    // JavaScript for real-time data updates
    client.println("<script>");
    client.println("setInterval(function() {");
    client.println("  fetch('/getData').then(response => response.text()).then(data => {");
    client.println("    const values = data.split(',');");
    client.println("    document.getElementById('soilMoisture').innerText = values[0];");
    client.println("    document.getElementById('temperature').innerText = values[1];");
    client.println("    document.getElementById('humidity').innerText = values[2];");
    client.println("  });");
    client.println("}, 1000);");
    client.println("</script>");

    client.println("</body></html>");
    client.println();
  }
}
