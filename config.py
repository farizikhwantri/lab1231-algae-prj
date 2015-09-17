import os
from bson.json_util import dumps
from pymongo import MongoClient

BASE_DIR = os.path.abspath(os.path.dirname(__file__))
url_test = "http://localhost:5000/"
hostname = socket.gethostname()

def config_db(file_config):
	db_conf = ConfigParser.ConfigParser()
	db_conf.read(os.path.join(BASE_DIR,file_config))
	db_address = db_conf.get('mongodb','address')
	db_port = db_conf.get('mongodb','port')
	db_name = db_conf.get('mongodb','dbname')
	client = MongoClient(db_address,db_port)
	return client, db_name

def config_device(file_config):
	nodes_conf = ConfigParser.ConfigParser()
	nodes_list = nodes_conf.sections()
	nodes = {}
	for node in nodes_list:
		for attr in nodes_conf.options(node):
			nodes[attr] = nodes_conf.get(node,attr)
	return dumps(nodes)

def config_schedule(argv):
	return True

if __name__ == "__main__":
	stat = config_db(argv)
