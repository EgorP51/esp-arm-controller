#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESP32Servo.h>  // Для работы с сервоприводом на ESP32

const char* ssid = "ESP32_SERVO_AP";
const char* password = "12345678";

const int servoPin = 13;  // Пин, к которому подключено серво
Servo servo1;  // Создаем объект для серво

WebSocketsServer webSocket = WebSocketsServer(81);  // WebSocket сервер на порту 81

// Переменная для текущего угла серво
int currentAngle = 0;

// WebSocket обработчик событий
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED: {
      Serial.printf("Client [%u] connected\n", num);
      break;
    }
    case WStype_TEXT: {
      String message = String((char*)payload);
      Serial.printf("Received from client: %s\n", message.c_str());

      // Если получена команда в формате "SET_SERVO:angle", перемещаем серво
      if (message.startsWith("SET_SERVO:")) {
        String angleStr = message.substring(10);  // Извлекаем угол после "SET_SERVO:"
        int angle = angleStr.toInt();  // Преобразуем строку в число
        if (angle >= 0 && angle <= 180) {
          servo1.write(angle);  // Перемещаем серво в заданный угол
          currentAngle = angle;  // Обновляем текущий угол
          webSocket.sendTXT(num, "Servo moved to angle: " + String(angle));
        } else {
          webSocket.sendTXT(num, "Invalid angle. Must be between 0 and 180.");
        }
      } else {
        webSocket.sendTXT(num, "Unknown command");
      }
      break;
    }
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  servo1.attach(servoPin);  // Подключаем серво к указанному пину

  WiFi.softAP(ssid, password);
  Serial.print("Access Point Started: ");
  Serial.println(ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  webSocket.loop();  // Обработка событий WebSocket

  // Здесь больше нет постоянного цикла движения серво
  // Мы будем двигать серво только по команде через WebSocket
}
