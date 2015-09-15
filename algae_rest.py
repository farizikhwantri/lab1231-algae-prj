import pymongo
from flask import Flask

app = Flask(__name__)

@app.route('/')
def home():
	pass

@app.route('/sensor',methods=['GET','POST'])
def node_request():
	if request.method == 'POST':
		send_to_node()
	else:
		view_node()
	pass

def view_node():
	pass

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=80, debug=True)

