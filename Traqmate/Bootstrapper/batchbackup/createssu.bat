copy ..\SU\SENSORUNIT "SENSORUNIT SU"
\keil\c51\bin\oh51 "SENSORUNIT SU" HEXfile (ssu.hex)
hex2bin -l 0x10000 -p ff -e bin ssu.hex
createtqc 2 ssu

pause
