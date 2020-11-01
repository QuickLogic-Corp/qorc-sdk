# -*- coding: utf-8 -*-
# convert_b64towav.py converts Base64 encoded audio lines into wav file
# Copy Right: QuickLogic

"""
Base64 to wav file converter
"""
import numpy as np
import matplotlib.pyplot as plt
import random
import datetime
import os

import base64

import sys
import struct
import wave
import argparse

#Note: These definitions are based on QF Application to auto detect whether it
# Raw Audio data or Opus Encoded Audio data. So change the values if the QF 
# Application prints different line sizes
RAW_AUDIO_LINE_LENGTH=81
OPUS_AUDIO_LINE_LENGTH=21

#just for test re-encode and compare
def encode_raw_data(output_data):
    a_length = len(output_data)
    #print(a_length)
    n_lines = a_length/60
    count = 0
    eol_byte = bytearray('\n', 'ascii')
    out = bytearray()

    while count != a_length:
        out.extend(bytearray(base64.b64encode(output_data[count:count+60])))
        out.extend(eol_byte)
        count += 60

    with open('a2.b64', 'wb') as f:
        f.write(out)

def decode_line_raw(line):
    dec_line = bytearray(base64.b64decode(line[:-1]))
    #out = bytearray(base64.b64encode(dec_line))
    #print('enc and dec')
    #print(line.decode())
    #print(out.decode())
    
    return dec_line


def decode_base64_file(input_file, output_file):
    count = 0
    output_array = bytearray() 
    with open(input_file, 'r') as f:
        start_found = 0
        encode_type = ''
        while True:
            line = f.readline()
            line_length = len(line)
            if(line_length == RAW_AUDIO_LINE_LENGTH):
                if (start_found == 0):
                    start_found = 1
                    encode_type = 'raw_audio'
                output_array.extend(decode_line_raw(bytearray(line, 'ascii')))
            else:
                if(start_found == 1):
                    break
            if not line:
                break

            #print(line)
            
            count +=1
            #if (count == 10):
            #    break
    #encode_raw_data(output_array)
        
    if(len(output_array) > 0):
        
        #just for test save as bin file
        #with open('audio.bin', 'wb') as w:
        #    w.write(output_array)

        #save data as wav file
        with wave.open(output_file, 'wb') as w:
            w.setsampwidth(2)           # 16 bit samples
            w.setframerate(16000)       # set sample rate as 16KHz
            w.setnchannels(1)           # number of channels is 1
            w.writeframes(output_array)
    return len(output_array)/2   # return the number of samples stored


def main(cmdline):
    parser = argparse.ArgumentParser(description='Info - Converts Base64 encoded file into an Audio WAV file. \n'
                   + ' An input file with Base64 encoded data must be supplied. \n'
                   + ' Output file names is optional. An output Audio file is always generated and saved with .wav extension. \n'
                   + ' If no output file is specified, same name as input file with .wav extension will be used.  '
                    )
    parser.add_argument("-i", "--input", dest="input_file", metavar='\'file\'',
                        help="Input Base64 encoded file")

    parser.add_argument("-o", "--output", dest="output_file", metavar='\'file\'',
                        help="Output WAV file")

    input_args = parser.parse_args()
    
    #print(f'input filename { input_args.input_file }')
    #print(f'outut filename { input_args.output_file }')
    
    
    if(input_args.input_file == None):
        #input_file = 'audio.b64'
        print(f'\n---Error in the Usage - No input file supplied --- \n\n')
        parser.print_help()
        return
    else:
        input_file = input_args.input_file
        
    if(input_args.output_file == None):
        dot_index = input_file.find('.')
        if(dot_index >= 0):
            output_file = input_file[:dot_index] + '.wav'
        else:
            output_file = input_file + '.wav'
    else:
        wav_extn = input_args.output_file.find('.wav')
        if (wav_extn > 0):
            output_file = input_args.output_file
        else:
            output_file = input_args.output_file + '.wav'

    print(f'input filename =  { input_file }')
    print(f'outut filename =  { output_file }')
    
    stored_samples = int(decode_base64_file(input_file, output_file))
    if(stored_samples > 0):
        print(f'  Stored {stored_samples} samples')

if __name__ == '__main__':
    args = sys.argv
    main(args)

