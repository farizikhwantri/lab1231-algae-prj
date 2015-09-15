# GrovePi+ & Grove Light Sensor
import signal
import time
import sys
import grovepi

# Connect the Grove Light Sensor to analog port A0
# SIG,NC,VCC,GND
light_sensor = 0
dht_sensor = 7

threshold = 10

grovepi.pinMode(light_sensor,"INPUT")
grovepi.pinMode(dht_sensor,"INPUT")

def run_light_sensor():
	light_sense = grovepi.analogRead(light_sensor)	
	resistance = (float)(1023 - light_sense) * 10 / light_sense
	if resistance > threshold:
		print("arrrgh light")
	#print "sensor_value = ", light_sense, " resistance =  ", resistance
	return light_sense, resistance


def run_temp_humid_sensor():
	[temp,hum] = grovepi.dht(dht_sensor,1)
	#print "temperature_value = ", temp, " humidity =  ", hum
	return temp, hum

def exit_gracefully(signum, frame):
	# restore the original signal handler as otherwise evil things will happen
	# in raw_input when CTRL+C is pressed, and our signal handler is not re-entrant
	signal.signal(signal.SIGINT, original_sigint)
	try:
		if raw_input("\nReally quit? (y/n)> ").lower().startswith('y'):
			sys.exit(1)
	except KeyboardInterrupt:
		print("Ok ok, quitting")
		sys.exit(1)
	# restore the exit gracefully handler here    
	signal.signal(signal.SIGINT, exit_gracefully)

if __name__ == '__main__':
	# store the original SIGINT handler
	original_sigint = signal.getsignal(signal.SIGINT)
	signal.signal(signal.SIGINT, exit_gracefully)
	while True:
		ls,r=run_light_sensor()
		t,h=run_temp_humid_sensor()
		print "light : ", ls, "resistance ", r
		print "temperature : ", t, "humidity ", h
		time.sleep(5)
