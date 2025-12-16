#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include "DHT.h"

#define DHTPIN 18
#define DHTTYPE DHT22

#define LED_ROJO 13
#define LED_VERDE 14
#define LED_AZUL 27
#define BUZZER 16
#define SERVO_PIN 17

const float TEMP_UMBRAL_VENTILACION = 30.0;
const float TEMP_UMBRAL_ALERTA = 37.0;
const float HUMEDAD_MINIMA = 40.0;

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

const char* mqtt_topic_alertas = "/iot/alertas";
const char* mqtt_topic_datos = "/iot/datos";
const char* mqtt_topic_control = "/iot/control/ventilador";
const char* mqtt_topic_estado = "/iot/estado/ventilador";

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo ventilador;

bool ventilador_manual = false;
int estado_ventilador = 0;
String ultimo_estado_publicado = "";

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado.");
}

void publicar_estado_ventilador() {
  String estado = (estado_ventilador > 0) ? "ON" : "OFF";
  if (estado != ultimo_estado_publicado) {
    client.publish(mqtt_topic_estado, estado.c_str());
    ultimo_estado_publicado = estado;
    Serial.print("Estado ventilador publicado: ");
    Serial.println(estado);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String mensaje;
  for (int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }

  Serial.printf("Mensaje recibido en %s: %s\n", topic, mensaje.c_str());

  if (String(topic) == mqtt_topic_control) {
    ventilador_manual = true;

    if (mensaje == "ENCENDER") {
      ventilador.write(180);
      estado_ventilador = 180;
      digitalWrite(LED_VERDE, HIGH);
      Serial.println("Ventilador encendido por MQTT");
      publicar_estado_ventilador();
    } else if (mensaje == "APAGAR") {
      ventilador.write(0);
      estado_ventilador = 0;
      digitalWrite(LED_VERDE, LOW);
      Serial.println("Ventilador apagado por MQTT");
      publicar_estado_ventilador();
    }
  }

  if (String(topic) == mqtt_topic_alertas) {
    tone(BUZZER, 1000, 800);
    digitalWrite(LED_ROJO, HIGH);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alerta MQTT:");
    lcd.setCursor(0, 1);
    lcd.print(mensaje.substring(0, 16));
    delay(3000);
    lcd.clear();
    digitalWrite(LED_ROJO, LOW);
  }
}

void reconnect() {
  while (!client.connected()) {
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT conectado");
      client.subscribe(mqtt_topic_alertas);
      client.subscribe(mqtt_topic_control);
    } else {
      Serial.print("Fallo MQTT, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  lcd.init();
  lcd.backlight();
  ventilador.attach(SERVO_PIN);

  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  lcd.setCursor(0, 0);
  lcd.print("Supervision IoT");
  delay(1500);
  lcd.clear();

  publicar_estado_ventilador();
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  Serial.printf("Temp: %.1f°C, Hum: %.1f%%\n", temp, hum);

  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(temp, 1); lcd.print((char)223); lcd.print("C ");
  lcd.print("H:"); lcd.print(hum, 0); lcd.print("%");

  digitalWrite(LED_ROJO, LOW);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AZUL, LOW);
  noTone(BUZZER);

  if (!ventilador_manual) {
    if (temp < TEMP_UMBRAL_VENTILACION && hum >= HUMEDAD_MINIMA) {
      digitalWrite(LED_VERDE, HIGH);
      ventilador.write(0);
      estado_ventilador = 0;
      publicar_estado_ventilador();
    } else if (temp >= TEMP_UMBRAL_VENTILACION && temp < TEMP_UMBRAL_ALERTA) {
      digitalWrite(LED_AZUL, HIGH);
      ventilador.write(90);
      estado_ventilador = 90;
      publicar_estado_ventilador();
    } else if (temp >= TEMP_UMBRAL_ALERTA) {
      digitalWrite(LED_ROJO, HIGH);
      tone(BUZZER, 1500);
      ventilador.write(180);
      estado_ventilador = 180;
      publicar_estado_ventilador();

      String alerta = String("ALERTA: ") + temp + "C, " + hum + "%";
      client.publish(mqtt_topic_alertas, alerta.c_str());
    }
  } else {
    ventilador.write(estado_ventilador);
  }

  String payload = String("{\"temp\":") + temp + ",\"hum\":" + hum + "}";
  client.publish(mqtt_topic_datos, payload.c_str());

  delay(3000);
}
