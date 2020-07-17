# qf_usbspeed

The qf_usbspeed application is a simple application that can be used to test the write
performance of the USB serial link.

The application uses the HW UART (TX and RX pins on QuickFeather) for control, and
the USB serial IP attached to the microUSB connector for data transfer.

To use the application, connect the USB cable to a USB port on your computer, and connect a serial-to-USB
between another USB port and the RX & TX pins on QuickFeather.  From a command prompt connect a virtual terminal,
such as PuTTY to each of the USB ports.  The baud rate of the PuTTY connected to the HW UART should be set to 115,200.
The application will use this terminal as a console.  You control the write activity using the CLI (Command Line Interface) on the console.
The baud rate on the PuTTY connected to the USB serial port is ignored.

To measure USB write performance, enter the usb submenu and issue the 'write' command.
The write command will ask for the number of bytes to write, and when that is entered it will write that many bytes
on the USB serial port.  You will see data on the PuTTY connected to the USB serial port, and when the write is completed
you will see the elasped time on the console.