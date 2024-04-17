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

	def myFunction(self, v):
		debug(v)

	def PromotedFunction(self, v):
		debug(v)
		
	def GenerateTopics(self):
		self.mqttAddresses = list()
		for r in self.topics.rows():
			a = 'morphose/'+self.name+"/"+r[0]
			self.mqttAddresses.append(a)		
		
	def SubscribeToAllTopics(self):
		for t in self.mqttAddresses:
			print("Subscribring to {}".format(t))
			self.logger.Info("Subscribring to {}".format(t))
			self.mqtt.subscribe(t)
			
	def GetMqttAddress(self,idx):
		return self.mqttAddresses[idx]
		
	def UpdateMotorTemperature(self, payload):
		p= payload.decode().split()
		self.temp.par.Steertemp = int(float(p[0]))
		self.temp.par.Speedtemp = int(float(p[1]))
		
	def UpdateBattery(self,voltage):
		self.battery.par.Voltage = voltage
		
	def ParseData(self,payload):
		p = payload.decode()
		json = TDJSON.textToJSON(p)
		self.UpdateBattery(json["battery"])

	def SetConnected(self, status):
		self.status.val = status
	
	def MessageIn(self):
		self.msgIn.pulse()
		
	def Reboot(self):
		topic = "morphose/{}/reboot".format(self.name)
		self.logger.Info("Reboot asked from UI")
		self.mqtt.publish(topic,b'1')
		
	def LogDebug(self,payload):
		p = payload.decode()
		self.logger.Info("Received from Debug: {}".format(p))
		