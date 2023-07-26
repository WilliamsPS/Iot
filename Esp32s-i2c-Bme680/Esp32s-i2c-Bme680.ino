#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme;

const char* ssid = "WILLIAMS";
const char* password = "williams@7363";
const char* serverURL = "http://192.168.1.35:7896/iot/json?k=4jggokgpepnvsb2uv4s40d59ov&i=thing003";

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println(F("Prueba asíncrona de BME680"));

  if (!bme.begin()) {
    Serial.println(F("No se pudo encontrar un sensor BME680 válido, ¡verifique la conexión!"));
    while (1);
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi!");
}

void loop() {
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    Serial.println(F("Error al comenzar la lectura :("));
    return;
  }

  delay(50);

  if (!bme.endReading()) {
    Serial.println(F("Error al completar la lectura :("));
    return;
  }

  float temperatura = bme.temperature;
  float presion = bme.pressure / 100.0;
  float humedad = bme.humidity;
  float resistenciaGas = bme.gas_resistance / 1000.0;
  float altitud = bme.readAltitude(SEALEVELPRESSURE_HPA);

  String payload = "{\"t\":" + String(temperatura) + ",";
  payload += "\"p\":" + String(presion) + ",";
  payload += "\"h\":" + String(humedad) + ",";
  payload += "\"gs\":" + String(resistenciaGas) + ",";
  payload += "\"alt\":" + String(altitud) + "}";

  Serial.print(payload);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    int codigoRespuestaHTTP = http.POST(payload);

    if (codigoRespuestaHTTP > 0) {
      Serial.print("Código de respuesta HTTP: ");
      Serial.println(codigoRespuestaHTTP);
      String respuesta = http.getString();
      Serial.println(respuesta);
    } else {
      Serial.print("Error al enviar la solicitud: ");
      Serial.println(codigoRespuestaHTTP);
    }

    http.end();
  } else {
    Serial.println("Desconectado de WiFi");
  }

  delay(5000);
}
