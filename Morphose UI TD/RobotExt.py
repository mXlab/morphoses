"""
Extension classes enhance TouchDesigner components with python. An
extension is accessed via ext.ExtensionClassName from any operator
within the extended component. If the extension is promoted via its
Promote Extension parameter, all its attributes with capitalized names
can be accessed externally, e.g. op('yourComp').PromotedFunction().

Help: search "Extensions" in wiki
"""

from TDStoreTools import StorageManager
import TDFunctions as TDF
import TDJSON
import json

class RobotExt:
	"""
	RobotExt description
	"""
	def __init__(self, ownerComp):
		# The component to which this extension is attached
		self.ownerComp = ownerComp
		
		self.mqtt = self.ownerComp.op('mqttclient1')
		self.topics = self.ownerComp.op('topics')
		self.mqttAddresses = list()
		
		self.name = self.ownerComp.name
		self.temp = self.ownerComp.op("temperature")
		self.battery = self.ownerComp.op("battery")
		self.msgIn = self.ownerComp.op("header").op("status").par.Messagein
		self.status = self.ownerComp.op("header").op("status").par.Connected

		self.logger = self.ownerComp.op("logger")

		# properties

		TDF.createProperty(self, 'MyProperty', value=0, dependable=True,
						   readOnly=False)

		# attributes:
		self.a = 0 # attribute
		self.B = 1 # promoted attribute

		# stored items (persistent across saves and re-initialization):
		storedItems = [
			# Only 'name' is required...
			{'name': 'StoredProperty', 'default': None, 'readOnly': False,
			 						'property': True, 'dependable': True},
		]
		# Uncomment the line below to store StoredProperty. To clear stored
		# 	items, use the Storage section of the Component Editor
		
		# self.stored = StorageManager(self, ownerComp, storedItems)
		
		self.GenerateTopics()

		
	def GenerateTopics(self):
		self.mqttAddresses = list()
		for r in self.topics.rows():
			a = 'morphoses/'+self.name+"/"+r[0]
			self.mqttAddresses.append(a)		
		
	def SubscribeToAllTopics(self):
		for t in self.mqttAddresses:
			print("Subscribring to {}".format(t))
			self.logger.Info("Subscribring to {}".format(t))
			self.mqtt.subscribe(t)
			
	def GetMqttAddress(self,idx):
		return self.mqttAddresses[idx]
		
	def UpdateMotorTemperature(self, payload):
		p= payload.decode().split(',')
		self.temp.par.Speedtemp = int(float(p[0]))
		self.temp.par.Speedstatus = int(float(p[1]))
		self.temp.par.Steertemp = int(float(p[2]))
		self.temp.par.Steerstatus = int(float(p[3]))
		
	def UpdateBattery(self,voltage):
		self.battery.par.Voltage = voltage
		
	def ParseData(self,payload):
		
		p = payload.decode()
		#json = TDJSON.textToJSON(p)
		
		self.UpdateBattery(float(p))

	def SetConnected(self, status):
		self.status.val = status
	
	def MessageIn(self):
		self.msgIn.pulse()
		
	def Reboot(self):
		topic = "morphoses/{}/reboot".format(self.name)
		self.logger.Info("Reboot asked from UI")
		self.mqtt.publish(topic,b'1')
		
	def LogDebug(self,payload):
		p = payload.decode()
		self.logger.Info("Received from Debug: {}".format(p))
		
	def Stop(self):
		topicLights = "morphoses/{}/animation".format(self.name)
		topicPower = "morphoses/{}/power".format(self.name)
		topicSpeed = "morphoses/{}/speed".format(self.name)
		
		animation = {
        	"base": [0,0,0],
        	"alt": [0,0,0],
        	"period": 0,
        	"noise": 0,
        	"region": 0,
        	"type": 1
        }

		res_bytes = json.dumps(animation).encode('utf-8')
		self.logger.Info("Stopping Robot asked from UI")
		self.mqtt.publish(topicPower,b'0')
		self.mqtt.publish(topicSpeed,b'0')
		print(topicLights)
		self.mqtt.publish(topicLights,b'0')
		
	
	def IdleOff(self):
		topic = "morphoses/{}/idle".format(self.name)
		self.mqtt.publish(topic,b'0')
		
	def IdleOn(self):
		topic = "morphoses/{}/idle".format(self.name)
		self.mqtt.publish(topic,b'1')
		