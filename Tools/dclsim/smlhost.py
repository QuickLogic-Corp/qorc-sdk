# Example PC host application for SensiML AI App
# usage scenerios
# Scenerio 1 Live stream sensor data for the given
#      Sensor ID, sensor sample rate, sensor sample range
#      count down (if any to throttle streaming speed),
#      samples per packet
# Scenerio 2 Recognition mode: publish recognition results
#
import paho.mqtt.client as mqtt
import struct
import time
import argparse

parser = argparse.ArgumentParser(description="Example host app for SensiML AI App")
parser.add_argument('--timeout', dest='timeout', default=5, type=int, help='perform for timeout secs')
parser.add_argument('--log', dest='log', default=False, action='store_true', help='Log MQTT messages')
parser.add_argument('--filename', dest='filename', default='outdata.bin', type=str, help='filename to store live-streaming data')

group = parser.add_mutually_exclusive_group(required=True)
group.add_argument('--live', action='store_true')
group.add_argument('--recog', action='store_true')
group.add_argument('--clear', action='store_true', help='clear device state, return it to clean state')

accel_group = parser.add_argument_group('accel', 'Accelermoter group')
accel_group.add_argument('--accel', dest='accel', action='store_true', help='add accelerometer sensor for tests')
accel_group.add_argument('--accel_rate', dest='accel_rate', default=100, type=int, help='Sampling rate in Hz')
accel_group.add_argument('--accel_range', dest='accel_range', default=2, type=int, help='sample range multiple of gravity constant g')
accel_group.add_argument('--accel_count_down', dest='accel_count_down', default=0, type=int, help='count down for live-streaming, actual live rate will be rate/(count_down+1)')
accel_group.add_argument('--accel_spp', dest='accel_spp', default=8, type=int, help='samples per packet for live-streaming')

ad7476_group = parser.add_argument_group('ad7476', 'AD7476 group')
ad7476_group.add_argument('--ad7476', dest='ad7476', action='store_true', help='add AD7476 ADC sensor for tests')
ad7476_group.add_argument('--ad7476_rate', dest='ad7476_rate', default=1000000, type=int, help='Sampling rate in Hz')
ad7476_group.add_argument('--ad7476_count_down', dest='ad7476_count_down', default=999, type=int, help='count down for live-streaming, actual live rate will be rate/(count_down+1)')
ad7476_group.add_argument('--ad7476_spp', dest='ad7476_spp', default=8, type=int, help='samples per packet for live-streaming')

audio_group = parser.add_argument_group('audio', 'Audio group')
audio_group.add_argument('--audio', dest='audio', action='store_true', help='add audio sensor for tests')
accel_group.add_argument('--audio_rate', dest='audio_rate', default=16000, type=int, help='Sampling rate in Hz')
audio_group.add_argument('--audio_count_down', dest='audio_count_down', default=3, type=int, help='count down for live-streaming, actual live rate will be rate/(count_down+1)')
audio_group.add_argument('--audio_spp', dest='audio_spp', default=8, type=int, help='samples per packet for live-streaming')

recog_group = parser.add_argument_group('recog', 'Recognition group')
recog_group.add_argument('--features', dest='features', default=False, action='store_true', help='publish feature vector in recognition results')

topics = [ ("sensiml/sys/device/uuids/rsp", 1),
           ("sensiml/sys/version/rsp", 1),
           ("sensiml/sys/compdatetime/rsp", 1),
           ("sensiml/sys/status/rsp", 1),
           ("sensiml/sys/error", 1),
           ("sensiml/recog/model/uuid/rsp", 1),
           ("sensiml/sensor/list/rsp", 1),
           ("sensiml/snap/list/rsp", 1),
           ("sensiml/snap/data/rsp", 1),
           ("sensiml/live/sensor/list/rsp", 1),
           ("sensiml/live/set/rate/rsp", 1),
           ("sensiml/live/raw/data", 1),
           ("sensiml/result/class/data", 1),
           ("sensiml/sys/will/status", 1)]

sysver = 'Unknown'
dev_response = 0     # indicates if device responded to the request
total_bytes = 0      # indicates bytes received during live stream
total_packets = 0    # indicates packets received durign live stream
live_stream_start_time = 0   # live streaming start time
live_stream_end_time = 0     # live streaming end time

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

class ADC_AD7476(SENSOR):
    def __init__(self, sensor_id, rate, bit, cd, spp, *chan):
        super().__init__(sensor_id, rate, bit, cd,spp,*chan)

    def generate_fields(self):
        super().generate_fields()
        self.sensor_add +=struct.pack('>B',0)

class AUDO(SENSOR):
    def __init__(self,sensor_id, rate, bit, cd,spp,*chan):
        super().__init__(sensor_id, rate, bit, cd,spp,*chan)

    def generate_fields(self):
        super().generate_fields()
        self.sensor_add +=struct.pack('>B',self.bit)
        for i in self.chan:
             self.sensor_add +=struct.pack('>B',i)
        
# Process log messages
def on_log(client, userdata, level, buf):
    print("log: ", buf)

def on_publish(client, userdata, mid):
    pass

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

# Process received message
def on_message(client, userdata, msg):
    print(msg.topic+" "+msg.payload.hex())

# Process device response
# the following topics are expected to be registered for this callback
# sensiml/sys/#
# sensiml/sensor/#
# sensiml/live/+/+/rsp
#
def cb_dev_response(client, userdata, msg):
    global dev_response
    dev_response = 1

def cb_sysver_response(client, userdata, msg):
    global dev_response
    global sysver
    if ('C ' in str(msg.payload[:2])):
       sysver = 'Collection'
    elif ('R ' in str(msg.payload[:2])):
       sysver = 'Recognition'
    else:
       sysver = 'Unknown'
    dev_response = 1

# Process live streaming data
# the following topics are expected to be registered for this callback
# sensiml/live/raw/data
# userdata is expected to contain a file object
# received data will be written to this file object
def cb_live_raw_data(client, userdata, msg):
    global total_bytes
    global total_packets
    global live_stream_start_time
    global live_stream_end_time
    userdata.write(msg.payload)
    if (total_bytes == 0):
        live_stream_start_time = time.time()
    else:
        live_stream_end_time = time.time()
    total_bytes = total_bytes + len(msg.payload)
    total_packets = total_packets + 1

# Process recognition data
# the following topics are expected to be registered for this callback
# sensiml/result/class/data
#
def cb_result_class_data(client, userdata, msg):
    print(msg.topic+" "+msg.payload.hex())
    return
    data_type, = struct.unpack('<H', msg.payload[0:2])
    time_stamp, model_index, res = struct.unpack('>QBH', msg.payload[2:13])
    count = 0
    fv = []
    if (data_type == 2):
        count, = struct.unpack('B', msg.payload[13:14])
        for indx in range(count):
            fv_indx, = struct.unpack('B', msg.payload[14+indx:15+indx])
            fv.append(fv_indx)
    if (data_type == 2):
       print("RECOG{}: {} Model Index:{}, Result:{}, FV:{}".format(data_type, time_stamp, model_index, res, fv))
    else:
       print("RECOG{}: {} Model Index:{}, Result:{}".format(data_type, time_stamp, model_index, res))

def wait_response():
    global dev_response
    while dev_response == 0:
        time.sleep(0.1)
    dev_response = 0

def send_message(client, topic, payload, qos, wait=False):
    ret = client.publish(topic, payload, qos)
    #print("Sending {:31s} {} qos={} ret={}".format(topic, payload.hex(), qos, ret))
    ret.wait_for_publish()
    time.sleep(.2)
    if (wait):
        wait_response()

def test_live_streaming(sensorobj, filename, streaming_time=30, log=False):
    global total_bytes
    global total_packets
    total_bytes = 0
    empty = b''
    with open(filename,'wb') as outfile:
        mqttc = mqtt.Client(client_id="SensiML-Host")
        mqttc.on_connect = on_connect
        mqttc.on_message = on_message
        mqttc.on_publish = on_publish
        if (log):
           mqttc.on_log  = on_log

        def live_disconnect(mqttc):
            send_message(mqttc, "sensiml/live/stop", empty, qos=1, wait=False)
            send_message(mqttc, "sensiml/sys/status/clr", empty, qos=1, wait=False)
            mqttc.message_callback_remove("sensiml/live/raw/data")
            mqttc.message_callback_remove("sensiml/sys/#")
            mqttc.message_callback_remove("sensiml/sensor/#")
            mqttc.message_callback_remove("sensiml/live/+/+/rsp")
            mqttc.disconnect()

        mqttc.message_callback_add("sensiml/sys/#", cb_dev_response)
        mqttc.message_callback_add("sensiml/sys/version/rsp", cb_sysver_response)
        mqttc.message_callback_add("sensiml/sensor/#", cb_dev_response)
        mqttc.message_callback_add("sensiml/live/+/+/rsp", cb_dev_response)
        mqttc.message_callback_add("sensiml/live/raw/data", cb_live_raw_data)
        mqttc.connect("localhost", 1883, 60)

        mqttc.loop_start()
        mqttc.subscribe(topics)

        mqttc.on_message = on_message
        mqttc.on_publish = on_publish
        mqttc.user_data_set(outfile)
        mqttc.message_callback_add("sensiml/sys/#", cb_dev_response)
        mqttc.message_callback_add("sensiml/sensor/#", cb_dev_response)
        mqttc.message_callback_add("sensiml/live/+/+/rsp", cb_dev_response)

        send_message(mqttc, "sensiml/sys/all/stop", empty, qos=1, wait=False)
        send_message(mqttc, "sensiml/sys/status/clr", empty, qos=1, wait=False)
        send_message(mqttc, "sensiml/sys/status/req", empty, qos=1, wait=True)
        send_message(mqttc, "sensiml/sys/version/req", empty, qos=1, wait=True)
        if (sysver == 'Collection'):
            pass
        else:
            print("Current device mode: ", sysver, "does not perform Collection/Live-streaming")
            print("Load data collection enabled image and retry")
            live_disconnect(mqttc)
            return
        send_message(mqttc, "sensiml/sys/compdatetime/req", empty, qos=1, wait=True)
        send_message(mqttc, "sensiml/sys/device/uuids/req", empty, qos=1, wait=True)

        msg = struct.pack('>I', int(time.time()-time.timezone))
        send_message(mqttc, "sensiml/sys/unixtime/set", msg, qos=1, wait=False)

        send_message(mqttc,"sensiml/sensor/list/req", empty, qos=1, wait=True)
        send_message(mqttc, "sensiml/sensor/clr", empty, qos=1, wait=False)

        #msg = b'IMUA' + struct.pack('>I', sensor_rate) + b'\x14'
        msg = sensorobj.sensor_add
        send_message(mqttc, "sensiml/sensor/add", msg, qos=1, wait=False)
        send_message(mqttc, "sensiml/sensor/done", empty, qos=1, wait=False)
        send_message(mqttc, "sensiml/sys/status/req", empty, qos=1, wait=True)
        send_message(mqttc, "sensiml/live/sensor/list/req", empty, qos=1, wait=True)

        #msg = b'\x01' + b'IMUA' + struct.pack('>I', sensor_count_down)
        msg = sensorobj.live_set_rate
        send_message(mqttc, "sensiml/live/set/rate/req", msg, qos=1, wait=False)

        #msg = b'\x01' + b'IMUA' + struct.pack('>B', sensor_samples_per_packet)
        msg = sensorobj.live_start
        send_message(mqttc, "sensiml/live/start", msg, qos=1, wait=False)
        mqttc.message_callback_add("sensiml/live/raw/data", cb_live_raw_data)

        total_bytes = 0
        total_packets = 0

        if streaming_time != 0:
            time.sleep(streaming_time)
            live_disconnect(mqttc)
        else:
            while(True):
                try:
                    time.sleep(1)
                except KeyboardInterrupt:
                    live_disconnect(mqttc)
                    break

def test_disconnect():
    mqttc = mqtt.Client(client_id="SensiML-Host")
    mqttc.on_connect = on_connect
    mqttc.on_message = on_message
    mqttc.on_publish = on_publish
    mqttc.on_log     = on_log

    mqttc.message_callback_add("sensiml/sys/#", cb_dev_response)
    mqttc.connect("localhost", 1883, 60)

    mqttc.loop_start()
    mqttc.subscribe(topics)

    send_message(mqttc, "sensiml/sys/all/stop", b"", qos=1, wait=False)
    mqttc.disconnect()

def test_recog(SENSOR, filename, streaming_time=30, features=False, log=False):
    global total_bytes
    total_bytes = 0
    empty = b''
    with open(filename,'wb') as outfile:
        mqttc = mqtt.Client(client_id="SensiML-Host")
        mqttc.on_connect = on_connect
        mqttc.on_message = on_message
        mqttc.on_publish = on_publish
        if (log):
           mqttc.on_log     = on_log

        def recog_disconnect(mqttc):
            send_message(mqttc, "sensiml/result/class/stop", "", qos=1, wait=False)

            mqttc.message_callback_remove("sensiml/sys/#")
            mqttc.message_callback_remove("sensiml/sensor/#")
            mqttc.disconnect()

        mqttc.message_callback_add("sensiml/sys/#", cb_dev_response)
        mqttc.message_callback_add("sensiml/sys/version/rsp", cb_sysver_response)
        mqttc.message_callback_add("sensiml/sensor/#", cb_dev_response)
        mqttc.message_callback_add("sensiml/live/+/+/rsp", cb_dev_response)
        mqttc.message_callback_add("sensiml/live/raw/data", cb_live_raw_data)
        mqttc.message_callback_add("sensiml/result/class/data", cb_result_class_data)
        mqttc.connect("localhost", 1883, 60)

        mqttc.loop_start()
        mqttc.subscribe(topics)

        mqttc.on_message = on_message
        mqttc.on_publish = on_publish
        mqttc.user_data_set(outfile)
        mqttc.message_callback_add("sensiml/sys/#", cb_dev_response)
        mqttc.message_callback_add("sensiml/sensor/#", cb_dev_response)

        send_message(mqttc, "sensiml/sys/status/req", empty, qos=1, wait=True)
        send_message(mqttc, "sensiml/sys/version/req", empty, qos=1, wait=True)
        if (sysver == 'Recognition'):
            pass
        else:
            print("Current device mode: ", sysver, "does not perform Recognition")
            print("Load recognition enabled image and retry")
            recog_disconnect(mqttc)
            return
        send_message(mqttc, "sensiml/sys/compdatetime/req", empty, qos=1, wait=True)
        send_message(mqttc, "sensiml/sys/device/uuids/req", empty, qos=1, wait=True)

        msg = struct.pack('>I', int(time.time()-time.timezone))
        send_message(mqttc, "sensiml/sys/unixtime/set", msg, qos=1, wait=False)

        send_message(mqttc, "sensiml/sys/status/req", empty, qos=1, wait=True)

        send_message(mqttc, "sensiml/recog/start", "", qos=1, wait=False)

        msg = b'\x00'
        send_message(mqttc, "sensiml/result/class/set/rate", msg, qos=1, wait=False)
        msg = b'\x01'
        if (features):
           msg = b'\x02'
        send_message(mqttc, "sensiml/result/class/start", msg, qos=1, wait=False)

        if streaming_time != 0:
            time.sleep(streaming_time)
            recog_disconnect(mqttc)
        else:
            while(True):
                try:
                    time.sleep(1)
                except KeyboardInterrupt:
                    recog_disconnect(mqttc)
                    break;
        # time.sleep(streaming_time)
        # send_message(mqttc, "sensiml/result/class/stop", "", qos=1, wait=False)

        # mqttc.message_callback_remove("sensiml/sys/#")
        # mqttc.message_callback_remove("sensiml/sensor/#")
        # mqttc.disconnect()


args = parser.parse_args()
print(args)

if (args.accel):
   print('Configuring Accelerometer @{} Hz, {}G, {} count-down, {} samples-per-packet'.format( args.accel_rate, args.accel_range, args.accel_count_down, args.accel_spp))
   sensor_rate = args.accel_rate        # sensor sample rate in Hz
   sensor_count_down = args.accel_count_down  # sub-sampling, if any for the sensor
   sensor_samples_per_packet = args.accel_spp # 10 samples per packet
   sensor_live_rate = sensor_rate / (sensor_count_down + 1)
   sample_size = 6
   sensorobj = IMU(b'IMUA',  args.accel_rate, 16, args.accel_count_down, args.accel_spp, args.accel_range)

if (args.audio):
   print('Configuring audio @{} Hz, {} count-down, {} samples-per-packet'.format( args.audio_rate, args.audio_count_down, args.audio_spp))
   sensor_rate = args.audio_rate        # sensor sample rate in Hz
   sensor_count_down = args.audio_count_down  # sub-sampling, if any for the sensor
   sensor_samples_per_packet = args.audio_spp # 10 samples per packet
   sensor_live_rate = sensor_rate / (sensor_count_down + 1)
   sample_size = 2
   sensorobj = AUDO(b'AUDO',  args.audio_rate, 16, args.audio_count_down, args.audio_spp, 1)

if (args.ad7476):
   print('Configuring AD7476 ADC @{} Hz, {} count-down, {} samples-per-packet'.format( args.ad7476_rate, args.ad7476_count_down, args.ad7476_spp))
   sensor_rate = args.ad7476_rate        # sensor sample rate in Hz
   sensor_count_down = args.ad7476_count_down  # sub-sampling, if any for the sensor
   sensor_samples_per_packet = args.ad7476_spp # 10 samples per packet
   sensor_live_rate = sensor_rate / (sensor_count_down + 1)
   sample_size = 2
   #ADC_AD7476(b'AD\x1d4',1000000,16, 2499, 4)
   sensorobj = ADC_AD7476(b'AD\x1d\x34',args.ad7476_rate,16, args.ad7476_count_down, args.ad7476_spp)

if (args.live == True):
   print("Testing live-streaming ... ")
   test_live_streaming(sensorobj, filename=args.filename, streaming_time=args.timeout, log=args.log)
   live_stream_time = live_stream_end_time - live_stream_start_time
   print('{} bytes received in {} secs ({} packets)'.format(total_bytes, live_stream_time, total_packets))
   spp = (total_bytes - 5*total_packets) / sample_size / total_packets
   sample_rate_estimate = total_packets * spp / live_stream_time
   print('sample rate estimate = {} (Expected: {})'.format(sample_rate_estimate, sensor_live_rate))

if (args.recog == True):
   print("Testing recognition mode ...")
   test_recog(sensorobj, filename=args.filename, streaming_time=args.timeout, features=args.features, log=args.log)

if (args.clear == True):
   print("Disconnecting ...")
   test_disconnect()
