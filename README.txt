Kimberly Tom
11/28/19
CS 372

Readme file for Project 2, TCP File Transfer System:

ftserver.c was written in C.
ftclient.py was written in python.


The testing machine I used was a Lenovo Z51 operating with Windows 10.
Testing was done using two instances of PuTTY on one computer on flip1.engr.oregonstate.edu, 
flip2.engr.oregonstate.edu, and flip3.engr.oregonstate.edu.

Unzip the folder containing the two programs, ftserver.c and ftclient.py

Open an instance of PuTTY, log in, and navigate to the folder containing ftclient.py
Make note of which flip server you are on. You can do this by typing "hostname" into the command line. 

To compile ftserver.c, type: 
gcc -o ftserver ftserver.c

To run ftserver.c type into the command line:
ftserver [PORTNUM1]
For example [PORTNUM1] can be replaced with "3356", hit enter. If that port is not in use, ftserver should now be running and you should see a message
that says "Server open on 3356".

Open a second instance of PuTTY on the same computer, log in, and navigate to the folder containing ftclient.py

To compile and run ftclient.py type:
python ftclient.py [flipX] [PORTNUM1] -l [PORTNUM2]
or 
python ftclient.py [flipX] [PORTNUM1] -g [TEXTFILE] [PORTNUM2]

For [flipX], you should enter the flip server of where the ftserver program is running.  For example, if your ftserver.c program is running on flip2.engr.oregonstate.edu,
you will want to enter "flip2" for the [flipX] portion. Otherwise, you will get a connection refused error.
For [PORTNUM1], you should enter the same PORTNUM1 you entered in when you began running ftserver.  Otherwise, you will get a connection refused error.
The next portion is the command, which must either be "-l" or "-g" or else you will get an error message.

for "-l":
for [PORTNUM2] enter a port number that is different from [PORTNUM1]. This will be the port used to transfer data.
After hitting enter, the program should be running and requesting a directory list from the server.  If no errors, the list of files in server's directory
will be printed on client's screen. If no errors, ftclient program should then terminate, while ftserver remains running.

for "-g":
for [TEXTFILE] enter the name of a text file you want the client to receive.  
for [PORTNUM2] enter a port number that is different from [PORTNUM1]. This will be the port used to transfer data.
After hitting enter, the program should be running.  Server should be checking if it has a file with the name TEXTFILE in its directory.  If it does not have file name named FILENAME, it will send information 
to client which will then inform client that no files with that name are found. ftclient will show error message, then terminate. ftserver will remain running.
If there is a file named FILENAME, server will transfer file to client.
If client already has a file in their directory with the name FILENAME, then a file with the same name but with the word "new" in front of it will be created in the client's directory.
After file is created, if no errors, ftclient will terminate. ftserver will remain running.

ftserver should be displaying what's happening as it receives commands from ftclient.
ftserver will not close and may keep handling one request at a time until SIGINT (control + C) is received.
ftclient will termiante after each command is completed or if there is an error.