import pymongo
from flask import Flask
from flask import make_response
from config import *
from bson.json_util import dumps

app = Flask(__name__)
connection, dbname = db_config('config.d/server.cfg')
db = connection[dbname]

@app.route('/')
def home_index():
	nodes = db.find()
	nodes_list = dumps(node)
	return render_template('index_node.html',node=nodes_list)

@app.route('/sensor/<node>',methods=['POST','PUT'])
def write_node():
	if request.method == 'POST':
		return True
	elif request.method == 'PUT':
		return True
	return False

@app.route('/sensor/<node>',method=['GET'])
def read_node():
	return True

@app.route('/temperature',method=['GET'])
def get_temperature():
	pass

@app.route('/humidity',method=['GET'])
def get_humidity():
        pass

@app.route('/lightintensity',method=['GET'])
def get_light_intensity():
        pass

@app.route('/carbondioxide',method=['GET'])
def get_CO2():
        pass

def config_node():
	pass

def view_node():
	pass

def query_node():
	pass

def store_data():
	pass

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000, debug=True)

