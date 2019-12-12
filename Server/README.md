# Installation
* You should install the python dependencies using virtualenv. To install virtualenv follow this [tutorial]( https://gist.github.com/Geoyi/d9fab4f609e9f75941946be45000632b)
    * Create a new virtualenv somewhere outside the project directory with `virtualenv -p python3 smart-door-v3-venv`
    * Activate the virtualenv using `source smart-door-v3-venv/bin/activate`
* Install python dependencies
    * `cd smart-door-v3/Server`
    * `pip install -r requirements.txt`
* Install npm dependencies
    * `cd static/lib`
    * `npm install`
* Install a local mqtt broker. To install using docker:
    * `docker run -it -p 1883:1883 -p 9001:9001 eclipse-mosquitto`

# Running
* `source smart-door-v3-venv/bin/activate`
* `cd PROJECT-ROOT/Server`
* `export FLASK_APP=ABSOLUTE-PATH-TO-PROJECT-ROOT/Server/server.py`
* `export FLASK_DEBUG=1`
* `flask run --host=0.0.0.0`
* Always run the last command from inside the project file else csv file cannot be found