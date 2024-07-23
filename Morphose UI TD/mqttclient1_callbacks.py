# me - this DAT
# 

# Called when connection established
# dat - the OP which is cooking
def onConnect(dat):
	#print("Connected")
	#parent().SetConnected(True)
	op('logger').Info("Connected to Mqtt broker")
	parent().SubscribeToAllTopics()

	return

# Called when connection failed
# dat - the OP which is cooking
# msg - reason for failure
def onConnectFailure(dat, msg):
	op('logger').Error("Mqtt connection failure : {}".format(msg))
	#parent().SetConnected(False)
	return

# Called when current connection lost
# dat - the OP which is cooking
# msg - reason for failure
def onConnectionLost(dat, msg):
	op('logger').Error("Mqtt connection Lost : {}".format(msg))
	#parent().SetConnected(False)
	return

# Called when the subscribe call succeeds and is sent to the server
# dat - the OP which is cooking
def onSubscribe(dat):
	#print("Subscribe successfull")
	op('logger').Info("Subscribe successfull")
	return

# Called when subscription request fails.
# dat - the OP which is cooking
# msg - reason for failure
def onSubscribeFailure(dat, msg):
	op('logger').Error("Mqtt Failed to subscribe : {}".format(msg))
	return

# Called when the unsubscribe call succeeds and is sent to the server
# dat - the OP which is cooking
def onUnsubscribe(dat):
	return

# Called when unsubscription request fails.
# dat - the OP which is cooking
# msg - reason for failure
def onUnsubscribeFailure(dat, msg):
	return

# Called when the publish call succeeds and is sent to the server
# dat - the OP which is cooking
def onPublish(dat, topic):
	return

# Called when new content received from server
# dat - the OP which is cooking
# topic - topic name of the incoming message
# payload - payload of the incoming message
# qos - qos flag for of the incoming message
# retained - retained flag of the incoming message
# dup - dup flag of the incoming message
def onMessage(dat, topic, payload, qos, retained, dup):
	
	#print(topic,payload)
	parent().MessageIn()
	if topic == parent().GetMqttAddress(0): #temperature
		parent().UpdateMotorTemperature(payload)
	elif topic == parent().GetMqttAddress(1): #data
		parent().ParseData(payload)
	elif topic == parent().GetMqttAddress(2): #debug
		parent().LogDebug(payload)
	elif topic == parent().GetMqttAddress(3): #battery critical
		parent().BatteryCritical(payload)
	elif topic == parent().GetMqttAddress(4): #ack
		parent().RobotAcknowledge(payload)
		return

	return
