import paho.mqtt.publish as publish

def mqtt_publish(mensaje, topic="/iot/control/ventilador"):
    publish.single(
        topic=topic,
        payload=mensaje,
        hostname="broker.hivemq.com",
        port=8883,
    )
