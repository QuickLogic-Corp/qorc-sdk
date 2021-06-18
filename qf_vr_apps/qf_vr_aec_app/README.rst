Voice recognition with AEC example application
==============================================

The purpose of this application is to provide a software framework to
easily plug-in a 3rd party Achostic Echo Canceller and Voice Recognition
engine for keyword detection. The example application waits for Audio
data to be played on Speaker and tries to do Keyword detection.
The Audio data that is being played on the Speaker is estimated by
the reference signal over I2S peripheral and removed from the signal
given to VR Engine using AEC Algorithm.

This application project should be used with a HUZZAH32 binary on a 
QF-DUO board.

The Application:
----------------

Generally the Application will have a Host Processor along with the Device
Processor (ie S3). Device Processor (S3) will be monitoring the Devices like
Microphone, Accelerometer, Gyroscope etc. When a voice command like "Alexa"
is said, S3 recognizes that and immediately wakes up the Host processor.

The Alexa phrase is recognized by Amazons Voice Recognition Engine (in the
example project) which takes ~28MHz on S3 processor. Once the Host processor
is woken up by S3, the Audio data is transmitted over SPI (for example) using
D2H Protocol from S3 to Host. Also, after the first recognition of "Alexa",
the VR Engine is turned off, which reduces the MHz needed by the S3.

The current AEC project allows Host to play Music or Audio on its own I2S 
peripheral, which will be picked up by the PDM Microphone connected to S3.
Since Host is up and running during this time, S3 need not worry about the
power while Host is playing the Audio. While the Host is playing the Audio,
the User may want to give an Alexa command. Since the playing of the Audio
can be loud, S3 may not be able to recognize the Alexa command.

To reduce the "noise" due to this play but recognize the Alexa phrase, S3
needs to run the AEC algorithm on the PDM signal before passing it to VR 
Engine. This is achieved through some Hardware and Software.

The Hardware:
-------------

The Audio data that is played by the Host on the Speaker needs to be obtained
by S3. So, it is sent to S3 over I2S (only 3 Pins) and a GPIO pin to indicate
it is valid. The I2S should connect CLK, WCLK, DIN. The GPIO pin can be any pin.

The Software:
-------------

S3 grabs the Audio data when the GPIO pin says it is valid, and uses it
to estimate the value picked up by the PDM microphone. It is done by the AEC
software which takes ~51MHz. The estimate is subtracted from the PDM input
and the VR Engine is fed with that.

Since AEC and VR Engine together (51MHz + 28MHz) along with the other Interrupt
processing (~5MHz) exceeds the S3 processor capacity of 80MHz, "VR Engine is
disabled" when I2S data from Host is valid.

The AEC algorithm starts when GPIO pin from  Host goes up. It stays high
as long as there is some Audio data on I2S line.

The delay between the PDM data and I2S reference data should be less than 32ms
due to the AEC Algorithm used in the Example. Since the DataBlock of size 15ms 
are used, the delay varies and should be within 32ms.

(Note that I2S Data from Host can be made to be 5ms instead of 15ms to increase
the accuracy of delay by reducing the size of I2S SDMA buffers.)


HUZZAH32 Software:
------------------

The software on Huzzah32 (ie ESP32 processor with Wi-Fi) will send 30sec of 
a song on I2S and will wait 2 minutes. This repeats continuously.

Everytime I2S data is sent, it is played on the UDA1334, which converts I2S data
to be played on the Speaker. By increasing the Speaker Volume, the data picked up
by S3 PDM can increased.





