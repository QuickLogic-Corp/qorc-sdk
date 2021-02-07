
wifi_audio_client.py
====================

This is a Python Client module that runs on the PC and waits
to connect to a Server on Port 3333 (default).

Once the TCP connection is made, the Audio stream is decoded and
will be saved as a wav file (default name wifi_audio.wav)

Intial timeout is 30 secs. But once the client is connected to 
the server it will run indefinitely.

To exit it type "Ctrl + c" at the command prompt (and wait a few seconds).

Use "wifi_streaming.py --help" to get more info with the script.


