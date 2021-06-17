QuickThing+ `Simple Streaming Interface <https://sensiml.com/documentation/simple-streaming-specification/introduction.html>`__ AI Application Project
=======================================================================================================================================================

This project performs either data collection or recognition based on the
build mode. Data collection uses `Simple Streaming
Interface <https://sensiml.com/documentation/simple-streaming-specification/introduction.html>`__
to connect and stream sensor data to SensiML's `Data Capture
Lab <https://sensiml.com/products/data-capture-lab/>`__. Adding external
sensors to collect data, analyze and build models is made simple
requiring only a sensor configuration API function and data reading
function API to be supplied for the new sensor. This project provides an
Arduino friendly Wire interface for easily integrating Arduino sensor
code.

Building and running the project for data collection mode:
----------------------------------------------------------

1. Verify that the following macros are set for data collection mode in
   the source file `app_config.h <inc/app_config.h>`__.

::

   #define S3AI_FIRMWARE_IS_COLLECTION  1		/* Enable sensor data collection    */
   #define S3AI_FIRMWARE_IS_RECOGNITION 0		/* Enable knowledgepack recognition */


2. Use the provided `Makefile <GCC_Project/Makefile>`__ and an
   appropriate ARM GCC toolchain to build the project

3. Use the flash programming procedure to flash the binary to
   QuickThing+ board.

4. Reset the board to start running the qt_ssi_ai_app application.

5. Use `Data Capture
   Lab <https://sensiml.com/products/data-capture-lab/>`__ to connect,
   stream and capture the sensor data.

   Audio data stream could also be captured using the python script
   `getaudio.py <../../Tools/uartaudio/getaudio.py>`__. 

Building and running the project for recognition mode:
------------------------------------------------------

1. Verify that the following macros are set for recognition mode in the
the source file `app_config.h <inc/app_config.h>`__.

::

   #define S3AI_FIRMWARE_IS_COLLECTION  0		/* Enable sensor data collection    */
   #define S3AI_FIRMWARE_IS_RECOGNITION 1		/* Enable knowledgepack recognition */

2. Use the provided `Makefile <GCC_Project/Makefile>`__ and an
   appropriate ARM GCC toolchain to build the project

3. Use the flash programming procedure to flash the binary to
   QuickThing+ board.

4. Reset the board to start running the qt_ssi_ai_app application.

5. Connect a UART to the QuickThing+ board. Open a terminal
   application, set its baud to 460800 bps to get the recognition
   results from the running application.

For details on data collection, building an AI model, and recognition
please refer to `SensiML Getting
Started <https://sensiml.com/documentation/guides/getting-started/index.html>`__

Adding a sensor
---------------

The default project uses the onboard Accelerometer sensor for data
collection. This section provides basic guideline on adding a new sensor
to the project for data collection. Sensor data acquisition and
processing or transfer to an external application such as `Data Capture
Lab <https://sensiml.com/products/data-capture-lab/>`__ uses `datablock
manager <../../qf_vr_apps#datablock-manager>`__
(qorc-sdk/Libraries/DatablockManager) and `datablock
processor <../../qf_vr_apps#datablock-processor>`__
(qorc-sdk/Tasks/DatablockProcessor) for acquiring samples and processing
these acquired samples. Qorc-sdk uses `Simple Streaming
Interface <https://sensiml.com/documentation/simple-streaming-specification/introduction.html>`__
protocol to send the acquired sensor data over to the [DataCaptureLab]

FreeRTOS software timer is used to trigger a timer event to read 1
sample from the sensor and fill the datablock. When enough samples are
collected (determined by the sensor sample rate, and latency), the
datablock is processed and the samples are sent over the UART using the
`Simple Streaming
Interface <https://sensiml.com/documentation/simple-streaming-specification/introduction.html>`__.

The `Wire interface <inc/Wire.h>`__ may be used to provide the requrired configuration
and sample acquisition member functions to configure and read data from
the new sensor.

Configure the sensor
--------------------

To add a new sensor start with the sensor_ssss.h and sensor_ssss.c
source files. Sensor sampling rate and number of channels are specified
in the macros defined in the header file sensor_ssss.h. Configuring and
reading from the sensor requires atleast 3 member functions:

-  begin() this member function initializes and configures the new
   sensor.
-  set_sample_rate() this member function sets the desired sample rate
   for this sensor
-  read() reads 1 sample of data from this sensor. To make synchronizing
   and fusing multiple sensor data easier, this function simply retries
   the current sample available in the sensor and returns the value.

.. _sensor_ssssh:

sensor_ssss.h
~~~~~~~~~~~~~

Modify the header file sensor_ssss.h and update the following macros

-  SENSOR_SSSS_SAMPLE_RATE to specify the desired sensor sampling rate
-  SENSOR_SSSS_CHANNELS_PER_SAMPLE to specify the desired number of
   channels for the new sensor
-  SENSOR_SSSS_LATENCY default latency is set to 20ms. This value
   determines how often the samples are processed and transmitted to the
   DCL (`Data Capture
   Lab <https://sensiml.com/products/data-capture-lab/>`__). The default
   value may be left as is.

The above macros determine the number of samples held in one datablock.
These datablocks are held in the array sensor_ssss_data_blocks[].

.. _sensor_ssssc:

sensor_ssss.c
~~~~~~~~~~~~~

-  Update the function sensor_ssss_configure() to initialize and setup
   the sensor configuration. The example code uses the following code
   snippet to configure the onboard accelerometer sensor.

   ::

      LIS2DH12  qorc_ssi_accel;

   ::

      qorc_ssi_accel.begin();
      qorc_ssi_accel.set_sample_rate(sensor_ssss_config.rate_hz);
      qorc_ssi_accel.set_mode(LIS2DH12_MODE_CWAKE);

Output data description
-----------------------

Update the string value definition of json_string_sensor_config in
sensor_ssss.cpp for the new sensor added to this project. The example
project which uses 3-channel onboard accelerometer is described using
the following string:

::

       {
          sample_rate:100,
          samples_per_packet:6,
          column_location:{
             AccelerometerX:0,
             AccelerometerY:1,
             AccelerometerZ:2
          }
       }

Refer the SensiML `Data Capture
Lab <https://sensiml.com/products/data-capture-lab/>`__ for details

Acquring and processing sensor samples
--------------------------------------

Based on the sensor sample rate, a FreeRTOS soft timer triggers
requesting 1 sensor sample to be filled-in the datablock.

.. _sensor_ssssc-1:

sensor_ssss.c
~~~~~~~~~~~~~

-  Update the function sensor_ssss_acquisition_buffer_ready to read 1
   sample (16-bits per channel) into the current datablock. This
   function returns 1 if datablock is ready for processing, returns 0
   otherwise.

   The example code uses the following code snippet to configure the
   onboard accelerometer sensor.

.. code:: c++

       xyz_t accel_data = qorc_ssi_accel.read();  /* Read accelerometer data from LIS2DH12 */
       
       /* Fill this accelerometer data into the current data block */
       int16_t *p_accel_data = (int16_t *)p_dest;
       
       *p_accel_data++ = accel_data.x;
       *p_accel_data++ = accel_data.y;
       *p_accel_data++ = accel_data.z;
       
       p_dest += 6; // advance datablock pointer to retrieve and store next sensor data

Capturing the sensor samples
----------------------------

-  Sensor samples are sent using the `Simple Streaming
   Interface <https://sensiml.com/documentation/simple-streaming-specification/introduction.html>`__.
   A 16-bit little-endian data format is used for sending each channel's
   sample data. QuickThing+ uses either an S3 UART. Sensor samples may be captured using `Data
   Capture Lab <https://sensiml.com/products/data-capture-lab/>`__

Accelerometer sensor example
----------------------------

An example accelerometer LIS2DH12 sensor available onboard is
provided as part of this application. The LIS2DH12 class interface to
configure and read data from the sensor is available in the source files
lis2dh12_wire.cpp and lis2dh12_wire.h. The sensor configuration function
sensor_ssss_configure() uses the begin() function of the class LIS2DH12 to
configure and set up the accelerometer for acquiring samples
approximately at the chosen sampling rate (SENSOR_SSSS_SAMPLE_RATE).

To read samples the configured sampling rate, sensor data read is
performed when the FreeRTOS soft timer triggers the function
sensor_ssss_acquisition_buffer_ready(). The read() member function is
used to read three 16-bit samples and fill-in the current data block.
When 20ms (= SENSOR_SSSS_LATENCY) samples are filled in the data block,
these samples are processed by the function
sensor_ssss_livestream_data_processor() to send these samples over UART
using `Simple Streaming
Interface <https://sensiml.com/documentation/simple-streaming-specification/introduction.html>`__.

SparkFun ADS1015 Example
------------------------

This section describes the steps to add `SparkFun Qwiic 12-bit
ADC <https://www.sparkfun.com/products/15334>`__ sensor (ADS1015) to
this project.

Obtain the `SparkFun ADS1015 Arduino
Library <https://github.com/sparkfun/SparkFun_ADS1015_Arduino_Library/tree/master/src>`__
code and add these source files to the qf_ssi_ai_app/src folder. Update
the SparkFun_ADS1015_Arduino_Library.cpp to resolve the missing function
delay(), and provide definitions for the following data types

-  boolean
-  byte

Update sensor_ssss.h and sensor_ssss.cpp as described in the above
sections. For example, to replace the accelerometer with only the
`SparkFun Qwiic 12-bit ADC <https://www.sparkfun.com/products/15334>`__
sensor update following macro definition for
SENSOR_SSSS_CHANNELS_PER_SAMPLE in sensor_ssss.h with the following code
snippet:

::

   #define SENSOR_SSSS_CHANNELS_PER_SAMPLE  ( 1)  // Number of channels

Add a class instance of the ADS1015 to the source file sensor_ssss.cpp
as shown below:

::

       ADS1015 qorc_ssi_adc ;

Update the function sensor_ssss_configure in sensor_ssss.cpp to replace
the accelerometer initialization and sample readings with following code
snippet:

::

     qorc_ssi_adc.begin();
     qorc_ssi_adc.setSampleRate(sensor_ssss_config.rate_hz);

Update the sensor_ssss_acquisition_buffer_ready function in
sensor_ssss.cpp to replace the accelerometer sensor reading with the
following code snippet to read Channel 3 of the ADS1015 sensor.

::

       int16_t adc_data = qorc_ssi_adc.getSingleEnded(3);
       *p_dest = adc_data;
       p_dest += 1; // advance datablock pointer to retrieve and store next sensor data    

Update the string value definition of json_string_sensor_config in
sensor_ssss.cpp as described in above section.

Build and load the project to the QuickThing+.

Connect a `SparkFun Qwiic 12-bit
ADC <https://www.sparkfun.com/products/15334>`__ sensor to the
QuickThing+ using the following pinouts

============== ============
ADS1015 module QuickThing+
============== ============
SCL            J2.11
SDA            J2.12
GND            J8.16
Vcc            J8.15
============== ============

SparkFun Qwiic Scale NAU7802 Example
------------------------------------

This section describes the steps to add `SparkFun Qwiic Scale -
NAU7802 <https://www.sparkfun.com/products/15242>`__ sensor to this
project.

Obtain the `SparkFun Qwiic Scale NAU7802 Arduino
Library <https://github.com/sparkfun/SparkFun_Qwiic_Scale_NAU7802_Arduino_Library>`__
code and add these source files to the qf_ssi_ai_app/src folder. Update
the SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.cpp to resolve the
missing functions delay(), and millis()

Add a class instance of the NAU7802 to the source file sensor_ssss.cpp
as shown below:

::

       NAU7802 qorc_ssi_scale;

Update sensor_ssss.h and sensor_ssss.cpp as described in the above
sections. For example, to replace the accelerometer with only the
`SparkFun Qwiic Scale -
NAU7802 <https://www.sparkfun.com/products/15242>`__ sensor update
following macro definition for SENSOR_SSSS_CHANNELS_PER_SAMPLE in
sensor_ssss.h with the following code snippet:

.. code:: c++

   #define SENSOR_SSSS_CHANNELS_PER_SAMPLE  ( 1)  // Number of channels

Update the function sensor_ssss_configure in sensor_ssss.cpp to replace
the accelerometer initialization and sample readings with following code
snippet:

.. code:: c++

     qorc_ssi_scale.begin();
     qorc_ssi_scale.setSampleRate(sensor_ssss_config.rate_hz);

Update the sensor_ssss_acquisition_buffer_ready function in
sensor_ssss.cpp to replace the accelerometer sensor reading with the
following code snippet to read a sample from the scale. Qwiic scale
outputs a 24-bit value where as the data capture is only capable of
16-bit sensor readings. So, adjust the returned reading to write 16-bit
value into the datablock buffer as shown in the code snippet below.

.. code:: c++

       int16_t scale_data = qorc_ssi_scale.getReading() >> 8;
       *p_dest = scale_data;
       p_dest += 1; // advance datablock pointer to retrieve and store next sensor data    

Update the string value definition of json_string_sensor_config in
sensor_ssss.cpp as described in above section.

Build and load the project to the QuickThing+.

Connect a `SparkFun Qwiic Scale -
NAU7802 <https://www.sparkfun.com/products/15242>`__ sensor to the
QuickThing+ using the following pinouts

============== ============
NAU7802 module QuickThing+
============== ============
SCL            J2.11
SDA            J2.12
GND            J8.16
Vcc            J8.15
============== ============

Refer `Qwiic Scale Hookup
Guide <https://learn.sparkfun.com/tutorials/qwiic-scale-hookup-guide?_ga=2.193267885.1228472612.1605042107-1202899191.1566946929>`__
for details. QuickThing+ is now ready to stream data to `Data Capture
Lab <https://sensiml.com/products/data-capture-lab/>`__
