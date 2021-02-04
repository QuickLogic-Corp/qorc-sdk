
wifi_streaming.py
===================

This is a Python server module that runs on the PC and waits
for Wifi client to be connected on Port 3333.

Once the TCP connection is made, the Audio stream is decoded and
will be saved as a wav file (default name wifi_audio.wav)

For preventing other clients connecting to it, User can pass
the Client IP address also.

Intial timeout is 30 secs. But once a client is connected then
the server will run indefinitely.

To exit it type "Ctrl + c" at the command prompt.

Use "wifi_streaming.py --help" to get more info with script.

