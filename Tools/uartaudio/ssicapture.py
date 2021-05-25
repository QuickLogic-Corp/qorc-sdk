"""Simple streaming audio data capture over UART

This script capture audio data over UART (or USB2Serial) from Quickfeather 
running simple streaming application project (qf_ssi_ai_app). 

usage: getaudio.py [-h] [--port COMPORT] [--out AUDIO_FILE]

Capture audio data from SSI App

optional arguments:
  -h, --help        show this help message and exit
  --port COMPORT    COM port, default is COM5
  --out AUDIO_FILE  Audio filename, default is audio.bin

"""

import serial
import argparse
import time
import struct
import itertools

parser = argparse.ArgumentParser(description="Capture audio data from SSI App")
parser.add_argument('infile', nargs='?', type=argparse.FileType('rb'), default=None)
parser.add_argument('--port', dest='comport', default=None, type=str,
                    help='COM port, default is COM5')
parser.add_argument('--txport', dest='txport', default=None, type=str)
parser.add_argument('--out', dest='audio_file', default='audio.bin', type=str,
                    help='Audio filename, default is audio.bin')
parser.add_argument('--rawdata', dest='rawdata', default=None, type=str,
                    help='raw data filename, default is rawdata.bin')
parser.add_argument('--payload', dest='expected_dlen', default=224, type=int, 
                     help='Expected payload length')

def decodessiv2(serbytes, payload_length=224):
    decodessiv2.seqnum = getattr(decodessiv2, 'seqnum', 0)
    decodessiv2.sync_data = getattr(decodessiv2, 'sync_data', b'\xFF')
    decoded_bytes = b''
    #print('serial bytes = ', serbytes)
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
           #outbytes = outbytes + dlen
           #packet_count = packet_count + 1
           return textstr, serbytes[indx+8:]
         else:
           serbytes = serbytes[(indx+8):]
           print('checksum [{}, {}] error at packet sequence number {}'.format(checksum, cs, decodessiv2.seqnum))
      else:
         serbytes = serbytes[1:]
    return decoded_bytes, serbytes

args = parser.parse_args()
print(args)
sample_count=0
samples_per_sec = 0
samples_secs_length = 0
syncString = b'AUDIOSTREAMSTART'

if (args.comport != None):
  try:
    ser = serial.Serial(args.comport, 46080, timeout=1)
    if (args.txport == None):
      sertx = ser
    else:
      sertx = serial.Serial(args.txport, 115200, timeout=1)
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
with open(args.audio_file, 'wb') as af:
   if (args.comport):
      ser.reset_input_buffer()
      sertx.write(b'connect')
   start = time.time()
   currstr = b''
   while True:
     try:
       l = len(currstr)
       r = 512 - l
       nextstr = ser.read(r) # read upto 512 bytes
       if (len(nextstr) == 0):
          break
       bytes_in = bytes_in + len(nextstr)
       if (rawdatafd != None):
         rawdatafd.write(nextstr)
       nextstr = currstr + nextstr
       out, currstr = decodessiv2(nextstr, payload_length=args.expected_dlen)
       bytes_out = bytes_out + len(out)
       if (len(out) > 0):
          af.write(out)
       sample_count = sample_count + len(nextstr)
       samples_per_sec = samples_per_sec + len(nextstr)
       if time.time() - start > 1:
          start = time.time()
          samples_per_sec = 0
          samples_secs_length = samples_secs_length + 1
       if (samples_per_sec == 0) and ( samples_secs_length % 60 == 1 ):
          print('bytes in = ', bytes_in, 'out = ', bytes_out, 
               'samples = ', sample_count, samples_secs_length//60, ' mins')
     except KeyboardInterrupt:
       print('{} samples written to {}'.format(sample_count, args.audio_file))
       if (args.comport):
         sertx.write(b'disconnect')
       break

