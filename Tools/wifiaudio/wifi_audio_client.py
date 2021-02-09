# -*- coding: utf-8 -*-
# wifi_streaming.py receives streaming audio data over wifi and saves as wav file
# Copy Right: QuickLogic

# For Python sockect programming reference see -
# https://realpython.com/python-sockets/
#

"""
Wifi streaming audio to wav file
"""
import random
import datetime
import os
import sys
import struct
import wave
import argparse
import socket
import selectors
import types
import time
import ipaddress

import d2h_protocol

# for test
HOST = "127.0.0.1"  # Standard loopback interface address (localhost)
DEFAULT_PORT = 3333 # Port to listen on (non-privileged ports are > 1023)
CLIENT_TIME_OUT   = 30 #(10*60) # 10 minutes
MAX_LISTEN_SKTS  = 1
MAX_AUDIO_BUF_SIZE = 100*1024*1024

d2h_dec = None
output_file = 'wifi_audio.wav'
time_out = CLIENT_TIME_OUT

def service_connection(slctr, key, mask):
    global d2h_dec
    global output_file
    global time_out
    skt = key.fileobj
    data = key.data
    if mask & selectors.EVENT_READ:
        time_out = (1000*CLIENT_TIME_OUT) # wait until user closes, so return a large value
        recv_data = skt.recv(1024)  # Should be ready to read
        if recv_data:
            data.outb += recv_data
            #if(len(recv_data) > 8):
            #    print("Rx data: ", recv_data[0:8])
            #print("Rx data: ", repr(recv_data))
            if(d2h_dec):
                d2h_dec.dec_rx_pkt(recv_data)
        else:
            print("closing connection to", data.addr)
            slctr.unregister(skt)
            skt.close()
    if mask & selectors.EVENT_WRITE:
        if data.outb:
            #print("echoing", repr(data.outb), "to", data.addr)
            #sent = skt.send(data.outb)  # Should be ready to write
            #data.outb = data.outb[sent:]
            data.outb = b""
        #slctr.unregister(skt)
        #skt.close()
    if(d2h_dec):
        if d2h_dec.check_end():
            #d2h_dec.append_silence()
            #d2h_dec.append_square_wave()
            #d2h_dec.append_silence()
            d2h_dec.store_audio(output_file)
            d2h_dec.reset_session()
            data.outb = b""
            
    return time_out

def start_wifi_client(host, port):

    #create a selector for async IO
    slctr = selectors.DefaultSelector()
    
    # open a TCP/IP socket
    skt = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    #use non-block for async IO
    skt.setblocking(False)
    server_addr = (host, port)
    skt.connect_ex(server_addr)
    data = types.SimpleNamespace(addr=host, port=port, inb=b"", outb=b"")
    events = selectors.EVENT_READ | selectors.EVENT_WRITE
    slctr.register(skt, events, data)

    global d2h_dec
    d2h_dec = d2h_protocol.D2H_Decoder()

    start_time = time.time()
    loop_time = time.time()
    client_timeout = CLIENT_TIME_OUT

    client_list = []
    client_list.append(skt)
    try:
        while time.time() < start_time + client_timeout  :
            events = slctr.select(timeout=5)
            #print(f"time = { int(time.time() - start_time) } seconds, events = { events } ")
            if not events:
                print(f"time = { int(time.time() - start_time) } seconds, events = { events } ")
                #print(f"time = { int(time.time() - start_time) } seconds")
            else:
                if time.time() > (loop_time + 5):
                    print(f"time = { int(time.time() - start_time) } seconds")
                    loop_time = time.time()
            for key, mask in events:
                client_timeout = service_connection(slctr, key, mask)

    except KeyboardInterrupt:
        print("Exiting client ")
    finally:
        for skt in client_list:
            skt.close()
        slctr.close()

    return 0

def validate_ip_address(ip_address):
    valid_ip = ip_address
    if (ip_address[0] == '\'') or (ip_address[0] == '\"'):
        valid_ip = ip_address[1:-1];
    try:
        ip = ipaddress.ip_address(valid_ip)
    except ValueError:
        print(f'===Error: IP address is invalid: { ip_address }')
        sys.exit(1)
    return valid_ip

def main(cmdline):
    parser = argparse.ArgumentParser(description='Info - Receives Audio Stream over Wifi and saves to an Audio WAV file. \n'
                   + ' An output Audio file is generated if server streams the Audio data and saved with .wav extension. \n'
                   + ' If no output file name is specified, saved as wifi_audio.wav. \n'
                   + ' The Server IP address has to be specified. \n'
                   + ' Optionally Port number can also be specified. \n'
                    )
    parser.add_argument("-o", "--output", dest="output_file", metavar='\'file\'',
                        help="Output WAV file")

    parser.add_argument("-s", "--server", dest="host", metavar='\'ip_address\'',
                        help="Server IP address to be used in the format x.x.x.x (ex:192.168.1.6)")

    parser.add_argument("-p", "--port", dest="port", metavar='\'port_number\'',
                        help="Server Port number to connect. Must be greater than 3000 and less than 32767 (ex: 3333)")

    input_args = parser.parse_args()
    
    global output_file
    if(input_args.output_file == None):
        print(f'\n---Using default outfile { output_file }--- \n')
    else:
        wav_extn = input_args.output_file.find('.wav')
        if (wav_extn > 0):
            output_file = input_args.output_file
        else:
            output_file = input_args.output_file + '.wav'

    print(f'output filename =  { output_file }')

    #host_name = socket.gethostname()
    #host_addr = socket.gethostbyname(host_name)
    #print(host_name);
    #print(host_addr);
    
    host = None
    port = DEFAULT_PORT

    if(input_args.host == None):
        print("ERROR - Must provide Server IP address using option -s ")
        sys.exit(1)
    else:
        host = validate_ip_address(input_args.host)

    if(input_args.port == None):
        port = DEFAULT_PORT
    else:
        try:
            port_number = int(input_args.port)
            if 3000 <= port_number <= 32767:
                port = input_args.port
            else:
                raise ValueError
        except ValueError:
            print("ERROR - Port number is not  valid. Should be a number between 3000 and 32767")
            sys.exit(1)

    print(f'Using server address = { host }, port = { port } => Client Timeout ={ CLIENT_TIME_OUT } seconds')
    print(f'===Press Ctrl + C and wait, to exit any time during client timeout=== ')
    
    start_wifi_client(host, port)


if __name__ == '__main__':
    args = sys.argv
    main(args)

