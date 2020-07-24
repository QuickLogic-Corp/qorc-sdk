Quickfeather AI Application Project
=================================

This demo project performs SensiML data collection and recognition tasks 
for the Quickfeather platform using QORC-SDK.
(Refer: https://github.com/QuickLogic-Corp/qorc-sdk)

Building and running the project:
---------------------
The project supports either collection or recognition modes.
Select the desired mode by updating the macro S3AI_FIRMWARE_MODE in 
Fw_global_config.h.

1. Use the provided Makefile in qf_mqttsn_ai_app/GCC_Project folder
and appropriate ARM GCC toolchain to build the project

2. Use the flash programming procedure to flash the binary to Quickfeather board.

3. Reset the board to start running the AI application.

For data collection, building an AI model, and recognition 
please refer to https://sensiml.com/ and https://sensiml.com/documentation/guides/getting-started/index.html

MQTT-SN over USB-serial Notes
------------

The MQTT-SN SensiML AI application uses FPGA USB-serial IP to send and receive message over
USB-serial cable. The USB transmit FIFO will hold the outgoing data bytes until a host terminal 
application starts draining this data. According as the MQTT-SN protocol, the application attempts
to send CONNECT messages at regular intervals until it receives a response for these messages. 
To avoid queueing up multiple CONNECT messages in the transmit FIFO which may cause the USB 
transmit function to enter busy-wait state when the FIFO fills-up, this application sends CONNECT
messages only if the FIFO is empty.

