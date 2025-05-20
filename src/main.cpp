#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESP32Servo.h>

// Wi-Fi AP настройки
const char* ssid = "ESP32_SERVO_AP";
const char* password = "12345678";

// WebSocket сервер
WebSocketsServer webSocket = WebSocketsServer(81);

// Пины для сервоприводов
const int SERVO1_PIN = 13;
const int SERVO2_PIN = 12;
const int SERVO3_PIN = 14;

// Объекты сервоприводов
Servo servo1;
Servo servo2;
Servo servo3;

// ===== Получение серво по ID =====
Servo* getServoById(int id) {
  switch (id) {
    case 1: return &servo1;
    case 2: return &servo2;
    case 3: return &servo3;
    default: return nullptr;
  }
}

// ===== Установка угла серво =====
void setServoAngle(int servoId, float angle) {
  Serial.printf("[SIMULATION] M%d -> %.1f degrees\n", servoId, angle);

  // Раскомментируй при подключении серво:
  // Servo* servo = getServoById(servoId);
  // if (servo != nullptr) {
  //   servo->write(angle);
  // }
}

// ===== Обработка команды (M1:90.0) =====
void handleCommand(const String& cmd, uint8_t client_id) {
  if (!cmd.startsWith("M")) return;

  int colonIndex = cmd.indexOf(':');
  if (colonIndex <= 1) return;

  int servoId = cmd.substring(1, colonIndex).toInt();
  float angle = cmd.substring(colonIndex + 1).toFloat();

  if (angle < 0 || angle > 180) {
    webSocket.sendTXT(client_id, "ESP:ERR:M" + String(servoId) + " (invalid angle)");
    return;
  }

  setServoAngle(servoId, angle);
  webSocket.sendTXT(client_id, "ESP:OK:M" + String(servoId) + ":" + String(angle));
}

// ===== Обработка WebSocket =====
void onWebSocketEvent(uint8_t client_id, WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_TEXT) {
    String message = String((char*)payload);
    Serial.println("Received: " + message);

    int index = 0;
    while (index < message.length()) {
      int nextComma = message.indexOf(',', index);
      String cmd = (nextComma == -1) ? message.substring(index) : message.substring(index, nextComma);
      cmd.trim();
      index = (nextComma == -1) ? message.length() : nextComma + 1;
      handleCommand(cmd, client_id);
    }
  }
}

// ===== setup() =====
void setup() {
  Serial.begin(115200);

  // Запуск Wi-Fi в режиме AP
  WiFi.softAP(ssid, password);
  Serial.println("Access Point started");
  Serial.println("IP address: " + WiFi.softAPIP().toString());

  // Привязка сервоприводов (если используешь)
  // servo1.attach(SERVO1_PIN);
  // servo2.attach(SERVO2_PIN);
  // servo3.attach(SERVO3_PIN);

  // Инициализация WebSocket
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

// ===== loop() =====
void loop() {
  webSocket.loop();
}
