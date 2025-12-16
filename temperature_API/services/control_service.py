from mqtt_client import mqtt_publish
from models.event_model import save_event

class ControlService:
    def active_fan(self, mensaje: str) -> str:
        mqtt_publish(mensaje)
        save_event("ventilador", mensaje)
        return {"status": "ventilador activado", "mensaje": mensaje}