import RPi.GPIO as GPIO
import time
from Ultrasonic import *
from threading import Thread
import signal,sys

GPIO.setmode(GPIO.BCM)
 
US1 = Ultrasonic("US1",3,4,70)
US2 = Ultrasonic("US2",14,15,70)
try:
	thread1 = Thread(target = US1.initialize)
	thread2 = Thread(target = US2.initialize)

	thread1.start()
	thread2.start()
	signal.pause() # instead of: while True: time.sleep(100)
except (KeyboardInterrupt, SystemExit):
  print '\n! Received keyboard interrupt, quitting threads.\n'
  sys.exit()
