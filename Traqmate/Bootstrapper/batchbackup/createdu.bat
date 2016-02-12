copy ..\DU\DISPLAYUNIT DISPLAYUNIT
\keil\c51\bin\oh51 DISPLAYUNIT HEXfile (du.hex)
hex2bin -l 0x10000 -p ff -e bin du.hex
createtqc 3 du

pause
