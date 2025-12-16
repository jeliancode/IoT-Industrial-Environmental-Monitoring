from services.control_service import ControlService

def active_control(mensaje: str):
    service = ControlService()
    return service.active_fan(mensaje)

