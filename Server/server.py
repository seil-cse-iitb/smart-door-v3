from flask import Flask
from flask import render_template,jsonify
from models import *
from sqlalchemy.ext.serializer import loads, dumps
import paho.mqtt.client as mqtt
# app = Flask(__name__)



# The callback for when the mqttc receives a CONNACK response from the server.
def on_connect(mqttc, userdata, flags, rc):
	print("Connected to MQTT broker with result code "+str(rc))
# The callback for when a PUBLISH message is received from the server.
def on_message(mqttc, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
mqttc = mqtt.Client()
mqttc.on_connect = on_connect
mqttc.on_message = on_message

mqttc.connect("localhost", 1883, 60)
mqttc.loop_start()


@app.route('/')
def index():
    return render_template('index.html', name="Smart Door v3")

@app.route('/api/occupants/')
def occupants():
	occupants = User.query.all()
	people = [{'name':"Sapan"},{'name':"Shaunak"},{'name':"Shinjan"}]
	# list_of_dict = [{i} for i in range(5)]
	occupants = [i.as_dict() for i in occupants]
	return jsonify(occupants)

@app.route('/api/occupants/update')
def occupancy_update():
	occupants = User.query.all()
	# people = [{'name':"Sapan"},{'name':"Shaunak"},{'name':"Shinjan"}]
	mqttc.publish("smartdoor/events", str(occupants[0].as_dict()))
	# list_of_dict = [{i} for i in range(5)]
	occupants = [i.as_dict() for i in occupants]
	return jsonify(occupants)


