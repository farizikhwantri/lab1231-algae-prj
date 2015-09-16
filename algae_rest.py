import pymongo
from flask import Flask
from flask import make_response
from config import *

app = Flask(__name__)
db = db_config('config.d/server.cfg')

@app.route('/')
def home_index():
	return render_template('index_node.html')

@app.route('/sensor/<node>',methods=['POST','PUT'])
def write_data():
	if request.method == 'POST':
		return True
	elif request.method == 'PUT':
		return True
	return False

@app.route('/sensor/<node>',method=['GET'])
def read_node():
	return True

def send_to_node():
	return True

def view_node():
	return True:

def query_node():
	return True:

def store_data():
	return True:

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=5000, debug=True)

