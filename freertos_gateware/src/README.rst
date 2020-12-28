USB-Serial driver link down Notes
=================================

The USB transmit FIFO will hold the outgoing data bytes until a host
terminal application starts draining this data. When the FIFO fills-up,
the USB transmit function may enter a busy-wait state if there is no
host application draining these data bytes. Accordingly, the end-user
application should ensure that transmit FIFO does not fill-up when there
is no host application that drains the data. An example to work around
this problem is provided in `MQTT-SN over USB-Serial
Notes <../qf_apps/qf_mqttsn_ai_app/README.rst#mqtt-sn-over-usb-serial-notes>`__
