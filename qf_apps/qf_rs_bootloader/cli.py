#!python3.7
import sys
import os
import subprocess
import serial

# need to find the correct location for bundled packages
script_path = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, os.path.join(script_path, 'pkgs'))
sys.path.insert(0, os.path.join(script_path, 'a-series-programmer', 'python'))
sys.path.insert(0, os.path.join(script_path, 'q-series', 'python'))

try:
   from tinyfpgaq import TinyFPGAQ
except:
   print('To run this program, copy the source code {} to '.format(__file__), 
         '\nTinyFPGA-Programmer-Application folder and run from that folder')
   sys.exit(0)

# Host command set
# flashfpga <filename>     Load contents of <filename> to APPFPGA flash region
# runfpga                  Reset the FPGA, read APPFPGA flash region and load/configure FPGA
# change-boot-mode {1, 2}  1 ==> boot-loader, 2 ==> old-behavior

comport = None

# Command parsing loop
def help_cmd(*args):
    print('help command ', args)
    print('Available commands are')
    [ print('{:20s} {}'.format(cmd, cmdhelptxt[cmd])) for cmd in cmdacts ]

def run_application(*args):
    print('Running the command: ', ' '.join(args))
    p = subprocess.Popen(args)
    try:
      out, err = p.communicate(timeout=10*60)
      #print(out)
      #print(err)
    except subprocess.TimeoutExpired:
      print('[S3-CLI] timeout exiting ', ' '.join(args))
      p.kill()
      out, err = p.communicate()

def flash_fpga_cmd(*args):
    if (comport == None):
       print('Please specify COM port using the comport command')
       return
    print('flash fpga command ', args, 'type(args) = ', type(args), 'len(args) = ', len(args))
    runcmd = [ 'python', 'tinyfpga-programmer-gui.py', '--mode', 'fpga', '--appfpga', args[0], '--port', comport ]
    run_application(*runcmd)

def run_fpga_cmd(*args):
    if (comport == None):
       print('Please specify COM port using the comport command')
       return
    print('run fpga command ', args)
    runcmd = [ 'python', 'tinyfpga-programmer-gui.py', '--mode', 'fpga', '--reset', '--port', comport ]
    run_application(*runcmd)

def change_boot_mode_cmd(*args):
    if (comport == None):
       print('Please specify COM port using the comport command')
       return
    new_mode = int(args[0])
    print('change boot mode command', args, 'new mode=', new_mode)
    with serial.Serial(comport, 115200, timeout=60, writeTimeout=60) as ser:
       addr = 0x1F000 + 8
       if new_mode == 1:
          bitstream = b'\x55' * 4
       else:
          bitstream = b'\xAA' * 4
       print('Setting boot mode to ', new_mode, ':', bitstream.hex()) 
       fpga = TinyFPGAQ(ser)
       print(fpga)
       ret = fpga.program_bitstream(addr, bitstream, 'boot-mode')

def quit_cmd(*args):
    sys.exit(0)

def set_comport_cmd(*args):
    global comport
    print('set comport command ', args, args[0])
    comport = args[0]

def flash_m4app_cmd(*args):
    if (comport == None):
       print('Please specify COM port using the comport command')
       return
    print('flash m4app command ', args, 'type(args) = ', type(args), 'len(args) = ', len(args))
    runcmd = [ 'python', 'tinyfpga-programmer-gui.py', '--mode', 'm4', '--m4app', args[0], '--port', comport ]
    run_application(*runcmd)

def run_m4app_cmd(*args):
    if (comport == None):
       print('Please specify COM port using the comport command')
       return
    print('run m4app command ', args)
    runcmd = [ 'python', 'tinyfpga-programmer-gui.py', '--mode', 'm4', '--reset', '--port', comport ]
    run_application(*runcmd)

cmdacts = { 'help': help_cmd, 
            'flashfpga': flash_fpga_cmd,
            'runfpga': run_fpga_cmd,
            'change-boot-mode': change_boot_mode_cmd,
            'quit': quit_cmd,
            'comport': set_comport_cmd
          }

cmdhelptxt = { 'help': 'List available commands and basic usage', 
            'flashfpga': 'Usage: flashfpga <filename> Write file bitstream contents to the APPFPGA flash region',
            'runfpga': 'Configure FPGA and run the program',
            'change-boot-mode': 'Change boot mode, 1 => change to boot-loader-mode, 2 => change to application-mode ',
            'quit': 'Quit this program',
            'comport': 'Usage: comport <comport>, set comport'
          }

def readAndExecuteCommandLineInput():
    try:
        s = input('[S3]> ')
        cmdargs = s.split()
        if len(cmdargs) == 0:
           return False
        if (cmdargs[0] == 'quit'):
           return True
        if (cmdargs[0] in cmdacts):
           cmdacts[cmdargs[0]](*cmdargs[1:])
           return False
        else:
           print('Unknown command:', cmdargs[0])
    except:
        print('Invalid input:', s, '==')
        [ print(cmd, end=' ') for cmd in cmdargs ]
        print('======')
        pass

if __name__ == "__main__":
   quitprog = False
   try:
      while not quitprog:
         quitprog = readAndExecuteCommandLineInput()
   except SystemExit:
      print('Exiting...')
      os._exit(1)
   except KeyboardInterrupt:
      sys.exit(1)
