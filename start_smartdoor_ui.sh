source /home/shinjan/virtualenv/smart-door/bin/activate
# nohup mosquitto -c /home/shinjan/Programs/mqtt/mosquitto.conf &
export FLASK_APP=/home/shinjan/Workspaces/murata/smart-door-v3/Server/server.py
export FLASK_DEBUG=1
cd /home/shinjan/Workspaces/murata/smart-door-v3/Server/
flask run --host=0.0.0.0
