Quickfeather AI Application Project
===================================

This demo project performs SensiML data collection and recognition tasks
for the Quickfeather platform using QORC-SDK. (Refer:
https://github.com/QuickLogic-Corp/qorc-sdk)

An AD7476 Analog-to-digital converter (ADC) is added to this project.
The project uses ADC FPGA IP design from
`s3-gateware <https://github.com/QuickLogic-Corp/s3-gateware>`__.
Communication interface for MQTT-SN messages uses the ASSP UART
interface.

Audio data capture and inference feature added to this project. To
enable live-streaming 1-channel audio data captured at @16kHz sampling
rate, 921600bps baudrate is used for the ASSP UART interface.

Building and running the project:
---------------------------------

The project supports either collection or recognition modes. Select the
desired mode by updating the macro S3AI_FIRMWARE_MODE in
Fw_global_config.h.

1. Use the provided Makefile in qf_mqttsn_ai_app/GCC_Project folder and
   appropriate ARM GCC toolchain to build the project

2. Use the flash programming procedure to flash the binary to
   Quickfeather board.

3. Reset the board to start running the AI application.

For data collection, building an AI model, and recognition please refer
to `SensiML
QF <https://sensiml.com/documentation/firmware/quicklogic-quickfeather/quicklogic-quickfeather.html>`__
and `SensiML Getting
Started <https://sensiml.com/documentation/guides/getting-started/index.html>`__

Using with an AD7476 ADC module
-------------------------------

Connect a
`PmodAD1 <https://reference.digilentinc.com/reference/pmod/pmodad1/start>`__
module to the quickfeather using the following pinout

============== ============
PmodAD1 module Quickfeather
============== ============
CS             J8.5
D0             J8.9
CLK            J8.11
GND            J8.13
Vcc            J8.15
============== ============

Connect desired sensor to the A0 pin on the PmodAD1 module.

Reset the Quickfeather board to start running the AI application. Refer
`SensiML
QF <https://sensiml.com/documentation/firmware/quicklogic-quickfeather/quicklogic-quickfeather.html>`__
to collect data or get inferences from this connected sensor.

.. _mqtt-sn-over-usb-serial-notes:

MQTT-SN over USB-serial Notes
-----------------------------

The MQTT-SN SensiML AI application may be configured to use FPGA
USB-serial IP to send and receive message over USB-serial cable provided
this FPGA IP is not used for other purposes such as ADC AD7476 IP used
for ADC sensor. The USB transmit FIFO will hold the outgoing data bytes
until a host terminal application starts draining this data. According
as the MQTT-SN protocol, the application attempts to send CONNECT
messages at regular intervals until it receives a response for these
messages. To avoid queueing up multiple CONNECT messages in the transmit
FIFO which may cause the USB transmit function to enter busy-wait state
when the FIFO fills-up, this application sends CONNECT messages only if
the FIFO is empty.
