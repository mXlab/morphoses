"""Small example OSC server

This program listens to several addresses, and prints some information about
received packets.
"""
import argparse
import math
import os
from pythonosc import udp_client
from pythonosc.dispatcher import Dispatcher
from pythonosc import osc_server
import time


filename = " "


doOnce =False


def confirmFlush(client_address: tuple[str, int], address: str, *osc_args: list[any]):
    global doOnce
    global file
    
    if doOnce == False:
        named_tuple = time.localtime() # get struct_time
        time_string = "./logs/{}_{}_{}.txt".format(named_tuple[0],named_tuple[1],named_tuple[2])
        filename = time_string
        if(not os.path.isfile(filename)):
            print("No such file")
            print(filename)
            print("creating it")
        
        
        file = open(filename, "a")

        if (file.writable() == True):
            file.write(time.asctime(named_tuple))
            file.write("\n")

        
        print("Ready to receive log")
        udp_client.SimpleUDPClient(client_address[0], client_address[1]).send_message("/flush",1)
        doOnce = True
    return

def writeLog(address: str, *osc_args: list[any]):
    global file
    if (doOnce == True):
        if (file.writable() == True):
            file.write("    {}\n".format(osc_args[0]))
            print(osc_args[0])

def endOfLog(client_address: tuple[str, int], address: str, *osc_args: list[any]):
    global doOnce
    global file

    if (doOnce == True):
        print("closing file")
        file.write("------------\n")
        file.close()
        udp_client.SimpleUDPClient(client_address[0], client_address[1]).send_message("/endLog",1)
        doOnce = False
    
        
    
    

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--ip",
        default="192.168.0.148", help="The ip to listen on")
    parser.add_argument("--inport",
        type=int, default=8110, help="The port to listen on")
    parser.add_argument("--outport",
        type=int, default=800, help="The port to listen on")
    args = parser.parse_args()





dispatcher = Dispatcher()
dispatcher.map("/flush", confirmFlush,needs_reply_address= True)
dispatcher.map("/endLog", endOfLog,needs_reply_address= True)
dispatcher.map("/log", writeLog)
  

if(not os.path.exists("./logs")):
    print("No logs directory. Creating one.")
    os.makedirs("./logs")


named_tuple = time.localtime() # get struct_time
time_string = "./logs/{}_{}_{}.txt".format(named_tuple[0],named_tuple[1],named_tuple[2])
filename = time_string
if(not os.path.isfile(filename)):
    print("No such file")
    print(filename)
    print("creating it")
    
file = open(filename, "a")



server = osc_server.ThreadingOSCUDPServer(
    (args.ip, args.inport), dispatcher)
print("Serving on {}".format(server.server_address))
server.serve_forever()
