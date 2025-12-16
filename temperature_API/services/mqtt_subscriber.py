import paho.mqtt.client as mqtt
import json
from models.event_model import save_event

BROKER = "broker.hivemq.com"
PORT = 1883

TOPICO_ALERTAS = "/iot/alertas"
TOPICO_DATOS = "/iot/datos"

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Conectado al broker MQTT")
        client.subscribe(TOPICO_ALERTAS)
        client.subscribe(TOPICO_DATOS)
    else:
        print(f"Error al conectar: {rc}")

def on_message(client, userdata, msg):
    payload = msg.payload.decode()
    topic = msg.topic

    print(f"Mensaje recibido en {topic}: {payload}")

    if topic == TOPICO_ALERTAS:
        save_event("alerta", payload)
    elif topic == TOPICO_DATOS:
        try:
            data = json.loads(payload)
            temp = data.get("temp")
            hum = data.get("hum")
            mensaje = f"Temp: {temp}C - Hum: {hum}%"
            save_event("lectura", mensaje)
        except Exception as e:
            print(f"Error procesando JSON: {e}")

def start_subscriber():
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(BROKER, PORT, 60)
    client.loop_forever()

if __name__ == "__main__":
    start_subscriber()
