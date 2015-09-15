import requests
import json
import datetime
import time
import socket
import sys

from crontab import CronTab

url_test = "http://localhost:80/conf/"
hostname = socket.gethostname()

def get_config(dest):
	request_url = dest + hostname
	r = requests.get(request_url)
	write_config(r.json)
	return r.json

def post_data(postdata, dest):
	request_url = dest + hostname
	r = requests.post(request_url,data=None,json=postdata)
	if r.Response == 200:
		print "OK"
	elif r.Reponse == 401:
		print "Unauthorized"
	return r.Response

def query_data():
	pass

def write_config(config):
	tab = CronTab(user='www',fake_tab='True')
	

def main(argv):
	url_destination = argv[1]
	cmd_action = argv[2]
	pass

if __name__ == "__main__":
	main(sys.argv) 
	
