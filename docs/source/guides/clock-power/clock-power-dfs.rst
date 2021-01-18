
QORC SDK CLOCK/POWER INFRA : DYNAMIC FREQUENCY SCALING
======================================================

|WORK IN PROGRESS|

.. contents::

Intro
-----

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

- :code:`policySleep` is used by LPM to invoke sleep at a specific DFS policy node.

  ::

    TODO : cover this in more detail as part of the Low Power Management (LPM) implementation.


Example Design using DFS
------------------------

::

  TODO


  

.. |WORK IN PROGRESS| image:: https://img.shields.io/static/v1?label=STATUS&message=WORK-IN-PROGRESS&color=red&style=for-the-badge
   :target: none
