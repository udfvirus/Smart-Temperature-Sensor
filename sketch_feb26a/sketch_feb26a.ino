#include <ESP8266WiFi.h>
#include <WiFiManager.h>

#include <ESP8266HTTPClient.h>  // Для HTTP-запросов
#include <ArduinoJson.h>        // Для парсинга JSON (установите через библиотечный менеджер)

#include <DHT.h>


#define RESET_BUTTON_PIN 0  // Кнопка на GPIO0 (подключите к GND)

#define DHTPIN 2  // DHT11 подключен к GPIO2
#define DHTTYPE DHT11

WiFiManager wifiManager;
DHT dht(DHTPIN, DHTTYPE);

const char* serverUrl = "http://smart.udfsoft.com/api/sensor/temperature/add";

const char* deviceID = "d9a31cde-bf03-4fd8-a9c6-8a123b6e2ad7";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

  dht.begin();

  // Устанавливаем таймаут 30 секунд на подключение к Wi-Fi
  wifiManager.setTimeout(30);

  // Автоматическая настройка Wi-Fi
  if (wifiManager.autoConnect("ESP_Config")) {
    Serial.println("Не удалось подключиться к Wi-Fi, запущен режим настройки...");
  } else {
    Serial.println("Wi-Fi подключен!");

    Serial.print("IP-адрес: ");
    Serial.println(WiFi.localIP());

    // Вывод имени сети (SSID)
    Serial.print("Подключено к сети: ");
    Serial.println(WiFi.SSID());
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  if (digitalRead(RESET_BUTTON_PIN) == LOW) {
    Serial.println("Кнопка нажата! Перезагрузка...");
    wifiManager.resetSettings();
    ESP.restart();
  }

  if (WiFi.status() == WL_CONNECTED) {
    // getJsonData();  // Запрос к серверу

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    if (!isnan(temperature) && !isnan(humidity)) {
      Serial.print("Температура: ");
      Serial.print(temperature);
      Serial.print(" °C | Влажность: ");
      Serial.print(humidity);
      Serial.println(" %");

      sendJsonData(temperature, humidity);
    } else {
      Serial.println("Ошибка чтения DHT11");
    }
    
  } else {
    Serial.println("Wi-Fi отключен, жду...");
  }

  delay(10000);  // Запрос каждые 10 секунд
}

void sendJsonData(float temp, float hum) {
    WiFiClient client;
    HTTPClient http;
    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-DEVICE-ID", deviceID);

    String jsonString = "{\"temperature\": " + String(temp) + ", \"humidity\": " + String(hum) + "}";
    int httpCode = http.POST(jsonString);

    Serial.print("Ответ сервера: ");
    Serial.println(httpCode);
    http.end();
}