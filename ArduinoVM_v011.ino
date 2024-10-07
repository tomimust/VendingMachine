/*History
05102024 optimisations, removing onesteponly function
01102024 new variable hatchOpenDelay with 500ms value added to openHatch functions and 
sensorReadDelay from 5 to 10ms
removed revolverLock functions and variables
29092024 fix for StartFromZerodispenseBottleType2 and StartFromZerodispenseBottleType1 functions
28292024 new function added for slot count start from ZeroSlot
20092024 1ms switch polling timer to 5ms. Added +1 to slotcount
31082024 added sensorReadDelay variable - decreased delay from 100ms to 5ms
21082024 SWversion inquiry added
11062024 name changed to arduinoVM_v010
11062024 revolver stop mechanism - started
11062024 1 step count when inventario starts -added
11062024 last bottle decetion : added Zero slot detection with delay to findBottleType1 and findBottleType2 functions
13032024 input pin mode changed from INPUT to INPUT_PULLUP. Changed input bits from HIGH to LOW -active
07032024 return chnaged from drum-method to "return to the revolver"
09022024 barcodeScan control added
08022024 return bottle functions added
28012024 hatch input bit flipped from HIGH to LOW-active. 
Loop delay changed from 90ms to 60ms
OpenHatch delay changed from 1000ms to 500ms
1000ms Delay after Motor stopped moved if-statement in findBottleType2-function
13012024 dispense variable set to false when findBottleType1 or findBottleType2 is called
30122023 door alarm sending added
06122023 Z added for motor stopped, no bottle found -case
25112023 

Open topics
- add timeout in case slotcounter fails

*/
int swversion = 010;
//Control output bits
const int motor = 3;     
const int OpenHatch1 = 4;
const int OpenHatch2 = 5;
//const int revolverLock = 6;

//input bits
int FullBottleHatchOpen = 7; 
int BottleType1Sensor = 8;  
int BottleType2Sensor = 9;
int slotSensor = 10;
int DoorSensor = 11;
int zeroSlot = 12;

//variables
String inString = "";  // string to hold input from serial

boolean FullBottle1Empty = false;
boolean FullBottle2Empty = false;
boolean MachineFail = false;
boolean dispense1 = false;
boolean dispense2 = false;
boolean slotSensorHasBeenClosed = false;

int ctrlInput = 66;          //variable for Serial input
int slotCounter = 0;
int slotId = 0;

int sensorReadDelay = 10; //delay for sensor reading delay
int hatchOpenDelay = 1000; //delay between motor stop vs open hatch
int counter1 = 20100;
int counter2 = 20005;

char StatusReport;    //variable for Status reporting
char MotorRunning;    //X (88dec) = motor running, Y (89dec) = motor stopped because bottle found, Z (90dec) = motor stopped due timeout ie bottle not found

//const int debounceDelay = 500;

void setup() {  
Serial.begin(115200);       //Start the serial monitor at 115200 baud  

//Set pin modes
pinMode(motor,OUTPUT);
pinMode(OpenHatch1,OUTPUT);
pinMode(OpenHatch2,OUTPUT);
pinMode(FullBottleHatchOpen,INPUT_PULLUP); 
pinMode(BottleType1Sensor, INPUT_PULLUP);
pinMode(BottleType2Sensor, INPUT_PULLUP);
pinMode(DoorSensor, INPUT_PULLUP);
pinMode(slotSensor, INPUT_PULLUP);
pinMode(zeroSlot, INPUT_PULLUP);
pinMode(LED_BUILTIN, OUTPUT);

//set motor pin and hatch control pins to LOW in the startup
digitalWrite(motor, LOW);  
digitalWrite(OpenHatch1, LOW);
digitalWrite(OpenHatch2, LOW);

// Attach an interrupt to the input pin for detecting changes
// attachInterrupt(digitalPinToInterrupt(slotSensor), countSlots, FALLING);
}

void loop() {
 ReceiveSerialData();
 delay (50);
 DoorAlarmCheck();
}

void  ReceiveSerialData(){
  if (Serial.available() > 0) {
    int inChar = Serial.read();
//      Serial.print("inChar:");
//      Serial.println(inChar);
    if (isDigit(inChar)) {
      // convert the incoming byte to a char & add it to the string:
//      Serial.print("isDigit:");
//      Serial.println(isDigit(inChar));    
     inString += (char)inChar;
    }
    // if newline received, put string to ctrlInput, then clear variables:
    if (inChar == '\n') {
//      Serial.print("Value:");
//      Serial.println(inString.toInt());
//      Serial.print("String: ");
//      Serial.println(inString);
      // clear the string for new input:
      ctrlInput = (inString.toInt());
      inChar = 0;
      inString  = "";
      ctrlInputCheck();
   }
 }
}

void ctrlInputCheck() {
 if(ctrlInput == 99){
     dispense1 = true;
     slotSensorHasBeenClosed = false;
     if (digitalRead(zeroSlot) == LOW){
      StartFromZerodispenseBottleType1();
      } else {
     dispenseBottleType1();}}
 else if(ctrlInput == 98){
     dispense2 = true;
     slotSensorHasBeenClosed = false;
     if (digitalRead(zeroSlot) == LOW){
     StartFromZerodispenseBottleType2();
     } else {
     dispenseBottleType2();}}
 else if(ctrlInput == 97){
     step1slot();
     dispense1 = false;
     findBottleType1();}
 else if(ctrlInput == 96){
     step1slot();
     dispense2 = false;
     findBottleType2();}
 else if(ctrlInput >= 0 && ctrlInput <= 20){ //slotit 0-19 käytössä, 20 value abnormal
     if (digitalRead(zeroSlot) == LOW){
//      slotId = ctrlInput + 1;
     slotId = ctrlInput;
      } else {
     slotId = ctrlInput;}
     selector();}
 else if(ctrlInput == 95){
     CheckAlarmStatus();}
 else if(ctrlInput == 94){
     BottleType1SensorStatusRequest(); } 
 else if(ctrlInput == 93){
     BottleType2SensorStatusRequest(); } 
 else if(ctrlInput == 92){
     DoorStatus(); }
 else if(ctrlInput == 91){
     ZeroSlot(); }
 else if(ctrlInput == 90){
     openHatch1(); }
 else if(ctrlInput == 89){
     openHatch2(); }
 else if(ctrlInput == 88){
     CheckBottle1InfoStatus(); } 
 else if(ctrlInput == 87){
     CheckBottle2InfoStatus(); }  
 else if(ctrlInput == 86){
     isHatchOpen(); }
 else if(ctrlInput == 85){
     clearAlarms(); }    
 else if(ctrlInput == 84){
     swVersion(); }
  ctrlInput = 66;
}

void swVersion(){
 Serial.write(swversion); 
}

void clearAlarms(){
 FullBottle1Empty = false;
 FullBottle2Empty = false;
 MachineFail = false;
}

void selector(){
  if (slotId == 0){
  ZeroSlot(); 
   } else if (slotId > 20){
  FullBottle1Empty = true; 
  } else {
   SlotSearch(); }
}

void CheckBottle1InfoStatus(){
   if (digitalRead(BottleType1Sensor) == LOW){
     StatusReport = 'C';}
     else {StatusReport = '1';}
     Serial.write(StatusReport); 
}

void CheckBottle2InfoStatus(){
   if (digitalRead(BottleType2Sensor) == LOW){
     StatusReport = 'C';}
     else {StatusReport = '1';}
     Serial.write(StatusReport); 
}

void ZeroSlot() {
    if (digitalRead(zeroSlot) == LOW) {
      digitalWrite(motor, LOW);
      MotorRunning = 'Y';
      Serial.write(MotorRunning);
      MachineFail = false;
    } else {
        findZeroSlot();
    }
}

void findZeroSlot(){
  digitalWrite(motor, HIGH);
  delay(200); // to make sure revolver has passed zero slot. maybe zero slot step needed instead of hardcoded delay...?
    int counter = 0;
    while (counter < counter1) {
        if (digitalRead(zeroSlot) == LOW){
            digitalWrite(motor, LOW);
            MotorRunning = 'Y';
            Serial.write(MotorRunning);
            digitalWrite(LED_BUILTIN, HIGH);
            MachineFail = false;
            break;  // Exit the loop when zero slot is found
        } else {
            delay(sensorReadDelay);
            MotorRunning = 'X';
            Serial.write(MotorRunning);
            if (counter > counter2) {
                digitalWrite(motor, LOW);
                digitalWrite(LED_BUILTIN, LOW);
                MotorRunning = 'Z';
                Serial.write(MotorRunning);
                MachineFail = true;
                break;  // Error case - Exit the loop when the timeout is reached ie no zero slot found
            }
        }
        counter++;
  }
}

void DoorAlarmCheck(){
 if (digitalRead(DoorSensor) == LOW){
   StatusReport = 'D';
   Serial.write(StatusReport);}
}

void DoorStatus(){
  if (digitalRead(DoorSensor) == LOW){
    StatusReport = 'D';}
    else {StatusReport = '0';}
    Serial.write(StatusReport);
}

void BottleType1SensorStatusRequest(){
  if (digitalRead(BottleType1Sensor) == LOW){
    StatusReport = 'H';}
    else {StatusReport = '1';}
    Serial.write(StatusReport);
}

void BottleType2SensorStatusRequest(){
  if (digitalRead(BottleType2Sensor) == LOW){
    StatusReport = 'J';}
    else {StatusReport = '1';}
    Serial.write(StatusReport);
}
//volatile int slotCounter = 0;

void SlotSearch(){
//  Serial.println(slotId);
  if (slotId == 0) {
    ZeroSlot();
  } else {
    slotSensorHasBeenClosed = false;
    slotCounter = 0;
    //slotCount();
        if (digitalRead(slotSensor) == HIGH){
    slotCountSlotSensorHigh();
    } else {
      slotCountSlotSensorLow(); }
  }
}

void step1slot(){
    slotSensorHasBeenClosed = false;
    if (digitalRead(zeroSlot) == LOW){ //when zeroSlot sensor is LOW (closed) then SlotSensor is LOW
    slotId = 1;
    } else {
      slotId = 1;
    }
    slotCounter = 0;
    if (digitalRead(slotSensor) == HIGH){
    slotCountSlotSensorHigh();}
    else {
      slotCountSlotSensorLow(); }
}

void slotCountSlotSensorHigh(){
 if (slotCounter < slotId) {
        digitalWrite(motor, HIGH);
        if (slotSensorHasBeenClosed == true){
          if (digitalRead(slotSensor) == HIGH){ 
             slotSensorHasBeenClosed = false;
                    slotCounter++;
//                    Serial.println(slotCounter);
                    MotorRunning = 'X';
                    Serial.write(MotorRunning);
                    delay(sensorReadDelay);
                    slotCountSlotSensorHigh();
              } else {
                    delay(sensorReadDelay);
                    slotCountSlotSensorHigh();}
      } else {
        if (digitalRead(slotSensor) == LOW) {
          slotSensorHasBeenClosed = true;
          delay(sensorReadDelay);
         slotCountSlotSensorLow();
        } else {
        delay(sensorReadDelay);
        slotCountSlotSensorLow();}} 
    } else {     // Stop the motor when the slotId is reached and slotSensor is HIGH
     stopMotor();
 }
}

void stopMotor(){
  if (digitalRead(slotSensor) == HIGH){
          digitalWrite(motor, LOW);
          MotorRunning = 'Y';
          Serial.write(MotorRunning); 
          slotSensorHasBeenClosed = false;
          slotId = 0;
          slotCounter = 0;
          } else {
            stopMotor();
          }
}

void slotCountSlotSensorLow(){
 if (slotCounter < slotId) {
        digitalWrite(motor, HIGH);
        if (slotSensorHasBeenClosed == true){
          if (digitalRead(slotSensor) == LOW){ 
             slotSensorHasBeenClosed = false;
                    slotCounter++;
//                    Serial.println(slotCounter);
                    MotorRunning = 'X';
                    Serial.write(MotorRunning);
                    delay(sensorReadDelay);
                    slotCountSlotSensorLow();
              } else {
                    delay(sensorReadDelay);
                    slotCountSlotSensorLow();}
      } else {
        if (digitalRead(slotSensor) == HIGH) {
          slotSensorHasBeenClosed = true;
          delay(sensorReadDelay);
          slotCountSlotSensorLow();
          } else {
          delay(sensorReadDelay);
          slotCountSlotSensorLow();}} 
    } else {     // Stop the motor when the slotId is reached and slotSensor is HIGH
    stopMotor();
 }
}

void StartFromZerodispenseBottleType1() { //in zeroSlot SlotSensor is LOW
  digitalWrite(motor, HIGH);
  if (slotSensorHasBeenClosed == true) {
    if (digitalRead(slotSensor) == LOW) {
      digitalWrite(LED_BUILTIN, LOW);
        findBottleType1();
    } else {
      delay(sensorReadDelay);
      StartFromZerodispenseBottleType1();  
    }
  } else {
    if (digitalRead(slotSensor) == HIGH) {
      digitalWrite(LED_BUILTIN, HIGH);
      slotSensorHasBeenClosed = true;
      MotorRunning = 'X';
      Serial.write(MotorRunning);
      //delay(100);
      StartFromZerodispenseBottleType1(); 
    } else {
      delay(sensorReadDelay);
      StartFromZerodispenseBottleType1(); 
    }
  }
}

void dispenseBottleType1() {
  digitalWrite(motor, HIGH);
  if (slotSensorHasBeenClosed == true) {
    if (digitalRead(slotSensor) == HIGH) {
      digitalWrite(LED_BUILTIN, LOW);
        findBottleType1();
    } else {
      delay(sensorReadDelay);
      dispenseBottleType1();  
    }
  } else {
    if (digitalRead(slotSensor) == LOW) {
      digitalWrite(LED_BUILTIN, HIGH);
      slotSensorHasBeenClosed = true;
      MotorRunning = 'X';
      Serial.write(MotorRunning);
      //delay(100);
      dispenseBottleType1(); 
    } else {
      delay(sensorReadDelay);
      dispenseBottleType1(); 
    }
  }
}

void zeroSlotFound(){
   FullBottle1Empty = true;
   digitalWrite(motor, LOW);
   MotorRunning = 'Z';
   Serial.write(MotorRunning);
}

void stopMotorFindBottle(){
    if (digitalRead(slotSensor) == HIGH){
       digitalWrite(motor, LOW); //bottle is found and zeroSlotSensor is not active
       digitalWrite(LED_BUILTIN, HIGH);
       MotorRunning = 'Y';
       Serial.write(MotorRunning);
       FullBottle1Empty = false;
       if (dispense1 == true) {
              delay(hatchOpenDelay);
              openHatch1();      
              dispense1 = false;
            }
    } else {
       stopMotorFindBottle();
 }
}

void findBottleType1(){  //no need to step slot because this function is called always when slot is empty (inventario)
  digitalWrite(motor, HIGH);
    int counter = 0;
    while (counter < counter1) {
      if (digitalRead(zeroSlot) == LOW){
        zeroSlotFound();
           break;  } // Exit the loop if zero slot is found - there are no full bottles
        if (digitalRead(BottleType1Sensor) == LOW) {  
          if (digitalRead(zeroSlot) == LOW) {   //maybe need to add small delay??
           zeroSlotFound();
           break;  // Exit the loop if zero slot is found - there are no full bottles            
          } else {
          stopMotorFindBottle();
           }
            break;  // Exit loop when bottle is found
        } else {
            delay(sensorReadDelay);
            MotorRunning = 'X';
            Serial.write(MotorRunning);
            if (counter > counter2) {
                FullBottle1Empty = true;
                digitalWrite(motor, LOW);
                MotorRunning = 'Z';
                Serial.write(MotorRunning);
                break;  // Exit the loop when the timeout is reached
            }
        }
        counter++;
    }
}

void openHatch1(){
   digitalWrite(OpenHatch1, HIGH);
   delay(500);
   digitalWrite(OpenHatch1, LOW);
}

void StartFromZerodispenseBottleType2() {
  digitalWrite(motor, HIGH);
  if (slotSensorHasBeenClosed == true) {
    if (digitalRead(slotSensor) == LOW) {
      digitalWrite(LED_BUILTIN, LOW);
        findBottleType2();
    } else {
      delay(sensorReadDelay);
      StartFromZerodispenseBottleType2();  
    }
  } else {
    if (digitalRead(slotSensor) == HIGH) {
      digitalWrite(LED_BUILTIN, HIGH);
      slotSensorHasBeenClosed = true;
      MotorRunning = 'X';
      Serial.write(MotorRunning);
      //delay(100);
      StartFromZerodispenseBottleType2(); 
    } else {
      delay(sensorReadDelay);
      StartFromZerodispenseBottleType2(); 
    }
  }
}

void dispenseBottleType2() {
  digitalWrite(motor, HIGH);
  if (slotSensorHasBeenClosed == true) {
    if (digitalRead(slotSensor) == HIGH) {
      digitalWrite(LED_BUILTIN, LOW);
        findBottleType2();
    } else {
      delay(sensorReadDelay);
      dispenseBottleType2();  
    }
  } else {
    if (digitalRead(slotSensor) == LOW) {
      digitalWrite(LED_BUILTIN, HIGH);
      slotSensorHasBeenClosed = true;
      MotorRunning = 'X';
      Serial.write(MotorRunning);
      delay(sensorReadDelay);
      dispenseBottleType2(); 
    } else {
      delay(sensorReadDelay);
      dispenseBottleType2(); 
    }
  }
}

void findBottleType2(){   //no need to step slot because this function is called always when slot is empty (inventario)
  digitalWrite(motor, HIGH);
    int counter = 0;
    while (counter < counter1) {
      if (digitalRead(zeroSlot) == LOW) {
           zeroSlotFound();
           break;  // Exit the loop if zero slot is found - there are no full bottles
      }
        if (digitalRead(BottleType2Sensor) == LOW) {
        //  delay(10)     //to be tested if delay is needed
          if (digitalRead(zeroSlot) == LOW) {
           zeroSlotFound();
           break;  // Exit the loop if zero slot is found - there are no full bottles            
          }
          else {     
            stopMotorFindBottle();
           }
            break;  // Exit the loop when the bottle is found
        } else {
            delay(sensorReadDelay);
            MotorRunning = 'X';
            Serial.write(MotorRunning);
            if (counter > counter2) {
                FullBottle2Empty = true;
                digitalWrite(motor, LOW);
                MotorRunning = 'Z';
                Serial.write(MotorRunning);
                break;  // Exit the loop when the timeout is reached
            }
        }
        counter++;
    }
}

void openHatch2(){
   digitalWrite(OpenHatch2, HIGH);
   delay(500);
   digitalWrite(OpenHatch2, LOW);
}

void isHatchOpen() {
  if (digitalRead(FullBottleHatchOpen) == LOW){
        StatusReport = '6';
        } else {
          StatusReport = '0'; }
        Serial.write(StatusReport);
}

void CheckAlarmStatus(){
     if (digitalRead(DoorSensor) == LOW){
        StatusReport = 'D';}
     else if (digitalRead(FullBottleHatchOpen) == LOW){
        StatusReport = '6';}
     else if (FullBottle1Empty == true){
        StatusReport = '8';}
     else if (FullBottle2Empty == true){
        StatusReport = '9';}
     else if (MachineFail == true){
        StatusReport = '5';}
     else {StatusReport = '0';}
     Serial.write(StatusReport);
}

// DoorOpen();
//  delay (500); 
//void DoorOpen() {
//  //Door is opened
//  if (DoorAlarm == false && digitalRead(DoorSensor == HIGH)){
//    DoorAlarm = true;
//    digitalWrite(motor, LOW); }
//  //Door is closed, start inventory
//  else if (DoorAlarm == true && digitalRead(DoorSensor == LOW)){
//    DoorAlarm = false; 
//    digitalWrite(motor, LOW);
//    delay(3000);}
//  //  dispenseBottleType1(); }
//  //Door is open
//  else if (DoorAlarm == true && digitalRead(DoorSensor == HIGH)){
//     DoorAlarm = true;
//     digitalWrite(motor, LOW);}
//  //Door is close
//  else if (DoorAlarm == false && digitalRead(DoorSensor == LOW)){
//     DoorAlarm = false;}
//    Serial.write(DoorAlarm);   
//}
