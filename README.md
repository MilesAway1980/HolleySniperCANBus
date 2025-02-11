# HolleySniperCANBus
Arduino code to use a CAN Bus to read from a Holley Sniper EFI unit

This is a simple Arduino file to read CAN Bus data from a Holley Sniper.
All that is needed (aside from the base Arduino board) is a CAN Bus transceiver.  The one used in this code is an MCP2515.
Optional is a splitter cable for the Sniper EFI so that the handheld can still be connected, and the second cable can run to the CAN Bus chip.

This code is set up for a Mega 2560 R3 board, but the pin numbers for an UNO (and other boards) are listed in the code and can easily be changed.

A diagram of the pinout is also in the code.

      _________________
      |    |__|__|    |
      |     HI LO     |
      |               |
      |               |
      |               |
      |     pins      |
      | 1 2 3 4 5 6 7 |
      -----------------

                          Mega 2560    UNO
    1   Int               2
    2   SCK               52            13
    3   (MO)SI            51            11
    4   (MI)SO            50            12
    5   CS                53            10
    6   GND
    7   VCC
