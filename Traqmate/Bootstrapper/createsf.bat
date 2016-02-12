copy ..\SUPERFLASH\SUPERFLASH "SUPERFLASH DU"
\keil\c51\bin\oh51 "SUPERFLASH DU" HEXfile (sf.hex)
hex2bin /L0x10000 /P0xFF sf.hex sf.bin
createtqc 2 sf

pause
