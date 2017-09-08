import RPi.GPIO as GPIO
import time
 
class Ultrasonic:
	def __init__(self, id, trigger_pin, echo_pin, motion_detection_limit):
		self.ID = id
		self.TRIGGER_PIN = trigger_pin
		self.ECHO_PIN = echo_pin
		self.pulse_start=0
		self.pulse_end=0
		self.pulse_duration=0
		self.echo_state=1
		self.MOTION_DETECTION_LIMIT = motion_detection_limit

		GPIO.setup(self.TRIGGER_PIN,GPIO.OUT)
		GPIO.setup(self.ECHO_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP) 
		GPIO.add_event_detect(self.ECHO_PIN, GPIO.BOTH, callback=self.echo_handler)  

	
	def initialize(self):

		try:
			GPIO.output(self.TRIGGER_PIN, False)
			print "Waiting For Sensor To Settle"
			time.sleep(2)
			print "Sensor ready"
			while True:
				GPIO.output(self.TRIGGER_PIN, True)
				time.sleep(0.00001)
				GPIO.output(self.TRIGGER_PIN, False)
				time.sleep(0.1)

		finally:
			print "cleaning up"
			GPIO.cleanup()

	def echo_handler(self,channel):
		if self.echo_state==1: # falling edge detected
			self.pulse_start = time.time()
			self.echo_state=2 # level low

		elif self.echo_state==2: # rising edge detected
			self.pulse_end = time.time()
			self.pulse_duration = self.pulse_end - self.pulse_start
			self.distance = self.pulse_duration * 17150
			self.distance = round(self.distance, 2)
			if self.distance <= self.MOTION_DETECTION_LIMIT:
				self.motion_detection_handler()
				self.is_triggered = True
			else:
				self.is_triggered = False
			# print "Distance:",self.distance,"cm"
			self.echo_state =1

	def motion_detection_handler(self):
		print self.ID,": Motion event detected"
