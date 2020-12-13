.. include:: /common.rst


GCC Arm Embedded Toolchain
==========================


  1. Download tarball according to the system configuration from: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads
    
    Current stable version tested with is :bash:`9-2020-q2-update`
       
  2. Extract the tarball to a preferred path(/BASEPATH/TO/TOOCHAIN/)
    
    :bash:`sudo tar xvjf gcc-arm-none-eabi-your-version.tar.bz2 -C /BASEPATH/TO/TOOCHAIN/`
       
    The usual preferred path is for example :bash:`/usr/share`
       
    :bash:`sudo tar xvjf gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2 -C /usr/share/`
       
  3. Add the /BASEPATH/TO/TOOCHAIN/gcc-arm-none-eabi-your-version/bin/ to PATH (only for current terminal session)
    
    :bash:`export PATH=/BASEPATH/TO/TOOCHAIN/gcc-arm-none-eabi-your-version/bin/:$PATH`
       
    For the preferred path of :bash:`/usr/share` and current tested stable version :bash:`9-2020-q2-update` for example:
       
    :bash:`export PATH=/usr/share/gcc-arm-none-eabi-9-2020-q2-update/bin/:$PATH`
       
  4. If the path settings need to be permanent, it can be added to the :bash:`~/.bashrc` or :bash:`~/.bash_profile.`
    
    One reference for examples and illustrations of setting up :bash:`path` is `here <https://stackabuse.com/how-to-permanently-set-path-in-linux/>`_

