# ble_scan_connect.py:
from bluepy.btle import Peripheral, UUID
from bluepy.btle import Scanner, DefaultDelegate
import sys
import threading
import time
import random

FREQ = 1
LOCK = threading.Lock()
DEV = 0
WC = 0 #writing characteristic
RC = 0 #reading characteristic

SERVICE_UUID = "00000000-0001-11e1-9ab4-0002a5d5c51b"
WRITING_UUID = "acc0f000-0001-11e1-ac36-0002a5d5c51b"
READING_UUID = "acc1da7a-0001-11e1-ac36-0002a5d5c51b"

def p():
	global LOCK
	global DEV
	global RC
	global FREQ
	while(1):
		notify = DEV.waitForNotifications(10)
		if(notify):
			with LOCK:
				b = RC.read()
				print(int.from_bytes(b[2:4], signed=True), int.from_bytes(b[4:6], signed=True), int.from_bytes(b[6:8], signed=True))
		else:
			print("no notify")	

class ScanDelegate(DefaultDelegate):
	def __init__(self):
		DefaultDelegate.__init__(self)
	def handleDiscovery(self, dev, isNewDev, isNewData):
		if isNewDev:
			print ("Discovered device", dev.addr)
		elif isNewData:
			print ("Received new data from", dev.addr)



#=======================main=================================

#scanning
scanner = Scanner().withDelegate(ScanDelegate())
devices = scanner.scan(10.0)
n=0
addr = []

target_mac = "f8:0f:51:84:33:7a"

for dev in devices:
	print ("%d: Device %s (%s), RSSI=%d dB" % (n, dev.addr, dev.addrType, dev.rssi))
	addr.append(dev.addr)
	if(dev.addr == target_mac):
		print("---------------------------------------------------found--------------------------------------------------")
	n += 1
	for (adtype, desc, value) in dev.getScanData():
		print (" %s = %s" % (desc, value))

number = input('Enter your device number: ')
if(number == -1):
	sys.exit()
print ('Device', number)
num = int(number)
print (addr[num])

# Select Device and Connect
print ("Connecting...")
dev = 0
while 1:
	try:
		dev = Peripheral(addr[num], 'random')
	except:
		print("failed")
		continue
	if dev != 0:
		break
	print("tried")
DEV = dev

#print all services and characteristics
print("Services...")
services = dev.getServices()
for svc in services:
	print("UUID:", svc.uuid)

	if(svc.uuid == SERVICE_UUID): 
		#Print descriptor of target service
		print("-- Descriptors of Service", SERVICE_UUID)
		des = svc.getDescriptors()
		for (i, d) in enumerate(des):
			print(i, d.uuid)
		#write cccd to 1 to enable notification
		cccd = int(input("Choose CCCD's uuid"))
		des[cccd].write(b'x\01x\00')
	
print("Characteristics...")
characs = dev.getCharacteristics()
for c in characs:
	print("Characteristic:", c.uuid)
	if(c.uuid == WRITING_UUID):
		WC = c
	elif(c.uuid == READING_UUID):
		RC = c
print(WC)
print(RC)

#assign initial frequency
FREQ = int(input("Assign frequency: "))
WC.write(FREQ.to_bytes(2, 'big'), withResponse = True)

#start second thread for read/print
t = threading.Thread(target = p)
t.start()

#main loop: wait for [Enter] to break the loop and re-assign a freq
while 1:
    input()     # Waiting for user to press Enter
    with LOCK:
        FREQ = int(input("Assign new frequency: "))
        WC.write(FREQ.to_bytes(2, 'big'), withResponse = True)