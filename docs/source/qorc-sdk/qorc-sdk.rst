.. include:: /common.rst

|QORC-LOGO|

QORC-SDK
========

QuickLogic Open Reconfigurable Computing (QORC) SDK provides components needed to get started on the QuickLogic's EOSS3 device and open source development boards such as Quickfeather.

The QORC SDK is currently based on FreeRTOS.

Easiest way to get started with the QORC-SDK is to build and run example application projects on the QuickFeather Board.

Refer to :doc:`/qorc-setup/qorc-setup` for setting up the development environment and to be able to build and flash a simple application on the QuickFeather Board.

There are various building blocks that can be used to simplify building an application :

- HAL provides an abstraction layer to all of the EOS S3 underlying features and peripherals
- LIBRARIES provides a set of useful libraries to work with the various features of the EOS S3 in a reusable way
- s3-gateware provides a set of FPGA designs that can be used
- FreeRTOS contains the V10.1.1 sources that can be used in the applications

A set of test/example applications are available in

- qf_apps
- qf_vr_apps
- qorc-testapps
- qorc-example-apps



.. todo: Add a brief explanation of the various components of the SDK here.


.. toctree::
   :hidden:
   :maxdepth: 2

   qorc-sdk-hal/qorc-sdk-hal
   qorc-sdk-libraries/qorc-sdk-libraries
   qorc-sdk-s3-gateware/qorc-sdk-s3-gateware
   qorc-sdk-qf-apps/qorc-sdk-qf-apps
   qorc-sdk-qf-vr-apps/qorc-sdk-qf-vr-apps
   qorc-sdk-qorc-test-apps/qorc-sdk-qorc-test-apps
   qorc-example-apps/qorc-example-apps

