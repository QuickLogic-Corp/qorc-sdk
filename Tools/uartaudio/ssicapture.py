"""Simple streaming audio data capture over UART or decode raw SSI data file

This script capture audio data over UART (or USB2Serial) from Quickfeather 
running simple streaming application project (qf_ssi_ai_app). 

This script also provides an offline decoder to decode a raw SSI data file 
in SSI enhanced protocol

usage: ssicapture.py [-h] [infile | --port COMPORT] [--out AUDIO_FILE]
                     [--txport TXPORT] [--rawdata rawfile] [--payload length]
                     [infile]

Capture audio data from SSI App

optional arguments:
  -h, --help        show this help message and exit
  --port COMPORT    COM port, default is None
  --baud baudrate   COM port baudrate (default is 460800)
  --out AUDIO_FILE  Audio filename, default is audio.bin
  --txport TXPORT   COM port to send connect/disconnect commands
  --rawdata rawfile save raw data to rawfile
  --payload length  expected payload length in each packet, 
                    default is 224 bytes
  infile            Input file to decode
"""

import serial
import argparse
import time
import struct
import itertools
import logging
import sys

parser = argparse.ArgumentParser(description="Capture audio data from SSI App")
parser.add_argument('infile', nargs='?', type=argparse.FileType('rb'), default=None)
parser.add_argument('--port', dest='comport', default=None, type=str,
                    help='capture data from COM port')
parser.add_argument('--baud', dest='baudrate', default=115200, type=int,
                    help='baudrate, default is 115200')
parser.add_argument('--txport', dest='txport', default=None, type=str)
parser.add_argument('--out', dest='audio_file', default='audio.bin', type=str,
                    help='Audio filename, default is audio.bin')
parser.add_argument('--rawdata', dest='rawdata', default=None, type=str,
                    help='raw data filename')
parser.add_argument('--payload', dest='expected_dlen', default=480, type=int, 
                     help='Expected payload length')

def decodessiv2(serbytes, payload_length=480):
    decodessiv2.seqnum = getattr(decodessiv2, 'seqnum', 0)
    decodessiv2.sync_data = getattr(decodessiv2, 'sync_data', b'\xFF')
    decoded_bytes = b''
    #print('in  = ', serbytes)
    while (len(serbytes) >= 8):
      # scan for sync byte
      indx = serbytes.find( decodessiv2.sync_data )
      serbytes = serbytes[indx:]
      if ( len(serbytes) < 7 ):
         break
      indx = 0
      #print("length = {} {}".format(len(serbytes), serbytes[indx:(indx+7)]))
      sync, seqnum, dlen = struct.unpack("<BIH", serbytes[indx:(indx+7)])
      if (dlen == payload_length) and (len(serbytes) >= (dlen+8)):
         decodessiv2.seqnum = decodessiv2.seqnum + 1
         textstr = serbytes[ (indx+7):(indx+7+dlen) ]
         csstr   = serbytes[ (indx+7+dlen):(indx+8+dlen) ]
         checksum = struct.unpack('<B', csstr)[0]
         payload_iter = struct.iter_unpack('<h', textstr)
         bytes_iter = struct.iter_unpack('B', textstr)
         bytes_data = [ b[0] for b in bytes_iter ]
         payload = [ d[0] for d in payload_iter ]
         cs = 0
         for b in bytes_data:
           cs = cs ^ b
         if (cs == checksum):
           #print('out = ', textstr)
           return textstr, serbytes[indx+8+dlen:]
         else:
           serbytes = serbytes[(indx+8+dlen):]
           #print('checksum [{}, {}] error at packet sequence number {}'.format(checksum, cs, decodessiv2.seqnum))
           logging.error('checksum [%d, %d] error at packet sequence number %d', checksum, cs, decodessiv2.seqnum)
      else:
         logging.error('sync [%d, %d, %d] error at packet sequence number %d', sync, seqnum, dlen, decodessiv2.seqnum)
         serbytes = serbytes[1:]
    return decoded_bytes, serbytes

args = parser.parse_args()
logging.basicConfig(stream=sys.stdout, level=logging.INFO, 
                    format='%(asctime)s %(message)s', datefmt='%Y-%d-%m %I:%M:%S %p')
print(args)
samples_per_sec = 0
samples_secs_length = 0
syncString = b'AUDIOSTREAMSTART'

if (args.comport != None):
  try:
    ser = serial.Serial(args.comport, args.baudrate, timeout=1)
    logging.info('Opened COMPORT %s', args.comport)
    if (args.txport == None):
      sertx = ser
    else:
      sertx = serial.Serial(args.txport, args.baudrate, timeout=1)
  except:
    print('Error opening port ', args.comport)
    raise SystemExit(1)
elif (args.infile != None):
      ser = args.infile
else:
    print('no input specified')
    raise SystemExit(1)

rawdatafd = None
if (args.rawdata != None):
   rawdatafd = open(args.rawdata, "wb")
bytes_in = 0
bytes_out = 0
bytes_dropped = 0
connected = False
with open(args.audio_file, 'wb') as af:
   if (args.comport):
      ser.reset_input_buffer()
      sertx.write(b'connect')
      logging.info('Connect request sent')
   start = time.time()
   currstr = b''
   while True:
     try:
       l = len(currstr)
       r = 512 - l
       nextstr = ser.read(r) # read upto 512 bytes
       if ((args.comport == None) and (args.infile != None) and len(nextstr) == 0):
          break
       bytes_in = bytes_in + len(nextstr)
       if (rawdatafd != None):
         rawdatafd.write(nextstr)
       nextstr = currstr + nextstr
       out, currstr = decodessiv2(nextstr, payload_length=args.expected_dlen)
       bytes_out = bytes_out + len(out)
       if (bytes_out > 0) and (not connected):
          connected = True
          logging.info('Connected')
       if (len(out) > 0):
          af.write(out)
       samples_per_sec = samples_per_sec + len(nextstr)
       if time.time() - start > 1:
          start = time.time()
          samples_per_sec = 0
          samples_secs_length = samples_secs_length + 1
       if (samples_per_sec == 0) and ( samples_secs_length % 60 == 1 ):
          bytes_dropped = (args.expected_dlen*bytes_in) // (args.expected_dlen+8)
          bytes_dropped = bytes_dropped - bytes_out
          logging.info('bytes in = %d, out = %d, dropped = %d', bytes_in, bytes_out, bytes_dropped)
     except KeyboardInterrupt:
       if (args.comport):
         sertx.write(b'disconnect')
       bytes_dropped = (args.expected_dlen*bytes_in) // (args.expected_dlen+8)
       bytes_dropped = bytes_dropped - bytes_out
       logging.info('bytes in = %d, out = %d, dropped = %d', bytes_in, bytes_out, bytes_dropped)
       break

