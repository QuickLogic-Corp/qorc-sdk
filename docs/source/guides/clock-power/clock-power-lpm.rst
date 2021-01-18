
QORC SDK CLOCK/POWER INFRA : LOW POWER MODE
===========================================

|WORK IN PROGRESS|


.. contents::


Intro
-----

Low Power Mode in the QORC SDK is achieved by leveraging the FreeRTOS tickless IDLE power saving technique.

When there are no active tasks in FreeRTOS, only the IDLE task is active, and causes the CPU to enter sleep.

However, "tick interrupts" still are enabled, and at each tick, the CPU has wakeup, and sleep, doing nothing.

So, when the CPU goes IDLE, and if no tasks are scheduled are run "soon", then we can disable 
the "tick interrupt" preventing unnecessary CPU wake up when there is nothing to do.

Instead, we can calculate the next "expected tick" by when any task would be ready to run, if any.

If none, then the wake up happens from any configured interrupt in the system.

Before entering the "tickless IDLE" mode, we can define "PRE_SLEEP" and "POST_SLEEP" processing, which can 
be invoked just before the CPU goes to sleep, and just after the CPU wakes up.

We can include further clock configurations here, for example, we can turn off peripheral clocks in 
"PRE_SLEEP" and turn them back on in "POST_SLEEP".


Implementation
--------------

::
  
  TODO

LPM and DFS
-----------

::
  
  TODO


References
----------

- https://www.freertos.org/low-power-tickless-rtos.html

- https://www.freertos.org/low-power-ARM-cortex-rtos.html