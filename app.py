from flask import Flask, render_template
from flask_socketio import SocketIO
import paho.mqtt.client as mqtt
import json

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

MQTT_BROKER = "broker.hivemq.com"

# ---------------- MQTT ----------------
def on_message(client, userdata, msg):
    data = msg.payload.decode()
    print("MQTT:", str(data))


    try:
        data = json.loads(data)
    except:
        pass

    socketio.emit(msg.topic, data)

client = mqtt.Client()
client.on_message = on_message
client.connect(MQTT_BROKER, 1883)

client.subscribe("iot/vehicle/live")
client.subscribe("iot/vehicle/data")
client.subscribe("iot/vehicle/binlevel")

client.loop_start()

# ---------------- ROUTE ----------------
@app.route("/")
def home():
    return render_template("index.html")

# ---------------- RUN ----------------
if __name__ == "__main__":
    socketio.run(app, debug=True)

