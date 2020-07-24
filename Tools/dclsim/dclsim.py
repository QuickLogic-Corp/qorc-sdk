# DCL simulator simulates functions and APIs for real DCL apps
# This purpose of this tool is to test and debug sensiml APIs only
#  
# Please install two following packages first
# install paho-mqtt for MQTT functions
# install PyCRC for CRC16 
# Copy Right: QuickLogic, SensiML
"""
This is an MQTT v3.1 client module. MQTT is a lightweight pub/sub messaging
protocol that is easy to implement and suitable for low powered devices.
DCL is a child module of MQTT3.1 and provides SensiML Interfaces and complies
to SensiML Interface Specification
"""
import paho.mqtt.client
import time
import struct
import queue
import os
import sys
import uuid
import random
import math
import logging
from PyCRC.CRCCCITT import CRCCCITT

__version__= "0.2"
__date__ = "2020/01/24"
#mq_host = "test.mosquitto.org"
#mq_host = "mqtt.eclipse.org"
mq_host = 'localhost'
mq_port = 1883
pub_delay = 0.5
sub_delay = 0.05
reg_delay = 0.05

TIME_COLLECT = 5
TIME_LIVE = 10
IMUA_CASES = [ 0, 1, 2, 3, 4, 5, 6]
IMUG_CASES = [ 7, 8, 9,10,11,12,13]
IMU3_CASES = [14,15,16,17,18,19,20]
AUDO_CASES = [21]
LTC1589_CASES = [22,23,24,25,26,27,28,29,30]
AD7476_CASES  =  [31]
TEST_CASES = IMUA_CASES # + IMUG_CASES + IMU3_CASES + AUDO_CASES +LTC1589_CASES
# Chilkat support IMU upto 400HZ and AUDO only.
# TEST_CASES = [ 0, 1, 2, 3, 4] + [ 7, 8, 9,10,11] + [14,15,16,17,18] + AUDO_CASES
COLLECT = False # True
LIVE = True
RECOG =  False
STORAGE = False
VERIFY = True
ONESENSOR = True
DEBUG = False


#IMUA range 2,4,8,16G = 20,40,80,160, setting=10*range
#IMUG range 2000,1000,500,250,125DPS =0,1,2,3,4, 2^setting =2000/range
#IMU sensitivity bit=16 not used 
#IMU\x03 = IMUA+IMUG
#AUDO b'AUDO',rate=16000, bit=16,chan=1,0,0,0,0,0,0,0
#ADC_LTC1589 b'LTH\x9e', rate=100k,16k,8k,4k,2k,1k;bit=16, chan off=0xff
#ADC_LTC1589 4 channs, 0b10001000,bit0:sleep=1,bit2,3: chan2=10,bit7:single=1
#ADC_AD7476 b'AD\x1d4',rate=1M,bit=12,chan=1

class SENSOR:
    def __init__(self, sensor_id, rate, bit, cd, spp, *chan):
        self.sensor_id = sensor_id #should be 4 bytes
        self.rate      = rate      #
        self.bit       = bit       #only used by AUDO
        self.chan      = chan      #chans varies from 1 to 8 or range
        self.cd        = cd        #live rate = rate/(count_down+1)
        self.spp       = spp       #live sample# per packet
        self.generate_fields()

    def generate_fields(self):
        self.sensor_add = self.sensor_id + struct.pack('>I',self.rate)
        self.live_set_rate = b'\x01' + self.sensor_id + \
                                   struct.pack('>I',self.cd)
        self.live_start    = b'\x01' + self.sensor_id + \
                                   struct.pack('>B',self.spp)

class IMU(SENSOR):
    def __init__(self,sensor_id,rate,bit,cd,spp,*rang):
        if sensor_id == b'IMUA':
            range_setting = (10*rang[0],)
        elif sensor_id == b'IMUG':
            range_setting = (int(math.log2(2000/rang[0])),)
        elif sensor_id == b'IMU\x03':
            range_setting =(10*rang[0], int(math.log2(2000/rang[1])))
        super().__init__(sensor_id,rate,rang,cd,spp,*range_setting)

    def generate_fields(self):
        super().generate_fields()
        for i in self.chan:
             self.sensor_add +=struct.pack('>B',i)

class AUDO(SENSOR):
    def __init__(self,sensor_id, rate, bit, cd,spp,*chan):
        super().__init__(sensor_id, rate, bit, cd,spp,*chan)

    def generate_fields(self):
        super().generate_fields()
        self.sensor_add +=struct.pack('>B',self.bit)
        for i in self.chan:
             self.sensor_add +=struct.pack('>B',i)
        
class ADC_LTC1859(SENSOR):
    def __init__(self, sensor_id, rate, bit, cd, spp, *chan):
        super().__init__(sensor_id, rate, bit, cd,spp,*chan)

    def generate_fields(self):
        super().generate_fields()
        for i in self.chan:
             self.sensor_add +=struct.pack('>B',i)

class ADC_AD7476(SENSOR):
    def __init__(self, sensor_id, rate, bit, cd, spp, *chan):
        super().__init__(sensor_id, rate, bit, cd,spp,*chan)

def tests(freq):
    cases = []

    #0 add IMUA rate 25-1664Hz,bit,cound_down, sample_per_packet,2G
    cases.append(IMU(b'IMUA',    freq, 16, 0, 3, 2))
    cases.append(IMU(b'IMUA',  2*freq, 16, 0, 3, 2))
    cases.append(IMU(b'IMUA',  4*freq, 16, 0, 3, 2))
    cases.append(IMU(b'IMUA',  8*freq, 16, 0, 3, 2))
    cases.append(IMU(b'IMUA', 16*freq, 16, 0, 3, 2))
    cases.append(IMU(b'IMUA', 32*freq, 16, 1, 3, 2))
    cases.append(IMU(b'IMUA', 64*freq, 16, 3, 2, 2))

    #7 add 7 IMUG rate 25-1664Hz,bit,cound_down, sample_per_packet,2000DPS
    cases.append(IMU(b'IMUG',    freq, 16, 0, 3, 2000))
    cases.append(IMU(b'IMUG',  2*freq, 16, 0, 3, 2000))
    cases.append(IMU(b'IMUG',  4*freq, 16, 0, 3, 2000))
    cases.append(IMU(b'IMUG',  8*freq, 16, 0, 3, 2000))
    cases.append(IMU(b'IMUG', 16*freq, 16, 0, 3, 2000))
    cases.append(IMU(b'IMUG', 32*freq, 16, 1, 3, 2000))
    cases.append(IMU(b'IMUG', 64*freq, 16, 3, 4, 2000))

    #14 add 7 IMU\x03 rate 25-1664Hz,bit,cound_down, sample_per_packet,
    # 2G, 2000DPS
    cases.append(IMU(b'IMU\x03',    freq, 16, 0, 3, 2, 2000))
    cases.append(IMU(b'IMU\x03',  2*freq, 16, 0, 3, 2, 2000))
    cases.append(IMU(b'IMU\x03',  4*freq, 16, 0, 3, 2, 2000))
    cases.append(IMU(b'IMU\x03',  8*freq, 16, 0, 3, 2, 2000))
    cases.append(IMU(b'IMU\x03', 16*freq, 16, 1, 3, 2, 2000))
    cases.append(IMU(b'IMU\x03', 32*freq, 16, 3, 2, 2, 2000))
    cases.append(IMU(b'IMU\x03', 64*freq, 16, 7, 2, 2, 2000))

    #21 add AUDO rate,bit,countdown,sampel/p, chan 1,0,0,0,0,0,0,0
    cases.append(AUDO(b'AUDO',16000,16,40,1,1,0,0,0,0,0,0,0))

    #22 add 6 LTC1589 mayhew rate from 100K(1chan) to 1K,
    # bit16, countdown, sample/packet,
    # 4 channels, bit7:1single,0differential bit5:4 address
    # bit3:2 00-5v,01-10v, bit1:0 00enable, 0xff=alloff
    
    cases.append(ADC_LTC1859(b'LTH\x9e',100000,16,500,1,
                             0b10000000,0b11111111,
                             0b11111111,0b11111111))
    cases.append(ADC_LTC1859(b'LTH\x9e',16000,16, 80, 1,
                             0b10000000,0b10000100,
                             0b10001000,0b10001100))
    cases.append(ADC_LTC1859(b'LTH\x9e',8000, 16, 40, 1,
                             0b10000000,0b10000100,
                             0b10001000,0b10001100))
    cases.append(ADC_LTC1859(b'LTH\x9e',4000, 16, 20, 1,
                             0b10000000,0b10000100,
                             0b10001000,0b10001100))
    cases.append(ADC_LTC1859(b'LTH\x9e',2000, 16, 10, 1,
                             0b10000000,0b10000100,
                             0b10001000,0b10001100))
    cases.append(ADC_LTC1859(b'LTH\x9e',1000, 16,  5, 1,
                             0b10000000,0b10000100,
                             0b10001000,0b10001100))

    cases.append(ADC_LTC1859(b'LTH\x9e',16000,16,80,1,
                             0b10000000,0b11111111,
                             0b11111111,0b11111111))
    cases.append(ADC_LTC1859(b'LTH\x9e',16000,16, 80, 1,
                             0b10000000,0b10000100,
                             0b11111111,0b11111111))
    cases.append(ADC_LTC1859(b'LTH\x9e',16000,16, 80, 1,
                             0b10000000,0b10000100,
                             0b10001000,0b11111111))

    #31 add 1 AD7476 case rate=1000000, bit=12 not used
    cases.append(ADC_AD7476(b'AD\x1d4',1000000,16, 5000, 4))

    return cases

def add_tests(imu_sample_rates):
    cases = []

    #0 add IMUA rate 25-1664Hz,bit,cound_down, sample_per_packet,2G
    for srate in imu_sample_rates:
        cases.append(IMU(b'IMUA', srate, 16, 0, 3, 2))

    #21 add AUDO rate,bit,countdown,sampel/p, chan 1,0,0,0,0,0,0,0
    cases.append(AUDO(b'AUDO',16000,16,40,1,1,0,0,0,0,0,0,0))

    #22 add 6 LTC1589 mayhew rate from 100K(1chan) to 1K,
    # bit16, countdown, sample/packet,
    # 4 channels, bit7:1single,0differential bit5:4 address
    # bit3:2 00-5v,01-10v, bit1:0 00enable, 0xff=alloff
    
    cases.append(ADC_LTC1859(b'LTH\x9e',100000,16,500,1,
                             0b10000000,0b11111111,
                             0b11111111,0b11111111))
    cases.append(ADC_LTC1859(b'LTH\x9e',16000,16, 80, 1,
                             0b10000000,0b10000100,
                             0b10001000,0b10001100))
    cases.append(ADC_LTC1859(b'LTH\x9e',8000, 16, 40, 1,
                             0b10000000,0b10000100,
                             0b10001000,0b10001100))
    cases.append(ADC_LTC1859(b'LTH\x9e',4000, 16, 20, 1,
                             0b10000000,0b10000100,
                             0b10001000,0b10001100))
    cases.append(ADC_LTC1859(b'LTH\x9e',2000, 16, 10, 1,
                             0b10000000,0b10000100,
                             0b10001000,0b10001100))
    cases.append(ADC_LTC1859(b'LTH\x9e',1000, 16,  5, 1,
                             0b10000000,0b10000100,
                             0b10001000,0b10001100))

    cases.append(ADC_LTC1859(b'LTH\x9e',16000,16,80,1,
                             0b10000000,0b11111111,
                             0b11111111,0b11111111))
    cases.append(ADC_LTC1859(b'LTH\x9e',16000,16, 80, 1,
                             0b10000000,0b10000100,
                             0b11111111,0b11111111))
    cases.append(ADC_LTC1859(b'LTH\x9e',16000,16, 80, 1,
                             0b10000000,0b10000100,
                             0b10001000,0b11111111))

    #31 add 1 AD7476 case rate=1000000, bit=12 not used
    cases.append(ADC_AD7476(b'AD\x1d4',1000000,16, 5000, 4))

    return cases

DCL_SUB_LIST = [
    ['sensiml/sys/device/uuids/rsp',1],
    ['sensiml/sys/version/rsp',1],
    ['sensiml/sys/compdatetime/rsp',1],
    ['sensiml/sys/status/rsp',1],
    ['sensiml/sys/error',1],
    ['sensiml/recog/model/uuid/rsp',1],
    ['sensiml/sensor/list/rsp',1],
    ['sensiml/snap/list/rsp',1],
    ['sensiml/snap/data/rsp',1],
    ['sensiml/live/sensor/list/rsp',1],
    ['sensiml/live/set/rate/rsp',1],
    ['sensiml/live/raw/data',1],
    ['sensiml/result/class/data',1],
    ['sensiml/sys/will/status',1]
    ]

DCL_PUB_LIST = [
    'sensiml/sys/all/stop',
    'sensiml/sys/status/clr',
    'sensiml/sys/status/req',
    'sensiml/sys/version/req',
    'sensiml/sys/compdatetime/req',
    'sensiml/sys/device/uuids/req',
    'sensiml/sys/unixtime/set',
    'sensiml/sys/reboot',
    'sensiml/sensor/list/req',
    'sensiml/sensor/clr',
    'sensiml/sensor/add',
    'sensiml/sensor/done',
    'sensiml/live/sensor/list/req',
    'sensiml/live/set/rate/req',
    'sensiml/live/start',
    'sensiml/live/stop',
    'sensiml/recog/model/uuid/req',
    'sensiml/recog/start',
    'sensiml/recog/stop',
    'sensiml/result/class/start',
    'sensiml/result/class/stop',
    'sensiml/result/class/set/rate',
    'sensiml/collect/prefix/set',
    'sensiml/collect/start',
    'sensiml/collect/stop',
    'sensiml/storage/space/req',
    'sensiml/storage/dir/req',
    'sensiml/storage/del',
    'sensiml/storage/get/start',
    'sensiml/storage/get/data/req',
    'sensiml/storage/get/stop',
    'sensiml/storage/put/start',
    'sensiml/storage/put/data/req',
    'sensiml/storage/put/stop'
    ]

def dcl_on_message(client, userdata, msg):
    """
    client object use this function to update new message event
    """
    client.rec_queue.put(msg)

class DCL(paho.mqtt.client.Client):
    """DCL class, a Client child class
    DCL is a child class of Client with extra sensiml functionalities.

    This class communicates with sensiml devices through an MQTT broker
    such as RSMB, EMQX or other commercial brokers

    General usage:
    
    * Create and DCL object with and client_id
    * Use connect()/connect_async() to connect to a broker
    * User loop_start() to set a thread running to call loop() for you.
    * Use subscribe() to subscribe to a topic and receive messages or
    * Use sub_all() to subscribe all needed topics and receive messages
    * Use publish() to send messages
    * Use pub() to send messages and prints time stamps and messages
    * Use disconnect() to disconnect from the broker
    
    """
    def __init__(self, client_id="", clean_session=True, userdata=None):
        """follows Client initialization ref paho.mqtt.client for details

        sensiml parameter:

        self.rec_queue: a queue holds all incoming messages from devices
        self.last_publish: last published topic, message and qos  
        self.sensor_list: list of SENSOR_IDs from topic_sensor_list_rsp
        self.sensor_active: list of acive sensors by topic_sensor_add
        self.sensor_config: list of config bytes for added sensors
        self.
        self.

        """
        super().__init__(client_id, clean_session, userdata)
        self.rec_queue = queue.Queue()
        self.last_pub = {'timestamp':0,'topic':'','message':'','qos':0}
        self.device_uuids = {'dclass_uuid':'','dunique_uuid':''}
        self.sw_version = ''
        self.compdatetime = ''
        self.status = {'bytes_saved':0,'bit_flags':0,'rx_count':0,
                       'tx_count':0,'live_oe_count':0, 'collect_oe_count':0,
                       'sticky_error_code':0,'error_count':0}
        self.error = {'msg_id':0,'dyn_topic_id':0,'fixed_topic_id':0,
                      'sticky_error':0,'extended_error':0}
        self.sensor_list = []
        self.live_sensor_list = []
        self.live_sensor_rate = []
        self.live_count = 0
        self.live_seq = -1
        self.result = {'data_type':'','time_stamp':'','model':'',
                       'class_result':'','fv_count':0,'fv_list':[]}
        self.storage = {'total_size':0,'in_use':0,'get_size':1024,'put_size':1024}
        self.dir_root = []
        self.dir_default = []
        self.dir_spiflash = []
        self.errors_list = []
        
        
    def sub_all(self,sub_list):
        """subscribe all needed topics to communicate with sensiml devices"""
        print(f'{int(time.monotonic())} DCL starts subscription')
        logging.debug(f'DCL starts subscription')
        for [topic, qos] in sub_list:
            try:
                self.subscribe(topic,qos)
                time.sleep(sub_delay)
                print(int(time.monotonic()), "DCL", topic, "qos=",qos, " subscribed")
                logging.debug(f"DCL {topic} qos= {qos} subscribed")
            except:
                print(int(time.monotonic()), "DCL", topic, "qos=",qos, " failed sub")
                logging.debug(f"DCL {topic} qos= {qos} failed sub")

    def pub(self,topic,msg,qos):
        """publish a topic and record it in last_pub for response verification 
        """
        timestamp = int(time.monotonic())
        if topic not in DCL_PUB_LIST: print(topic, "not in the DCL_PUB_LIST")
        print(timestamp,"DCL",topic, msg, "publishing")
        logging.debug(f"DCL {topic} {msg} publishin")
        temp=self.publish(topic,msg,qos=qos)
        
        self.last_pub['timestamp'] = timestamp 
        self.last_pub['topic']     = topic
        self.last_pub['message']   = msg
        self.last_pub['qos']       =qos
        time.sleep(pub_delay)
        
    def resp(self):
        """prints and verifies the response from sensiml devices"""
        while self.rec_queue.empty() == False:
            temp =self.rec_queue.get()
            print(int(temp.timestamp),"QLS3",temp.topic, temp.payload)
            logging.debug(f"QLS3 {temp.topic} {temp.payload}")
            if VERIFY == True:
                self.verify(int(temp.timestamp),temp.topic, temp.payload)

    def verify(self,timestamp,topic, payload):
        """Verifies if response complies with its request """
        result = 'PASS!!!'
        reason = ''
        
        if (topic == 'sensiml/sys/device/uuids/rsp'
            and self.last_pub['topic'] == 'sensiml/sys/device/uuids/req'):
            try:
                self.device_uuids['dclass_uuid'] = payload[0:16]
                self.device_uuids['dunique_uuid'] = payload[16:32]
            except:
                result = 'FAIL!!!'
                reason = "sensiml/sys/device/uuids/rsp data length is not 32 bytes"
 
        elif (topic == 'sensiml/sys/version/rsp'
              and self.last_pub['topic'] == 'sensiml/sys/version/req'):
            printable = True
            print("sw_version payload", payload)
            for i in payload:
                if i < 32 or i>126:
                    printable = False
            if printable == True:
                self.sw_version = payload.decode()
                print("sw_version", self.sw_version)
            else:
                result = 'FAIL!!!'
                reason = 'sensiml/sys/version/rsp format error' 
                self.errors_list.append((timestamp,self.last_pub['topic'],topic, payload))

        elif (topic == 'sensiml/sys/compdatetime/rsp'
              and self.last_pub['topic'] == 'sensiml/sys/compdatetime/req'):
            try:
                self.compdatetime = payload.decode()
                compdatetime = payload.decode().split()
                month = compdatetime[0]
                date = compdatetime[1]
                year = compdatetime[2]
                hms = compdatetime[3]
                if not (month in ['Jan','Feb','Mar','Apr','May','Jun',
                                  'Jul','Aug','Sept','Oct','Nov','Dec']
                        and int(date) < 32 and int(date) >0
                        and int(year) >=2019
                        and len(hms.split(':')) == 3):
                    result = 'FAIL!!!'
                    reason = 'sensiml/sys/compdatetime/rsp format error'

            except:
                result = 'FAIL!!!'
                reason = 'sensiml/sys/compdatetime/rsp format error'
                
        elif (topic == 'sensiml/sys/status/rsp'
              and self.last_pub['topic'] == 'sensiml/sys/status/req'):
            try:
                (self.status['bytes_saved'],
                 self.status['bit_flags'],
                 self.status['rx_count'],
                 self.status['tx_count'],
                 self.status['live_oe_count'],
                 self.status['collect_oe_count'],
                 self.status['sticky_error_code'],
                 self.status['error_count']) = struct.unpack('>IIHHHHBB',payload)
            except:
                result = 'FAIL!!!'
                reason = 'sensiml/sys/status/rsp data format is wrong'

        elif topic == 'sensiml/sys/error':
            try:
                (self.error['msg_id'],
                 self.error['dyn_topic_id'],
                 self.error['fixed_topic_id'],
                 self.error['sticky_error'],
                 self.error['extended_error'] ) = struct.unpack('>HHHBI',payload)
                raise Exception(f'sensiml/sys/error {payload}')
            except:
                result = 'FAIL!!!'
                reason = 'sensiml/sys/error data format is wrong'
            
        elif (topic == 'sensiml/recog/model/uuid/rsp'
              and self.last_pub['topic'] == 'sensiml/recog/model/uuid/req'):
            try:
                (self.recog['model'],
                 self.recog['model_uuid']) = struct.unpack('>HH', payload)
            except:
                result = 'FAIL!!!'
                reason = "sensiml/recog/model/uuid/rsp data length is not 4"

        elif (topic == 'sensiml/storage/dir/rsp'
              and self.last_pub['topic'] == 'sensiml/storage/dir/req'):
            try:
                (index, byte_size, unix_time) = struct.unpack('>HII',payload[:10])
                if self.last_pub['message'][:9]=='/default/':
                    if index != 65535 : #FFFF
                        if index == 0: 
                            self.dir_default = []
                        self.dir_default.append([index,
                                                 byte_size,
                                                 time.ctime(unix_time),
                                                 payload[10:].decode()])
                    else:
                        if byte_size != 4294967295: raise Exception(byt_size)
                        if unix_time != 4294967295: raise Exception(unix_time)
                elif self.last_pub['message'][:10]=='/SPIFLASH/':
                    if index != 65535 : #FFFF
                        if index == 0: 
                            self.dir_spiflash = []
                        self.dir_spiflash.append([index,
                                                 byte_size,
                                                 time.ctime(unix_time),
                                                 payload[10:].decode()])

                    else:
                        if byte_size != 4294967295: raise Exception(byt_size)
                        if unix_time != 4294967295: raise Exception(unix_time)
                elif self.last_pub['message']=='/*.*':
                    if index != 65535 : #FFFF
                        if index == 0: 
                            self.dir_root = []
                        self.dir_root.append([index,
                                                 byte_size,
                                                 time.ctime(unix_time),
                                                 payload[10:].decode()])

                    else:
                        if byte_size != 4294967295: raise Exception(byt_size)
                        if unix_time != 4294967295: raise Exception(unix_time)
                else:
                    pass
                
            except Exception as err:
                result = 'FAIL!!!'
                reason = 'sensiml/storage/dir/rsp is too short' + str(err.args[0])
                
        elif (topic == 'sensiml/storage/space/rsp'
              and self.last_pub['topic'] == 'sensiml/storage/space/req'):
            try:
                (self.storage['total_size'],
                 self.storage['in_use'],
                 self.storage['get_size'],
                 self.storage['put_size'] ) = struct.unpack('>IIII',payload)
                if (self.storage['put_size'] not in [1024,512,256]
                    and self.storage['get_size'] not in [1024,512,256]):
                    result = 'FAIL!!!'
                    reason = 'sensiml/storage/space/rsp size are not 1024, 512, 256'
            except:
                result = 'FAIL!!!'
                reason = 'sensiml/storage/space/rsp data format is wrong'
               

        elif topic == 'sensiml/storage/get/data/rsp':
            try:
                (self.transaction_id,
                 self.block_number,
                 self.actual_size,
                 self.crc16 ) = struct.unpack('>IIHH',payload[:12])
                self.byte_buffer = payload[12:(12+self.actual_size)]
                if self.crc16 != CRCCCITT().calculate(self.byte_buffer):
                    result = 'FAIL!!!'
                    reason = 'sensiml/storage/get/data/rsp CRC16 is wrong'
            except:
                result = 'FAIL!!!'
                reason = 'sensiml/storage/get/data/rsp data format is wrong'
 
        elif (topic == 'sensiml/sensor/list/rsp'
              and self.last_pub['topic'] == 'sensiml/sensor/list/req'):
            try:
                self.sensor_list = []
                num_of_sensor = len(payload) // 4
                for i in range(num_of_sensor):
                    self.sensor_list.append(payload[i*4:(i*4+4)])   
            except:
                result = 'FAIL!!!'
                reason = "sensiml/sensor/list/rsp data length is not 32*N bytes"
  

        elif topic == 'sensiml/snap/list/rsp':
            pass

        elif topic == 'sensiml/snap/data/rsp':
            pass
        elif (topic == 'sensiml/live/sensor/list/rsp'
              and self.last_pub['topic'] == 'sensiml/live/sensor/list/req'):
            try:
                self.live_sensor_list = []
                self.live_sensor_rate = []
                num_of_sensor = len(payload) // 8
                for i in range(num_of_sensor):
                    self.live_sensor_list.append(payload[(i*8):(i*8+4)])
                    self.live_sensor_rate.append(payload[(i*8+4):(i*8+8)])
            except:
                result = 'FAIL!!!'
                reason = "sensiml/live/sensor/list/rsp data length is not 8*N bytes"
            
        elif (topic == 'sensiml/live/set/rate/rsp'
              and self.last_pub['topic'] == 'sensiml/live/set/rate/req') :
            try:
                if self.last_pub['message'][1:] != payload:
                    result = 'FAIL!!!'
                    reason = "sensiml/live/set/rate/rsp data is not configured"
            except:
                result = 'FAIL!!!'
                reason = "sensiml/live/set/rate/rsp data length is not 8*N bytes"

        elif (topic == 'sensiml/live/raw/data'
              and payload[:4] in self.live_sensor_list
              and (self.last_pub['topic'] == 'sensiml/live/start'
                   or self.last_pub['topic'] == 'sensiml/live/stop')):
            try:              
                count = payload[4]
                if self.live_seq == -1:  self.live_seq = count
                if count != (self.live_seq)%256 :
                    result = 'FAIL!!!'
                    reason = 'live data out of order'
                self.live_count += 1
                self.live_seq +=1

            except:
                result = 'FAIL!!!'
                reason = 'sensiml/live/raw/data format is out of dated'
                
        elif topic == 'sensiml/result/class/data':
            try:
                self.result['data_type']    = payload[0:2]
                self.result['time_stamp']   = payload[2:10]
                self.result['model']        = payload[10:12]
                self.result['class_result'] = payload[12:14]
                if self.result['data_type'] == b'\x00\x02':
                    self.result['fv_count'] = payload[14]
                    self.result['fv_list']=[]
                    for i in range(self.result['fv_count']):
                        self.result['fv_list'].append(payload[15+i])
            except:
                result = 'FAIL!!!'
                reason = 'sensiml/result/class/data length is not correct'

        else:
            result = 'FAIL!!!'
            reason = 'rsp and req do not match!!!'

        if result ==  'FAIL!!!':
            print(timestamp, "QLS3", result, reason)
            self.errors_list.append((timestamp,self.last_pub['topic'],topic, payload))
        
    def get(self,file_name,save_name=None):
        '''get a file from the device and save it locally'''
        filename = file_name.split('/')
        file_exist = False
        if filename[1] == 'default' :
            for i in self.dir_default:
                if filename[2] == i[3]: 
                    file_exist = True
                    file_size=i[1]
                    break

        if file_exist != True:
            return file_name + " not found"
        try:
            transaction_id = struct.pack('>I',random.randint(0,0xffffffff))
            block_size = self.storage['get_size']
            header = transaction_id + struct.pack('>I', block_size)
            total_blocks = math.ceil(file_size / block_size)
            
            self.pub('sensiml/storage/get/start' ,header + file_name.encode(), 1)
            self.resp()
            if save_name == None: save_name = filename[2]
            with open(save_name,'wb') as file:
                for i in range(total_blocks):
                    block_number = struct.pack('>I', i)
                    self.pub( 'sensiml/storage/get/data/req', transaction_id + block_number , 1)
                    time.sleep(0.5)
                    while self.rec_queue.empty() == False:
                        temp =self.rec_queue.get()
                        (self.actual_size, self.crc16) = struct.unpack('>HH',temp.payload[8:12])
                        self.header = temp.payload[:12]
                        self.byte_buffer = temp.payload[12:(12+self.actual_size)]
                        if DEBUG == True:
                            print('CRC size: ',self.actual_size, ' crc: ', self.crc16)
                            print(self.header,self.byte_buffer)
                            print(temp.payload)
                        if transaction_id != temp.payload[0:4]: raise Exception('id error')
                        if block_number != temp.payload[4:8]: raise Exception('block number error')
                        if self.crc16 != CRCCCITT().calculate(self.byte_buffer):
                            raise Exception(f'block {block_number} CRC16 error {self.crc16}')
                        file.write(self.byte_buffer)
                        
        #    self.pub('sensiml/storage/get/stop', transaction_id,1)
        #    self.resp()
        except Exception as err:
            print(str(err.args))
        self.pub('sensiml/storage/get/stop', transaction_id,1)
        self.resp()

               
    def put(self,file_name,out_name = None):
        '''put a file from local to the device'''
        if os.path.exists(file_name) == False:
            return file_name + " not found"
        try:
            if out_name == None:
                out_name = '/default/' + file_name
            with open(file_name,'rb') as file:
                file_buffer = file.read()
            
            transaction_id = struct.pack('>I',random.randint(0,0xffffffff))
            total_size = len(file_buffer)
            block_size = self.storage['put_size']
            total_blocks = math.ceil(total_size / block_size)
            total_crc16 = CRCCCITT().calculate(file_buffer)
            header = transaction_id + struct.pack('>IIH',total_size, block_size,total_crc16)
            self.pub('sensiml/storage/put/start' ,header + out_name.encode(), 1)
            self.resp()
            for i in range(total_blocks):
                block_number = struct.pack('>I', i)
                start = i* block_size
                stop = start + block_size
                if stop > total_size: stop = total_size
                actual_size = stop - start
                crc16 = CRCCCITT().calculate(file_buffer[start:stop])
                header = transaction_id + struct.pack('>IHH', i, actual_size, crc16)
                self.pub('sensiml/storage/put/data',header + file_buffer[start:stop] ,1)
                time.sleep(0.5)
                self.resp()
            self.pub('sensiml/storage/put/stop',transaction_id,1)
            self.resp()
        
        except Exception as err:
            print(str(err.args))

    def delete(self,file_name):
        '''delete a file from the device and save it locally'''
        '''filename = file_name.split('/')
        file_exist = False
        if filename[1] == 'default' :
            for i in self.dir_default:
                if filename[2] == i[3]: 
                    file_exist = True
                    break
        if file_exist != True:
            return file_name + " not found"'''
        try:
            self.pub('sensiml/storage/del',file_name,1)
            time.sleep(1)
        except Exception as err:
            print(str(err.args))

    def dir(self, space='/*.*'):
        ''' dir will first check /*.* then check all subdirectory
        obtain directory structure
        '''
        try:
            self.pub('sensiml/storage/dir/req',space,1)
            time.sleep(1)
            self.resp()
            if space == '/*.*':
                for i in self.dir_root:
                    self.pub('sensiml/storage/dir/req',i[3]+'*.*',1)
                    time.sleep(2)
                    self.resp()
                    
                
        except Excption as err:
            print(str(err.arg))


if __name__ == "__main__":
    '''main test procedures'''
    logging.basicConfig(filename = 'log_'+time.strftime("%Y%m%d-%H%M%S")+'.txt',
                        filemode = 'w',
                        level=logging.DEBUG,
                        format='%(asctime)s - %(levelname)s - %(message)s')
    dcl = DCL('dcl_'+str(int(time.monotonic())))
    if dcl.connect(mq_host,mq_port,60) == 0:
        print(int(time.monotonic()), "DCL connected")
        logging.debug("DCL connected" )
    time.sleep(0.2)
    dcl.loop_start()
    dcl.sub_all(DCL_SUB_LIST)
    dcl.on_message = dcl_on_message

    
    for index in TEST_CASES:
        
        n=1
        print(int(time.monotonic()),f"step {n} --- all stop")
        logging.debug(f"step {n} --- all stop")
        dcl.pub('sensiml/sys/all/stop', "", 1)
        dcl.resp()

        n+=1
        print(int(time.monotonic()),f"step {n} --- status clear")
        logging.debug(f"step {n} --- status clear")
        dcl.pub('sensiml/sys/status/clr',"",1)
        dcl.resp()
        n+=1
        print(int(time.monotonic()),f"step {n} --- status req")
        logging.debug(f"step {n} --- status req")
        dcl.pub('sensiml/sys/status/req',"",1)
        dcl.resp()

        n+=1
        print(int(time.monotonic()),f"step {n} --- version req")
        logging.debug(f"step {n} --- version req")
        dcl.pub('sensiml/sys/version/req',"",1)
        dcl.resp()

        print("QAI version", dcl.sw_version)
        if dcl.sw_version.split()[0] == 'Chilkat':
            imu_freq = 25
        else:
            imu_freq = 26

        test_cases = add_tests([14, 28, 54, 105, 210, 400, 600])
        
        n+=1
        print(int(time.monotonic()),f"step {n} --- compile time req")
        logging.debug(f"step {n} --- compile time req")
        dcl.pub('sensiml/sys/compdatetime/req',"",1)
        dcl.resp()

        n+=1
        print(int(time.monotonic()),f"step {n} --- device UUID req")
        logging.debug(f"step {n} --- compile time req")
        dcl.pub('sensiml/sys/device/uuids/req',"",1)
        dcl.resp()

        n+=1
        print(int(time.monotonic()),f"step {n} --- set UNIX time")
        msg = struct.pack('>I',int(time.time()-time.timezone))
        logging.debug(f"step {n} --- set UNIX time")
        dcl.pub('sensiml/sys/unixtime/set',msg,1)
        dcl.resp()

        n+=1
        print(int(time.monotonic()),f"step {n} --- sensor list req")
        logging.debug(f"step {n} --- sensor list req")
        dcl.pub('sensiml/sensor/list/req',"",1)
        dcl.resp()
        
        n+=1
        print(int(time.monotonic()),f"step {n} --- sensor clear")
        logging.debug(f"step {n} --- compile time req")
        dcl.pub('sensiml/sensor/clr',"",1)
        dcl.resp()
            
        n+=1
        print(int(time.monotonic()),
              f"step {n} --- add {test_cases[index].sensor_id} {test_cases[index].rate}Hz")
        logging.debug(f"step {n} --- add {test_cases[index].sensor_id} {test_cases[index].rate}Hz")
        if ONESENSOR == True:
            dcl.pub('sensiml/sensor/add',test_cases[index].sensor_add,1)
        else:
            #IMUA
            dcl.pub('sensiml/sensor/add',b'IMUA\x00\x00\x00d\x14',1)
            dcl.pub('sensiml/sensor/add',b'IMUG\x00\x00\x00d\x01',1)
        dcl.resp()
        
        n+=1
        print(int(time.monotonic()),f"step {n} --- sensor done")
        logging.debug(f"step {n} --- sensor done")
        dcl.pub('sensiml/sensor/done','',1)
        dcl.resp()
        
        n+=1
        print(int(time.monotonic()),f"step {n} --- stutus req ")
        logging.debug(f"step {n} --- stutus req ")
        dcl.pub('sensiml/sys/status/req',"",1)
        dcl.resp()

        n+=1
        print(int(time.monotonic()),f"step {n} --- live list req ")
        logging.debug(f"step {n} --- compile time req")
        dcl.pub('sensiml/live/sensor/list/req',"",1)
        dcl.resp()

        if COLLECT == True:
            n+=1
            print(int(time.monotonic()),f"step {n} --- collect prefix")
            logging.debug(f"step {n} --- collect prefix")
            dcl.pub('sensiml/collect/prefix/set', dcl.sw_version.split()[0], 1)
            time.sleep(pub_delay)

            n+=1
            data_uuid=uuid.uuid4().bytes
            print(int(time.monotonic()),f"step {n} -- collect start with UUID", data_uuid)
            logging.debug(f"step {n} -- collect start with UUID" + str(data_uuid))
            dcl.pub('sensiml/collect/start',data_uuid,1)
            dcl.resp()
            time.sleep(TIME_COLLECT)
           
            n+=1
            print(int(time.monotonic()),f"step {n} -- collect stop")
            logging.debug(f"step {n} -- collect stop")
            dcl.pub('sensiml/collect/stop','', 1)
            dcl.resp()
            #saving to sd card. so there is a ~2second delay
            time.sleep(2)
            
            n+=1
            print(int(time.monotonic()),f"step {n} --- live list req ")
            logging.debug(f"step {n} --- live list req ")
            dcl.pub('sensiml/live/sensor/list/req',"",1)
            dcl.resp()

        if LIVE == True and \
           test_cases[index].sensor_id != b'LTH\x9e' :#and \
           #test_cases[index].sensor_id != b'AUDO':
            n+=1
            print(int(time.monotonic()),
                  f"step {n} --- live count down {test_cases[index].cd}")
            logging.debug(f"step {n} --- live count down {test_cases[index].cd}")
            if ONESENSOR:
                dcl.pub( 'sensiml/live/set/rate/req', test_cases[index].live_set_rate, 1)
                dcl.resp()
            else:
                #IMUA
                dcl.pub('sensiml/live/set/rate/req', b'\x01IMUA\x00\x00\x00\x00', 1)
                dcl.resp()
                dcl.pub('sensiml/live/set/rate/req', b'\x01IMUG\x00\x00\x00\x03', 1)
                dcl.resp()
            
            n+=1
            print(int(time.monotonic()),f"step {n} -- live streams for {TIME_LIVE} seconds")
            logging.debug(f"step {n} -- live starts")
            if ONESENSOR:
                dcl.pub('sensiml/live/start',test_cases[index].live_start,1)
            else:
                #IMUA,G
                dcl.pub('sensiml/live/start',b'\x02IMUA\x02IMUG\x01',1)
                
            time.sleep(TIME_LIVE)
            
            n+=1
            print(int(time.monotonic()),f"step {n} -- live stops")
            logging.debug(f"step {n} -- live stops")

            #dcl.pub('sensiml/collect/stop','', 1)
            

            dcl.pub('sensiml/live/stop','', 1)
            dcl.live_count = 0
            dcl.live_seq = -1
            dcl.resp()
            print(int(time.monotonic()), f'total live data {dcl.live_count}, ')
            assert dcl.live_count >= \
                   test_cases[index].rate /\
                   (test_cases[index].cd + 1) /\
                   test_cases[index].spp * TIME_LIVE* 0.9
            assert dcl.live_count <= \
                   test_cases[index].rate /\
                   (test_cases[index].cd + 1) /\
                   test_cases[index].spp * TIME_LIVE * 1.1                

        if RECOG == True:
            #n+=1
            #print(int(time.monotonic()),f"step {n} -- recog request modle UUDI")
            #logging.debug(f"tep {n} -- recog request modle UUDI")
            #dcl.pub("sensiml/recog/model/uuid/req","", 1)
            #dcl.resp()

            n+=1
            print(int(time.monotonic()),f"step {n} -- recog start default ")
            logging.debug(f"step {n} -- recog start default ")
            dcl.pub("sensiml/recog/start",'',1)

            n+=1
            print(int(time.monotonic()),f"step {n} -- classification set countdown 0")
            logging.debug(f"step {n} -- classification set countdown 0")
            dcl.pub( 'sensiml/result/class/set/rate',b'\x00',1)


            n+=1
            # data_type U8 could be either 1 or 2 (+fv)
            print(int(time.monotonic()),f"step {n} -- classification start b'\x01'")
            logging.debug(f"step {n} -- classification start b'\x01'")
            dcl.pub( 'sensiml/result/class/start',b'\x01',1)

            n+=1
            time.sleep(20)
            print(int(time.monotonic()),f"step {n} -- classification stop")
            logging.debug(f"step {n} -- classification stop")
            dcl.pub("sensiml/result/class/stop","",1)
            dcl.resp()

            n+=1
            print(int(time.monotonic()),f"step {n} -- classification start fv b'\x02'")
            logging.debug(f"step {n} -- classification start fv b'\x02'")
            dcl.pub("sensiml/result/class/start",b'\x02',1)

            time.sleep(20)
            n+=1
            print(int(time.monotonic()),f"step {n} -- classification stop fv")
            logging.debug(f"step {n} -- classification stop fv")
            dcl.pub("sensiml/result/class/stop","",1)

            dcl.resp()

            n+=1
            print(int(time.monotonic()),f"step {n} -- recog stop")
            logging.debug(f"step {n} -- recog stop")
            dcl.pub("sensiml/recog/stop","",1)

        if STORAGE == True:
            n+=1
            print(int(time.monotonic()),f"step {n} -- storage /*.* req")
            logging.debug(f"step {n} -- storage /*.* req")
            dcl.pub( 'sensiml/storage/dir/req','/*.*',1)
            time.sleep(2)
            dcl.resp()

            n+=1
            print(int(time.monotonic()),f"step {n} -- storage /default/*.* req")
            logging.debug(f"step {n} -- storage /default/*.* req")
            dcl.pub( 'sensiml/storage/dir/req','/default/*.*',1)
            time.sleep(2)
            dcl.resp()

            n+=1
            print(int(time.monotonic()),f"step {n} -- storage space /default/ req")
            logging.debug(f"step {n} -- storage space /default/ req")
            dcl.pub( 'sensiml/storage/space/req','/default/',1)
            time.sleep(5)
            dcl.resp()

            n+=1
            print(int(time.monotonic()),f"step {n} -- get  /default/ last file")
            logging.debug(f"step {n} -- get  /default/ last file")
            dcl.get('/default/' + dcl.dir_default[-1][3])
                 
            n+=1
            print(int(time.monotonic()),f"step {n} -- storage put config.txt as /default/test.qlsm")
            logging.debug(f"step {n} -- storage put config.txt as /default/test.qlsm")
            dcl.put(dcl.dir_default[-1][3],'/default/test.qlsm')
            
            n+=1
            print(int(time.monotonic()),f"step {n} -- storage /default/*.* req")
            logging.debug(f"step {n} -- storage /default/*.* req")
            dcl.pub( 'sensiml/storage/dir/req','/default/*.*',1)
            time.sleep(2)
            dcl.resp()

            n+=1
            print(int(time.monotonic()),f"step {n} -- storage del /default/test.qlsm")
            logging.debug(f"step {n} -- storage del /default/test.qlsm")
            dcl.delete('/default/test.qlsm')
            dcl.resp()

            n+=1
            print(int(time.monotonic()),f"step {n} -- storage /default/*.* req")
            logging.debug(f"step {n} -- storage /default/*.* req")
            dcl.pub( 'sensiml/storage/dir/req','/default/*.*',1)
            time.sleep(2)
            dcl.resp()

        dcl.dir()
        file_numbers = len(dcl.dir_default)
        if file_numbers>10:
            n+=1
            print(int(time.monotonic()),f"step {n} -- remove extra /default/ qlsm files")
            logging.debug(f"step {n} -- remove extra /default/ qlsm files")
            file_numbers = len(dcl.dir_default)
            if file_numbers>10:
                for i in range(file_numbers -5):
                    temp=dcl.dir_default[i][3].split('.')
                    if len(temp)>1 and temp[1] =='qlsm':
                        dcl.delete('/default/'+dcl.dir_default[i][3])
            
            print(int(time.monotonic()),f"step {n} -- storage /default/*.* req")
            logging.debug(f"step {n} -- storage /default/*.* req")
            dcl.pub( 'sensiml/storage/dir/req','/default/*.*',1)
            time.sleep(2)
            dcl.resp()
        for i in dcl.errors_list:
            print(i)
        assert (not dcl.errors_list)
    #dcl.disconnect()


