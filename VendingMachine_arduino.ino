const int DispenseBottleType1 = 9;     //variables and pins for controlling motors       Tablet app <<< ----- RS232 ---->>> Arduino IO kortti
const int DispenseBottleType2 = 8;
const int OpenReturnBottleHatch = 2;
const int ReturnBottleInBox = 10;

//status report bits
int FullBottleHatchOpen = 11;
int ReturnBottleInOk = 12;
int motorAlarmpin = 7;       
int FullBottle1Empty = 5;
int FullBottle2Empty = 4;
int ReturnBottleBoxFull = 3;

char ctrlInput;          //variable for Serial input & output data
char StatusReport = 0;    //variables for Status reporting

void setup() {  
//
Serial.begin(115200);       //Start the serial monitor at 115200 baud  
pinMode(DispenseBottleType1,OUTPUT);    //set pin modes 
pinMode(DispenseBottleType2,OUTPUT);
pinMode(OpenReturnBottleHatch,OUTPUT);
pinMode(ReturnBottleInBox, OUTPUT);
pinMode(motorAlarmpin,INPUT); 
pinMode(FullBottleHatchOpen, INPUT);
pinMode(ReturnBottleInOk, INPUT);
pinMode(FullBottle1Empty, INPUT);
pinMode(FullBottle2Empty, INPUT);
pinMode(ReturnBottleBoxFull, INPUT);

digitalWrite(DispenseBottleType1, LOW);  //set both motor pins LOW in the startup
digitalWrite(DispenseBottleType2, LOW);
digitalWrite(OpenReturnBottleHatch, LOW);
digitalWrite(ReturnBottleInBox, LOW);
}
  
void loop() {
 // ReceiveSerialData();
//  delay (500);
 if(Serial.available() > 0){
     delay(10); //delay for all data available in buffer. 1Byte 80us
    ctrlInput = Serial.read();        }     //if so, then read the incoming data.    
     if(ctrlInput == '1' ){
      digitalWrite(DispenseBottleType1, HIGH);
      //dispenseBottle1();
      delay (1000);
      digitalWrite(DispenseBottleType1, LOW);}
     else if(ctrlInput == '2'){
      digitalWrite(DispenseBottleType2, HIGH);
      delay (1000);
      digitalWrite(DispenseBottleType2, LOW);}
     else if(ctrlInput == 'E'){
      digitalWrite(OpenReturnBottleHatch, HIGH);
      delay (1000);
      digitalWrite(OpenReturnBottleHatch, LOW);}
     else if(ctrlInput == 'F'){
      digitalWrite(ReturnBottleInBox, HIGH);
      delay (1000);
      digitalWrite(ReturnBottleInBox, LOW);}
     else if(ctrlInput == '7'){
      CheckAlarmStatus(); }
     else if(ctrlInput == 'B'){
      CheckInfoStatus(); }
  ctrlInput = 0;

}
void CheckInfoStatus(){
     if (digitalRead(ReturnBottleInOk) == 1){
        StatusReport = 'C';}
        else {StatusReport = '0';}
      Serial.write(StatusReport);  
}
void CheckAlarmStatus(){
     if (digitalRead(ReturnBottleBoxFull) == 1){
        StatusReport = 'A';}
     else if (digitalRead(motorAlarmpin)==1){
        StatusReport = '5';}
     else if (digitalRead(FullBottleHatchOpen)==1){
        StatusReport = '6';}
     else if (digitalRead(FullBottle1Empty)==1){
        StatusReport = '8';}               
     else if (digitalRead(FullBottle2Empty)==1){
        StatusReport = '9';}   
     else {StatusReport = '0';}
      Serial.write(StatusReport);   
}
