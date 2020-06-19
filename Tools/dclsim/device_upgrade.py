# Upgrade script
#
# A simple USB relay toggles devices ON and OFF
# upgrading fw is done through UART console
# make sure the command CommandApp_USBRelay.exe in the python root folder
# filepath : latest app file is in the right folder 
# comport : com9

import subprocess, os
import time
import logging

filepath = r'C:\WORK\QAI_v2.0_rc5\Release_packages\images\chilkat_ai_app\IAR'
filename = r'flash_via_uart_iar.bat'
uart_port = 'com9'



def device(state):
    '''state = 1 for ON; state = 0 for OFF'''
    if state == 1:
        ret=subprocess.call('CommandApp_USBRelay.exe BITFT open 1',shell=True)
        return 1
    elif state == 0:
        ret=subprocess.call('CommandApp_USBRelay.exe BITFT close 1', shell=True)
        return 0
    else:
        return -1
    
def upgrade(uart_port):
    pro = subprocess.run([filepath+'\\'+filename,uart_port],
                         capture_output=True,
                         text=True,
                         cwd=filepath)
    return pro.stdout


def main():
    logging.basicConfig(filename = 'upgrade_'+time.strftime("%Y%m%d-%H%M%S")+'.txt',
                        filemode = 'w',
                        level=logging.DEBUG,
                        format='%(asctime)s - %(levelname)s - %(message)s')
    print('Device OFF')
    if device(0) == 0 : 
        logging.debug('Device OFF')
    time.sleep(1)

    print('Device ON')
    if device(1) == 1:
        logging.debug(f'Device ON')
    time.sleep(1)

    print(f'Device Upgrading through port {uart_port}')
    print(f'{time.asctime()}')
    logging.debug(f'Device Upgrading through port {uart_port}')
    logging.debug(f'{time.asctime()}')
    
    ret = upgrade(uart_port)
    logging.debug(ret)
    logging.debug(f'{time.asctime()}')
    result = ret.split('\n')[-3]
    print(f'Device finished upgrading {result}')
    print(f'{time.asctime()}')
    
    device(0)
    time.sleep(1)
    device(1)
    time.sleep(20) # bootup time

if __name__ == "__main__":
    main()
