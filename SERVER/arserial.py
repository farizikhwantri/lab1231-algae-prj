import serial
import os
import json

def receive(self, ser):
	global last_received
	buffer = ''
	while True:
		try:
			temp = ser.readline()
			json_object = json.loads(temp)			
		except ValueError, e:
			print("Invalid JSON")
		sleep(10)

def receive_last_line(self, ser):
	global last_received
	data = ["","",""]
        buffer = ''
	while True:
		buffer = buffer + ser.read()
		if '\r\n' in buffer:
			splitbuffer = buffer.split('\r\n')
			buffer = splitbuffer[1]
			while True:
			buffer = buffer + ser.read()
			if '\r\n' in buffer:
				data[0] = buffer
				# splitbuffer = buffer.split('\r\n')
				# buffer = splitbuffer[1] 
				matchTemperature = re.match( r'(.*)Temperature: (.*) \*C', buffer, re.M|re.I)
				if matchTemperature:
					data[1] = matchTemperature.group(2)
				matchHumidity = re.match( r'(.*)Humidity: (.*) %', buffer, re.M|re.Iif matchHumidity:
                            data[2] = matchHumidity.group(2)                            
                        return data
