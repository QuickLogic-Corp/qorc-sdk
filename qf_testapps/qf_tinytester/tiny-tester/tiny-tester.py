import json
import csv
import numpy as np
import serial

errors = False
error_count = 0

with open("scan.json", "r") as read_file:
    scan_config = json.load(read_file)
print(scan_config)

x = scan_config["channels"][0]


print(x, len(scan_config["channels"]))

oe = 0
p0 = 0
p1 = 0
p2 = 0
p3 = 0

mpcsvcol2channel = []

def send_cmd(cmd):
    print(cmd)
    tester.write(cmd.encode('utf_8'))
    tester.write(b'\n')
    line = tester.readline()
    #print(line)
    while line != b'[0] > ':
        line = tester.readline()
        #print(line)

def execute_vector(line_num, data2DUT, expected_input_mask, expected_data):
    cmd = "x " + hex(data2DUT)
    error_vector = list("-" * signals)
    #print(cmd, hex(expected_input_mask), hex(expected_data))
    tester.write(cmd.encode('utf_8'))
    tester.write(b'\n')
    line = tester.readline()
    #print(line)
    while line != b'[0] > ':
        line = tester.readline()
        #print(line)
        t = line.decode().split()
        #print(t)
        if t[0] == "di:":
            actual_data = int(t[1],0)
            #print("ad:", hex(actual_data))
            mismatch = False
            for i in range(signals):
                bit = 1 << i
                error_vector[i] = '-'
                if (bit & expected_input_mask) != 0:
                    bed = (bit & expected_data)
                    bad = (bit & actual_data)
                    #print("check", hex(bed), hex(bad))
                    if bed == 0 and bad != 0:
                        error_vector[i] = '^'
                        mismatch = True
                    if bed != 0 and bad == 0:
                        error_vector[i] = 'v'
                        mismatch = True
            if mismatch:
                err_str = ""
                print(line_num, err_str.join(error_vector), hex(actual_data))

def execute_binary_vector(first, line_num, data2DUT, expected_input_mask, expected_data):
    global error_count
    if first:
        tester.write(b'\n')
        line = tester.readline()
        #print(line)
        while line != b'[0] > ':
            line = tester.readline()
            print(line)
        cmd = "b"
        tester.write(cmd.encode('utf_8'))
        tester.write(b'\n') # triggers a newline from feather
        line = tester.readline()  # ignore the newline
    # Action
    tester.write(b'0')  # continue ('\r' terminates)
    tester.write(data2DUT.to_bytes(4, 'little'))
    data = tester.read(4)
    #print(hex(data[3]), hex(data[2]), hex(data[1]), hex(data[0]))
    actual_data = (((((data[3] << 8) | data[2]) << 8) | data[1]) << 8) | data[0]
    error_vector = list("-" * signals)
    mismatch = False
    for i in range(signals):
        bit = 1 << i
        error_vector[i] = '-'
        bed = (bit & expected_data)
        bad = (bit & actual_data)
        #print("check", hex(bed), hex(bad))
        if (bit & expected_input_mask) != 0:
            if bed == 0 and bad != 0:
                error_vector[i] = '^'
                mismatch = True
            if bed != 0 and bad == 0:
                error_vector[i] = 'v'
                mismatch = True
        else:
            bdrv = (bit & data2DUT)
            bp3  = (bit & p3)
            #if bp3 and bdrv != bad:
            #    e = 1 if bdrv else 0;
            #    a = 1 if bad  else 0;
            #    print(line_num, scan_config["channels"][mpcsvcol2channel[i]]["signal"], "ch", mpcsvcol2channel[i], "drive error (e/a). ", e, a)
            #    #print(hex(data2DUT), hex(actual_data))
            #    error_count = error_count + 1
            #    if error_count > 10:
            #        print("stopping: reached error_count limit")
            #        exit()
    if mismatch:
        err_str = ""
        error_vector2 = list(" " * (signals + int(signals/4)))
        for i in range(len(error_vector)):
            index = len(error_vector2)-(i+int(i/4))-1
            error_vector2[index] = error_vector[i]
        print(line_num, err_str.join(error_vector2), hex(actual_data), hex(expected_data))
        error_count = error_count + 1
        if error_count > 50:
            print("stopping: reached error_count limit")
            exit()
    
        
with open("chain.stil.csv", newline='') as csvfile:
    stil = csv.reader(csvfile, delimiter=',', quotechar='|')
    print(stil, stil.line_num)
    with serial.Serial("/dev/ttyS15", 115200, timeout=0.1, writeTimeout=1) as tester:
        tester.write(b'\r')
        line = b''
        while line != b'[0] > ':
            line = tester.readline()
            print("got", line)
            if line == b'':
                tester.write(b'\r')

        header_row = True
        line_num = 1
        for row in stil:
            signals = len(row)
            if header_row:
                print("Header")
                header_row = False
                
                for signal in row:
                    signal = signal.replace(' ','')
                    print("signal", signal)
                    found = False
                    channel = 0
                    for config in scan_config["channels"]:
                        if signal == config["signal"]:
                            mpcsvcol2channel.append(channel)
                            found = True
                            # set characteristics
                            if config["direction"] == "in":
                                oe = oe | (1 << channel) # An input to DUT, so set oe
                                drive = config["drive"]
                                if (len(drive) != 4):
                                    print("error: expected 4 characters for 4 drive phases", drive, signal)
                                    errors = True
                                else:
                                    if drive[0] == 'D':
                                        p0 = p0 | (1 << channel)
                                    if drive[1] == 'D':
                                        p1 = p1 | (1 << channel)
                                    if drive[2] == 'D':
                                        p2 = p2 | (1 << channel)
                                    if drive[3] == 'D':
                                        p3 = p3 | (1 << channel)
                        channel = channel + 1
                    if not found:
                        print("Could not find channel assignment for signal", signal)
                        errors = True
                    
                if errors:
                    print("Stopping until errors fixed")
                    break
                send_cmd("oe " + hex(oe))
                send_cmd("p0 " + hex(p0))
                send_cmd("p1 " + hex(p1))
                send_cmd("p2 " + hex(p2))
                send_cmd("p3 " + hex(p3))
                drive = np.array([0] * signals)
                print(mpcsvcol2channel)
            else:
                csvcol = 0
                line_num = line_num + 1
                if line_num%100 == 0:
                    print(line_num)
                input2DUT = 0;
                expected_data_mask = 0;
                expected_data = 0;
                for action in row:
                    signal = scan_config["channels"][mpcsvcol2channel[csvcol]]["signal"]
                    if scan_config["channels"][mpcsvcol2channel[csvcol]]["direction"] == "in": # Input to DUT
                        if action == '1':  # Driving input to a 1
                            input2DUT = input2DUT | (1 << mpcsvcol2channel[csvcol])
                        elif action == '0' or action == 'X': # Driving to 0 (which is default) or 'X'
                            input2DUT = input2DUT & ~(1 << mpcsvcol2channel[csvcol])
                        else:                                # Unexpected action
                            print("line_num", line_num, "input signal", signal, "has unexpected value", action)
                            errors = True
                    elif scan_config["channels"][mpcsvcol2channel[csvcol]]["direction"] == "out": # Input to DUT
                        if action == 'L':
                            expected_data_mask = expected_data_mask | (1 << mpcsvcol2channel[csvcol])
                        elif action == 'H':
                            expected_data_mask = expected_data_mask | (1 << mpcsvcol2channel[csvcol])
                            expected_data = expected_data | (1 << mpcsvcol2channel[csvcol])
                        elif action == 'X': #ignore
                            expected_data_mask = expected_data_mask & ~(1 << mpcsvcol2channel[csvcol])
                        else:
                            print("line_num", line_num, "output signal", signal, "has unexpected value", action)
                            errors = True
                    csvcol = csvcol + 1
                execute_binary_vector(line_num == 2, line_num, input2DUT, expected_data_mask, expected_data)
                #drive = np.vstack((drive, tmp_drive))
