
1. Flash the ESP32 binary "qf_wifi_server_02_04_2021.bin"

2. Open the COM port associated with ESP32 in TeraTerm.
   Note: Make sure you select "CR" under Setup->Terminal->Transmit
   
Note: To use it along with the Python Client first get the Server 
   IP address from ESP32.

3. Then set Wifi network <ssid> <passphrase> <server port number>
   using the command "wifi-set". You can use "wifi-get" to find
   the credentials any time.

   For example:  

     wifi-set myssid mypassword 3333
     
     where 
       -myssid is your wifi network id
       -mypassword is your wifi network passphrase
       -3333 is Server port number that the Python client needs
         to connect to

4. Disconnect and reconnect. (If ESP32 does not reset, open and
   close the Teraterm). You should see messages similar to the  
   following from ESP32
   
        I (890) wifi:AP's beacon interval = 102400 us, DTIM period = 3
        Waiting for AP connection... 1 seconds
        I (2210) [app_main]: SYSTEM_EVENT_STA_GOT_IP6
        I (2210) [app_main]: IPv6: FE80::2662:ABFF:FED1:B408
        Waiting for AP connection... 2 seconds
        I (2700) [app_main]: Connected to AP
        I (2700) [app_main]: Socket created
        I (2700) [app_main]: Socket binded
        I (2700) [app_main]: Socket listening
        I (2710) event: sta ip: 192.168.1.5, mask: 255.255.255.0, gw: 192.168.1.1
        I (2710) [app_main]: SYSTEM_EVENT_STA_GOT_IP

    The important messages are "Connected to AP", "Socket listening"
    along with "IP address for the Server". You need to supply it
    when running the Python script.

6. Run the Python script for Client, "wifi_audio_client.py" from 
   Tools/wifiaudio. (Type "wifi_audio_client.py --help" to get info
   on running the script.)

7. You should see "Socket accepted" message in ESP32 terminal

8. Then load the QF with any binary,(using J-link or any other Debugger) 
   and stream the Audio upto 3 channels. 
   Note: There is a default binary loaded by ESP32 firmware every time it
   is reset, which recognises and sends "only alexa" phrase in 3 channels.

9. If there is problem setting the Wifi credentials, then
   erase the flash for easy typing. (else you may see disconnection messages).
   But "wifi-set" command will overwrite everytime it is called.
   
