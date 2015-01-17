# ZigBeeTempSensor
Wireless Remote Temperature Sensor

Code and Schematics for implementing a Wireless Remote Temperature Sensor

Equipment used
-------------
- 2 ATMEGA AT89C51RC2 8051 based micro-controller with 11.0592MHz crystal
- 1 RHT03/DHT22 sensor
[https://www.sparkfun.com/products/10167]
- 1 16x4 LCD which uses a HD44780U Display Controller
[http://academy.cba.mit.edu/classes/output_devices/44780.pdf]
- 2 Series 1 802.15 Xbee Zigbee Modules
[https://www.sparkfun.com/datasheets/Wireless/Zigbee/XBee-Datasheet.pdf]

Based on the RHT03 temperature sensor, well-timed polling signals from the
8051 microcontroller was expected. The output from the sensor was serially
transmitted to an Xbee module working on Transparent mode. 

Another receiving Xbee module on the main board serially interfaced to 
the 8051 would output the temperature, relative humidity and received signal strength indiation (RSSI) on a memory-mapped 16x4 LCD. 

Bi-direction Logic Level converters [https://www.sparkfun.com/products/12009]
are used to interface the serial ports from the 8051 and the XBEE modules. 

SPLD glue logic code used for memory mapping LCD to the 8051. SPLD can be avoided to interface LCD.

C code written for SDCC compiler
-----------------------
Any updates, comments, changes, suggestions or revisions - please contact
swapnil.paratey@gmail.com
