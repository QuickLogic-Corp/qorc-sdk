# QORC SDK

The qf_baremetal app tests qf_baremetalsetup which sets up the power domains and clocks *without* using
the S3X_CLK_XXX management, or the power management schemes.  As a result the code is smaller and simpler,
however all of teh responsibility is on the user to get the right power domains enabled, and the clocks set
correctly.