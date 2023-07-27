#Arduino Code
 Arduino code to gather and understand data from different sensors of the FSStudent car obtained through via canbus 


## Sensors Used
We use the following sensors from which the data needs to be extracted:
1. Motor Controller: DTI HV 500 LC (https://drivetraininnovation.com/webshop/inverters/dti-hv-500-liquid-cooled)
<img src="./readme images/mc.jpg" alt="mc" width="300"/>


2. Battery Management System(BMS): BMS Orion 2 (https://www.orionbms.com/products/orion-bms-standard)
<img src="./readme images/bms.jpg" alt="bms" width="300"/>


3. Mass Flow rate Sensor (MFR)
<img src="./readme images/mfr.jpg" alt="mfr" width="300"/>


## Data Extracted
We fetch the following data:
1. From Motor Controller: Motor Temperature, Motor Controller Temperature, ERPM, Throttle
2. From BMS: Accumulator current, Accumulator Voltage, State of charge, Highest temperature of a cell in the battery, lowest temperature of a cell in the battery
3. From MFR: Speed of the fluid in the cooling system tubes (unit: L/minute)


## Connections
1. Connect the CAN2.0 High and CAN2.0 Low wires of Motor Controller and BMS to the CANBUS wiring. Make sure these nodes are connected in parallel.
2. Also connect the MCP2515 module in parallel to the nodes.
<img src="./readme images/canbus.jpeg" alt="canbus" width="350"/>
3. Now connect the MCP2515 with the Arduino Mega in the following way:
<img src="./readme images/megatomcp.jpg" alt="megatomcp" width="350"/>
4. Also connect the MFR sensor to the arduino. The 'data' wire should go in pin 4 of arduino. And the V+ve and V-ve wires should go in 5V and GND pins respectively.
5. Connect the HC12 module such that it's RX and TX pins are connected to pin 10 and pin 11 of the arduino respectively. Also it's Vcc and GND pins should be connected to the 5V and GND pins of arduino

6. Power the arduino using Type A to Type B connector from the raspberry pi
7. Raspberry pi should get it's power from the general GPIO pins which are connected to the LV battery via a buck converter. The buck converter's potentiometer should be adjusted such that it converts the 12V input into 5V output

8. Raspberry pi should further be connected to the monitor present in the dashboard


