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

class MorphoseEXT:
	"""
	MorphoseEXT description
	"""
	def __init__(self, ownerComp):
		# The component to which this extension is attached
		self.ownerComp = ownerComp

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

	def GetEnabledRobots(self):
		robots = list()
		if op.robot1.par.Enablerobot.val :
			robots.append(op.robot1.name)
		
		if op.robot2.par.Enablerobot.val :
			robots.append(op.robot2.name)
			
		
		if op.robot3.par.Enablerobot.val :
			robots.append(op.robot3.name)
		
		if not robots:
			print('No robots enable')
		else:
			return robots
			
	def SetIdleOn(self):
		robots = self.GetEnabledRobots()
		
		if not robots:
			print('No robots enable')
		else:
			for r in robots:
				eval("op.{}.IdleOn()".format(r))
		return
		
	def RebootAll(self):
		robots = self.GetEnabledRobots()
		
		if not robots:
			print('No robots enable')
		else:
			for r in robots:
				eval("op.{}.Reboot()".format(r))
		return
		
	def SetIdleOff(self):
		robots = self.GetEnabledRobots()
		
		if not robots:
			print('No robots enable')
		else:
			for r in robots:
				eval("op.{}.IdleOff()".format(r))
		
		return
	
		
			