
QORC SDK CLOCK/POWER INFRASTRUCTURE : BASIC CONCEPTS
====================================================


|PRE RELEASE|


Introduction
------------

The clock/power control on the EOSS3 involves primarily using the PMU to control
the power domains, and CRU for clock frequencies of the various parts of the SoC.

There are 2 aspects to the clock/power control on the EOSS3:

- Hardware Capability

  - Power Domains available and ways to perform shutdown/sleep/wakeup

  - Clock Tree of the various parts of the SoC and ways to adjust clock frequencies

- Software Implementation

  - Power Domain and Clock Control

  - Low Power Mode (LPM) management

  - Dynamic Frequency Scaling (DFS)

  - Quality Of Service (QOS)

It is worthwhile to note that most of the time, the software implementation components 
often work together and need to be understood as a set.


Hardware Capability
-------------------


A condensed representation of all the clocks and power domains in the EOS S3 architecture is shown below.

.. image:: clock-tree.svg

Power Domains
~~~~~~~~~~~~~

There are a total of 31 power domains available in EOSS3 and can be individually
controlled.

All of the domains can be controlled from the PMU registers.

Clock Tree
~~~~~~~~~~

There are a total of 19 clock domains, and further provide "leaf" clocks until the 
various peripheral blocks in the EOSS3.

All of the clocks can be controlled from the CRU registers.

The clock tree (and power domains) is described in detail in `Clock Tree <./clock-power-clocktree.rst>`__


Software Implementation
------------------------

Power Domain and Clock Control
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The public APIs available for control of clock/power management are in:

| :code:`qorc-sdk/HAL/inc/s3x_clock_hal.h`
| :code:`qorc-sdk/HAL/src/s3x_clock_hal.c`
|

The (internal) APIs available for control of the power domains are in:

| :code:`qorc-sdk/Libraries/Power/inc/s3x_pi.h`
| :code:`qorc-sdk/Libraries/Power/src/s3x_pi.c`
|

The (internal) APIs available for control of clocks are in:

| :code:`qorc-sdk/Libraries/Power/inc/s3x_clock.h`
| :code:`qorc-sdk/Libraries/Power/src/s3x_clock.c`
|

:code:`TODO add specific document for clock/power API`

Low Power Mode (LPM)
~~~~~~~~~~~~~~~~~~~~

In general, lower power mode is used to describe a way to identify when the system has nothing to do 
(idle) - for example, when waiting for an event, or regular intervals between processing data, and 
use this time to turn off power domains, and turn off or scale down clocks to minimize power consumption.

In the context of an RTOS, LPM is used when the CPU is in "IDLE" state, and 
there are no tasks pending to be run anytime "soon" (in terms of timing ticks).

LPM management in QORC SDK is described in detail in `LPM <./clock-power-lpm.rst>`__


Dynamic Frequency Scaling (DFS)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

DFS is used to adjust system frequencies according to changing context, to always try to balance 
performance and power-consumption.

For example, consider an application where I2C and UART are used, and a relatively intensive processing 
needs to be done on the data obtained from I2C at a regular interval.

In the context where the SW is only reading values from the I2C peripheral, and maybe sending some data 
out of the UART, then we can manage with a lower frequency of M4 core, and the I2C and the UART, which 
means that the HSOSC can also be lower.

At regular intervals, when we need to run the intensive processing on the data, we would need the M4 core to 
run at a higher frequency to be able to finish processing within time limits, and this means that the HSOSC 
would also run at a higher frequency, while I2C and UART continue to run at same frequencies as before.

This kind of application, with a simple naive approach, can be implemented by always running clocks at the 
higher end, at the cost of power-consumption increase.

With the DFS system, we can balance the overall power-consumption and performance of the application.

In the DFS system, the frequency scaling is mostly based on how the CPU load varies over time (indicating whether 
the CPU is heavily loaded, and we have to scale up, or lightly loaded, and we can scale down).

DFS in QORC SDK is described in detail in `DFS <./clock-power-dfs.rst>`__


Quality Of Service (QOS)
~~~~~~~~~~~~~~~~~~~~~~~~

With DFS system in place to dynamically change the clock frequencies as needed, sometimes there are cases 
where we need some of the system frequencies to remain high depending on application-specific factors.

This can be taken as "fine-tuning" the DFS, and guarantee that clocks are maintained at a defined 
minimum for certain conditions - maintaining the "quality of service".

For example, consider the same example above, and expanding the "intensive processing" into 2 types: 
"medium" and "heavy" and from profiling we know what frequencies the M4 core must be run in both types 
to be able to finish processing withing time limits.

Using this information, and the condition when the specific type of processing ("medium"/"heavy"), we can 
ask the QOS system to guarantee a minimum operating frequency for both types separately, until the processing 
has finished.

QOS in QORC SDK is described in detail in `QOS <./clock-power-qos.rst>`__





.. |PRE RELEASE| image:: https://img.shields.io/static/v1?label=STATUS&message=PRE-RELEASE&color=yellow&style=for-the-badge
   :target: none

.. |WORK IN PROGRESS| image:: https://img.shields.io/static/v1?label=STATUS&message=WORK-IN-PROGRESS&color=red&style=for-the-badge
   :target: none