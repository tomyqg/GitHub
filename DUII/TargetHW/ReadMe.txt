Readme.txt for BSP for AT91SAM9RL64-EK

Supported hardware:
===================
The sample project for ATMEL AT91SAM9RL64 is prepared
to run on an ATMEL AT91SAM9RL64-EK eval board,
but may be used on other target hardware as well.

Configurations
==============
- Debug_iRAM:
  This configuration may be used to debug an application using
  CSpy and J-Link.
  The application runs in iRAM.
  AT91SAM9RL64_iRAM.mac is used to init the PLL.

- Release_SDRAM:
  This configuration uses release libraries.
  It runs with CSpy and J-Link.
  The application runs in SDRAM.
  AT91SAM9RL64_SDRAM.mac is used to initialize the SDRAM.


Using different target hardware may require modifications.
