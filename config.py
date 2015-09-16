import os
from pymongo import MongoClient

BASE_DIR = os.path.abspath(os.path.dirname(__file__))
url_test = "http://localhost:5000/"
hostname = socket.gethostname()

def config_db(file_config):
	db_conf = ConfigParser.ConfigParser()
	db_conf.read(os.path.join(BASE_DIR,file_config))
	db_address = db_conf.get('mongodb','address')
	db_port = db_conf.get('mongodb','port')
	client = MongoClient(db_address,db_port)
	return client

def config_device(argv):
	return True

def config_schedule(argv):
	return True

if __name__ == "__main__":
	stat = config_db(argv)
