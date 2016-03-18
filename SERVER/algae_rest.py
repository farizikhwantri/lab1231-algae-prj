import os, sys
import traceback
import time, sleep
import pymongo
from flask import Flask
from flask import make_response
from config import *
from bson.json_util import dumps

app = Flask(__name__)
connection, dbname = db_config('config.d/server.cfg')
db = connection[dbname]

CURRENTDIR = os.path.dirname(os.path.abspath(__file__))
if CURRENTDIR not in sys.path:
    sys.path.append(CURRENTDIR)
    
def wsErrorTextHTML(txt):
    ## displays the python error on the web page - useful for debugging
    ## but on a live system would have it save the error to a database and give the user an error number
    err = "<p>ERROR!</p>"
    err += "<pre>"+ txt + "</pre>"
    return err

@application.errorhandler(500)
def internalServerError(error): 
    err = "<p>ERROR! 500</p>"
    err += "<pre>"+ str(error) + "</pre>"
    err += "<pre>"+ str(traceback.format_exc()) + "</pre>"
    return err


@app.route('/')
def index():
	try:
		## ...your code goes here...
		#from arserial import *
		#incoming_request = algaeCore()
		htmlData = query_data()
		## output html code
		response = make_response(htmlData)
		response.headers['Content-Type'] = 'text/html'
		return response
	except:
		## error message
		response = make_response(wsErrorTextHTML(traceback.format_exc()))
		response.headers['Content-Type'] = 'text/html'
		return response
	response = make_response("unkown!")
	response.headers['Content-Type'] = 'text/html'
	return response

@app.route('/sensor/<node>',methods=['POST','PUT'])
def write_node():
	if request.method == 'POST':
		return True
	elif request.method == 'PUT':
		return True
	return False

@app.route('/sensor/<node>',method=['GET','POST'])
def read_node():
	return True

@app.route('/temperature',method=['GET','POST'])
def get_temperature():
	pass

@app.route('/humidity',method=['GET','POST'])
def get_humidity():
        pass

@app.route('/lightintensity',method=['GET','POST'])
def get_light_intensity():
        pass

@app.route('/carbondioxide',method=['GET','POST'])
def get_CO2():
        pass

def receiving(ser):
	global last_received
	temp = ''
	while True:
		try:
			temp = ser.readline()
			json_object = json.loads(temp)
		except ValueError, e:
			print("Invalid JSON")
		sleep(5)


def poll_data():
	pass

def config_node():
	pass

def view_node():
	pass

def query_node():
	pass

def query_data():
	pass

def store_data():
	pass

if __name__ == "__main__":
	serial_port = '/dev/ttyACM0'
	serial_baud = 96000
	ser = serial.Serial(serial_port, serial_baud)
	ser.flush()
	thr = Thread(target=receiving, args=(ser))
	thr.daemon = false
	thr.start()
	app.run(host='0.0.0.0', port=5000, debug=True)

