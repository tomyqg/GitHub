copy ..\SU\SENSORUNIT "SENSORUNIT DU"
\keil\c51\bin\oh51 "SENSORUNIT DU" HEXfile (dsu.hex)
hex2bin -l 0x10000 -p ff -e bin dsu.hex
createtqc 2 dsu

pause
