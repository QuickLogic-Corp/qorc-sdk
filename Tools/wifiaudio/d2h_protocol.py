# -*- coding: utf-8 -*-
# d2h_protocol.py decodes the streaming data using D2H protocol
# Copy Right: QuickLogic

"""
D2H protocol decoder
"""
import sys
import struct
import wave
import datetime
import os

#
# D2H Protocol uses 8 byte Pkt Header + (optional) varaible size Data
#
# Pkt Header = { byte[0] byte[1]  | byte[2] byte[3] | byte[4] byte[5] byte[6]  byte[7] }
#
# { byte[0], byte[1]} - 16-bit fiekd, interpreted as
#                           - 4-bit sequence number 
#                           - 6-bit channel number
#                           - 6-bit event type
# { byte[2], byte[3]} - 16-bit length of the data after Header
# { byte[4], byte[5] , byte[6], byte[7]} - 32-bit datapointer, not used for wifi 
#--------------------------------------------------------------------------------------------------------------------------------
# sequence    = byte[0] bits [7:4]
# channel     = {byte[0] bits [3:0], byte[1] bits [7:6] }
# event       = byte[1] bits [5:0]
# datalength  = {byte[2] byte[3] }
# datapointer = { byte[4], byte[5] , byte[6], byte[7]} - ignored
#

# These are constants taken from D2HProtocol library
#from d2h_protocol.h file
MAX_NUM_CHANNEL  = 64
MAX_DATA_LEN_IN_BYTES = 6
H2D_PACKET_SIZE_IN_BYTES = 8

#from d2h_protocol.c file
H2D_DATA_NUM_BYTES = 6
H2D_SEQ_MASK       = 0xF
H2D_SEQ_BIT_POS    = 4
H2D_CHANNEL_MASK_0 = 0xF
H2D_CHANNEL_MASK_1 = 0x3
H2D_CHANNEL_MASK   = 0x3F
H2D_CHANNEL_BIT_POS= 2
H2D_CMD_MASK       = 0x3F

#from s3_host_proto_defs.h file
EVT_KP_DETECTED     = 0x10
EVT_OPUS_PKT_READY  = 0x11
EVT_OPUS_PKT_END    = 0x12
EVT_THRESHOLD       = 0x13
EVT_PING            = 0x14
EVT_STREAM_KP_DETECTED   = 0x15
EVT_RAW_PKT_READY_2   = 0x16
EVT_RAW_PKT_END_2     = 0x17
EVT_RAW_PKT_READY_3   = 0x18
EVT_RAW_PKT_END_3     = 0x19
EVT_RAW_PKT_READY   = 0x21
EVT_RAW_PKT_END     = 0x22
EVT_MUTE_START      = 0x30
EVT_MUTE_STOP       = 0x31
EVT_PTT_START       = 0x32
EVT_PTT_STOP        = 0x33
EVT_EOT             = 0x34
EVT_GET_MONINFO     = 0x35,
EVT_COMMAND_ACK     = 0x3F

#This is the max Tx buffer from S3. Should match the S3 code.
MAX_TX_DATA_BUFFER_SIZE = (4*1024) 

# This recieves stream data and decodes D2H Pkts
# When the EVT_EOT pkt is received, save the Audio data to WAV file
class D2H_Decoder:
    def __init__(self):
        self.reset_session()
    
    def reset_session(self):
      self.rx_buffer = b""
      self.tx_buffer = b""
      self.rx_buflength = 0
      
      self.audio_buffer = b""
      self.msg_buffer = b""
      self.audio_channels = 1
      
      self.rx_pkt_seq = 0
      self.rx_pkt_chan = 0
      self.rx_pkt_event = 0
      self.rx_pkt_datasize = 0
      self.rx_pkt_pointer = 0
      self.rx_pkt_data = b""
      
      self.rx_pkt_count = 0
      self.rx_raw_buf_count = 0
      self.rx_pkt_errors = 0

      return


    def dec_pkt_hdr(self, hdr):
      #print(f'Decoding .. { hdr[0:8] } ')
      # this must start with Pkt header. decode it
      seq = (hdr[0] >> H2D_SEQ_BIT_POS) & H2D_SEQ_MASK
      channel = (hdr[0] << 8) | hdr[1]
      channel = (channel >> (8 - H2D_CHANNEL_BIT_POS)) & H2D_CHANNEL_MASK
      event = hdr[1] & H2D_CMD_MASK
      data_size = (hdr[3] << 8) | hdr[2] # since little endian short integer

      self.rx_pkt_seq = seq
      self.rx_pkt_chan = channel
      self.rx_pkt_event = event
      self.rx_pkt_datasize = data_size
      self.rx_pkt_pointer = (hdr[4] << 24) | (hdr[5] << 16) | (hdr[6] << 8) | (hdr[7])

      self.rx_pkt_data = b""
      self.rx_pkt_count += 1

      if self.rx_pkt_datasize > MAX_TX_DATA_BUFFER_SIZE:
        print(f'Error Pkt Hdr Data Size { self.rx_pkt_datasize }, exceeded max { MAX_TX_DATA_BUFFER_SIZE }. Ignoring ..')
        self.rx_pkt_datasize = 0
        
      #for debugging use Pointer value as a signature
      if self.rx_pkt_pointer != 0xABC00DEF:
         self.rx_pkt_errors += 1
         print(f'Error invalid Pkt Hdr signature { hex(self.rx_pkt_pointer) }, expecting 0xABC00DEF')
      
      # remove the processed hdr bytes for rx_buffer
      self.rx_buffer = self.rx_buffer[H2D_PACKET_SIZE_IN_BYTES : self.rx_buflength]
      self.rx_buflength = len(self.rx_buffer)

      if (data_size == 0):
        print(f'Received  { self.rx_pkt_event } event') 
      return
    
    def dec_rx_pkt(self, pkt_data):
      new_data = bytearray(pkt_data[0:len(pkt_data)])
      self.rx_buffer += new_data
      self.rx_buflength = len(self.rx_buffer)
      while True:
          # decode pkt only after if full header is received
          if self.rx_buflength < H2D_PACKET_SIZE_IN_BYTES :
            return 0

          if self.rx_pkt_datasize == 0:
            self.dec_pkt_hdr(self.rx_buffer[0:H2D_PACKET_SIZE_IN_BYTES])

          #take data until the Pkt datasize
          data_size = self.rx_pkt_datasize - len(self.rx_pkt_data)
          if (self.rx_buflength < data_size):
            data_size = self.rx_buflength

          if (data_size > 0):
            self.rx_pkt_data += self.rx_buffer[0: data_size]
            self.rx_buffer = self.rx_buffer[data_size : self.rx_buflength]
            self.rx_buflength -= data_size

          #copy the data and reset pkt variables if fully received
          if (self.rx_pkt_datasize > 0) and (self.rx_pkt_datasize == len(self.rx_pkt_data)):
             self.rx_pkt_datasize = 0
             if (self.rx_pkt_event == EVT_RAW_PKT_READY):
                self.audio_channels = 1
                self.audio_buffer += self.rx_pkt_data
                self.rx_raw_buf_count += 1
                print(f'.{ self.rx_raw_buf_count }', end="");
             elif (self.rx_pkt_event == EVT_RAW_PKT_READY_2):
                self.audio_channels = 2
                self.audio_buffer += self.rx_pkt_data
                self.rx_raw_buf_count += 1
                print(f'.{ self.rx_raw_buf_count }', end="");
             elif (self.rx_pkt_event == EVT_RAW_PKT_READY_3):
                self.audio_channels = 3
                self.audio_buffer += self.rx_pkt_data
                self.rx_raw_buf_count += 1
                print(f'.{ self.rx_raw_buf_count }', end="");
             else:
                self.msg_buffer += self.rx_pkt_data
                #print(f'Received  { self.rx_pkt_event } event')
                #print(str(self.msg_buffer));
                self.msg_buffer = b""
      return

    def check_end(self):
      if self.rx_pkt_event == EVT_EOT :
        return 1
      else:
        return 0


    # Adds 1000 silence samples to all channels in the audio buffer
    # This can be used to receive multiple sessions with EVT_EOT
    def append_silence(self):
        silence_bytes = bytearray(1000*2*self.audio_channels)
        self.audio_buffer += silence_bytes
        return

    # This appends 100 large positive samples (0x5F5F) followed 
    # by 100 large negative samples (0xA0a0) 5 times
    # This will be more visible square wave when looking at Audio wave 
    def append_square_wave(self):
        dc_bytes1 = bytearray([0x5F]*100*2*self.audio_channels)
        dc_bytes2 = bytearray([0xA0]*100*2*self.audio_channels)
        for x in range(5):
            self.audio_buffer += dc_bytes1
            self.audio_buffer += dc_bytes2
        return

    # Store all the collected Audio data as wav file with multiple channels
    def store_audio(self, output_file):
        audio_length = len(self.audio_buffer)
        print(f'Received a total of { self.rx_pkt_count } stream pkts')
        print(f'Received { self.rx_raw_buf_count } audio data pkts')
        if (self.rx_pkt_errors):
            print(f'===There are { self.rx_pkt_errors } Pkt decoding errors')
        else:
            print(f'There are no Protocol errors')
        if(audio_length == 0):
            print(f'===There is no Audio data to save===')
            return
        else:
            print(f'Audio buffer of size { audio_length } bytes received')
            print(f'Audio data of length = { audio_length } samples, channels = { self.audio_channels }')

        #save data as wav file
        try:
            with wave.open(output_file, 'wb') as w:
                w.setsampwidth(2)           # 16 bit samples
                w.setframerate(16000)       # set sample rate as 16KHz
                w.setnchannels(self.audio_channels)  # number of channels is 1 or more
                w.writeframes(self.audio_buffer)
                audio_length = audio_length/(2*self.audio_channels)
                print(f'Saved Audio data to file { output_file }')
        except:
            print(f'===Error saving Audio data to file { output_file }')

        return 
