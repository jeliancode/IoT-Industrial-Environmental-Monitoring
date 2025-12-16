from flask import Flask
from routes.control_routes import control_bp
from database import init_db

app = Flask(__name__)
app.register_blueprint(control_bp)

if __name__ == '__main__':
    init_db()
    app.run(debug=True)
