#include <Arduino.h>  // Включення основної бібліотеки Arduino
#include <ArduinoJson.h>  // Включення бібліотеки для роботи з JSON
#include <Adafruit_Sensor.h>  // Включення бібліотеки для датчиків Adafruit
#include <Adafruit_HTU21DF.h>  // Включення бібліотеки для датчика вологості та температури HTU21D
#include <Adafruit_BMP280.h>  // Включення бібліотеки для датчика тиску та температури BMP280
#include <Wire.h>  // Включення бібліотеки для роботи з шиной I2C
#include <LiquidCrystal_I2C.h>  // Включення бібліотеки для роботи з LCD через I2C
#include <ESP8266WiFi.h>  // Включення бібліотеки для роботи з Wi-Fi на ESP8266
#include <FirebaseESP8266.h>  // Включення бібліотеки для роботи з Firebase на ESP8266
#include <EEPROM.h>  // Включення бібліотеки для роботи з EEPROM
#include <NTPClient.h>  // Включення бібліотеки для роботи з NTP сервером
#include <WiFiUdp.h>  // Включення бібліотеки для роботи з UDP
#include <addons/TokenHelper.h>  // Включення додаткової бібліотеки для TokenHelper
#include <addons/RTDBHelper.h>  // Включення додаткової бібліотеки для RTDBHelper

// Оголошення змінних та об'єктів для підключення до мережі Wi-Fi та Firebase
const char* ssid = "INF-LAN";  // Ім'я мережі Wi-Fi
const char* password = "Rezerv-Dannyh36";  // Пароль для мережі Wi-Fi
WiFiServer server(5653);  // Створення об'єкту сервера на порту 5653
WiFiClient client;  // Створення об'єкту клієнта для підключення

#define WIFI_SSID "INF-LAN"  // Ім'я мережі Wi-Fi
#define WIFI_PASSWORD "Rezerv-Dannyh36"  // Пароль для мережі Wi-Fi
#define API_KEY "AIzaSyDL7SVHzV2y19O6nuslXbvCQUJFiXftzUw"  // API ключ Firebase
#define DATABASE_URL "https://diplomarduin-default-rtdb.firebaseio.com"  // URL адреса бази даних Firebase
#define USER_EMAIL "kotovnikitos71@gmail.com"  // Електронна адреса користувача Firebase
#define USER_PASSWORD "Nikitos_1401"  // Пароль користувача Firebase
FirebaseData fbdo;  // Об'єкт для роботи з Firebase
FirebaseAuth auth;  // Об'єкт аутентифікації Firebase
FirebaseConfig config;  // Конфігурація Firebase
String databasePath1, databasePath2, databasePath3, databasePath4;  // Шляхи до різних даних в Firebase
String tempPath = "/temperature";  // Шлях до даних температури
String humPath = "/humidity";  // Шлях до даних вологості
String presPath = "/pressure";  // Шлях до даних тиску
String luxPath = "/lux";  // Шлях до даних освітленості
String parentPath1, parentPath2, parentPath3, parentPath4;  // Батьківські шляхи до даних в Firebase
FirebaseJson json1, json2, json3, json4;  // JSON об'єкти для Firebase
unsigned long sendDataPrevMillis = 0;  // Змінна для попереднього часу відправки даних
WiFiUDP ntpUDP;  // UDP об'єкт для роботи з NTP сервером
NTPClient timeClient(ntpUDP, "pool.ntp.org");  // Клієнт для отримання часу від NTP сервера
const byte HTU21D_I2C_ADDR = 0x40;  // Адреса датчика HTU21D на шині I2C
const byte BMP280_I2C_ADDR = 0x76;  // Адреса датчика BMP280 на шині I2C
Adafruit_HTU21DF htu21d = Adafruit_HTU21DF();  // Об'єкт датчика HTU21D
Adafruit_BMP280 bmp280;  // Об'єкт датчика BMP280
int sensorPin = A0;  // Пін для зчитування аналогового сигналу вводу
IPAddress ipAddress;  // IP адреса
LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD об'єкт для роботи з дисплеєм

#if defined(ARDUINO_RASPBERRY_PI_PICO_W)
WiFiMulti multi;  // Об'єкт для роботи з Wi-Fi на Raspberry Pi Pico
#endif
void setup() {
  Wire.begin();  // Ініціалізація шини I2C
  Serial.begin(9600);  // Ініціалізація послідовного порту з швидкістю 9600 бод
  Serial.println("HTU21D, BMP280, TEMT6000 test");  // Вивід рядка на послідовний порт

  // Перевірка наявності датчиків HTU21D і BMP280
  if (!htu21d.begin()) {
    Serial.println("Couldn't find HTU21D sensor!");  // Вивід помилки у разі відсутності датчика HTU21D
    while (1); // Зациклення, у разі відсутності датчика
  }
  if (!bmp280.begin(BMP280_I2C_ADDR)) {
    Serial.println("Couldn't find BMP280 sensor!");  // Вивід помилки у разі відсутності датчика BMP280
    while (1); // Зациклення, у разі відсутності датчика
  }

  lcd.init(); // Ініціалізація LCD дисплею
  lcd.backlight(); // Вмикаємо підсвічування LCD дисплею
  lcd.begin(16, 2); // Запускаємо роботу дисплею розміром 16x2 символи

  lcd.setCursor(0, 0); // Встановлюємо позицію курсору на рядку 0, колонці 0
  lcd.print("Connecting..."); // Виводимо текст "Connecting..." на LCD дисплей
  Wire.begin(); // Ініціалізація шини I2C
  Serial.begin(9600); // Ініціалізація з'єднання через порт Serial з швидкістю 9600 бод
  Serial.println("HTU21D, BMP280, TEMT6000 test"); // Виводимо повідомлення про тестування датчиків на порт Serial


  // Повторна перевірка наявності датчиків HTU21D і BMP280 після повторної ініціалізації
  if (!htu21d.begin()) {
    Serial.println("Couldn't find HTU21D sensor!");  // Вивід помилки у разі відсутності датчика HTU21D
    while (1); // Зациклення, у разі відсутності датчика
  }
  if (!bmp280.begin(BMP280_I2C_ADDR)) {
    Serial.println("Couldn't find BMP280 sensor!");  // Вивід помилки у разі відсутності датчика BMP280
    while (1); // Зациклення, у разі відсутності датчика
  }

  lcd.init(); // Ініціалізація LCD дисплею
  lcd.backlight(); // Вмикаємо підсвічування LCD дисплею
  lcd.begin(16, 2); // Запускаємо роботу дисплею розміром 16x2 символи

  lcd.setCursor(0, 0); // Встановлюємо позицію курсору на рядку 0, колонці 0
  lcd.print("Connecting..."); // Виводимо текст "Connecting..." на LCD дисплей

  Serial.begin(115200); // Ініціалізація з'єднання через порт Serial з швидкістю 115200 бод

  #if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  multi.addAP(WIFI_SSID, WIFI_PASSWORD); // Додавання Wi-Fi мережі
  multi.run(); // Запуск підключення до Wi-Fi мережі
  #else
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Підключення до Wi-Fi мережі для інших плат
  #endif

  Serial.print("Connecting to Wi-Fi"); // Вивід повідомлення про спробу підключення до Wi-Fi мережі
  unsigned long ms = millis(); // Отримання поточного часу у мілісекундах

  // Очікування підключення до мережі Wi-Fi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("."); // Вивід крапки для візуалізації процесу очікування підключення
    delay(300); // Затримка 300 мілісекунд для наступної перевірки статусу підключення

  #if defined(ARDUINO_RASPBERRY_PI_PICO_W)
    // Перевірка часу очікування підключення 
    if (millis() - ms > 10000)
      break; // Вихід з циклу, якщо час очікування перевищений
  #endif
  }

  lcd.clear(); // Очищення вмісту LCD дисплею
  ipAddress = WiFi.localIP(); // Отримання IP адреси підключеного пристрою до Wi-Fi мережі
  lcd.setCursor(0, 0); // Встановлення позиції курсору на рядку 0, колонці 0
  lcd.print("IP Address:"); // Виведення тексту "IP Address:" на LCD дисплей
  lcd.setCursor(0, 1); // Встановлення позиції курсору на рядку 1, колонці 0
  lcd.print(ipAddress); // Виведення IP адреси на LCD дисплей
  delay(2000); // Затримка 2 секунди для відображення адреси перед виконанням наступних дій

  server.begin(); // Запуск HTTP сервера для обробки вхідних HTTP-запитів
  Serial.println("Server started"); // Вивід повідомлення про успішний запуск сервера

  // Налаштування конфігурації Firebase для використання в програмі
  config.api_key = API_KEY; // Встановлення API ключа для доступу до Firebase
  auth.user.email = USER_EMAIL; // Встановлення електронної пошти користувача Firebase
  auth.user.password = USER_PASSWORD; // Встановлення пароля користувача Firebase
  config.database_url = DATABASE_URL; // Встановлення URL адреси бази даних Firebase
  config.token_status_callback = tokenStatusCallback; // Встановлення зворотнього виклику для статусу токена

  #if defined(ARDUINO_RASPBERRY_PI_PICO_W)
  config.wifi.clearAP(); // Очищення списку доступних Wi-Fi мереж
  config.wifi.addAP(WIFI_SSID, WIFI_PASSWORD); // Додавання доступної Wi-Fi мережі
  #endif

  Firebase.begin(&config, &auth); // Ініціалізація та початок роботи з Firebase Realtime Database
  delay(1000); // Затримка на 1 секунду для стабілізації підключення
  Firebase.reconnectWiFi(true); // Повторне підключення до мережі Wi-Fi з увімкненою перепідключенням
  Firebase.setDoubleDigits(5); // Встановлення кількості цифр після десяткової коми у Firebase (5 цифр)
  delay(1000); // Затримка на 1 секунду перед продовженням виконання програми


  const int timeZone = 2;  // Встановлення часового поясу
  timeClient.begin();  // Початок роботи з NTP клієнтом
  databasePath1 = "/temperature";  // Шлях до данних температури у Firebase
  databasePath2 = "/humidity";  // Шлях до данних вологості у Firebase
  databasePath3 = "/pressure";  // Шлях до данних тиску у Firebase
  databasePath4 = "/lux";  // Шлях до данних освітленості у Firebase
}
// Функція getTime, яка повертає поточний час у форматі Unix Epoch
unsigned long getTime() {
  timeClient.update(); // Оновлюємо дані часу з сервера NTP
  unsigned long now = timeClient.getEpochTime(); // Отримуємо час у форматі Unix Epoch
  return now; // Повертаємо поточний час
}

// Функція isSummerTime, яка визначає, чи зараз літній час
bool isSummerTime(int day, int month, int hour) {
  if (month == 3) { // Якщо місяць березень
    int lastSunday = 31 - ((5 * day / 4 + 4) % 7); // Визначаємо день останньої неділі березня
    if (day > lastSunday || (day == lastSunday && hour >= 3)) {
      return true; // Повертаємо true, якщо день пізніше останньої неділі березня або рівний та година >= 3
    }
  } else if (month == 10) { // Якщо місяць жовтень
    int lastSunday = 31 - ((5 * day / 4 + 1) % 7); // Визначаємо день останньої неділі жовтня
    if (day > lastSunday || (day == lastSunday && hour < 3)) {
      return false; // Повертаємо false, якщо день пізніше останньої неділі жовтня або рівний та година < 3
    }
  }
  return false; // Повертаємо false за замовчуванням, якщо умови вище не виконані
}

void loop() {
  // Очікуємо підключення клієнта до сервера
  client = server.available();
  if (client) {
    // Виводимо повідомлення про нове підключення клієнта
    Serial.println("New client connected.");

    // Зчитуємо запит клієнта до моменту отримання символу '\r'
    String request = client.readStringUntil('\r');
    client.flush();

    // Перевіряємо, чи запит клієнта містить GET /data HTTP/1.1
    if (request.indexOf("GET /data HTTP/1.1") != -1) {
      // Отримуємо дані з датчиків температури, вологості, тиску та освітленості
      float temperature = htu21d.readTemperature();
      float humidity = htu21d.readHumidity();
      float pressure = bmp280.readPressure() / 100.0F;
      int sensorValue = analogRead(sensorPin);

      // Формуємо рядок JSON зі зчитаними даними
      String data = "{\"Temperature\":" + String(temperature) +
                    ",\"Humidity\":" + String(humidity) +
                    ",\"Pressure\":" + String(pressure) + 
                    ",\"Lux\":" + String(sensorValue) + "}";

      // Відправляємо дані клієнту
      client.print(data);

      // Виводимо повідомлення про відправку даних клієнту
      Serial.println("Sent sensor data to client.");
    }

    // Закриваємо підключення клієнта
    client.stop();
    Serial.println("Client disconnected.");
  }

  // Зчитуємо дані з датчиків та виводимо на LCD дисплей
  float temperature = htu21d.readTemperature();
  float humidity = htu21d.readHumidity();
  float pressure = bmp280.readPressure() / 100.0F;
  int sensorValue = analogRead(sensorPin);
  String data = "Temp: " + String(temperature) + "C  Hum: " + String(humidity) + "%" +
                " Press: " + String(pressure) + "hPa  Lux: " + String(sensorValue);

  // Очищаємо вміст LCD дисплею
  lcd.clear();
  
  // Виводимо дані температури, вологості, тиску та освітленості на LCD дисплей
  lcd.setCursor(0, 0);
  lcd.print("Temp: " + String(temperature) + "C  Hum: " + String(humidity) + "%");
  lcd.setCursor(0, 1);
  lcd.print("Press: " + String(pressure) + "hPa  Lux: " + String(sensorValue));
  
  // Якщо довжина рядка даних перевищує 16 символів, виконуємо прокрутку на LCD дисплеї
  if (data.length() > 16) {
    for (int i = 0; i < data.length() - 15; i++) {
      delay(750);
      lcd.scrollDisplayLeft();
    }
  }

  // Періодично відправляємо дані в Firebase Realtime Database
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    // Отримуємо поточний час від NTP сервера
    unsigned long currentTime = getTime(); // Отримання поточного часу в форматі Unix epoch

    char formattedTime[20]; // Масив символів для зберігання форматованого часу
    struct tm *timeinfo; // Структура, що містить інформацію про час
    time_t rawTime = (time_t)currentTime; // Перетворення поточного часу в тип time_t
    timeinfo = localtime(&rawTime); // Отримання локального часу з поточного часу

    int currentDay = timeinfo->tm_mday; // Отримання поточного дня місяця
    int currentMonth = timeinfo->tm_mon + 1; // Отримання поточного номера місяця
    int currentHour = timeinfo->tm_hour; // Отримання поточної години

    // Форматуємо отриманий час у відповідний формат для Firebase
    sprintf(formattedTime, "%04d:%02d;%02d;%02d:%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min);
    bool isSummer = isSummerTime(currentDay, currentMonth, currentHour);

    // Виправляємо час у випадку літнього часу
    if (isSummer) {
      sprintf(formattedTime, "%04d:%02d;%02d;%02d:%02d", timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour+1, timeinfo->tm_min);
    }

    // Формуємо шляхи до даних у Firebase
    String parentPath1 = databasePath1 + "/" + String(formattedTime);
    String parentPath2 = databasePath2 + "/" + String(formattedTime);
    String parentPath3 = databasePath3 + "/" + String(formattedTime);
    String parentPath4 = databasePath4 + "/" + String(formattedTime);

    // Відправляємо дані температури, вологості, тиску та освітленості в Firebase
    Firebase.RTDB.setString(&fbdo, parentPath1.c_str(), String(temperature).c_str());
    Firebase.RTDB.setString(&fbdo, parentPath2.c_str(), String(humidity).c_str());
    Firebase.RTDB.setString(&fbdo, parentPath3.c_str(), String(pressure).c_str());
    Firebase.RTDB.setString(&fbdo, parentPath4.c_str(), String(sensorValue).c_str());
  }

  delay(2000);  // Затримка для стабілізації роботи
}