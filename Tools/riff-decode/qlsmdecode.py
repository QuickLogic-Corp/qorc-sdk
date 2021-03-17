# !/usr/bin/env python
"""
About QuickAI RIFF files

Assumed knowledge:

Read this: https://en.wikipedia.org/wiki/Resource_Interchange_File_Format

The file format for QuickAI RIFF files is a form of RIFF.

This file is defined as an array of RIFF blocks.

Each block begins with an 8 byte header.
* Bytes [0:3] is a little endian 32bit TAG defining the content.
* Bytes [4:7] is a little endian length of the data for this block.

* The LENGTH does not include the header component.
* Thus a 4K block would be 4096 = hdr(8) + payload(4088) bytes

Recursive Format
The format is recursive, a block might contain sub-blocks.

Padding & Alignment.
If required, all blocks are padded to 32bit boundaries with zeros.
The RIFF standard uses 2 byte alignment.

Nested Blocks & Padding
During parsing, if a TAG or LENGTH is found to be zero then
the rest of the block is unused and filled with zeros.

As an example: In the normal course of data capture only 'full RIFF blocks'
of data are written to the file, however the very last block might not be
full. Thus that block will be padded with ZEROs (null bytes)

This also occurs in the QLSM/JSON block in the header. This
block contains a JSON string describing the data capture configuration
The block is always at least 4K bytes long, the JSON string is much smaller.
Thus, zeros are used as padd after the JSON block within the header block.

"""

import os
from pathlib import Path
import sys
import struct
import datetime
import io
import json
import wave
import argparse
import pandas

PRINT_PARSING_DEBUG = False

# we handle the audio sensor special, so we must detect its presence.
# this magic number is defined in the embedded side.
audio_sensor_id = 1096107087


class ChunkParseError(Exception):
    """Raised when parsing a chunk goes bad"""
    pass


class InvalidExtensionException(Exception):
    pass


class AbsoluteTimeStampMismatchException(Exception):
    pass


class QLSMDataNotLoadedException(Exception):
    pass


class RIFF_Block():
    """
    This represents a RIFF format block
    A RIFF block begins with an 8 byte header
    Bytes [0:3] is a 32bit tag identifying the data following.
    Bytes [4:7] is the length in bytes of the data following.
    Both are Little Endian format.

    In this system, the tag 0x00000000 (zero) is illegal and never used.
    If found the end of something has been reached
    See below for more details (QLSM_FileHeader and QLSM_DataBlock)

    Note: All RIFF blocks are 32bit aligned
    thus if the (length MOD 4) == 1, then
    there are 3 bytes zero padding to enforce a 32bit alignment.
    The RIFF standard says padd to 2 bytes.

    For example a 4085 byte data block would be 4096 bytes long.

    Size:  (8 byte header) + (4085 bytes of data) + (3 nulls) = 4096
    Thus the block following this block will be 32bit aligned.

    """

    def __init__(self, file_block):
        self.tag = None
        self.payload = bytearray()
        self.file_block = file_block
        self.file_offset = 0

    def from_riff_block(self, riffblock):
        self.tag = riffblock.tag
        self.payload = riffblock.payload[:]
        self.file_offset = riffblock.file_offset

        # Point to parent block
        if riffblock.file_block is not None:
            self.file_block = riffblock.file_block


class QLSM_FileHeader(RIFF_Block):
    """
    A QLSM file is defined to be an array of RIFF blocks.
    Block 0 is a header, with the tag QLSM
    Block 1 to N are data blocks with the tag SNSR.

    Generally all blocks in a QLSM file are the same size
    (this simplfies the embedded software, but may change in future)

    This class represents the header block (tag=QLSM) at the start of a file

    This RIFF block contains one 'sub-element' with the tag JSON
    The JSON element describes the data in the other blocks.

    After the JSON element, the block is padded with zeros (null bytes)

    """

    def __init__(self, file_block):
        RIFF_Block.__init__(self, file_block)
        self.json = None

    def find_sensor_details(self, sensor_id):
        """Given a sensor id, find it's json data"""
        for n, v in self.json['sensors'].items():
            if v['sensor_id'] == sensor_id:
                return v
        # not found
        return None

    def parse(self):
        """Parse & validate the QLSM header block"""
        assert (self.tag == 'QLSM')
        # verify size of the block is a reasonable size & power of 2
        n = len(self.payload) + 8
        assert (((n & (n - 1)) == 0) and (n > 512))
        # parse the sub-chunks
        parser = ChunkParser()
        # parse the payload
        chunks = parser.parse_bytearray(self.file_block,
                                        self.file_offset,
                                        self.payload)
        assert (len(chunks) == 1)
        tmp = chunks[0]
        assert ('JSON' == tmp.tag)
        # extract json element
        eos = tmp.payload.find(0, 8)
        assert (eos > 0)
        # it's a null terminated string
        s = tmp.payload[0:eos].decode('ascii')
        self.json = json.loads(s)
        if PRINT_PARSING_DEBUG:
            print('')
            print('=======JSON==========')
            print(s)
            print('=======END===========')


class QLSM_SensorHDR(RIFF_Block):
    """
    All sensor data blocks use the tag 'SNSR'
    A SNSR block contains 2 sub blocks, a 'HDR!' and 'DATA' block.

    The HDR! contains information about the content of the DATA block.
    Specifically the HDR! contains the sensor id, and timestamp
    """

    def __init__(self, file_block, parent):
        RIFF_Block.__init__(self, file_block)
        self.parent = parent

    def parse(self):
        """Parse a HDR! block"""
        assert ('HDR!' == self.tag)
        assert (24 == len(self.payload))
        a, b, c, d = struct.unpack('<IIQQ', self.payload)
        self.parent.sensor_id = a
        self.parent.sequence_number = b
        self.parent.first_timestamp_usecs = c
        self.parent.last_timestamp_usecs = d
        if PRINT_PARSING_DEBUG:
            print('Block sensor: 0x%08x' % self.parent.sensor_id)
            print('sequence-number: %d' % self.parent.sequence_number)
        tmpC = datetime.datetime.fromtimestamp((float)(c) / 1000000.0)
        self.parent.first_timestamp_datetime = tmpC
        tmpD = datetime.datetime.fromtimestamp((float)(d) / 1000000.0)
        self.parent.last_timestamp_datetime = tmpD
        if PRINT_PARSING_DEBUG:
            print('Block start: %s' % tmpC.isoformat())
            print('       stop: %s' % tmpD.isoformat())

        tmp = self.file_block.header.find_sensor_details(self.parent.sensor_id)
        self.parent.json_info = tmp

        if (self.parent.json_info['version'] == 1) or (self.parent.json_info['version'] == 2):
            self._extract_json1()
        else:
            # future ...
            raise NotImplementedError('write me')

    def _extract_json1(self):
        p = self.parent
        j = p.json_info
        p.column_titles = j['titles'].split(',')
        p.sensor_name = j['name']
        if PRINT_PARSING_DEBUG:
            print('sensor-name: %s' % p.sensor_name)
        p.unpack_str = j['format']
        p.rate_hz = j['rate']
        if PRINT_PARSING_DEBUG:
            print('sensor-rate: %d' % p.rate_hz)


class QLSM_SensorDATA(RIFF_Block):
    """
    This represents the DATA component of a SNSR block.
    The payload here is the sensor data in some packed format.
    as defined by the header->sensor->[NAME]->format json element.
    """

    def __init__(self, file_block, parent):
        RIFF_Block.__init__(self, file_block)
        self.parent = parent

    def parse(self):
        step = struct.calcsize(self.parent.unpack_str)
        # Data must be a multiple of the struct size
        w, f = divmod(len(self.payload), step)
        if f != 0:
            self.file_block.warning(
                "block %d invalid/truncated payload length" %
                self.parent.block_number);
            self.file_block.ew_more(
                "expected multiple of %d, got %d" %
                 (step,len(self.payload)))
        # unpack the data
        w = w * step
        for cursor in range(0, w, step):
            values = struct.unpack(
                self.parent.unpack_str,
                self.payload[cursor: cursor + step])
            self.parent.data.append(values)

        # determine the timestamp in uSecs of last value in this block
        # How many readings are there?
        n_samples = len(self.payload) // step
        self.parent.n_samples = n_samples
        if PRINT_PARSING_DEBUG:
            print('n-samples: %d' % n_samples)
            delta_usecs = self.parent.last_timestamp_usecs - \
                          self.parent.first_timestamp_usecs
            seconds = float(delta_usecs) / 1000000.0
            calc_rate = float(n_samples) / seconds
            print('calc-rate: %f' % calc_rate)


class QLSM_SensorSNSR(RIFF_Block):
    """
    This represents a SENSOR data block in a QLSM file.
    There should be MANY of these in a data file.

    This block contains 2 sub components,
    * A single 'HDR!' block
    * Multiple 'DATA' blocks.
    """

    def __init__(self, file_block):
        RIFF_Block.__init__(self, file_block)
        self.hdr_block = QLSM_SensorHDR(self.file_block, self)
        self.data_block = QLSM_SensorDATA(self.file_block, self)
        self.sensor_id = None
        self.first_timestamp_datetime = None
        self.first_timestamp_usecs = None
        self.last_timestamp_datetime = None
        self.last_timestamp_usecs = None
        self.sequence_number = None
        self.sensor_name = None
        self.column_titles = None
        self.unpack_str = None
        self.json_info = None
        self.data = []

    def parse(self):
        """Parse a data block (tag='SNSR')"""
        parser = ChunkParser()
        assert (self.tag == 'SNSR')
        if PRINT_PARSING_DEBUG:
            print('')
            print('snsr-block file-offset: 0x%08x' % self.file_offset)
        chunks = parser.parse_bytearray(self.file_block,
                                        self.file_offset,
                                        self.payload)
        assert (len(chunks) == 2)
        self.hdr_block.from_riff_block(chunks[0])
        self.hdr_block.parse()
        self.data_block.from_riff_block(chunks[1])
        self.data_block.parse()


class QLSM_SensorRECO_HDR(RIFF_Block):
    """
    All sensor data blocks use the tag 'SNSR'
    A SNSR block contains 2 sub blocks, a 'HDR!' and 'DATA' block.

    The HDR! contains information about the content of the DATA block.
    Specifically the HDR! contains the sensor id, and timestamp
    """

    def __init__(self, file_block, parent):
        RIFF_Block.__init__(self, file_block)
        self.parent = parent

    def parse(self):
        """Parse a HDR! block"""
        assert ('HDR!' == self.tag)
        assert (24 == len(self.payload))
        a, b, c, d = struct.unpack('<IIQQ', self.payload)
        self.parent.sensor_id = a
        self.parent.sequence_number = b
        self.parent.first_timestamp_usecs = c
        self.parent.last_timestamp_usecs = d
        if PRINT_PARSING_DEBUG:
            print('Block sensor: 0x%08x' % self.parent.sensor_id)
            print('sequence-number: %d' % self.parent.sequence_number)
        tmpC = datetime.datetime.fromtimestamp((float)(c) / 1000000.0)
        self.parent.first_timestamp_datetime = tmpC
        tmpD = datetime.datetime.fromtimestamp((float)(d) / 1000000.0)
        self.parent.last_timestamp_datetime = tmpD
        if PRINT_PARSING_DEBUG:
            print('Block start: %s' % tmpC.isoformat())
            print('       stop: %s' % tmpD.isoformat())

        self._extract_json1()
        #tmp = self.file_block.header.find_sensor_details(self.parent.sensor_id)
        #self.parent.json_info = tmp

        #if self.parent.json_info['version'] == 1:
        #    self._extract_json1()
        #else:
        #    # future ...
        #    raise NotImplementedError('write me')

    def _extract_json1(self):
        p = self.parent
        j = p.json_info
        p.column_titles = ['recog_result',] # j['titles'].split(',')
        p.sensor_name = 'sensor_recog' # j['name']
        if PRINT_PARSING_DEBUG:
            print('sensor-name: %s' % p.sensor_name)
        p.unpack_str = 's' # j['format']
        p.rate_hz = 100 # j['rate']
        if PRINT_PARSING_DEBUG:
            print('sensor-rate: %d' % p.rate_hz)

class QLSM_SensorRECO_DATA(RIFF_Block):
    """
    This represents the DATA component of a SNSR block.
    The payload here is the sensor data in some packed format.
    as defined by the header->sensor->[NAME]->format json element.
    """

    def __init__(self, file_block, parent):
        RIFF_Block.__init__(self, file_block)
        self.parent = parent

    def parse(self):
        # remove trailing zeros from the payload
        n_zeros = 1
        for val in self.payload[::-1]:
           if (val == 0):
              n_zeros = n_zeros + 1
              continue
           break
        model_strings = self.payload[:-n_zeros].decode().split('\n')
        n_samples = 0
        for model_string in model_strings:
            self.parent.data.append([model_string,])
            n_samples = n_samples + 1
        self.parent.n_samples = n_samples
        if (0):
            step = struct.calcsize(self.parent.unpack_str)
            # Data must be a multiple of the struct size
            w, f = divmod(len(self.payload), step)
            if f != 0:
                self.file_block.warning(
                    "block %d invalid/truncated payload length" %
                    self.parent.block_number);
                self.file_block.ew_more(
                    "expected multiple of %d, got %d" %
                     (step,len(self.payload)))
            # unpack the data
            w = w * step
            for cursor in range(0, w, step):
                values = struct.unpack(
                    self.parent.unpack_str,
                    self.payload[cursor: cursor + step])
                self.parent.data.append(values)

            # determine the timestamp in uSecs of last value in this block
            # How many readings are there?
            n_samples = len(self.payload) // step
            self.parent.n_samples = n_samples
        if PRINT_PARSING_DEBUG:
            print('n-samples: %d' % n_samples)
            delta_usecs = self.parent.last_timestamp_usecs - \
                          self.parent.first_timestamp_usecs
            seconds = float(delta_usecs) / 1000000.0
            calc_rate = float(n_samples) / seconds
            print('calc-rate: %f' % calc_rate)

class QLSM_SensorRECO(RIFF_Block):
    """
    This represents a SENSOR recognition result block in a QLSM file.
    There should be MANY of these in a data file.

    This block contains 2 sub components,
    * A single 'HDR!' block
    * Multiple 'DATA' blocks.
    """

    def __init__(self, file_block):
        RIFF_Block.__init__(self, file_block)
        self.hdr_block = QLSM_SensorRECO_HDR(self.file_block, self)
        self.data_block = QLSM_SensorRECO_DATA(self.file_block, self)
        self.sensor_id = None
        self.first_timestamp_datetime = None
        self.first_timestamp_usecs = None
        self.last_timestamp_datetime = None
        self.last_timestamp_usecs = None
        self.sequence_number = None
        self.sensor_name = None
        self.column_titles = None
        self.unpack_str = None
        self.json_info = None
        self.data = []

    def parse(self):
        """Parse a data block (tag='RECO')"""
        parser = ChunkParser()
        assert (self.tag == 'RECO')
        if PRINT_PARSING_DEBUG:
            print('')
            print('snsr-block file-offset: 0x%08x' % self.file_offset)
        chunks = parser.parse_bytearray(self.file_block,
                                        self.file_offset,
                                        self.payload)
        assert (len(chunks) == 2)
        self.hdr_block.from_riff_block(chunks[0])
        self.hdr_block.parse()
        self.data_block.from_riff_block(chunks[1])
        self.data_block.parse()

class QLSM_DataFile():
    """
    This represents an entire QLSM data file.

    QLSM files start with a single QLSM riff which services as a header
    The QLSM header contains a JSON description of data capture
    And should be followed by multiple SNSR data blocks.
    """

    def __init__(self, filename):
        self.filename = filename
        self.header = QLSM_FileHeader(self)
        # QLSM_DataBlocks()
        self.datablocks = []
        self.ew_messages = []
        self.warning_count = 0
        self.error_count = 0
        parser = ChunkParser()
        # that gives us an array of blocks
        if PRINT_PARSING_DEBUG:
            print('loading: %s' % filename)
        arrayofblocks = parser.load(self, self.filename)
        if PRINT_PARSING_DEBUG:
            print('Total blocks: %d' % len(arrayofblocks))
        self.parse_datablocks(arrayofblocks)

    def error( self, msg ):
        self.error_count += 1
        self.ew_messages.append(msg)
        sys.stderr.write("%s: error: %s\n" % (self.filename,msg))

    def ew_more( self, msg ):
        self.ew_messages.append(msg)
        sys.stderr.write("%s: more: %s\n" % (self.filename, msg ))

    def warning( self, msg ):
        self.warning_count += 1
        self.ew_messages.append(msg)
        sys.stderr.write("%s: warning: %s\n" % (self.filename,msg))


    def parse_datablocks(self, arrayofblocks):
        """Parse an array of blocks into a header & data"""
        assert (len(arrayofblocks) > 2)
        self.header.from_riff_block(arrayofblocks[0])
        self.header.parse()
        self.header.file_block = self
        parser = ChunkParser()
        for n,block in enumerate(arrayofblocks[1:]):
            assert (block.tag == 'SNSR') or (block.tag == 'RECO')
            if (block.tag == 'SNSR'):
                this_data_block = QLSM_SensorSNSR(self)
                this_data_block.block_number = n
                this_data_block.from_riff_block(block)
                this_data_block.parse()
                self.datablocks.append(this_data_block)
            if (block.tag == 'RECO'):
                this_data_block = QLSM_SensorRECO(self)
                this_data_block.block_number = n
                this_data_block.from_riff_block(block)
                this_data_block.parse()
                self.datablocks.append(this_data_block)


class ChunkParser(object):
    """
    A chunk is a RIFF block, it begins with an 8byte header
    Bytes[0:3] are the tag, in little endian order
    Bytes[4:7] is the length in bytes of the data following the header.

    Chunks *may* be nested, it depends on what the tag means.
    """
    SEEK_SET = 0

    def __init__(self):
        pass

    @staticmethod
    def tag_to_string(int_tag):
        """Convert an integer (riff-tag) into an ASCII string"""
        string_chars = [0, 0, 0, 0]
        int_tag, string_chars[0] = divmod(int_tag, 256)
        int_tag, string_chars[1] = divmod(int_tag, 256)
        int_tag, string_chars[2] = divmod(int_tag, 256)
        string_chars[3] = int_tag
        # make sure all is ascii
        for x in range(0, 4):
            if (string_chars[x] < 0x20) or (string_chars[x] > 0x7e):
                string_chars[x] = '<0x%02x>' % string_chars[x]
            else:
                string_chars[x] = chr(string_chars[x])
        return ''.join(string_chars)

    @staticmethod
    def string_to_tag(string_tag):
        """Convert an ascii string into a RIFF tag"""
        byte_1 = ord(string_tag[0])
        byte_2 = ord(string_tag[1])
        byte_3 = ord(string_tag[2])
        byte_4 = ord(string_tag[3])
        return (((((byte_4 * 256) + byte_3) * 256) + byte_2) * 256) + byte_1

    def load(self, file_block, filename):
        with io.open(filename, 'rb') as f:
            data = f.read()
        return self.parse_bytearray(file_block, 0, data)

    def parse_bytearray(self, file_block, file_offset, byte_array):
        """
        Parse an array of bytes as an array of RIFF blocks
        The file_block parameter is the QLSM file class.
        """
        cursor = 0
        result = []
        while cursor < len(byte_array):
            # read a header
            bytes_remaining = len(byte_array) - cursor
            a, b = divmod(bytes_remaining, 4)
            if b != 0:
                raise ChunkParseError('non-aligned block')
            if a < 3:
                break;
            tmp = byte_array[cursor: cursor + 8]
            index_tag, num_bytes = struct.unpack('<II', tmp)
            # Go past header & align to 32bit boundary
            end = (cursor + 8 + num_bytes + 3) & ~3
            # For QuickAI - unused bytes in a block are zero padded.
            # If we have a tag with value 0, we are done
            if (index_tag == 0) or (num_bytes == 0):
                # They both should be zero
                assert ((index_tag + num_bytes) == 0)
                break
            new_block = RIFF_Block(file_block)
            # make tags ascii
            new_block.tag = ChunkParser.tag_to_string(index_tag)
            # get the data component
            new_block.payload = byte_array[cursor + 8: end]
            new_block.file_offset = file_offset + cursor
            # Go past header + payload
            cursor = end
            result.append(new_block)
        return result


def load_qlsm_file(filename):
    """Load a qlsm file, returning a QLSM_File"""
    qlsm = QLSM_DataFile(filename)
    return qlsm


class QLSMDataParser(object):
    def __init__(self, file_path, base_file_name=None, print_info=False):

        self._file_path = file_path
        self._print_info = print_info

        if not self._file_path.endswith('.qlsm'):
            raise InvalidExtensionException('Files must have .qlsm extension!')
        if not base_file_name:
            self.base_file_name = Path(file_path).name[:-5]
        else:
            self.base_file_name = base_file_name

        self._qlsm_data = None
        self.load_qlsm_file()
        self._sensor_frames = []
        self._sensor_id_map = {}
        self._extract_data_frames()
        self._has_audio_data = self._check_for_audio()
        global PRINT_PARSING_DEBUG
        # Allows chunk parser to print debug without having to change it completely
        PRINT_PARSING_DEBUG = self.print_info

    @property
    def has_audio_data(self):
        return self._has_audio_data

    @property
    def qlsm_data(self):
        return self._qlsm_data

    @qlsm_data.setter
    def qlsm_data(self, value):
        """

        :param QLSM:
        """
        self._qlsm_data = value

    @property
    def base_file_name(self):
        """
        Gets user-specified base string
        :rtype: str
        """
        return self._base_file_name

    @base_file_name.setter
    def base_file_name(self, value):
        self._base_file_name = value

    @property
    def sensor_frames(self):
        return self._sensor_frames

    @property
    def print_info(self):
        return self._print_info

    @print_info.setter
    def print_info(self, value):
        self._print_info = value

    def _check_for_audio(self):
        if self.qlsm_data is None:
            return False
        else:
            for blk in self.qlsm_data.datablocks:
                if blk.sensor_id == audio_sensor_id:
                    return True
        return False

    def sensor_collection_times(self):
        """
        Get the collection DateTimes in ISO format
        :return: Dictionary with start and end Datetime objects
        """
        return {'Start': self.qlsm_data.datablocks[0].first_timestamp_datetime.isoformat(),
                'End': self.qlsm_data.datablocks[-1].last_timestamp_datetime.isoformat()}

    def collection_time_delta(self):
        """
        Get the datetime.timedelta for collection time
        :return: datetime.timedelta of start and end time.
        """
        return self.qlsm_data.datablocks[-1].last_timestamp_datetime - \
               self.qlsm_data.datablocks[0].first_timestamp_datetime

    def load_qlsm_file(self):
        """
        Load a qlsm file, returning a QLSM_File
        """
        try:
            self._qlsm_data = QLSM_DataFile(self._file_path)
        except ChunkParseError:
            self._qlsm_data = None

    def _turn_qlsm_to_dataframes(self, sensor_id):
        """
        Write *ONLY* this sensors data to the output file.
        """
        # Special case audio, write an audio WAVE file also.
        if self.qlsm_data is None:
            raise QLSMDataNotLoadedException('File not loaded or was invalid!')
        if self._print_info:
            print('')
            print('CSV for Sensor: 0x%08x' % sensor_id)

            # also create a CSV so do not return here.
        blk = None
        for blk in self.qlsm_data.datablocks:
            if blk.sensor_id != sensor_id:
                continue
            else:
                break
        if blk is None:
            sys.stderr.write('Data block not found for sensor ID {}'.format(
                sensor_id))
            sys.exit('error')

        fieldnames = ['year', 'month', 'day', 'hour', 'min', 'sec', 'us', 'abs']
        for tmp in blk.column_titles:
            fieldnames.append('{0}_{1}'.format(blk.sensor_name, tmp))

        temp_data_dict = {}
        for field_name in fieldnames:
            temp_data_dict[field_name] = []

        for blk in self.qlsm_data.datablocks:
            if blk.sensor_id != sensor_id:
                continue
            if self._print_info:
                print('')
                print('block-sequence: %d' % blk.sequence_number)
                print('start: %s' % blk.first_timestamp_datetime.isoformat())

                print('file-offset: 0x%08x' % blk.file_offset)

            delta_usecs = float(blk.last_timestamp_usecs - blk.first_timestamp_usecs)
            sample_number = float(0)
            for row in blk.data:
                abs_usec = blk.first_timestamp_usecs + ((sample_number / blk.n_samples) * delta_usecs)
                this_datetime = datetime.datetime.fromtimestamp(abs_usec / 1000000.0)

                temp_data_dict['year'].append(this_datetime.year)
                temp_data_dict['month'].append(this_datetime.month)
                temp_data_dict['day'].append(this_datetime.day)
                temp_data_dict['hour'].append(this_datetime.hour)
                temp_data_dict['min'].append(this_datetime.minute)
                temp_data_dict['sec'].append(this_datetime.second)
                temp_data_dict['us'].append(this_datetime.microsecond)
                temp_data_dict['abs'].append(abs_usec)
                row_index = 0
                for tmp in blk.column_titles:
                    temp_data_dict['{}_{}'.format(blk.sensor_name, tmp)].append(
                        row[row_index])
                    row_index += 1
                sample_number += 1.0
        df = pandas.DataFrame(temp_data_dict)

        if self._print_info:
            print('Sample Dataframe head for sensor {}: \n'.format(sensor_id))
            print(df.head())
        return df

    def _extract_data_frames(self):
        """
        Write each sensor into a seperate CSV file.
        """
        done_sensors = []
        # Loop over the file...
        for blk in self.qlsm_data.datablocks:
            # If we have done this sensor then do not repeat
            if blk.sensor_id in done_sensors:
                continue

            if blk.sensor_id == audio_sensor_id:
                continue
            done_sensors.append(blk.sensor_id)
            # Write this sensor
            self.sensor_frames.append(self._turn_qlsm_to_dataframes(blk.sensor_id))
            self._sensor_id_map[len(self.sensor_frames) - 1] = blk.sensor_name

    def frames_to_csv(self, combine=False, drop_extra=False):
        def check_abs_column_for_sample_match():
            abs_match = False
            for index in range(len(self.sensor_frames) - 1):
                if index + 1 >= len(self.sensor_frames):
                    break
                abs_match = (sum(self.sensor_frames[index]['abs'] -
                                 self.sensor_frames[index + 1]['abs']) == 0)
                if not abs_match:
                    return False
            return abs_match

        if combine:
            if not check_abs_column_for_sample_match():
                raise AbsoluteTimeStampMismatchException('Timestamps are not equal, cannot combine to one CSV')
            else:
                df_combined = self.sensor_frames[0]
                for i in range(len(self.sensor_frames) - 1):
                    df_combined = df_combined.merge(self.sensor_frames[i + 1],
                                                    on=['year',
                                                        'month',
                                                        'day',
                                                        'hour',
                                                        'min',
                                                        'sec',
                                                        'us',
                                                        'abs'])
                if self._print_info:
                    print('Combined DataFrame Shape : {}'.format(df_combined.shape))
                    print('\nCombined DataFrame Head:\n{}'.format(df_combined.head()))

                df_combined.to_csv('{}.csv'.format(self.base_file_name), index=None)
        else:
            index = 0
            for sensor_file in self.sensor_frames:
                sensor_file.to_csv('{0}_{1}.csv'.format(self.base_file_name, self._sensor_id_map[index]))
                index += 1

    def extract_audio_to_wave(self):
        """
        This extracts wave (audio) data from the RIFF file as WAVE
        """
        filename = '%s.wav' % self.base_file_name

        # we use the standard python wave writer
        wave_writer = wave.open(filename, 'wb')
        if PRINT_PARSING_DEBUG:
            print('WAVE file: %s' % filename)

        # STEP 1 - Write the WAVE header
        #
        # get details from the audio sensor block
        audio_json_data = self.qlsm_data.header.json['sensors']['audio']

        tmp = audio_json_data['nbits']
        assert (tmp == int(tmp))
        assert ((tmp == 32) or (tmp == 24) or (tmp == 16) or (tmp == 8))
        # this wants BYTES
        wave_writer.setsampwidth(tmp // 8)
        if PRINT_PARSING_DEBUG:
            print('wave: %d bit' % tmp)
        # rate comes from the JSON information
        tmp = audio_json_data['rate']
        wave_writer.setframerate(tmp)
        if PRINT_PARSING_DEBUG:
            print('wave: %d hz' % tmp)
        # how many channels are enabled, this gives channels
        num_channels = 0
        for ch in audio_json_data['mic']:
            # non-zero configuration means enabled
            if PRINT_PARSING_DEBUG:
                print('wave-ch: %d mode: %d (0x%02x)' %
                      (ch['ch'], ch['config'],ch['config']))
            if ch['config'] != 0:
                num_channels += 1
        wave_writer.setnchannels(num_channels)
        if PRINT_PARSING_DEBUG:
            print('n-channels: %d' % num_channels)
        # loop over data blocks.
        # Note first call to 'writeframes' will write the header.
        n = 0
        for blk in self.qlsm_data.datablocks:
            if blk.sensor_id == audio_sensor_id:
                wave_writer.writeframes(blk.data_block.payload)
                n += 1
                if (n > 0) and ((n % 100) == 1):
                    if PRINT_PARSING_DEBUG:
                        print('n-blocks: %d' % n)
        if PRINT_PARSING_DEBUG:
            print('Wrote: %d blocks' % n)
        wave_writer.close()


def main():
    parser = argparse.ArgumentParser(
        description='convert qlsm files to CSV and wave files')
    parser.add_argument('input_file', metavar='INFILE', help='Input filename')
    parser.add_argument('--debug', '-d',
                        dest='debug',
                        action='store_true',
                        default=False,
                        help='Enable debug')
    parser.add_argument('--basename', '-b',
                        metavar='BASENAME',
                        default=None,
                        help='sets the BASENAME of the output file(s)')
    parser.add_argument('--combine', '-c',
                        dest='combine',
                        action='store_true',
                        default=False,
                        help='Combine sensors (if applicable)')
    input_args = parser.parse_args()

    global PRINT_PARSING_DEBUG
    PRINT_PARSING_DEBUG = input_args.debug
    if not input_args.input_file.endswith('.qlsm'):
        sys.stderr.write('Input filename must have a ".qlsm" suffix')
        parser.print_help()
        sys.exit('sorry')
    if not os.path.isfile(input_args.input_file):
        sys.exit('Not a file: %s' % input_args.input_file)
    # qlsm = load_qlsm_file(input_args.input_file)
    bn = input_args.basename
    if bn is None:
        # remove the extension from the data file
        bn = input_args.input_file[:-5]
    sz = os.path.getsize(input_args.input_file)
    # sometimes we have very small (zero size) files
    # handle that gracefully here.
    if sz < 4096:
        sys.exit("file: %s\nFile size is %d bytes, which indicates this is not a valid QLSM file\n" % (input_args.input_file,sz) )
    parser = QLSMDataParser(input_args.input_file, bn, input_args.debug)
    parser.frames_to_csv(input_args.combine)
    if parser._has_audio_data:
        parser.extract_audio_to_wave()
    # write_csv_files(qlsm, bn, input_args.combine)
    return 0


if __name__ == '__main__':
    main()
