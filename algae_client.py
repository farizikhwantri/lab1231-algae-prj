import requests
import json
import datetime
import time
import socket
import pymongo
import sys
import urlparse

from config import *
from crontab import CronTab

#url_test = "http://localhost:5000/node/"

def get_config(dest):
	request_url = urlparse.urljoin(dest,hostname)
	r = requests.get(request_url)
	res_stat = write_config(r.json)
	return res_stat.json

def post_data(postdata, dest):
	request_url = urlparse.urljoin(dest,hostname)
	r = requests.post(request_url,data=None,json=postdata)
	if r.Response == 200:
		print "OK"
		return TRUE
	elif r.Reponse == 401:
		print "Unauthorized"
		return FALSE
	else :
		return r.Response

def query_data():
	return True

def write_config(config):
	tab = CronTab(user='www',fake_tab='True')
	cmd = os.path.join(BASE_DIR,'env/bin/python ') + 
		os.path.join(BASE_DIR,'config.py')
	return True

def main(argv):
	url_destination = argv[1]
	cmd_action = argv[2]
	return True

if __name__ == "__main__":
	main(sys.argv) 
	
