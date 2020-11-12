Quickfeather AI Application Project
=================================

This demo project performs SensiML data collection and recognition tasks 
for the Quickfeather platform using QORC-SDK.
(Refer: https://github.com/QuickLogic-Corp/qorc-sdk)

An AD7476 Analog-to-digital converter (ADC) is added to this project. The project
uses ADC FPGA IP design from [s3-gateware]. Communication interface for MQTT-SN
messages uses the ASSP UART interface.

Audio data capture and inference feature added to this project. To enable live-streaming
1-channel audio data captured at @16kHz sampling rate, 921600bps baudrate is used 
for the ASSP UART interface.

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
please refer to [SensiML QF] and [SensiML Getting Started]

Using with an AD7476 ADC module
------------

Connect a [PmodAD1] module to the quickfeather using the following pinout

| PmodAD1 module  | Quickfeather |
| --------------- | ------------ |
| CS              | J8.5         |
| D0              | J8.9         |
| CLK             | J8.11        |
| GND             | J8.13        |
| Vcc             | J8.15        |

Connect desired sensor to the A0 pin on the PmodAD1 module.

Reset the Quickfeather board to start running the AI application.
Refer [SensiML QF] to collect data or get inferences from this connected sensor.

MQTT-SN over USB-serial Notes
------------

The MQTT-SN SensiML AI application may be configured to use FPGA USB-serial IP to send and receive message over
USB-serial cable provided this FPGA IP is not used for other purposes such as ADC AD7476 IP used for ADC sensor. 
The USB transmit FIFO will hold the outgoing data bytes until a host terminal application starts draining this data. According as the MQTT-SN protocol, the application attempts
to send CONNECT messages at regular intervals until it receives a response for these messages. 
To avoid queueing up multiple CONNECT messages in the transmit FIFO which may cause the USB 
transmit function to enter busy-wait state when the FIFO fills-up, this application sends CONNECT
messages only if the FIFO is empty.

## Adding a sensor

This section provides basic guideline on adding a new sensor to the project for data collection.
Sensor data acquisition and processing or transfer to an external application such as [DataCaptureLab] 
uses [datablock manager] (qorc-sdk/Libraries/DatablockManager) and [datablock processor] (qorc-sdk/Tasks/DatablockProcessor) for acquiring samples and processing these acquired samples.
Qorc-sdk uses MQTT-SN protocol to send the acquired sensor data over to the [DataCaptureLab]

FreeRTOS software timer is used to trigger a timer event to read 1 sample from the sensor and
fill the datablock. When enough samples are collected (determined by the sensor sample rate, 
and latency), the datablock is processed and the samples are sent over the UART using the 
MQTT-SN topic sensiml/live/sensor/data.

## Configure the sensor

To add a new sensor start with the sensor\_ssss.h and sensor\_ssss.c source files. 

### sensor_ssss.h

Modify the header file sensor_ssss.h and update the following macros
- SENSOR\_SSSS\_SAMPLE\_RATE to specify the desired sensor sampling rate 
- SENSOR\_SSSS\_CHANNELS\_PER\_SAMPLE to specify the desired number of channels for the new sensor
- SENSOR\_BIT\_DEPTH to specify the bit-depth of the sensor samples. This can be either 16 or 32 bits.
- SENSOR\_SSSS\_LATENCY default latency is set to 20ms. This value determines how often the samples are processed and transmitted to the DCL ([DataCaptureLab]). The default value may be left as is.

The above macros determine the number of samples held in one datablock. These datablocks are held in 
the array sensor\_ssss\_data_blocks\[\]. 

### sensor_ssss.c

- Update the function sensor\_ssss\_configure() to initialize and setup the sensor configuration.

## Acquring and processing sensor samples

Based on the sensor sample rate, a FreeRTOS soft timer triggers requesting 1 sensor sample to
be filled-in the datablock.

### sensor_ssss.c

- Update the function sensor\_ssss\_acquisition\_buffer\_ready to read 1 sample into the current
  datablock. This function returns 1 if datablock is ready for processing, returns 0 otherwise.
  
## Capturing the sensor samples

- Sensor samples are sent using the MQTT-SN topic id sensiml/live/sensor/data. This topic uses
  the following packet format to transmit the payload.
  ```
  SensorID | SeqNum | SampleRate | NumChannels | BitDepth | SensorDataPayload
  ```
  
  Field Name  | # bits  | Description
  ----------  | ------  | -----------
  SensorID    | 32 bits | Sensor ID identifies the sensor for which the payload data belongs
  SeqNum      |  8 bits | An 8-bits sequence number for the current payload. 
  SampleRate  | 24 bits | Sample rate in Hz for the sensor(s)
  NumChannels |  7 bits | A 7-bit field indicating the sample rate (in Hz)
  BitDepth    |  1 bits | A 1-bit field, 0 indicates 16-bit data size, 1 indicates 32-bit data size
  SensorDataPayload | N bytes | N byte payload
  
  Quickfeather uses either an S3 UART or the USB serial to transmit these data. Sensor samples may be captured
using either [DataCaptureLab] or an MQTT client application. An example MQTT client application
(smlhost.py) is provided in the qorc-sdk/Tools/dclsim folder. To use this example application,
- Open a command terminal and navigate to the folder qorc-sdk/Tools
- run the rsmb broker ( [Tools/rsmb] ) 
```
  C:\> start "RSMB Server" rsmb\rsmb.exe -f rsmb\rsmb_config.txt
```
- run the MQTT-UART bridge ( [Tools/bridge] )
```
  C:\> start "mqttc" bridge\MqttConsoleApp.exe -G 1885 -n -d -c <COMport#> --baudrate 921600
```
- run the example MQTT client application 
```
  C:\> python smlhost.py --live --ssss --timeout 5 --log
```
  The above command would read sensor data samples for 5 secs.

## Loadcell sensor example

An example loadcell sensor on the SparkFun NAU7802 is provided as part of this application. This example loadcell sensor that uses SparkFun Qwiic Scale NAU7802 for configuring and reading sensor values
of a loadcell sensor. Refer [Qwiic Scale Hookup Guide] for hooking up the SparkFun NAU7802 to the
quickfeather board.

The sensor configuration function sensor\_ssss\_configure() uses the NAU7802_begin() function to 
configure and sets up the NAU7802 for acquiring samples approximately at the chosen sampling rate (SENSOR\_SSSS\_SAMPLE\_RATE) that is supported by the NAU7802. 

To read samples from at the configured sampling rate, sensor data read is performed when the FreeRTOS 
soft timer triggers the function sensor\_ssss\_acquisition\_buffer\_ready(). NAU7802_getReading() 
function is used to read a 24-bit sample and fill-in the current data block. When 20ms (= SENSOR\_SSSS\_LATENCY) samples are filled in the data block, these samples are processed by the
function sensor\_ssss\_livestream\_data_processor() to send these samples over UART using 
MQTT-SN sensiml/live/sensor/data topic using the packet format described in the above section.


[s3-gateware]: https://github.com/QuickLogic-Corp/s3-gateware
[SensiML QF]: https://sensiml.com/documentation/firmware/quicklogic-quickfeather/quicklogic-quickfeather.html
[SensiML Getting Started]: https://sensiml.com/documentation/guides/getting-started/index.html
[PmodAD1]: https://reference.digilentinc.com/reference/pmod/pmodad1/start
[datablock manager]: qf_vr_apps#datablock-manager
[datablock processor]: qf_vr_apps#datablock-processor
[DataCaptureLab]: https://sensiml.com/products/data-capture-lab/
[Qwiic Scale Hookup Guide]: https://learn.sparkfun.com/tutorials/qwiic-scale-hookup-guide?_ga=2.193267885.1228472612.1605042107-1202899191.1566946929
