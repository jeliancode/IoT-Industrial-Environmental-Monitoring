from flask import Blueprint, request, jsonify
from controllers.control_controller import active_control

control_bp = Blueprint('control', __name__)

@control_bp.route('/active-fan', methods=['POST'])
def active():
    result = active_control("ENCENDER")
    return jsonify({"estado": result})

@control_bp.route('/deactivate-fan', methods=['POST'])
def deactivate():
    result = active_control("APAGAR")
    return jsonify({"estado": result})
