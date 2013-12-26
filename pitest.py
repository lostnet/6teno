#! /usr/bin/python

import random
import RPi.GPIO as GPIO
import json
import time

inputpins = [4, 14, 15, 17]
testlog = []


class BCMKeyboard:
	def __init__(self):
		self.activekeys = []
		GPIO.setwarnings(False)
		GPIO.setmode(GPIO.BCM)
		for i in inputpins:
			GPIO.setup(i, GPIO.OUT, initial=GPIO.LOW)

	def delay(self):
		time.sleep(abs(random.gauss(0.01,.05)))

	def pressKey(self, key):
		self.delay()
		GPIO.output(key, GPIO.HIGH)

	def releaseKey(self, key):
		self.delay()
		GPIO.output(key, GPIO.LOW)

	def releaseAnyKey(self):
		self.releaseKey(random.choice(self.activekeys))

	def arePressed(self):
		return len(self.activekeys)
		
		

Ck = BCMKeyboard()

for i in range(255):
	chosen = random.sample(inputpins, random.choice(range(1,len(inputpins)+1)))
	random.shuffle(chosen)
	testlog.append(chosen)

with open("testlog.json", "w") as f:
	json.dump(testlog, f)


for i in testlog:
	Ck.pressKey(i.pop(0))
	while len(i) > 0:
		if Ck.arePressed() > 1 and random.choice([True,False]):
			Ck.releaseAnyKey()
		else:
			Ck.pressKey(i.pop())
	while Ck.arePressed() > 0:
		Ck.releaseAnyKey()


