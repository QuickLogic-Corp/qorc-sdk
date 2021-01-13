QORC SDK CLOCK INFRA
====================

|WORK IN PROGRESS|


.. contents::

Intro
-----

The first aspect of the clock infra is that all of the clocks in the EOS S3 system are derived from the HSOSC.

It is also possible to switch the clock source to the 32kHz "REF CLOCK" instead of the HSOSC, but in practice we don't use this approach.

Since all the clocks are derived from the HSOSC, anytime the HSOSC changes, the desired leaf clocks need to be kept at required frequencies.
The DFS implementation in SW controls the changes in the HSOSC w.r.t various parameters (mostly sleep and wakeup, performance requirements).

First, we will need to look at getting a system design in place, where we have defined frequencies at which certain leaf clocks should operate at.
For this set of desired frequencies, we would decide the DIVs and HSOSC value to get these or as close as possible to the desired frequencies, and ensure none of the system clocks are out-of-spec.

The REF CLOCK is always at 32768 Hz, and the HSOSC is derived using a mutliple of this.
All the leaf clocks are derived from the HSOSC using an integer divider in the path.

The major blocks/peripherals (depending on the application) that we usually look at are :

- UART
- SPI1 Master
- I2C0
- I2C1
- SPI0
- FPGA
- I2S
- PDM
- LPSD

UART
----

The UART clock is sourced from C11 with a max spec of 10MHz.

The UART peripheral has a fractional divider, which can divide down to produce standard baud rates (115200, 9600 etc.)

There is not much to worry about the baudrate matching, due to the fractional divider and any C11 frequency would do.

Whenever HSOSC changes, C11 will change, and we need to ensure that the UART fractional divider is set to output the required baudrate.

SPI1 Master
-----------

The SPI1M Clock is derived from C02 clock with a max spec of 40MHz.

The C02 goes through a SPI1M BAUDR divider which can have any *even* divider value (2 - 16384) to produce the SPI1M clock out.

So, the max SPI1M frequency is 20MHz (when C02 is at 40MHz, and BAUDR divider is 2).

Whenever the HSOSC changes, the C02 will change, and correspondingly, the SPI1M BAUDR divider needs to be set to ensure a specific frequency out.


I2C0, I2C1, SPI0
----------------

All of these peripherals are connected via a Wishbone bus, and clocks are derived from C08X1 clock, with a max spec of 10MHz.

I2C0 and I2C1 both have couple of 8-bit prescaler registers, which can divide down from C08X1 to produce common I2C frequencies.

SPI0 has a couple of 9-bit BAUDR registers, which can divide down to produce SPI0 clock out.


FPGA
----

The FPGA has 3 input clocks available to it :

- C16 (Sys_Clk0)
- C21 (Sys_Clk1)
- C02 (Sys_Pclk)

which can be used by the design inside it.

There are 2 clocks inside the FPGA domain, which need to be set if needed for WB/PKT FIFO access:

- C40 (provides clock to the WB interface on the AHB2WB bridge connecting the M4 AHB to the FPGA)
- C41 (provides clock the PKT FIFO interface on the FPGA)


In general, the C16 and C21 clocks can run upto the HSOSC frequency.

For specific designs, there may be limitations on the C16/C21 values - which in turn would mean limitation on the HSOSC.

For example, with the FPGA UART designs, the BAUD rate divider only has integer divider, which means that for obtaining standard baudrates (115200/9600/etc.)
we would need C16/C21 to be a multiple of 1.8432 MHz - hence HSOSC needs to be a multiple of 1.8432 MHz.

This has implications for other system clocks, which being derived from HSOSC, also would be multiples of 1.8432 MHz.

This needs to be taken into account while designing the system.

I2S, PDM, LPSD
--------------

All of these clocks are in the Audio subsystem.

PDM L, PDM R and I2S clocks are derived from C30 (max 5MHz)

LPSD clock is derived from C31 (which is derived from C30), max spec 1Mhz, typically 512kHz.


Calculations
------------

STEP 1
======

Suppose we have a system where we use the UART and an I2C sensor (on I2C0) via the M4.

UART <- C11 <- HSOSC
I2C0 <- C08X1 <- C08X4 <- HSOSC
M4F <- C10 <- HSOSC

UART -> 115200 baud, this can be achieved using the fractional divider, so C11 has no strict constraints.
I2C0 -> 400 kHz, this can be achieved using the prescaler, C08X1, and hence C08X4 has no strict constraints.

The application code will determine the performance required out of the M4F core, and most of them can be achieved with around 48MHz or lower.

It is generally preferred to have the clocks in a multiple of 6MHz as we see it leads to easier way to accomodate common frequency requirements of various peripherals.

So, we can then keep C11, C08X4 at 3 Mhz to be enough for UART/I2C0 output generation.

We can keep C10 at acceptable perf level, at say 12MHz or more.

STEP 2
======

Now, we add a requirement of using the FPGA UART - which has integer divider only, and requires C21 at a multiple of 1.8432 MHz.

At this point, we can go back and look at HSOSC, and see that it needs to be a multiple of 1.8432 MHz too.

If the FPGA UART has a baudrate requirement of 115200, then C21 at 1.8432 MHz can satisfy this.

Here, we know that the HSOSC is derived from a 32768 Hz ref clock.

Considering both of the equations we need to find HSOSC which can satisfy both:

multiple of 32768 and of 1843200, so we find the LCM of both, which is 7372800.

These 2 match the generally used frequencies in other cases:

- 73728000 (7372800 x 10)
- 36864000 (7372800 x 5)

Other than these, we could also use one of the other multiples of 7372800.

Also of note here, is that when we refer in code, to HSOSC or Core frequency of 72MHz, it actually means 73728000 Hz (72 x 1024 x 1000)
Similarly 36MHz is 36864000 Hz (32 x 1024 x 1000).

STEP 3
======

Consider that we need to add SPI1M in the design, which communicates with external device, with a max required frequency of 6 MHz.

Note that, here 6MHz indicates 6000000 Hz (as in general devices are specified with the regular MHz definition).

SPI1M is derived from C02 and needs a minimum div of 2 (and up, even dividers only).

Considering HSOSC = 36864000 Hz, C02 can have a div of 1, and SPI1M BAUD needs to be calculated.

C02 = 36864000 (within spec of 40 MHz, so ok)

SPI1MBAUD = 36864000/6000000 rounded to an even divider

exact divider = 6.144

Now, we can only have even div, so divider = 6

with this, the actual SPI1 freq = 36864000/6 = 6144000 (6.144 MHz) - this may be ok with most devices.

However, we should assume that the absolute max spec is defined, which is 6MHz, hence we would need to
take the next higher even divider which is 8

with this, the actual SPI1 freq = 36864000/8 = 4608000 (4.608 MHz) - this would be within spec of the device.

Note that, we would not be able to take advantage of the top speed available from the device point of view, because
we are constrained by the system design (including the FPGA UART requirement)

STEP 4
======

The same way, lets start with HSOSC = 73728000 Hz

In this case, C02 is constrained to a max of 40 MHz - hence div should be atleast 2.

With DIV 2 at C02, we get the same calculation as above.

With a bit of variation, for example, we can do:

C02 DIV = 7, so C02 = 73728000/7 = 10532571.4286 Hz (approx 10.5 MHz)

With this, we calculate the SPI1M BAUD as above:

exact divider = 10532571.4286/6000000 = 1.7554, rounding off to next even divider we have BAUD = 2

SPI1M BAUD = 2, SPI1 freq = 10532571.4286/2 = 5266285.71428 Hz (5.266 MHz)

We can see that we get a higher SPI1 frequency for the same constraints.

This type of optimization, requires some calculations to be done, to arrive at the best value we can manage.

It would help to have a utility calculator tool that can do this for us, this is *TODO*.


Step 5
======

Considering the audio use case, and assuming use of a PDM mic, and also including LPSD clock, complicates things further.

In general, PDM frequencies below are commonly preferred to be used for audio applications:

Actual frequencies depend on the microphone being used, and the oversampling (or decimation ratio)

- 512 kHz
- 768 kHz
- 1.024 MHz
- 1.536 MHz
- 2.4 MHz
- 3.072 MHz
- 4.8 MHz

PDM clocks are derived from C30 (max 5 MHz)

LPSD clocks are derived from C31 (max 1 MHz) - so 512 kHz/768kHz, preferably 512kHz.

For example, considering usage of 1024000 Hz, then C30 = 1024000 Hz.

HSOSC now needs to be a multiple of this as well, so :

HSOSC  = multiple of (LCM of 1024000, 32768 and 1843200)

which gives us 36864000, which happens to match our choices of 36864000 and 73728000 Hz as before.

With this requirement added, we would not be able to use any other values of HSOSC though!


Let's consider using 3.072 MHz as C30, and calculating LCM (3072000, 32768,1843200) = 36864000 which is the same as above.

So, we can definitely use 3.072 MHz for PDM clock for a better audio bandwidth.


Now, consider using 4.8 MHz as C30, and calculatin LCM (4800000,32768,1843200) = 921600000, 
which indicates that it would not be possible to use 4.8MHz with the rest of the design constraints at all !

For completeness, lets consider the LPSD case, where we want 512kHz C31.

With this, and 3.072MHz as C30, we have HSOSC constraint:

LCM(512000,3072000, 32768,1843200) -> giving us 36864000, same as above, so the LPSD clock is also possible to achieve.


Questions TBD
--------------

- Calculation Strategy and Utility to make it simpler.

- what are the various peripheral frequencies we need to consider (are all of them covered here?)

- w.r.t DFS can we add more clock domains in the DFS nodes (currently only 4) - C01, C09, C10, C08X4

- also, choice of nodes in the DFS would be (should be ?) different according to application.

- How to ensure correct div setting for the DFS policies for, say C30/C31?

- Need a bit of discussion on the clock infra SW implementation in general:
  
  Which are the "API"s that an application developer should be using, and which are meant for internal use ("use only if you understand" kind of functions)

  There are a lot of utility functions available in the clock implementation, and we can *possibly* make the above calculations dynamic.

  However, it would seem like its a better approach to statically design a few scenarios and design in the clock frequencies for optimal perf-power balance.
  

.. |WORK IN PROGRESS| image:: https://img.shields.io/static/v1?label=STATUS&message=WORK-IN-PROGRESS&color=red&style=for-the-badge
   :target: none