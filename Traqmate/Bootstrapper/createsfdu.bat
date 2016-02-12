copy ..\DU\DISPLAYUNIT DISPLAYUNIT
\keil\c51\bin\oh51 DISPLAYUNIT HEXfile (du.hex)
hex2bin /L0x10000 /P0xFF du.hex du.bin
createtqc 3 du
concattqc sf.tqc du.tqc dusf.tqc
pause
