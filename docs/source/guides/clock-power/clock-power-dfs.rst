
QORC SDK CLOCK/POWER INFRASTRUCTURE : DYNAMIC FREQUENCY SCALING
===============================================================

|PRE RELEASE|

.. contents::

Introduction
------------

It is assumed that the reader is acquainted with the clock infrastructure in the EOSS3.

For any application, if the clock tree is planned out according to the target frequencies needed 
then we have a functional state of the system, for one of (or a set of) HSOSC frequencies.

We have also seen that, the peripheral clocks can be satisfied with multiple sets of HSOSC + DIVIDER combos.

HSOSC has significant power consumption, which only goes up as the frequency generated needs to go higher.

Hence from a power-consumption perspective, it would be best to always aim for the lowest possible HSOSC.

Also, when the system is idle, say waiting for events to happen, we would like the HSOSC to be at the lowest 
possible frequency, so power consumption is minimized.

This is where Dynamic Frequency Scaling (DFS) comes in, so that the frequencies in the system can be changed 
in response to events happening to have a balance of performance, functional behavior and power consumption.


Operation Theory
----------------

DFS operates according to different "policies" (also referred to as "policy nodes" or just " DFS nodes" sometimes).

Each DFS policy node represents a blueprint of the state of various clocks in the system.

Additionally, the policy node would also contain information which can be used to determine when a change in 
the policy node should occur (when to step up to a higher policy node, or step down to a lower policy node)

For example, say we have simple application where we need to respond to a button press, 
do some processing, and go back to waiting.

We can have 2 DFS policy nodes defined so that, while waiting for button press, because all peripherals are 
at sleep/off - the HSOSC is at a minimal frequency (say 3.072 MHz), which we may call as node 0 (node zero).

Once the button press produces and interrupt, the DFS system can scale up the frequencies as needed, 
including HSOSC (to say, 49.152 MHz), and invoke a processing function, which we may call as node 1 (node one).

Once the processing is done, and CPU is idle again, the DFS system can again scale down frequencies, 
so we go back to node 0.

So, for a particular application, we would need to define DFS policy nodes, 
and what are the constraints on frequencies at each node, and how the trigger to change nodes is processed.


Implementation
--------------

In the current DFS implementation in the QORC SDK, we will start by looking at the following code:

::

  #define KPOLICY_CLK_DOMAINS 4

  typedef struct {
      UINT8_t     clk_domain[KPOLICY_CLK_DOMAINS];
      UINT32_t    rate[KPOLICY_CLK_DOMAINS];
      uint32_t    minHSOSC;
      UINT16_t    step_width;
      uint16_t    cpuload_downthreshold;  // Use next lower policy if cpu load is less than this value
      uint16_t    cpuload_upthreshold;    // Use next higher policy if cpu load is greater than this value
      uint8_t     policySleep;            // Set this if the sleep policy is not the current policy
  } S3x_Policy_Node;

A description of the structure and its usage is as below:

- Each DFS node here has a set of KPOLICY_CLK_DOMAINS number of a domain-rate table.
  
  In this case, we can have 4 clock domains, for which we can set the clock output rate for each node.

  For example, we could define that C10, C09, C30, C31 should be at 3072000, 3072000, 1024000, 512000 Hz 
  using the :code:`clk_domain[]` and :code:`rate[]`.

- Each DFS node has a :code:`minHSOSC` that can be defined, if we need to specifically control the HSOSC 
  frequency. 
  
  Most of the time, setting the :code:`clk_domain[]` and :code:`rate[]` is enough, as this in turn 
  will change the HSOSC according to requirements.

  The :code:`minHSOSC` can be used to override this automatic calculation with a user defined one.

- :code:`step_width`, :code:`cpuload_downthreshold` and :code:`cpuload_upthreshold` are used together.

  When the system is at a specific DFS node, these parameters indicate to the DFS system on when it needs to change 
  to a different DFS node (step-up a higher policy or step-down to a lower policy, switch to a sleep policy)

  CPU Load is generally indicated with "since how many milliseconds has the CPU been busy at current policy node?".

  If the cpu load exceeds :code:`cpuload_upthreshold` for :code:`step_width` milliseconds, DFS "steps up" 
  to the next higher policy.

  If the cpu load is lower than :code:`cpuload_downthreshold` for :code:`step_width` milliseconds, DFS "steps down" 
  to the next lower policy.

- :code:`policySleep` is used by LPM to invoke sleep at a specific DFS policy node, and disabled DFS to run 
  during LPM at specific policy nodes.

  A detailed description of this variable w.r.t LPM is in : `LPM <./clock-power-lpm.rst>`__ 
  in the :code:`LPM and DFS` section.

Example Design using DFS
------------------------

We would have multiple sets of frequencies depending on the application.

Each set of frequencies would be represented by a "DFS Policy Node".

In general, for each DFS Policy Node, we would calculate the desired frequencies using the 
approach followed in `Clock Tree <./clock-power-clocktree.rst>`__ in the 
:code:`Example Design - Calculations` section.

There will be 2 "categories" of frequency calculation:

1. Frequencies which remain the same irrespective of the DFS Policy Node
2. Frequencies which vary according to the DFS Policy Node

For example, we would generally want, say, I2C frequencies or SPI frequencies to remain the 
same all the time.

On the other hand, we would generally want the CPU frequency to scale according to the system 
load conditions. In this category, there would also be situations where we would like the system 
frequencies to remain stable at a particular DFS Policy Node.

For the frequencies which would remain the same all the time, irrespective of the DFS Policy Node 
changing (which can potentially change HSOSC) - we would set these up one time, and in the QORC SDK, 
this is done in the :code:`S3x_ClkD S3clk []` array in :code:`s3x_pwrcfg.c`.

For example, in :code:`qf_vr_apps/qf_1micvr_app/src/s3x_pwrcfg.c`, the PDM clock and LPSD clock 
are set to :code:`HSOSC_2MHZ` and :code:`HSOSC_512KHZ` respectively.

The clock framework code would try to keep these frequencies same whenever the HSOSC changes due to 
change in the current DFS Policy Node.

For the frequencies which need to change according to CPU Load, we would set these up in the 
:code:`S3x_Policy_Node dfs_node[]` array in :code:`s3x_pwrcfg.c`.

To take the simplest usage of DFS, we can look at 
:code:`qf_apps/qf_bootloader_uart/src/s3x_pwrcfg.c`.

This has only 2 DFS nodes, and starts with Node 1.
This has :code:`.policySleep = 0` so on entering LPM (CPU sleeps) DFS Policy Node 0 
is applied, which reduces the clocks defined in :code:`.clk_domain = {CLK_C01, CLK_C09, CLK_C10, CLK_C08X4}`.

However, we can see that the :code:`.minHSOSC = F_48MHZ` indicates that the HSOSC is 
not changed. This is because the HSOSC has to be maintained as it is required by the USBSERIAL 
FPGA design to keep running correctly.


Taking a more involved example, we can look at :code:`qf_vr_apps/qf_1micvr_app/src/s3x_pwrcfg.c`

We have "pairs" of DFS Policy Nodes, which working in sync with the FSM.

::
  
  TODO add link to the FSM documentation.

For example, the DFS starts with DFS Node 1, and due to :code:`.policySleep = 0`, on CPU 
going idle, DFS Policy Node is changed to Node 0, and on CPU waking up, we go back to Node 1.

Note that DFS only changes between these 2 nodes at this point.

The FSM defines events, which cause change from Node 1 to say Node 3.
This can be seen in :code:`qf_vr_apps/qf_1micvr_app/fsm/fsm_tables.h` in :code:`struct GSMrow afsmrow[KSTATES]`.
For example, we can see that on "LPSD ON event" DFS Policy Node is changed to Node 3.

From here, due to :code:`policySleep = 2`, on CPU going to sleep, DFS Policy Node changes to Node 2.

Note the :code:`.cpuload_downthreshold = 0,` in Node 2, which prevents DFS from going any 
lower in the DFS Policy Nodes, so in essence we switch between Node 3 and Node 2 until the state 
changes, according to the FSM for this particular application.

The same pattern can be seen for the following DFS Policy Node pairs:

- Node 0, Node 1 and Node 2, Node 3

Taking Node 4, we can see that :code:`.policySleep = 0xFF` which indicates that DFS will be active during 
CPU being idle as well.

With :code:`.step_width =  50` and :code:`.cpuload_downthreshold = 10`, it is indicated that 
if the CPU Load is <10% for more than 50msec, DFS will switch to the next lower DFS Policy Node, 
which would be Node 3 in this case.

Also, if :code:`.cpuload_upthreshold` is not explicitly specified, as in this case, then, 
if the CPU Load >98% for more than 50msec, then DFS will switch to the next higher DFS Policy Node, 
which would be Node 5 in this case.

Default (implicit) value for :code:`.cpuload_upthreshold` is 98 if not defined in the node.

Note that, DFS will only set the clock values specified in the list :code:`.clk_domain = {CLK_C01, CLK_C09, CLK_C10, CLK_C08X4},` 
according to the defined values in that node, for example :code:` .rate = {FREQ_6MHZ, FREQ_3MHZ, FREQ_48MHZ, FREQ_256KHZ},`

All other frequencies in the system are assumed to belong to the "category 1" mentioned above, 
and the clock framework will try and keep those frequencies same, in every DFS Policy Node.

It is possible that there is a slight change in the "category 1" frequency with change in DFS Policy Node.

For example, say SPI frequency is set to produce 5120000 Hz, at HSOSC of 20480000 Hz.

If the DFS node changes such that HSOSC needs to change to, say 4096000 Hz, then it is only 
possible to obtain SPI frequency of 4096000 Hz, even though it was expected to be at 5120000 Hz.

This caveat should be kept in mind while deciding on the frequencies in the DFS Policy Nodes.



.. |PRE RELEASE| image:: https://img.shields.io/static/v1?label=STATUS&message=PRE-RELEASE&color=yellow&style=for-the-badge
   :target: none

.. |WORK IN PROGRESS| image:: https://img.shields.io/static/v1?label=STATUS&message=WORK-IN-PROGRESS&color=red&style=for-the-badge
   :target: none
