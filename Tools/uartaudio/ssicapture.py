"""Simple streaming sensor data capture over UART or decode raw SSI data file

This script capture sensor data over UART (or USB2Serial) from Quickfeather 
running simple streaming application project (qf_ssi_ai_app). 

This script also provides an offline decoder to decode a raw SSI data file 
in SSI enhanced protocol

usage: ssicapture.py [-h] [infile | --port COMPORT] [--out OUTFILE]
                     [--txport TXPORT] [--rawdata rawfile] [--payload length]
                     [infile]

Capture sensor data from SSI App

optional arguments:
  -h, --help        show this help message and exit
  --port COMPORT    COM port, default is None
  --baud baudrate   COM port baudrate (default is 115200)
  --out OUTFILE     Output filename, default is outfile.bin
  --txport TXPORT   COM port to send connect/disconnect commands
  --rawdata rawfile save raw data to rawfile
  --payload length  expected payload length in each packet, 
                    default is 480 bytes
  infile            Input file to decode
"""

import serial
import argparse
import time
import struct
import itertools
import logging
import sys

parser = argparse.ArgumentParser(description="Capture sensor data from SSI App")
parser.add_argument('infile', nargs='?', type=argparse.FileType('rb'), default=None)
parser.add_argument('--port', dest='comport', default=None, type=str,
                    help='capture data from COM port')
parser.add_argument('--baud', dest='baudrate', default=115200, type=int,
                    help='baudrate, default is 115200')
parser.add_argument('--txport', dest='txport', default=None, type=str)
parser.add_argument('--out', dest='outfile', default='outfile.bin', type=str,
                    help='Output filename, default is outfile.bin')
parser.add_argument('--rawdata', dest='rawdata', default=None, type=str,
                    help='raw data filename')
parser.add_argument('--payload', dest='expected_dlen', default=480, type=int, 
                     help='Expected payload length')

def decodessiv2(serbytes, payload_length=480):
    decodessiv2.seqnum = getattr(decodessiv2, 'seqnum', 0)
    decodessiv2.prevseq = getattr(decodessiv2, 'prevseq', 0)
    decodessiv2.sync_data = getattr(decodessiv2, 'sync_data', b'\xFF')
    decodessiv2.bytes_in = getattr(decodessiv2, 'bytes_in', 0)
    decodessiv2.bytes_out = getattr(decodessiv2, 'bytes_out', 0)
    decodessiv2.bytes_dropped = getattr(decodessiv2, 'bytes_dropped', 0)
    decoded_bytes = b''
    
    decodessiv2.bytes_in = decodessiv2.bytes_in + len(serbytes)
    while (len(serbytes) >= 8):
      # scan for sync byte
      indx = serbytes.find( decodessiv2.sync_data )
      if (indx == -1) or ( len(serbytes) < (indx+7) ):
         break
      sync, seqnum, dlen = struct.unpack("<BIH", serbytes[indx:(indx+7)])
      if (dlen == payload_length):
        if (len(serbytes) >= (indx+dlen+8)):
          decodessiv2.prevseq = decodessiv2.seqnum
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
            serbytes = serbytes[indx+8+dlen:]
            decodessiv2.bytes_in = decodessiv2.bytes_in - len(serbytes)
            decodessiv2.bytes_out = decodessiv2.bytes_out + len(textstr)
            return textstr, serbytes
          else:
            serbytes = serbytes[(indx+8+dlen):]
            logging.error('seqnum=%d checksum [%d, %d]', decodessiv2.seqnum, checksum, cs)
        else:
          logging.info('seqnum=%d sync found, need more data [%d:%d %d, %d, %d]', 
                     decodessiv2.seqnum, indx, len(serbytes), sync, seqnum, dlen)
          serbytes = serbytes[indx:]
          decodessiv2.bytes_in = decodessiv2.bytes_in - len(serbytes)
          decodessiv2.bytes_out = decodessiv2.bytes_out + len(decoded_bytes)
          return decoded_bytes, serbytes
      else:
        if decodessiv2.seqnum != decodessiv2.prevseq:
           logging.error('seqnum=%d header [%d:%d %d, %d, %d] ',
                    decodessiv2.seqnum, indx, len(serbytes), sync, seqnum, dlen)
           decodessiv2.prevseq = decodessiv2.seqnum
        serbytes = serbytes[1:]

    decodessiv2.bytes_in = decodessiv2.bytes_in - len(serbytes)
    decodessiv2.bytes_out = decodessiv2.bytes_out + len(decoded_bytes)
    return decoded_bytes, serbytes

args = parser.parse_args()
logging.basicConfig(stream=sys.stdout, level=logging.INFO, 
                    format='%(asctime)s %(levelname)s:%(message)s', datefmt='%I:%M:%S %p')
print(args)

if (args.comport != None):
  try:
    ser = serial.Serial(args.comport, args.baudrate, timeout=1)
    logging.info('Opened port %s', args.comport)
    if (args.txport == None):
      sertx = ser
    else:
      sertx = serial.Serial(args.txport, args.baudrate, timeout=1)
  except:
    logging.error('opening port %s', args.comport)
    raise SystemExit(1)
elif (args.infile != None):
    ser = args.infile
    logging.info('Opened stream %s', args.infile)
else:
    print('no input specified, exiting')
    raise SystemExit(1)

rawdatafd = None
if (args.rawdata != None):
   rawdatafd = open(args.rawdata, "wb")

bytes_in = getattr(decodessiv2, 'bytes_in', 0)
bytes_out = getattr(decodessiv2, 'bytes_out', 0)
bytes_dropped = 0
connected = False
with open(args.outfile, 'wb') as af:
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
       if (args.comport == None) and (args.infile != None) and (len(nextstr) == 0):
          logging.info('End of file, l=%d, r=%d, len=%d, ser=%s', l, r, len(nextstr), ser)
          break
       bytes_in = getattr(decodessiv2, 'bytes_in', 0)
       if (rawdatafd != None):
         rawdatafd.write(nextstr)
       nextstr = currstr + nextstr
       out, currstr = decodessiv2(nextstr, payload_length=args.expected_dlen)
       bytes_out = getattr(decodessiv2, 'bytes_out', 0)
       if (bytes_out > 0):
         af.write(out)
         if (not connected):
           connected = True
           logging.info('Connected')
       if time.time() - start > 60:
         start = time.time()
         bytes_in = getattr(decodessiv2, 'bytes_in', 0)
         bytes_out = getattr(decodessiv2, 'bytes_out', 0)
         bytes_dropped = (args.expected_dlen*bytes_in) // (args.expected_dlen+8)
         bytes_dropped = bytes_dropped - bytes_out
         logging.info('bytes in = %d, out = %d, dropped = %d', bytes_in, bytes_out, bytes_dropped)
     except KeyboardInterrupt:
       if (args.comport):
         sertx.write(b'disconnect')
       break
   bytes_in = getattr(decodessiv2, 'bytes_in', 0)
   bytes_out = getattr(decodessiv2, 'bytes_out', 0)
   bytes_dropped = (args.expected_dlen*bytes_in) // (args.expected_dlen+8)
   bytes_dropped = bytes_dropped - bytes_out
   logging.info('bytes in = %d, out = %d, dropped = %d', bytes_in, bytes_out, bytes_dropped)

