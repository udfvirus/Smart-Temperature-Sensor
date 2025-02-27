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

const char* serverUrl = "http://example.com/api/data";

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

      // sendJsonData(temperature, humidity);
    } else {
      Serial.println("Ошибка чтения DHT11");
    }


  } else {
    Serial.println("Wi-Fi отключен, жду...");
  }

  delay(10000);  // Запрос каждые 10 секунд
}

void getJsonData() {
  WiFiClient client;
  HTTPClient http;

  Serial.println("Отправка запроса к серверу...");

  http.begin(client, serverUrl);  // Подключение к серверу
  int httpCode = http.GET();      // Выполняем GET-запрос

  if (httpCode > 0) {  // Проверяем успешность запроса
    Serial.print("Ответ сервера: ");
    Serial.println(httpCode);

    if (httpCode == HTTP_CODE_OK) {  // 200 OK
      String payload = http.getString();
      Serial.println("Полученные данные:");
      Serial.println(payload);

      // Парсим JSON (если сервер отправляет JSON)
      DynamicJsonDocument doc(512);  // Размер буфера (увеличить при сложных JSON)
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        const char* command = doc["command"];  // Пример ключа "command"
        Serial.print("Команда от сервера: ");
        Serial.println(command);
      } else {
        Serial.println("Ошибка парсинга JSON!");
      }
    }
  } else {
    Serial.print("Ошибка запроса: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }

  http.end();  // Завершаем соединение
}