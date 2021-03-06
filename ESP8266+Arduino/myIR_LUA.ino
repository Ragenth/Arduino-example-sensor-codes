// Lefteris Stamataros
// Arduino IR Controller
// 13/03/2015
// 27/12/2015 Web Interface added Arduino Code
// 27/12/2015 Web Interface runs from ESP8266 with LUA Script

// EEPROM POSITION
//   0      0       1       2-5           6-16
//---------------------------------------------------------
//| Addr| Button| Type	| Command	| Command Name    |
//| 0	| 9     | NEC	| 0x20DF10EF	| LG ON/OFF	  |
//| 1	| 10    | NEC	| 0x20DF40BF	| VOL UP   	  |
//| 2	| 11    | NEC	| 0x20DFC03F	| VOL DOWN   	  |
//| 3	| 255   | NEC	| 0x807F807F	| DIGIA 1/0	  |
//| 4	| 255   | RC5	| 0x0018C	| PHIL 1/0 	  |
//| 5	| 255   | NECX	| 0xE0E040BF	| SAMS 1/0 	  |
//---------------------------------------------------------
#include <IRLib.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define highByte(x) ( (x) >> (24) & (0xFF) ) //
#define middlehighByte(x) ( (x) >> (16) & (0xFF)) //
#define middlelowByte(x) ( (x) >> (8) & (0xFF)) //
#define lowByte(x) ( (x) & (0xFF) )
#define makeIRCommand(hi, low) (((long) hi) << 16 | (low))

int maxRec=0;
int curRec=0;

enum runThis {
  runCMD=1,
  recCMD,
  maxREC,
  readCMDS,
  deleteCMDS,  
};

const short CMDTableSize=5;
const char string_0[] PROGMEM = "run=";
const char string_1[] PROGMEM = "rec=";
const char string_2[] PROGMEM = "totalRec=";
const char string_3[] PROGMEM = "readCMDS";
const char string_4[] PROGMEM = "delCMDS";

const char* const cmd_table[] PROGMEM = {string_0, string_1, string_2, string_3,string_4};

char tmp_buffer[11];

#define KEY_D_PRESSED 253
#define KEY_R_PRESSED 254
#define KEY_S_PRESSED 255
#define NULL          255
#define LOWER_BUTTON_LIMIT  9
#define HIGHER_BUTTON_LIMIT 12
#define MAX_STORED_COMMANDS 16

#define Receiver_Pin 7
#define DEBUG true

SoftwareSerial esp8266(8,6);

IRrecv IRReceiver(Receiver_Pin);
IRsend IRTransmitter;
IRdecode IRDecoder;
IRTYPES codeType;          // The type of code
unsigned long codeValue;   // The data bits if type is not raw
int codeBits;              // The length of the code in bits

void setup(){
  Serial.begin(9600);
  esp8266.begin(9600);
  setButtonMode(NULL);
  displayHeader();
  displayMenu();  
}

void loop(){
  menuChoice();
}

void setButtonMode(int pinNumber){
  int startingAt=0;
  int endingAt=MAX_STORED_COMMANDS;
  if (pinNumber!=NULL){
    startingAt=pinNumber;
    endingAt=pinNumber+1;
  }
//  if (pinNumber==NULL){
  for (int i=startingAt;i<endingAt;i++){
    int Pin=EEPROM.read(i*16);
    if (Pin>=LOWER_BUTTON_LIMIT && Pin<=HIGHER_BUTTON_LIMIT){
      pinMode(Pin,INPUT);
      digitalWrite(Pin,HIGH);
    }
  }  
//  }
}

void displayMenu(){ 
  Serial.println(F("------------MENU------------"));
  Serial.println(F("| [1] Show Stored Commands |"));
  Serial.println(F("| [2] Add New Command      |")); 
  Serial.println(F("| [3] Record New Command   |"));
  Serial.println(F("| [4] Delete Command       |"));
  Serial.println(F("| [5] Execute Command      |"));
  Serial.println(F("| [6] Assign Button        |"));
  Serial.println(F("| [7] Discard Button       |"));
  Serial.println(F("| [8] restart ESP8266      |"));
  Serial.println(F("----------------------------")); 
}

String seperate_data(String partial_data,int dataPos){
  int seperator=partial_data.indexOf(":");
  if (seperator!=-1){
//    Serial.print(partial_data.substring(1,seperator-1));
//    Serial.print(":");
//    Serial.println(partial_data.substring(seperator+dataPos,partial_data.length()-1));
    return partial_data.substring(seperator+dataPos,partial_data.length()-1);
  }
}

int process_data(int savePosition,String data_string){
  int first_comma=data_string.indexOf(",");
  if (first_comma!=-1){
    int second_comma=data_string.indexOf(",",first_comma+1);
    if (second_comma!=-1){
      Serial.println("Updating IR Command");
      int third_comma=data_string.indexOf(",",second_comma+1);
      String ButtonNumStr=seperate_data(data_string.substring(1,first_comma),2);
      Serial.print("ButtonString=");
      Serial.println(ButtonNumStr);
      char ButtonNum=char(ButtonNumStr.toInt());
      char IRType=seperate_data(data_string.substring(first_comma+1,second_comma),2).toInt();
      String IRCommandString=seperate_data(data_string.substring(second_comma+3,third_comma),4);
      String commandName=seperate_data(data_string.substring(third_comma+1,data_string.length()-1),2);
      char tmp[9];
      IRCommandString.toCharArray(tmp,sizeof(tmp));
      long unsigned int irCode=strtoul(tmp,0,16);
      saveToEEPROM(savePosition*16,ButtonNum,IRType,irCode,commandName);
      return ButtonNumStr.toInt();
    }
  }
}

void SendJSON(String JSON){
  esp8266.println(JSON);
}

void createJSON(){
  String partialJSON;
  SendJSON("[");
  for (int i=0;i<MAX_STORED_COMMANDS;i++){
    if (EEPROM.read(i*16)!=0){
      partialJSON="address:";
      partialJSON+=EEPROM.read(i*16);
      partialJSON+=",IRType:";
      partialJSON+=EEPROM.read(i*16+1);
      partialJSON+=",IRCommand:";
      partialJSON+="0x";
      String tmp="";
      for (int j=2;j<6;j++){
        tmp=String(EEPROM.read(i*16+j),HEX);
        tmp.toUpperCase();
        if (EEPROM.read(i*16+j)<16) {
          partialJSON+="0";
        }
        partialJSON+=tmp;
      };
      partialJSON+=",description:";
      for (int j=6;j<16;j++){
        partialJSON+=char(EEPROM.read(i*16+j));
      }
      partialJSON+=",";
      delay(500);
      SendJSON(partialJSON);
    }
  }
  SendJSON("]");
}

void  esp8266_Restart(){
  delay(1000);
  esp8266.println("node.restart()");
  delay(1000);
}

void getData(){
  char tmp_char;
  String tmpstring="";
  boolean found_seperator=false;
  delay(20);
  do {
    tmp_char=esp8266.read();
    if (tmp_char=='}') {
      found_seperator=true;
      tmpstring+=tmp_char;
    } else if (((tmp_char==',') || (tmp_char=='\r')) && (found_seperator)) {
      found_seperator=false;
      int searchCMD=0;
      short i=0;
      short execCMD=0;
      while (i<CMDTableSize){
        strcpy_P(tmp_buffer, (char*)pgm_read_word(&(cmd_table[i]))); // Necessary casts and dereferencing, just copy.        
        searchCMD=tmpstring.indexOf(tmp_buffer);
        if (searchCMD!=-1){
          execCMD=i+1;  //exo afairesi to 0 giati mou ekeane ExecuteCommand apo mono tou otan den evriske entoli sto case
          curRec=curRec+1;
          break;
        } else {
          if (i<CMDTableSize) {
            i=i+1;
          } else {
            Serial.println("empty table");
            break;
          }
        }
      }
      switch (execCMD){
          case runCMD: executeCommand(tmpstring.substring(searchCMD+4,tmpstring.length()-1).toInt()); break;
          case recCMD: Serial.print("CurRec=");Serial.println(curRec-1);process_data(curRec-1,tmpstring.substring(searchCMD+4,tmpstring.length())); break;
          case deleteCMDS: deleteCommand(0); break;
          case readCMDS: createJSON();break;
          case maxREC: curRec=0;maxRec=tmpstring.substring(searchCMD+9,tmpstring.length()-1).toInt();Serial.print("maxRec=");Serial.println(maxRec);break;
          default: continue; //Serial.println("unknown command");
      };
      tmpstring="";
    } else {
      tmpstring+=tmp_char;
      found_seperator=false;
    }
//  }
  } while (esp8266.available()>0);
}

int readChoice(){
  rx_empty(1);
  rx_empty(0);
  String inputString="";
  char tmp_char;
  boolean fromSerial=false;
  int myChoiceIs=0;
  boolean stay_here=true; 
  while (stay_here) {
    boolean button_status=checkButton();
    if (Serial.available()){
      stay_here=false;
    } else if (button_status){
      stay_here=false;
      return 0;
    } else if (esp8266.available()){
      stay_here=false;
    }
  }
  while (esp8266.available()>0){
    getData();
    if ((maxRec>0) && (curRec>=maxRec)){
      esp8266_Restart();
      for(int i=maxRec;i<MAX_STORED_COMMANDS;i++){
        if (EEPROM.read(i*16)!=0){
          EEPROM.write(i*16,0);
        }
      }
      maxRec=0;
      curRec=0;
    }
    return 0;
  } 
  while (Serial.available()>0){
      delay(10);
      tmp_char=Serial.read();
      if (tmp_char!='\n') {
        inputString+=tmp_char;
        fromSerial=true;
      }    
  }  
  if (fromSerial){
    char tmp_val[5];
    inputString.toCharArray(tmp_val,sizeof(tmp_val));  
    tmp_char=tmp_val[0];
    if (tmp_char=='D' || tmp_char=='d') {
      return KEY_D_PRESSED;
    } else if (tmp_char=='R' || tmp_char=='r'){
      return KEY_R_PRESSED;
    } else if (tmp_char=='S' || tmp_char=='s'){
      return KEY_S_PRESSED;
    }  else return atoi(tmp_val);
  }
}

void menuChoice(){
  int myChoice=readChoice();
  if (myChoice>0){
    Serial.print(F("You Choose:"));
    Serial.println(myChoice);
    switch (myChoice){
      case 0:  return;
      case 1:  readEEPROM(); break;
      case 2:  addCommand(); break;
      case 3:  recordCommand(); break;
      case 4:  deleteCommand(NULL); break;
      case 5:  executeCommand(NULL); break;
      case 6:  assignButton(); break;
      case 7:  discardButton(); break;
      case 8:  esp8266_Restart(); break;
      case 9:  esp8266.print("stealthMode\r\n"); break;
      case 10: Serial.println("ok"); esp8266.print("testing\r\n"); break;
//      default: Serial.println("Unknown command");
    }
    displayMenu();
  }
}

boolean checkButton(){
  for (int Button=LOWER_BUTTON_LIMIT; Button<=HIGHER_BUTTON_LIMIT; Button++){
    if (digitalRead(Button)==0) {
        getIRCodeFromEEPROM(Button);
        return true;
    }
  };
  return false;
}

void getIRCodeFromEEPROM(int pressedButton){
  for (int thisCommand=0;thisCommand<MAX_STORED_COMMANDS;thisCommand++){  
    int storedButton=EEPROM.read(thisCommand*16);
    if (storedButton==pressedButton){
      while (digitalRead(pressedButton)==0);
      executeCommand(thisCommand);      
    }
  }
}


void readEEPROM(){
  Serial.println(F("-----------------------------------------------------------------"));
  Serial.println(F("| Addr\t| Button| Type\t\t| Command\t| Command Name\t|"));
  for (int i=0;i<MAX_STORED_COMMANDS;i++){
    if (EEPROM.read(i*16)!=0){
      Serial.print("| ");
      Serial.print(i,DEC);
      Serial.print('\t');
      Serial.print("| ");
      int pinNumber=EEPROM.read(i*16);
      if (pinNumber==NULL){
        Serial.print("NULL");
      } else Serial.print(pinNumber,DEC);
      Serial.print('\t');
      Serial.print("| ");
      Serial.print(Pnames(EEPROM.read(i*16+1)));
      Serial.print('\t');
      Serial.print('\t');
      Serial.print("| ");
      Serial.print("0x");
      for (int j=2;j<6;j++){
        int tmp_val=EEPROM.read(i*16+j);
        if (tmp_val<16){
          Serial.print("0");
        }
        Serial.print(tmp_val,HEX);
      };
      Serial.print('\t');
      Serial.print("| ");
      for (int j=6;j<16;j++){
        Serial.print(char(EEPROM.read(i*16+j)));
      }
      Serial.println("\t|");
    }
  }
  Serial.println(F("-----------------------------------------------------------------"));
}

void deleteCommand(int deleteALL){
  int myChoice=0;
  if (deleteALL!=NULL) {
    myChoice=KEY_D_PRESSED;
  } else {
    Serial.print(F("Enter Command Address or [D]elete All:"));
    myChoice=readChoice();
    Serial.println(myChoice);    
  }
  if (myChoice==KEY_D_PRESSED){
    for (int i=0;i<MAX_STORED_COMMANDS;i++){
      EEPROM.write(i*16,0);
    }
  } else EEPROM.write(myChoice*16,0);
}

byte getIRType(String IRType){
  IRType.toUpperCase();
  char irTypeByte;
  if (IRType=="NEC"){              return 1;
  } else if (IRType=="SONY"){      return 2;
  } else if (IRType=="RC5"){       return 3;
  } else if (IRType=="RC6"){       return 4;
  } else if (IRType=="PANASONIC"){ return 5;
  } else if (IRType=="JVC"){       return 6;
  } else if (IRType=="NECX"){      return 7;
  }  
}

void saveToEEPROM(int savePosition,char ButtonNum, byte IRType, long unsigned IRCommand, String commandName){
  Serial.print("savePosition=");
  Serial.println(savePosition);
  Serial.print("ButtonNum=");
  if (ButtonNum==0) {
    ButtonNum=NULL;
  }
  Serial.println(ButtonNum);
  EEPROM.write(savePosition,ButtonNum);
  savePosition++;
  EEPROM.write(savePosition,IRType);
  savePosition++;
  EEPROM.write(savePosition,highByte(IRCommand));
  savePosition++;
  EEPROM.write(savePosition,middlehighByte(IRCommand));
  savePosition++;
  EEPROM.write(savePosition,middlelowByte(IRCommand));
  savePosition++;
  EEPROM.write(savePosition,lowByte(IRCommand));
  savePosition++;
  for (int i=0;i<10;i++){
    EEPROM.write(savePosition+i,commandName[i]);
  } 
  setButtonMode(ButtonNum);
}

String readFromSerial(){
  rx_empty(0);
  char tmp_char;  
  String inputString="";
  while (!Serial.available());
  while (Serial.available()>0){
    delay(10);
    tmp_char=Serial.read();
    if (tmp_char!='\n') {
      inputString+=tmp_char;
    }
  }  
  return inputString;
}

void addCommand(){
  Serial.println(F("Enter Input String [Addr,Button,Type,Command,CommandName]"));
  String inputString="";
  inputString=readFromSerial();
  Serial.println(inputString);
  int first_comma=inputString.indexOf(",");
  if (first_comma!=-1){
    int second_comma=inputString.indexOf(",",first_comma+1);
    if (second_comma!=-1){
      int third_comma=inputString.indexOf(",",second_comma+1);
      if (third_comma!=-1){
        int forth_comma=inputString.indexOf(",",third_comma+1);
        if (forth_comma!=-1){
          String Pos=inputString,substring(0,10);
          Serial.println(Pos);
          int savePosition=inputString.substring(0,first_comma).toInt()*16;
          char isOccupied=EEPROM.read(savePosition);
          Serial.print(F("Address Occupied:"));
          int myChoice;
          isOccupied?Serial.println("TRUE"):Serial.println("FREE");  
          while (isOccupied!=0){
            Serial.println(F("[R]eplace, [D]iscard or enter new position"));
            myChoice=readChoice();
            if (myChoice<MAX_STORED_COMMANDS){
              savePosition=myChoice*16;
              isOccupied=EEPROM.read(savePosition);
            } else isOccupied=0;
          }
          char ButtonNum=inputString.substring(first_comma+1,second_comma).toInt();
          char IRType=getIRType(inputString.substring(second_comma+1,third_comma));
          String IRCommandString=inputString.substring(third_comma+1,forth_comma);
          String commandName=inputString.substring(forth_comma+1,inputString.length());  
          char tmp[9];
          IRCommandString.toCharArray(tmp,sizeof(tmp));
          long unsigned int irCode=strtoul(tmp, 0, 16);
          switch (myChoice){
            case KEY_D_PRESSED: break;
            default: saveToEEPROM(savePosition,ButtonNum,IRType,irCode,commandName);
          }
        }
      }
    }
  }
}

void readIRReceiver(){
  IRReceiver.enableIRIn(); // Start the receiver  
  while (!IRReceiver.GetResults(&IRDecoder));
  IRDecoder.decode();
  codeType = IRDecoder.decode_type;
  if (codeType!= UNKNOWN ){
    Serial.print("Received ");
    Serial.print(Pnames(codeType));
    if (IRDecoder.value == REPEAT) {
      // Don't record a NEC repeat value as that's useless.
      Serial.println("repeat; ignoring.");
    } else {
      codeValue = IRDecoder.value;
      codeBits = IRDecoder.bits;
    }
    Serial.print(" Value:0x");
    Serial.println(IRDecoder.value, HEX);   
  }
}

void recordCommand(){
  int myChoice=KEY_R_PRESSED;
  while (myChoice==KEY_R_PRESSED){
    Serial.println(F("Press Key Now"));
    readIRReceiver();
    Serial.println(F("[R]escan, [D]iscard or [S]ave")); 
    myChoice=readChoice();
  }
  if (myChoice==KEY_S_PRESSED){
    char isOccupied=1;
    while ((myChoice>=MAX_STORED_COMMANDS) && (isOccupied!=0)) {
      Serial.print(F("Enter Save Position:"));
      myChoice=readChoice();
      Serial.println(myChoice);
      int savePosition=myChoice*16; 
      isOccupied=EEPROM.read(savePosition);
      Serial.print(F("Address Occupied:"));
      isOccupied?Serial.println("TRUE"):Serial.println("FREE");  
      while (isOccupied!=0){
        Serial.println(F("[R]eplace, [D]iscard or enter new position"));
        myChoice=readChoice();
        switch (myChoice){
          case KEY_R_PRESSED: isOccupied=0; break;
          case KEY_D_PRESSED: return;
          default: savePosition=myChoice*16; isOccupied=EEPROM.read(savePosition);
        }
      }
      Serial.print(F("Enter Button Number:"));
      rx_empty(0);
      int ButtonNum=readChoice();
      Serial.println(ButtonNum);
      Serial.print(F("Enter Command Name:"));
      rx_empty(0);
      String IRCommandName="";
      char tmp_char;  
      IRCommandName=readFromSerial();
      Serial.println(IRCommandName);
      saveToEEPROM(savePosition,ButtonNum,IRDecoder.decode_type,IRDecoder.value,IRCommandName);
    }  
  }
}

void executeCommand(int buttonAddress){
  int cmdAddress;
  if (buttonAddress!=NULL){
    cmdAddress=buttonAddress*16;
  } else {
    Serial.print(F("Enter Command Address:"));
    int myChoice=readChoice();
    Serial.println(myChoice);
    cmdAddress=myChoice*16;
  };
  int cmdExists=EEPROM.read(cmdAddress);
  if (cmdExists!=0){
    String cmdName;
    for (int i=6;i<16;i++){
      cmdName+=char(EEPROM.read(cmdAddress+i));
    }
    Serial.print(F("Executing Command ["));
    Serial.print(cmdName);
    Serial.println(F("]"));
    int IRType=EEPROM.read(cmdAddress+1);
    long unsigned int IRCommand=makeIRCommand(word(EEPROM.read(cmdAddress+2),EEPROM.read(cmdAddress+3)),word(EEPROM.read(cmdAddress+4),EEPROM.read(cmdAddress+5)));
    IRTransmitter.send(IRType,IRCommand,20);
  } else Serial.println(F("No Command Stored"));
}

void assignButton(){
  Serial.print(F("Enter Pin Number:"));
  int pinNumber=readChoice();
  Serial.println(pinNumber);
  Serial.print(F("Enter Save Address:"));
  int addr=readChoice();
  if (addr<=MAX_STORED_COMMANDS){
    Serial.println(addr,DEC);
    EEPROM.write(addr*16,pinNumber);
    setButtonMode(pinNumber);
  }
}

void discardButton(){
  Serial.print(F("Enter Command Address or [D]iscard All:"));
  int cmdAddr=readChoice();
  if (cmdAddr==KEY_D_PRESSED){
    Serial.println(F("Discard All Assigments"));
    for (int i=0;i<MAX_STORED_COMMANDS;i++){
      if (EEPROM.read(i*16)!=0){
        EEPROM.write(i*16,NULL);
      }
    }    
  } else {
    if (cmdAddr<=MAX_STORED_COMMANDS){
      if (EEPROM.read(cmdAddr*16)!=0){
        Serial.println(cmdAddr,DEC);
        EEPROM.write(cmdAddr*16,NULL);      
      } else Serial.println();
    }
  }
}

void rx_empty(char SerialPort){
  if (SerialPort==1){
    while (esp8266.available()>0){
      esp8266.read();
      delay(10);
    }
  } else {
    while (Serial.available()>0){
      Serial.read();
      delay(10);
    }
  }
}

void displayHeader(){
  Serial.println(F("----------------------------"));
  Serial.println(F("|                          |"));  
  Serial.println(F("|  created by...           |"));  
  Serial.println(F("|     Lefteris Stamataros  |"));
  Serial.println(F("|                          |"));  
}
