Telduino
========

Repository for energy metering and telecommunication firmware.

Dependencies
------------

* avr-gcc 
* avr-libc 1.7.1 
* make
* avrdude

The above are available in the WinAVR package if you are on a windows system


Compiling
---------

    make compile

Programming Fuses
-----------------

    make programfuses
Note that this mode allows for on chip debugging and is insecure.

Read Fuses
----------

    make readfuses

Programming
-----------

    make program

Installing on a New Board
-------------------------

    make install
