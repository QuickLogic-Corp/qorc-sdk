
1. Flash the ESP32 binary "qf_wifi_server_02_04_2021.bin"

2. Open the COM port associated with ESP32 in TeraTerm.
   Note: Make sure you select "CR" under Setup->Terminal->Transmit
   
3. To use it along with the Python Client first get the 
   Server IP address from ESP32.

4. Then set Wifi network <ssid> <passphrase> <server port number>
   using the command "wifi-set". You can use "wifi-get" to find
   the credentials any time.

   For example:  

     wifi-set myssid mypassword 3333
     
     where 
       -myssid is your wifi network id
       -mypassword is your wifi network passphrase
       -3333 is Server port number that the Python client needs
         to connect to

5. Disconnect and reconnect. (If ESP32 does not reset, open and
   close the Teraterm)
   
6. Run the Python script for Client

7. You should see "Socket accepted" message in ESP32 terminal

8. Then load the QF with any binary and stream the Audio 
  upto 3 channels. There is default binary loaded by ESP32 firmaware
  which sends only "alexa" phrase in 3 channels.

9. If there is problem setting the Wifi credentials, then
   erase the flash for easy typing. (else you disconnection messages).
   But "wifi-set" command will overwrite everytime it is called.
   
