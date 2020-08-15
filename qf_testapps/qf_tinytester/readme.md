Tinytester
===========
A program that turns Quickfeather into a functional tester.

The tester supports up to 19 channels.  Each channel can be configured as an input or output.
Each tester cycle is divided into 4 phases.  All input channels are read at the end of phase 4.
All output channels have the option of driving to the specified value on any or all phases.

    * NRZ behavior is accomplished by driving on all 4 phases.  
    * RZ behavior is accomplished by driving on a subset of phases.  For instance, a clock might be driven only on phase 2.  Thus the clock would be would be zero for phase 0 and 1, then depending on the input vector 0 or 1 for phase 2, and zero for phase 3.

*Example*
---------

Configuration: output, true, true, true, true

    Data value 0: signal  xxx\__________________/xxxx
    Data value 1: signal  xxx/------------------\xxxx

Configuration: output, false, false, true, false

    Data value 0: signal  xxx\__________________/xxxx
    Data value 1: signal  xxx\_________/---\____/xxxx

Configuration: output, false, true, true, false

    Data value 0: signal  xxx\__________________/xxxx
    Data value 1: signal  xxx\______/-------\___/xxxx

*Vector data*
-------------
Vector data is provided as an array of uint32_t.  Bit *N* is assigned to channel *N*.
Data that is read is returned in a array of uint32_t.  Channel *N* is assigned to bit *N*.
Note that the vaslues of output channels will also be read.  NRZ and phase3 channels should return
the output value and other channels will return zero.

*Example*
---------
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

Channel Assignments
-------------------

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
| 10      | J8.10        | IO_6  | 62    |
| 11      | J8.12        | IO_8  | 61    |
| 12      | J2.5         | IO_20 | 37    |
| 13      | J2.6         | IO_19 | 36    |
| 14      | J2.7         | IO_17 | 42    |
| 15      | J2.8         | IO_16 | 40    |
| 16      | J2.10        | IO_3  | 2     |
| 17      | J2.11        | IO_0  | 4     |
| 18      | J2.12        | IO_1  | 5     |