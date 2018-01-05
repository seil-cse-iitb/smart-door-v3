from flask import Flask
from flask import render_template,jsonify
from models import *
import pandas as pd
from sqlalchemy.ext.serializer import loads, dumps
import paho.mqtt.client as mqtt
from svm import *
import datetime
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
try:
	train_model()
except Exception as e:
	print(e)
training ={'status':False,'id':None}

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


@app.route('/api/training/off')
def training_off():
	training['status'] = False
	training['id'] = None
	return "Training mode OFF"

@app.route('/api/training/<id>')
def training_on(id):
	training['status'] = True
	training['id'] = id
	return "Training mode ON"

@app.route('/api/retrain')
def retrain():
	try:
		train_model()
		return "Retrained"
	except Exception as e:
		print(e)
		return "Something went wrong"

@app.route('/api/prediction/<height>/<weight>/<steps>/<direction>')
def prediction(height,weight,steps,direction):
	# print(training)
	if training['status']: #training mode
		pd.DataFrame([[training['id'],height,weight,steps]]).to_csv('./data/train_data.csv',mode='a',header = False,index = False)
		return "added to training data"
	else: # prediction mode
		record = [float(height),float(weight),int(steps)]
		predicted_id = predict(record)
		occupant = User.query.get(int(predicted_id))
		if direction == 'entry':
			occupant.occupancy_status = OccupancyEnum.present
		else:
			occupant.occupancy_status = OccupancyEnum.absent

		db.session.add(Record(date=datetime.datetime.now(),height=height,weight=weight,predicted_user_id=predicted_id,steps=steps,direction=direction))
		db.session.commit()

		mqttc.publish("smartdoor/data/"+direction, str((occupant.as_dict())))

		return "predicted" + str(occupant)


@app.route('/api/events/<direction>/<action>')
def events(direction,action):
	mqttc.publish("smartdoor/events/"+direction+"/"+action)
	return "published"