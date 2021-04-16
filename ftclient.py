# Author: Kimberly Tom
# Project 2: File Transfer System
# Description: client side code written in Python.  This program can request to get a listing from a server's directory or obtain a file from the server's directory.
# This program uses TCP connection to transfer data.
# CS372 Intro To Computer Networks
# Last Modified: 11/28/19

import sys
import os
from socket import *


# validateArgs verifies the user input valid arguments
# preconditions: user has entered arguments on the command line
# postconditions: validates arguments and displays error message if invalid argument
# with help from https://stackoverflow.com/questions/35491845/checking-the-number-of-command-line-arguments-in-python
def validateArgs():

	# port numbers must be a number betwen 1024-65535
	minPort = int(1024)
	maxPort = int(65535)
	
	# verify the number of arguments is not less than 5
	if len(sys.argv) < 5:
		print ("You have entered too few arguments. Exiting Program.")
		sys.exit()
	
	# verify the number of arguments is not greater than 6
	if len(sys.argv) > 6:
		print ("You have entered too many arguments. Exiting Program.")
		sys.exit()
		
	# verify that the 2nd argument is a valid flip server
	if sys.argv[1] != "flip1" and  sys.argv[1] != "flip2" and sys.argv[1] != "flip3":
		print ("You have entered an incorrect server. It must be flip1 flip2 or flip3. Exiting Program.")
		sys.exit()	
		
	# verify that the command is either -l or -g
	if sys.argv[3] != "-l" and sys.argv[3] != "-g":
		print ("The command must either be -l or -g. Exiting Program.")
		sys.exit()
		
	# verify that if user wants to use the command -l that there are 5 arguments
	if sys.argv[3] == "-l" and len(sys.argv) != 5:
		print ("You have entered the incorrect number of arguments to use the command -l. Exiting Program.")
		sys.exit()
	
	# verify that if user wants to use the command -g that there are 6 arguments
	if sys.argv[3] == "-g" and len(sys.argv) != 6:
		print ("You have entered the incorrect number of arguments to use the command -g. Exiting Program.")
		sys.exit()
		
	# verify the port number is not too small
	if int(sys.argv[2]) < int(minPort):
		print ("You entered a reserved port number. Enter a port number between 1024 and 65535. Exiting Program.")
		sys.exit()	
		
	# verify the port number is not too big
	if int(sys.argv[2]) > int(maxPort):
		print ("You entered a port number that is too big. Enter a port number between 1024 and 65535. Exiting Program.")
		sys.exit()	
		
	# verify the port number is 1024-65535
	if sys.argv[3] == "-l" and (int(sys.argv[4]) < int(minPort) or int(sys.argv[4]) > int(maxPort)):
		print ("You entered an invalid port number for the 5th argument for command -l. Enter a port number between 1024 and 65535. Exiting Program.")
		sys.exit()	
		
	# verify the port number is 1024-65535
	if sys.argv[3] == "-g" and (int(sys.argv[5]) < int(minPort) or int(sys.argv[5]) > int(maxPort)):
		print ("You entered an invalid port number for the 6th argument for command -g. Enter a port number between 1024 and 65535. Exiting Program.")
		sys.exit()	
		

		
# clientSetup connects the client to the server by creating a new socket
# preconditions: user has entered a valid 2nd (flipx) and 3rd argument (port number)
# postconditions: returns socket with the server name and port number
# with help from 2.7 from Computer Networking - A Top-down Approach by James F. Kurose, Keith W. Ross, page 194
def clientSetup():

	url = ".engr.oregonstate.edu"
	serverHost = sys.argv[1] + url # the second argument is the flipX portion, add it to the rest of the url to get the name of the server host
	serverPort = int(sys.argv[2]) # cast the port number (3rd argument) to an integer https://stackoverflow.com/questions/17383470/argv-string-into-integer
	clientSocket = socket(AF_INET,SOCK_STREAM)
	clientSocket.connect((serverHost, serverPort))
	
	return clientSocket

	
# actionList sends the list command to the server and receives list of files in directory from server
# preconditions: clientSocket with valid flipX servername and port number must be provided, as well as a file name
# postconditions: returns a list of files in the directory to the console		
# with help from 2.7 from Computer Networking - A Top-down Approach by James F. Kurose, Keith W. Ross, page 196
def actionList(clientSocket):
	messageSize = 1024
	transferPort1 = sys.argv[4]
	# send the port number to the server (this is the 5th argument)
	clientSocket.send(transferPort1)
	
	# receive message that it is OK from server
	clientSocket.recv(messageSize)
	
	# Get the local host name
	# with help from https://pythontic.com/modules/socket/gethostname
	clientHostName = gethostname()

	# send our host name to the server and receive OK from server
	if clientHostName == "flip1.engr.oregonstate.edu":
		clientSocket.send("flip1")
		clientSocket.recv(messageSize)
	elif clientHostName == "flip2.engr.oregonstate.edu":
		clientSocket.send("flip2")
		clientSocket.recv(messageSize)
	else:
		clientSocket.send("flip3")
		clientSocket.recv(messageSize)

	
	# send the command to the server letting server know we want list of files in directory
	clientSocket.send("list")
	
	# receive message that it is OK from server
	clientSocket.recv(messageSize)
		
		
	# obtain our IP address
	# https://pythontic.com/modules/socket/gethostname
	clientIPAddress = gethostbyname(clientHostName)
	
	# send our IP address
	clientSocket.send(clientIPAddress)           
	
 
	# receive the message from the server and check if there is an error.  If error, exit program
	if clientSocket.recv(messageSize) == "1":
		print("You sent an invalid command to the server")
		sys.exit()
	
	# create socket	
	# with help from 2.7 from Computer Networking - A Top-down Approach by James F. Kurose, Keith W. Ross, page 196
	serverport = int(transferPort1) # use the 5th argument
	serversocket = socket(AF_INET, SOCK_STREAM)
	serversocket.bind(('', serverport))
	serversocket.listen(1)
	connSocket, address = serversocket.accept()
	

	print ("Receiving directory list from " + sys.argv[1] + ":" + transferPort1)

	
	# receive the first file name in the directory
	fileRecvd = connSocket.recv(100)
	
	# print the name of the file received, then keep receiving file names and printing until server tells us it is at the end of the directory
	while True:
		print fileRecvd
		fileRecvd = connSocket.recv(100)
		if (fileRecvd == "noMore"):
			break
	
	# close the connection
	connSocket.close()
			
# actionGet sends the get command to the server to get and receives a file from the server if it exists
# preconditions: clientSocket with valid flipX servername and port number must be provided, as well as a file name
# postconditions: creates a file in the client's current directory if file in server with provided filename exists, otherwise error message prints to console
# if there is no matching file name in the server's directory.
# with help from 2.7 from Computer Networking - A Top-down Approach by James F. Kurose, Keith W. Ross, page 196	
# with help from https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c
# with help from https://stackoverflow.com/questions/49981603/how-to-check-if-any-file-exists-in-a-certain-folder-with-python
def actionGet(clientSocket):
	
	messageSize = 1024
	fileWanted  = sys.argv[4]
	ourFileName = sys.argv[4]
	transferPort2 = sys.argv[5]
	# send the port number to the server (this is the 6th argument)
	clientSocket.send(transferPort2)
	# receive message that it is OK from server
	clientSocket.recv(messageSize)
	
	# checking if file already exists help from https://stackoverflow.com/questions/49981603/how-to-check-if-any-file-exists-in-a-certain-folder-with-python
	someFile = os.path.exists(fileWanted)
	
	# if file with the same name already exists, change name of the file name we will create so it doesn't overwrite previous file of same name
	# use while loop to check that our new file name that we generated isn't alread 
	while (someFile):
		if someFile:
			print("There already exists a file with the name " + ourFileName)
			ourFileName = "new" + ourFileName
			someFile = os.path.exists(ourFileName)
	
	
	# Get the local host name
	# with help from https://pythontic.com/modules/socket/gethostname
	clientHostName = gethostname()
	
	# send our host name to the server and receive OK from server
	if clientHostName == "flip1.engr.oregonstate.edu":
		clientSocket.send("flip1")
		clientSocket.recv(messageSize)
	elif clientHostName == "flip2.engr.oregonstate.edu":
		clientSocket.send("flip2")
		clientSocket.recv(messageSize)
	else:
		clientSocket.send("flip3")
		clientSocket.recv(messageSize)
	
	
	# send the command to the server letting server know we want to get a file
	clientSocket.send("get")
	# receive message that it is OK from server
	clientSocket.recv(messageSize)
		
	# obtain our IP address
	clientIPAddress = gethostbyname(clientHostName)
	
	# send our IP address
	clientSocket.send(clientIPAddress)
	
	# receive the message from the server and check if there is an error.  If error, exit program
	# idea to receive a variable indicating error borrowed from https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c
	if clientSocket.recv(messageSize) == "1":
		print("You sent an invalid command to the server")
		sys.exit()
	
	# send the name of the file to the server	
	clientSocket.send(fileWanted)
	
	# get the response from the server on whether the file with that name was there or not
	response = clientSocket.recv(messageSize)
	
	# if file doesn't exist, inform user and exit program
	# idea to receive text indicating whether file is present borrowed from https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c
	if response == "fileNotThere":
		print(sys.argv[1] + ":" + transferPort2 + " says the file " + fileWanted + " was not found. Exiting Program.")
		sys.exit()
		
	# else if file is there, create the socket and connection for sending the file
	elif response == "fileThere":
		print("Receiving " + fileWanted + " from " + sys.argv[1] + ":" + transferPort2)
		# create socket	
		# with help from 2.7 from Computer Networking - A Top-down Approach by James F. Kurose, Keith W. Ross, page 196
		serverport = int(sys.argv[5]) # use the 6th argument
		serversocket = socket(AF_INET, SOCK_STREAM)
		serversocket.bind(('', serverport))
		serversocket.listen(1)
		connSocket, address = serversocket.accept()
		
		# open new file for writing
		# with help from https://www.pythonforbeginners.com/files/reading-and-writing-files-in-python
		fileObj = open(ourFileName, "w")   

		# receive text from server
		fileText = connSocket.recv(1000)
    
		# while we have not encountered the phrase designated by the server indicating no more text lines, keep writing text to file
		while True:
			fileObj.write(fileText)
			fileText = connSocket.recv(1000)
			if fileText == "no!More!Text": # idea to receive text indicating no more text borrowed from https://github.com/TylerC10/CS372/blob/master/Project%202/ftserver.c
				break
		
		print("File transfer has been completed")
		
		
		# close the connection
		connSocket.close()
		
	# else if the response was not the file was found or not, then exit the program as there was an error	
	else:
		print("Error in response from server. Exiting Program.")
		sys.exit()
		
if __name__ == "__main__":

	# validate user's arguments
	validateArgs()
	
	# set up a connection to the server through a socket
	clientSocket = clientSetup()
	
	# if user sends -l command, user is requesting a list of files in the directory
	if sys.argv[3] == "-l":
		actionList(clientSocket)
		
	# if user sends -g command, user is requesting to get a file
	if sys.argv[3] == "-g":
		actionGet(clientSocket)
	
