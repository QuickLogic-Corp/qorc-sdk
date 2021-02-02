
QORC SDK CLOCK/POWER INFRASTRUCTURE : QUALITY OF SERVICE
========================================================

|PRE RELEASE|


.. contents::

Introduction
------------

Quality Of Service (QOS) is used in conjunction with Dynamic Frequency Scaling 
(DFS) to set constraints on the change in the HSOSC Frequency/CPU Frequency/
Domain Clock Frequency.

With DFS enabled, the clocks can change at any time the the CPU load changes, 
and comes back up. Also, it takes some time for the scaling up and down to complete, 
as HSOSC scaling takes time, and then all the downstream clocks need to get 
synchronized.

In certain situations, we would like to prevent a change in the clocking, 
or guarantee a minimum clock frequency, while a particular "task" is running 
until it completes. To satisfy such needs, we can use the 
Quality Of Service (QOS) API.

Using the QOS APIs, we can set the operating conditions on the HSOSC, 
or the Cortex-M4 core clock (CPU) or any of the other clocks in the system.


::

  NOTE : The SW snippets below will mention enums such as HSOSC_72MHz - this is actually 72000 KiHz, 
  as opposed to 72000 kHz and must be interpreted as such.
  
  HSOSC_XMHz = X * 1000 KiHz or X * 1000 * 1024 Hz 
  
  A more detailed description is present in the `Clock Tree <./clock-power-clocktree.rst>`__ page.


Use Cases of the QOS API
-------------------------

Ensure A Minimum HSOSC Frequency
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To illustrate usage of this, we will take the example of one common usage scenario using the UART.

UART - we can have a "buffered uart task" - which will take in all the logs and actually write into 
the UART peripheral when other tasks are suspended.
This does 2 things - save power, as UART will not work all the time, and prevent 
nasty timing issues (if I add a printf, it works... until it doesn't)

While this task runs, we would like to keep the UART frequency from changing.
If the UART peripheral frequency changes, then the baud-rate will not be as expected, 
leading to garbled output on the UART peripheral.

So, this requires us to inform the DFS system to hold the HSOSC frequency for the UART Peripheral clock.

We "register" a QOS as below:

::

    if(S3x_Register_Qos_Node(S3X_M4_PRPHRL_CLK) != STATUS_OK)
    {
        // handle error in register QOS node.
    }


Next, when the task is about to take all the buffered logs and output it via the UART 
peripheral, we make a QOS "request"

::
  
  if(S3x_Set_Qos_Req(S3X_M4_PRPHRL_CLK, MIN_HSOSC_FREQ, HSOSC_72MHZ) != STATUS_OK)
  {
      // handle error in QOS request.
  }

This indicates to the DFS system to set a QOS level for the HSOSC frequency, and that it should 
maintain the HSOSC minimum at 72000 KiHz until released.


Once all of the buffer is flushed out, we can "clear" the QOS "request"

::

  if(S3x_Clear_Qos_Req(S3X_M4_PRPHRL_CLK, MIN_HSOSC_FREQ) != STATUS_OK)
  {
      // handle error in clear QOS request.
  }
  
This indicates to the DFS system to clear the request for setting the QOS level to maintain the 
minimum HSOSC frequency, and it can do as programmed in the DFS configuration.


NOTE 1
^^^^^^
The maximum frequency of the HSOSC in the DFS system is also 72000 KiHz in this case.

This effectively means that the HSOSC frequency will be set at 72000 KiHz, and will 
not change during the time the QOS request is active.

So, this will maintain the UART peripheral clock at the same rate as programmed.


NOTE 2
^^^^^^

The minimum HSOSC frequency can also be set to something other than 72000 KiHz.

This would mean that the HSOSC can still change, but not below a certain frequency.

For the above example of UART we have only 1 condition : HSOSC frequency is set and not 
changed during UART output.

However, another part of the system (other tasks) may cause the HSOSC frequency to rise up 
(upto 72000 KiHz) at this time as well.

If that happens, then again we would see garbled UART output at this time.

To prevent that, we set the HSOSC min to 72000 KiHz.

This is very much dependent on the DFS system configuration, and profiling of the system 
behaviour is required.

For example, if we know that there is no way the HSOSC will go above, 
say 36 MHz, we can make a QOS request for that frequency as the minimum.

The power consumption of the HSOSC increases with the frequency, so the power consumption at 
72000 KiHz will be approximately double that at 36000 KiHz.

Hence, this decision should be taken according to the system configuration and profile data.


Ensure A Minimum Clock Domain Frequency
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For certain conditions, we would need a clock domain to be above a certain frequency for 
proper operation.

One example is where we need to ensure that the AHB frequency is above 1/2 of the HOST SPI 
frequency when the EOS S3 is in HOST mode.

We first "register" a QOS node

::

  if(S3x_Register_Qos_Node(S3X_M4_S0_S3_CLK) != STATUS_OK)
  {
      // handle error in register QOS node.
  }

When we know that this clock domain needs to be maintained at a certain minimum frequency:

::

  if(S3x_Set_Qos_Req(S3X_M4_S0_S3_CLK, MIN_OP_FREQ, HSOSC_24MHZ) != STATUS_OK) // C10
  {
      // handle error in QOS request.
  }

Once we no longer need to maintain at a specific min frequency:

::

  if(S3x_Clear_Qos_Req(S3X_M4_S0_S3_CLK, MIN_OP_FREQ) != STATUS_OK)
  {
      // handle error in clear QOS request.
  }

**NOTE**

If there are multiple dividers in the path to this clock, we would need to take a 
:code:`MIN_OP_FREQ` QOS on the upstream/downstream clocks too.

The DFS cannot automatically maintain the QOS for all clocks in a clock chain.


Ensure A Minimum Cortex-M4 Core Clock (CPU) Frequency
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This type of QOS would be needed when we need to maintain a minimum performance level 
from the Cortex-M4 core.

For example, while doing heavier Floating Point operations, we may need the CPU to be 
running at a particular frequency for the required MIPS.

Note that this will depend on the MIPS calculation, profiling the system at different 
CPU frequencies (along with other tasks in the system) to get at the minimum frequency 
needed to meet the MIPS requirement.

We "register" a QOS node:

::

  if(S3x_Register_Qos_Node(S3X_FFE_X4_CLK) != STATUS_OK)
  {
      // handle error in register QOS node.
  }

Set a "request" for QOS with minimum CPU frequency:

::

  if(S3x_Set_Qos_Req(S3X_FFE_X4_CLK, MIN_CPU_FREQ, HSOSC_48MHZ) != STATUS_OK)
  {
      // handle error in register QOS node.
  }

"clear" the QOS "request" when no longer needed:

::

  if(S3x_Clear_Qos_Req(S3X_FFE_X4_CLK, MIN_CPU_FREQ) != STATUS_OK)
  {
      // handle error in clear QOS request.
  }


Notes on the QOS API
~~~~~~~~~~~~~~~~~~~~

1. Only one QOS can be taken for one clock node, so for ensuring different conditions 
   (say HOSOSC min frequency as well as CPU min frequency) then the QOS on the 
   conditions should be taken on different clocks.

   For the HSOSC and CPU QOS types it does not matter on which clock node the QOS 
   request is associated with.

2. For I2C use case:

   I2C frequency is derived by setting the :code:`prescaler` value, which depends on 
   its source clock : :code:`C08_X1` clock.
   
   Once it is set for say, 400kHz, if the C08_X1 frequency is scaled up, 
   it is possible that the I2C frequency goes >400KHz according to calculation 
   which would be out of spec for most I2C peripherals.

   C08_X1 scaling down should not generally be a problem, just that the I2C 
   transactions will take more time due to lower frequency - this needs to be accounted for.

   This condition needs to be taken care of while setting the system configuration and DFS configuration.


.. |PRE RELEASE| image:: https://img.shields.io/static/v1?label=STATUS&message=PRE-RELEASE&color=yellow&style=for-the-badge
   :target: none

.. |WORK IN PROGRESS| image:: https://img.shields.io/static/v1?label=STATUS&message=WORK-IN-PROGRESS&color=red&style=for-the-badge
   :target: none
