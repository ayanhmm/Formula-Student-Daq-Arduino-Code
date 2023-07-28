//Order of printing data:

//mcdata: motortemp, controllertemp, erpm, throttle
//bms data: packcurrent, packinstvoltage, stateofcharge, hightemp, lowtemp
//mfr data in L/minute
//mc fault
//bms fault

//--------------------------------------------provides canbus and mcp interface-----------------------------------
#include <mcp_can.h>  
#include <mcp_can_dfs.h>
#include <SPI.h>

//--------------------------------------------provides telemetry-----------------------------------
#include<SoftwareSerial.h>

//--------------------------------------------declare hc12 pins-----------------------------------
SoftwareSerial telemetrySerial(5,3); // Create a software serial object for telemetry transmission
                                     //tx pin = 5 , rx pin = 3 
//SoftwareSerial HC12(5,6);

//--------------------------------------------data encapsulation-----------------------------------
// basically initialize a class for telemetry
struct TelemetryData { // Define a struct to hold the telemetry data
  double mctempmotor = 0;
  double mctempcontroller = 0;
  double mcerpm = 0;
  double mcthrottle = 0;
  double pack_current = 0;
  double pack_inst_voltage = 0;
  double state_of_charge = 0;
  double high_temp = 0;
  double low_temp = 0;
  double mcaccurrent = 0;
  double speed = 0;
  double lv = 0;
};

//--------------------------------------------defining some variables----------------------------------
const int SPI_CS_PIN = 10;
unsigned char len = 8;
unsigned char buf[8] = {0};
long unsigned int rxId; //determine priority order(identity) of canbus data

double mctempmotor = 0;
double mcerpm = 0;
double mcthrottle = 0;
double mctempcontroller = 0;
double speed = 0;
double mcaccurrent = 0 ;
double mcdccurrent = 0;
double mcvoltage = 0;
String mcfault = "Searching";

//BMS Variables
double pack_current = 0;
double pack_inst_voltage = 0;
double state_of_charge = 0;
double high_temp = 0;
double low_temp = 0;
double lv = 0;

//MFR Variables
volatile int sensor_frequency;
unsigned int water_minute;
unsigned char flowmeter = 4;
unsigned long present_time;
unsigned long closedlooptime;


//--------------------------------------------initializing CS pin----------------------------------
MCP_CAN CAN(SPI_CS_PIN);

//--------------------------------------------funtions to convert base of numbers----------------------------------
//defining two funstions that convert decimal to hexadecimal and vice versa
int hexToDec(String hexString) {
  int decimalValue = 0;
  int len = hexString.length();
  for (int i = 0; i < len; i++) {
    char c = hexString.charAt(i);
    int digit;
    if (isDigit(c)) {
      digit = c - '0';
    } else {
      c = toupper(c);
      digit = c - 'A' + 10;
    }
    decimalValue += digit * pow(16, len - i - 1);
  }
  return decimalValue;
}


String decToHex(int decimalValue) {
  String hexString = "";
  while (decimalValue > 0) {
    int remainder = decimalValue % 16;
    char hexDigit;
    if (remainder < 10) {
      hexDigit = '0' + remainder;
    } else {
      hexDigit = 'A' + remainder - 10;
    }
    hexString = String(hexDigit) + hexString;
    decimalValue /= 16;
  }
  return hexString;
}

void decToBinary(int decimalNum, int binaryArray[], int arraySize) {
  // Initialize binary array to all zeros
  for (int i = 0; i < arraySize; i++) {
    binaryArray[i] = 0;
  }

  // Convert decimal number to binary and store in array
  int i = 0;
  while (decimalNum > 0 && i < arraySize) {
    binaryArray[i] = decimalNum % 2;
    decimalNum /= 2;
    i++;
  }
}

//MFR Function
void flow(){
  sensor_frequency++;
}

//--------------------------------------------creating void setup----------------------------------
//void setup contains the code we want to run only once when the arduino is initially started
//while void loop is the code that is played in loop
void setup()
{
  //pinMode(3, OUTPUT);
  Serial.begin(460800);
  telemetrySerial.begin(460800);

//making sure code only proceeds when data is coming from serial  
  while (!Serial);

//can_ok is a function that fails when arduino to mcp connections are not proper
  if (CAN_OK != CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ))
  {
    Serial.println("CAN BUS FAIL!");
    while (1) delay(100);
  }
  else {
    Serial.println("CAN BUS OK!");
    delay(500);
  }
  //Serial.println("11");
  CAN.setMode(MCP_NORMAL);
  //Serial.println("22");
  pinMode(2, INPUT); //declaring input pin (INT) //--------------------------------------------very important----------------------------------
                                                 //be careful here CS pin and INT pin numbers are different for uno and mega
                                                 //check the valid pins on google
  //mfr
  pinMode(flowmeter,INPUT);
  attachInterrupt(0,flow,RISING);
  sei();
  present_time = millis();
  closedlooptime = present_time;
}


//--------------------------------------------creating void setup----------------------------------
//void setup contains the code we want to run only once when the arduino is initially started
//while void loop is the code that is played in loop

void loop()
{ 
  //digitalWrite(3, HIGH);
  // check if data coming
  if (CAN_MSGAVAIL == CAN.checkReceive())
  {
    if (1){   // that is - true
      //read data
      CAN.readMsgBuf(&rxId, &len, buf); // buf is a datatype similar to array to store the incoming data
    }

    // for (int i=0; i<=7; i++){
    //   Serial.println(buf[i]);
    // }


    //--------------------------------------------extracting useful data from the identifier----------------------------------
    
    long unsigned int rxId0 = rxId & 0x1FFFFFFF; //Converting the 32 bit id received via mcp into 29 bit id that the canbus wanted to give us
                                                 //this is important because the coding language (datatypes) works on 32bit while our canbus works on 29bit
                                                 // 0x1FFFFFFF = 00011111 11111111 11111111 11111111
                                                 // rxId = 32 bits
                                                 // when we take binary and we make the variable rxid0 whose left 3 bits are zeros
    //rxid consists of 2 parts - packet id and node id
    //packet id tells data incoming
    //node id tells sensor sending
    long unsigned int Packet_id = (rxId0 >> 8);
    long unsigned int Node_id = (rxId0 & 0xFF);

    //Converting decimal data that was earlier stored into buf datatype into hexadecimal strings
    String hex1 = decToHex(buf[0]);
    String hex2 = decToHex(buf[1]);
    String hex3 = decToHex(buf[2]);
    String hex4 = decToHex(buf[3]);
    String hex5 = decToHex(buf[4]);
    String hex6 = decToHex(buf[5]);
    String hex7 = decToHex(buf[6]);
    String hex8 = decToHex(buf[7]);

    //some variables
    int bms_fault_check = 0;

    //Converting data into useable form
    TelemetryData data;

//--------------------------------------------MC code----------------------------------
// to recieve a specific data from the MC refer to HV-500 CAN Manual 2.3 section 3.2 Description of transmitted signals by the inverter
    if (Packet_id == 2){
      
      //Motor Temperature
      String mctempmotorhex = hex3 + hex4;
      mctempmotor = 0.1*hexToDec(mctempmotorhex);
      
      //Controller Temperature
      String mctempcontrollerhex = hex1 + hex2;
      mctempcontroller = 0.1*hexToDec(mctempcontrollerhex);

      //Faults
      String mcfaulthex = hex5;
      if (mcfaulthex == "0"){
        mcfault = "No faults";
      }
      else if (mcfaulthex == "1"){
        mcfault = "Overvoltage Error";
      }
      else if (mcfaulthex == "2"){
        mcfault = "Undervoltage Error";
      }
      else if (mcfaulthex == "3"){
        mcfault = "DRV error";
      }
      else if (mcfaulthex == "4"){
        mcfault = "ABS. Overtemp.";
      }
      else if (mcfaulthex == "5"){
        mcfault = "Controller Overtemp.";
      }
      else if (mcfaulthex == "6"){
        mcfault = "Motor Overtemp.";
      }
      else if (mcfaulthex == "7"){
        mcfault = "Sensor Wire error";
      }
      else if (mcfaulthex == "8"){
        mcfault = "Sensor General Error";
      }
      else if (mcfaulthex == "9"){
        mcfault = "CAN command error";
      }
      else if (mcfaulthex == "A"){
        mcfault = "Analog input error";
      }
    }

    //ERPM
    if (Packet_id == 0){
      String mcerpmhex = hex1+hex2+hex3+hex4;
      mcerpm = hexToDec(mcerpmhex);
      speed = (mcerpm/10)*2*3.14;

      String volt = hex7 + hex8;
      mcvoltage = hexToDec(volt);
    }

    //Throttle
    if (Packet_id == 4){
      mcthrottle = buf[0];
    }

    if (Packet_id == 1){
      String dc = hex1 + hex2;
      mcdccurrent = hexToDec(dc)/10.0;
    }

//--------------------------------------------BMS code----------------------------------
    if(rxId==2147485360){

      //CURRENT
      String c = hex1 + hex2;
      pack_current=(hexToDec(c))/10;
      
      //VOLTAGE
      String v = hex3+hex4;
      pack_inst_voltage=(hexToDec(v))/10;
      
      //STATE OF CHARGE      
      state_of_charge =buf[4]/2.0;
      
      //TEMPERATURE
      high_temp = buf[5];
      low_temp = buf[6];
    }
    if (rxId==2147485361){
      String l = hex5 + hex6;
      lv=(hexToDec(l))/10.0;
    }

    //--------------------------------------------telemetry in action----------------------------------
    //assigning varibles to structure
    data.mctempmotor = mctempmotor;
    data.mctempcontroller = mctempcontroller;
    data.mcerpm = mcerpm;
    data.mcthrottle = mcthrottle;
    data.pack_current = pack_current;
    data.pack_inst_voltage = pack_inst_voltage;
    data.state_of_charge = state_of_charge;
    data.high_temp = high_temp;
    data.low_temp = low_temp;
    data.speed = speed;
    data.mcaccurrent = mcaccurrent;

    //converting above structure to byte datatype
    byte dataBytes[sizeof(data)];
    memcpy(dataBytes, &data, sizeof(data));
      

    // Transmit the encapsulated telemetry data
    telemetrySerial.write(dataBytes, sizeof(dataBytes));
  
    //telemetrySerial.print(data.pack_current);
    //telemetrySerial.print(",");
    //telemetrySerial.print(data.pack_inst_voltage);
    //telemetrySerial.print(",");
    //telemetrySerial.print(data.state_of_charge);
    //telemetrySerial.print(",");
    //telemetrySerial.print(data.high_temp);
    //telemetrySerial.print(",");
    //telemetrySerial.print(data.low_temp);
  
  
   // Wait for 1 second before sending the next telemetry packet

   //--------------------------------------------sending data to serial monitor----------------------------------
    //Printing MC data
    Serial.print(mctempmotor);
    Serial.print(",");

    Serial.print(mctempcontroller);
    Serial.print(",");
    
    Serial.print(mcerpm);
    Serial.print(",");

    Serial.print(speed);
    Serial.print(",");

    Serial.print(mcthrottle);
    Serial.print(",");

    //Printing BMS Data
    Serial.print(pack_current);
    Serial.print(",");

    Serial.print(pack_inst_voltage);
    Serial.print(",");

    Serial.print(state_of_charge);
    Serial.print(",");

    Serial.print(high_temp);
    Serial.print(",");

    Serial.print(low_temp);
    Serial.print(",");

    Serial.print(lv);
    Serial.print(",");
     
    Serial.print(mcvoltage);
    Serial.print(",");

    Serial.print(mcdccurrent);
    Serial.print(",");
    

    //Printing MFR data
    /*
    present_time = millis();
    if(present_time>=(closedlooptime+500)){
      closedlooptime = present_time;
      water_minute = (sensor_frequency/7.5);
      sensor_frequency = 0;
      Serial.print(water_minute,DEC);
      Serial.print(",");
    }else{
      Serial.print('0');
      Serial.print(",");
    }
    */

    // //Printing mc fault
    Serial.print(mcfault);
    Serial.print(";");
    
    //BMS faults
    if (rxId==2147485361){
      String l = hex5 + hex6;
      lv=(hexToDec(l))/10;
      
      //DTC FLAGS #1 - type of BMS fault
      int a=(buf[0]);
      int flag1[8];
      decToBinary(a, flag1, 8);
    
      if(flag1[7]==1)
      {
        Serial.print("Pack Too Hot Fault                ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag1[6]==1)
      {
        Serial.print("Lowest Cell Voltage Too Low       ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag1[5]==1)
      {
        Serial.print("Highest Cell Voltage Too High     ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag1[4]==1)
      {
        Serial.print("Internal Software Fault           ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag1[3]==1)
      {
        Serial.print("Internal Heatsink Thermistor Fault");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag1[2]==1)
      {
        Serial.print("Internal Hardware Fault           ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag1[1]==1)
      {
        Serial.print("Charger Safety Relay Fault        ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag1[0]==1)
      {
        Serial.print("Discharge Limit Enforcement Fault ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      

      //DTC FLAG #2
      int b=(buf[2]);
      int flag2[8];
      decToBinary(b, flag2, 8);

      
      if(flag2[7]==1)
      {
        Serial.print("Cell ASIC Fault                   ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag2[6]==1)
      {
        Serial.print("Highest Cell Voltage Over 5V Fault");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag2[5]==1)
      {
        Serial.print("Current Sensor Fault              ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag2[4]==1)
      {
        Serial.print("Open Wiring Fault                 ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag2[3]==1)
      {
        Serial.print("Low Cell Voltage Fault            ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag2[2]==1)
      {
        Serial.print("Weak Cell Fault                   ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag2[1]==1)
      {
        Serial.print("Cell Balancing Stuck Off Fault    ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag2[0]==1)
      {
        Serial.print("Internal Communication Fault      ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      
      //DTC FLAG #3
      int c=(buf[3]);
      int flag3[8];
      decToBinary(c, flag3, 8);

      
      if(flag3[7]==1)
      {
        Serial.print("Charge Limit Enforcement Fault    ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag3[6]==1)
      {
        Serial.print("Input Power Supply Fault          ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag3[5]==1)
      {
        Serial.print("High Voltage Isolation Fault      ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag3[4]==1)
      {
        Serial.print("Redundant Power Supply Fault      ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag3[3]==1)
      {
        Serial.print("External Communication Fault      ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag3[2]==1)
      {
        Serial.print("Thermistor Fault                  ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag3[1]==1)
      {
        Serial.print("Fan Monitor Fault                 ");
        Serial.print(":");
        bms_fault_check += 1;
      }
      if(flag3[0]==1)
      {
        Serial.print("Weak Pack Fault                   ");
        Serial.print(":");
        bms_fault_check += 1;
      }
    }

    if (bms_fault_check == 0){
      Serial.print("No Faults");
      Serial.print(":");
    }

    Serial.println();
    
    //giving delay
    delay(0);
  }
}
