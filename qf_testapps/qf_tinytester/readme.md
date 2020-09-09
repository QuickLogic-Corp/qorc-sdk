# qf_tinytester

A program that turns Quickfeather into a functional tester.

## *tiny-tester*
While  qf_tinytester can be used standalone, it is primarily intended to be used with the tiny-tester program.
The tiny-tester program is located in the tiny-tester directory located isn qf_tinytester.  It is a Python program
that reads the tester configuration from a json file (called scan.json, although that should really be user selectable) and reads the simulus and expected
output from a csv file (called chain.stil.csv in thsi case).

### tester configuration

The tester configuration file is a json array with descriptors for each channel.  A truncated example is:

    {
      "channels" : [
        {
          "signal"    : "STM",
          "direction" : "in",
          "drive"     : "DDDD"
        },
        {
          "signal"    : "scan_clk",
          "direction" : "in",
          "drive"     : "ZDZZ"
        },
        {
          "signal"    : "sin0",
          "direction" : "in",
          "drive"     : "DDDD"
        }
        {
          "signal"    : "sout0",
          "direction" : "out"
        }
      ]
    }

There are 4 elements in the 'channels' array.  These correspond to channels 0 to 3.  Channel 0 is associated with the signal called STM, which is an input to the DUT
(Device Under Test) that is driven on all 4 phases of a cycle (see technical details section below).
Channel 1 is associated with a signal called scan_clk which is also an input to the DUT, however this signal is only driven on phase 1, hence it pulses once during each
tester cycle before returning to zero.  Channel 2 is another input, and channel 3 is associated with signal sout0, which is an output from the DUT.

### stimulus file

The stimulus file is a csv file with a header row.  The header row specifies the name of the signal in each column of the csv file.  The signal names are used to lookup the
channel characteristics from the tester configuration file.  The csv columns should not have to be in the same order as the order in the tester configuration file,
however this has not been tested, so no guarantees.

A truncated example is:

    SYS_RSTn,sout4,sout3,sout2,sout1,sout0,sin4,sin3,sin2,sin1,sin0,m4,m3,m2,m1,m0,mlatch,scan_clk,STM
    1,X,X,X,X,X,X,X,X,X,X,0,0,1,X,0,1,0,0
    1,X,X,X,X,X,X,X,X,X,X,0,0,1,X,0,1,0,0
    1,X,X,X,X,X,X,X,X,X,X,0,0,1,X,0,1,0,1
    1,X,H,X,L,X,X,1,0,0,X,0,1,1,0,0,1,1,1
    
Meaning of the symbols is:
- 1 -> Drive a DUT input high (error if used on a DUT output)
- 0 -> Drive a DUT input low (error if used on a DUT output)
- H -> Expect a high on a DUT output (error if used on a DUT input)
- L -> Expect a low on a DUT output (error if used on a DUT input)
- X -> Don't check value if a DUT output, and does not matter what value is driven if a DUT input (tester drives a low)

# *Technical details*

The tester supports up to 19 channels.  Each channel can be configured as an input or output.
Each tester cycle is divided into 4 phases.  All input channels are read at the beginning of phase 0.
All output channels have the option of driving to the specified value on any or all phases.

    * NRZ behavior is accomplished by driving on all 4 phases.  
    * RZ behavior is accomplished by driving on a subset of phases.  For instance, a clock might be driven only on phase 2.  Thus the clock would be would be zero for phase 0 and 1, then depending on the input vector 0 or 1 for phase 2, and zero for phase 3.

### *Example*


Configuration: output, true, true, true, true

    Data value 0: signal  xxx\__________________/xxxx
    Data value 1: signal  xxx/------------------\xxxx

Configuration: output, false, false, true, false

    Data value 0: signal  xxx\__________________/xxxx
    Data value 1: signal  xxx\_________/---\____/xxxx

Configuration: output, false, true, true, false

    Data value 0: signal  xxx\__________________/xxxx
    Data value 1: signal  xxx\______/-------\___/xxxx

## *Vector data*

Vector data is provided as an array of uint32_t.  Bit *N* is assigned to channel *N*.
Data that is read is returned in a array of uint32_t.  Channel *N* is assigned to bit *N*.
Note that the values of output channels will also be read.  NRZ and phase3 channels should return
the output value and other channels will return zero.

###*Example*

qf_tinytester main.c:

    tinytester_channelconfig_t atinytester_channelconfig[] = {
        output, false, false, true, false,  // RZ (clock)
        output, true,  true,  true,  true,  // NRZ
        input,  false, false, false, false, // input
    };
    uint32_t kchannelconfig = sizeof(atinytester_channelconfig)/sizeof(atinytester_channelconfig[0]);

    uint32_t auxOutputvector[] = {
        2, 0, 3, 1, 3
    };
    uint32_t kvectorOutput = sizeof(auxOutputvector)/sizeof(auxOutputvector[0]);

    uint32_t auxInputvector[5];
    uint32_t kvectorInput = sizeof(auxInputvector)/sizeof(auxInputvector[0]);

The example configures 2 output channels and one input channel. The output vector has 5 cycles, and the input vector has room for 5 cycles.
The call to run the example is in main_debug_cli_menu.c.

It is triggered by 'tinytester run'.

The actual call is:

    tinytester_run(kchannelconfig,      // number of channels used (configured by the config structure) 
        &atinytester_channelconfig,     // array of configuration structures, one for each channel used
        kvectorOutput,                  // number of output vectors 
        &auxOutputvector,               // pointer to array of output vectors 
        kvectorInput,                   // number of elements in the input vector array
        &auxInputvector);               // pointer to array where input data will be stored

## Channel Assignments


| Channel | Quickfeather | IO    | QFN   |
| :-----: | :----------  | :---  |  ---: |
| 0       | J8.3         | IO_23 | 33    |
| 1       | J8.5         | IO_31 | 23    |
| 2       | J8.7         | IO_12 | 56    |
| 3       | J8.9         | IO_5  | 64    |
| 4       | J8.11        | IO_7  | 63    |
| 5       | J8.13        | IO_10 | 59    |
| 6       | J8.2         | IO_29 | 26    |
| 7       | J8.4         | IO_24 | 32    |
| 8       | J8.6         | IO_11 | 57    |
| 9       | J8.8         | IO_4  | 3     |
| 10      | J3.10        | IO_30 | 25    |
| 11      | J8.12        | IO_8  | 61    |
| 12      | J3.9         | IO_25 | 31    |
| 13      | J3.8         | IO_13 | 55    |
| 14      | J2.7         | IO_17 | 42    |
| 15      | J2.8         | IO_16 | 40    |
| 16      | J2.10        | IO_3  | 2     |
| 17      | J2.11        | IO_0  | 4     |
| 18      | J2.12        | IO_1  | 5     |