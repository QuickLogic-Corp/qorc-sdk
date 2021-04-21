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
parser = argparse.ArgumentParser(description="Capture audio data from SSI App")
parser.add_argument('--port', dest='comport', default='COM5', type=str, help='COM port, default is COM5')
parser.add_argument('--out', dest='audio_file', default='audio.bin', type=str, help='Audio filename, default is audio.bin')
args = parser.parse_args()
print(args)
sample_count=0
samples_per_sec = 0
syncString = b'AUDIOSTREAMSTART'
with open(args.audio_file, 'wb') as af:
     with serial.Serial(args.comport, 460800, timeout=1) as ser:
        ser.reset_input_buffer()
        ser.write(b'connect')
        s = ser.read(4096)
        if syncString in s:
           indx = s.find( syncString )
           print('Audio stream started', indx, s)
           af.write(s[indx+16:])
        start = time.time()
        while True:
             try:
               s = ser.read(480) # read 240 samples
               af.write(s)
               sample_count = sample_count + len(s)
               samples_per_sec = samples_per_sec + len(s)
               if time.time() - start > 1:
                  start = time.time()
                  samples_per_sec = 0
               print('samples = ', samples_per_sec, time.time()-start)
             except KeyboardInterrupt:
               print('{} samples written to {}'.format(sample_count, args.audio_file))
               ser.write(b'disconnect')
               break

