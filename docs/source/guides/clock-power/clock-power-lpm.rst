
QORC SDK CLOCK/POWER INFRASTRUCTURE : LOW POWER MODE
====================================================

|PRE RELEASE|


.. contents::


Introduction
------------

Low Power Mode in the QORC SDK is achieved by leveraging the FreeRTOS tickless IDLE power saving technique.

When there are no active tasks in FreeRTOS, only the IDLE task is active, and causes the CPU to enter sleep.

However, "tick interrupts" still are enabled, and at each tick, the CPU has to wake up, and sleep, but doing nothing useful.

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

Tickless Idle is enabled using :code:`FreeRTOSConfig.h` in the application:

::

  #define configUSE_TICKLESS_IDLE         1

The functions defined for :code:`portSUPPRESS_TICKS_AND_SLEEP`, :code:`configPRE_SLEEP_PROCESSING` 
and :code:`configPOST_SLEEP_PROCESSING` are defined in :code:`qorc-sdk/FreeRTOS/portable/GCC/ARM_CM4F_quicklogic_s3XX/portmacro.h`:

::

  #define portSUPPRESS_TICKS_AND_SLEEP( xExpectedIdleTime ) S3x_Lpm_Handler( xExpectedIdleTime )
  #define configPRE_SLEEP_PROCESSING( IdleTime ) S3x_Pre_Lpm( IdleTime )
  #define configPOST_SLEEP_PROCESSING( IdleTime )S3x_Post_Lpm( IdleTime )

All 3 functions are implemented in :code:`qorc-sdk/Libraries/Power/src/s3x_lpm.c`

When the CPU goes idle (WFI), and there is enough time before the next task is scheduled 
to run,  :code:`S3x_Pre_Lpm` and :code:`S3x_Lpm_Handler` is called, in which the processing 
stops at the WFI instruction.

When the CPU is awoken (either by pending task, or an interrupt) - execution continues after 
the WFI instruction in :code:`S3x_Lpm_Handler`, after which :code:`S3x_Post_Lpm` is invoked.

As a companion to the :code:`S3x_Pre_Lpm` and :code:`S3x_Post_Lpm` functions, we have the 
:code:`S3x_Register_Lpm_Cb` function, which is meant to be used from various peripheral drivers 
to register themselves with a callback function, which can turn off the clocks at :code:`S3x_Pre_Lpm` 
and turn clocks back on, and do a re-initialization at :code:`S3x_Post_Lpm`.

An example of this is present in :code:`qorc-sdk/HAL/src/eoss3_hal_uart.c`.

In the :code:`uart_lpm_callback` function, we can see that the UART is disabled, and clocks are 
turned off while entering LPM, and clocks are enabled and UART is initialized while exiting LPM.



LPM and DFS
-----------

::
  
  NOTE: It is assumed that the reader is familiar with the DFS theory of operation at this point.

LPM and DFS are related with the :code:`policySleep` variable in the policy node defined by :code:`S3x_Policy_Node`.

If this variable is not set to a valid policy node number, and instead set to the sentinel :code:`0xFF`, 
then the sleep is executed without any change to the DFS policy node, and DFS continues to be 
active (the DFS timer) and frequency scaling can happen when the DFS timeout (step-width) is reached.

It is can be seen that, with :code:`policySleep = 0xFF` if CPU is in sleep, and DFS is active, then at the time the DFS Timer is 
invoked, it would lead to DFS causing the policy to be scaled down, and eventually settle at 
policy node 0.

If the :code:`policySleep` is set to a valid policy node number, in that case, while entering LPM (CPU 
goes idle) DFS switches to the specified policy node and is then disabled, so the system will always stay at 
the specified policy node, until the CPU wakes up (at that policy node) and then DFS is "restored" to 
the policy node that was active before entering LPM.

The rationale for this would be that with certain applications, we would want to prevent frequency scaling 
for some time, right after an event occurred, and we expect the system to wake up to another event soon, 
for which we would like to maintain a slightly higher frequency so the system can respond faster, or 
prevent issues arising from change of HSOSC in this period.

It could also be because we do not want the frequency being scaled, as even though the CPU is idle, other 
peripherals are active, and changing the HSOSC would lead to problems in the peripheral, as with LPSD in this example.

An example, (taken from the :code:`qf_vr_apps/qf_1micvr_app/src/s3x_pwrcfg.c`) can be described as below:

DFS policy node 3 is reached when LPSD has been triggered, at which HSOSC is set at 12000 KiHz, and 
during this time, CPU goes idle (say, because LPSD data has not yet been dispatched to CPU for processing).

If :code:`policySleep` is not defined (:code:`0xFF` instead) then DFS can cause policy node to drop 
lower, say to policy node 1, at which point HSOSC would change lower, and hence cause change in the LPSD 
clock frequency during the change. 

This might lead to corruption in the audio data coming from the mic.

To prevent this, the :code:`policySleep` is defined to policy node 2, where the HSOSC is 
maintained at this policy as long as CPU is asleep, disabling DFS, so that the HSOSC frequency 
does not change unexpectedly.

In short, the :code:`policySleep` helps to set up a behavior similar to a state-machine during 
LPM to avoid DFS which might cause issues in known situations like these.

References
----------

- https://www.freertos.org/low-power-tickless-rtos.html

- https://www.freertos.org/low-power-ARM-cortex-rtos.html


.. |PRE RELEASE| image:: https://img.shields.io/static/v1?label=STATUS&message=PRE-RELEASE&color=yellow&style=for-the-badge
   :target: none

.. |WORK IN PROGRESS| image:: https://img.shields.io/static/v1?label=STATUS&message=WORK-IN-PROGRESS&color=red&style=for-the-badge
   :target: none