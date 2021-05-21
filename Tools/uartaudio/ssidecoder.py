import argparse
import binascii
import struct
import sys
import itertools

parser = argparse.ArgumentParser(description="decode simple stream interface data")
parser.add_argument('infile', nargs='?', type=argparse.FileType('rb'), default=sys.stdin)
parser.add_argument('--out', dest='outfile', default='outfile.bin',
                      type=str, help='output filename, default is outfile.bin')
parser.add_argument('--payload', dest='expected_dlen', default=480, type=int, help='Expected payload length')
args = parser.parse_args()
print(args)

args.infile.seek(0, 2)
filesize = args.infile.tell()
args.infile.seek(0, 0)
# Scan the first 1024 byte to check for the AUDIOSTREAMSTART sync string
syncString = b'AUDIOSTREAMSTART'
textstr = args.infile.read(1024)
if syncString in textstr:
   indx = textstr.find( syncString )
   print('sync string %s found @ %d' % (syncString, indx))
   args.infile.seek(indx+16)
else:
   args.infile.seek(0)

syncString = b'\xFF'
prevseq = 0
inbytes = 0
outbytes = 0
packet_count = 0
expected_dlen = args.expected_dlen
with open(args.outfile, 'wb') as af:
  while True:
    textstr = args.infile.read(7)
    inbytes = inbytes + len(textstr)
    if (len(textstr) < 7):
       break
    sync, seqnum, dlen = struct.unpack("<BIH", textstr)
    if (sync == 255) and (dlen == expected_dlen):   # and (seqnum == (prevseq+1)):
       prevseq = seqnum
       textstr = args.infile.read(dlen)
       inbytes = inbytes + len(textstr)
       csstr = args.infile.read(1)
       if (len(csstr) < 1):
          break
       inbytes = inbytes + len(csstr)
       checksum = struct.unpack('<B', csstr)[0]
       payload_iter = struct.iter_unpack('<h', textstr)
       bytes_iter = struct.iter_unpack('B', textstr)
       bytes_data = [ b[0] for b in bytes_iter ]
       payload = [ d[0] for d in payload_iter ]
       cs = 0
       for b in bytes_data:
           cs = cs ^ b
       if (cs == checksum):
         outbytes = outbytes + dlen
         packet_count = packet_count + 1
         af.write(textstr)
       else:
         print('checksum [{}, {}] error at packet sequence number {}'.format(checksum, cs, seqnum))
    else:
       args.infile.seek(-6, 1)
       inbytes = inbytes - 6
       print('Error at packet sequence number ', (prevseq+1))
print('{} packets decoded, {} bytes output from a stream of {} bytes'.format(packet_count, outbytes, inbytes))
print('output filesize = {} bytes, input file size = {} bytes'.format(outbytes, filesize))

