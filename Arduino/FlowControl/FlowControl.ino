/* YourDuino.com Example Software Sketch
 20 character 4 line I2C Display
 Backpack Interface labelled "YwRobot Arduino LCM1602 IIC V1"
 Connect Vcc and Ground, SDA to A4, SCL to A5 on Arduino
 terry@yourduino.com */

/*-----( Import needed libraries )-----*/
#include <Wire.h>  // Comes with Arduino IDE
// Get the LCD I2C Library here: 
// https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
// Move any other LCD libraries to another folder or delete them
// See Library "Docs" folder for possible commands etc.
#include <LiquidCrystal_I2C.h>

/*-----( Declare Constants )-----*/
/*-----( Declare objects )-----*/
// set the LCD address to 0x27 for a 20 chars 4 line display
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

// buttonchecker_nodelay_Adafruit.ino
// // http://www.ediy.com.my/index.php/tutorials/item/96-debouncing-multiple-switches
    #define DEBOUNCE 10 // how many ms to debounce, 5+ ms is usually plenty
    #define BUTTON_HOLD_DELAY 1500 //poklika ms drzeni buttonu zacne zrychlena zmena hodnot
    #define SPEED_INCR 40 //krok zrychlenych zmen v ms
    #define MAX_PRUTOK 100 // rozsah prutokomeru
//define the buttons that we'll use.
byte buttons[] = {A3, A2};

//determine how big the array up above is, by checking the size
#define NUMBUTTONS sizeof(buttons)
#include "SPI.h"; 


//track if a button is just pressed, just released, or 'currently pressed'
byte pressed[NUMBUTTONS], justpressed[NUMBUTTONS], justreleased[NUMBUTTONS];
byte previous_keystate[NUMBUTTONS], current_keystate[NUMBUTTONS];



word DACinput = 0;
const int DACoutputReadPin = A0;
const int remoteTriggerPin = 9;
const int pripojeno = 7;
boolean dalkove = false;
boolean remoteState = false;
boolean remoteStateLast;
String line = "";
char str[10];
unsigned long timerRED;
unsigned long timerBLACK;
unsigned long timerDACread;
unsigned long previousMillis = 0;
int lightOutput = 8;
boolean allowRED = false;
boolean allowBLACK = false;
int prevcounter;
int counter = 0; 
int prevnacteno;
int nacteno = 0;

void setup() {
  byte i;
//Serial.begin(9600); //set up serial port
//SPI.begin();
//pinMode(pinSelect, OUTPUT);
SPI_init(); //inicializace prenosu do DAC
lcd.begin(20,4);
lcd.backlight();
lcd.setCursor(0,0);
lcd.print("read: 0 l/m");
lcd.setCursor(0,2);
lcd.print("set: 0 l/m");
//SPI.begin();
//SPI.setBitOrder(MSBFIRST);
//pinMode(10, OUTPUT);

/*Serial.print("Button checker with ");
Serial.print(NUMBUTTONS, DEC);
Serial.println(" buttons");*/
pinMode(8, OUTPUT);
pinMode(10, OUTPUT);
digitalWrite(8, HIGH);
pinMode(pripojeno, INPUT);
digitalWrite(8, LOW);

pinMode(DACoutputReadPin, INPUT);
// Make input & enable pull-up resistors on switch pins
for (i=0; i< NUMBUTTONS; i++) {
  pinMode(buttons[i], INPUT);
  digitalWrite(buttons[i], HIGH);
  //Serial.println(0);
}
}


void loop() {  
   
    if(pressed[0]){
      if(!allowRED){
    timerRED = millis(); //Serial.println("start REDtimer");
    allowRED = true;
  }
  if(allowRED == true && (millis()-timerRED) > BUTTON_HOLD_DELAY){
    //po stisku tlacitka zacne pocitat cas
    //pokud timer > BUTTON_HOLD_DELAY, zacne rychle pricitat do counter     
    //LEDstate = !LEDstate; Serial.print("LED "); Serial.println(LEDstate);
    if((millis()-timerRED)%SPEED_INCR == 0){
        //Serial.println("PRODLEVA");
        //Serial.print("dasd");//delay(500);
        counter ++; 
        if(counter>=MAX_PRUTOK)counter=MAX_PRUTOK;
        //Serial.println (counter/4096.0*100,4);//Serial.println (timerRED);
        lcd.setCursor(0,0);
        lcd.print("set: "+String(counter));
        timerRED=timerRED-1;//rychlost loopu je vetsi nez zmena v millis()
      }
    }    
  }
//po odmacknuti tlacitka zapise novou hodnotu prutoku do DAC
if(justreleased[0]&&allowRED){
  allowRED=false;
  DACinput = (int)((counter*4095.0/100.0)); //prevod rozsahu prutokomeru (0-100) na 12bitove cislo (0-4095)
  setDAC(DACinput);
  lcd.setCursor(0,1);
  lcd.print(String(DACinput)+" "+String(DACinput/4095.0*5.0)+"V"+" ");
  //delay(50);  
  //Serial.print("Poslano do DAC: ");Serial.print(DACinput);Serial.print("  |  ");Serial.print(counter);Serial.print(" l/min");
  //Serial.print("  |  ");Serial.print(DACinput/4095.0*5.0, 4);Serial.println(" V");
  //nacteno = analogRead(DACoutputReadPin);
  //Serial.print("Precteno z DAC: ");Serial.print(nacteno);Serial.print("  |  ");
  //Serial.print(nacteno/1023.0*100.0, 3);Serial.print(" l/min");Serial.print("  |  ");
  //Serial.print(nacteno*5.0/1023.0, 4);Serial.println(" V");
  //Serial.print("\n");
}

if(pressed[1]){
  if(!allowBLACK){
    timerBLACK = millis(); //Serial.println("start BLACKtimer");
    allowBLACK = true;
  }
  if(allowBLACK == true && (millis()-timerBLACK) > BUTTON_HOLD_DELAY){
    //LEDstate = !LEDstate; Serial.print("LED "); Serial.println(LEDstate);
    if((millis()-timerBLACK)%SPEED_INCR == 0){

      if (counter <= 0)counter = 0;
      else counter --; 
      lcd.setCursor(0,0);
      lcd.print("set: "+String(counter));
        //Serial.println (counter/4096.0*100,4);
        timerBLACK=timerBLACK-1;
      }
    }    
  }
  if(justreleased[1]&&allowBLACK){
    allowBLACK=false;
    DACinput = (int)((counter*4095.0/100.0));
    setDAC(DACinput);
    lcd.setCursor(0,1);
    lcd.print(String(DACinput)+" "+String(DACinput/4095.0*5.0)+"V"+" "); 
  //delay(50); 
  //Serial.print("Poslano do DAC: ");Serial.print(DACinput);Serial.print("  |  ");Serial.print(counter);Serial.print(" l/min");
  //Serial.print("  |  ");Serial.print(DACinput/4095.0*5.0, 4);Serial.println(" V");
  //nacteno = analogRead(DACoutputReadPin);
  //Serial.print("Precteno z DAC: ");Serial.print(nacteno);Serial.print("  |  ");
  //Serial.print(nacteno/1023.0*100.0, 3);Serial.print(" l/min");Serial.print("  |  ");
  //Serial.print(nacteno*5.0/1023.0, 4);Serial.println(" V");
  //Serial.print("\n");
}

//digitalWrite(8, LEDstate); 
byte thisSwitch=thisSwitch_justPressed();
switch(thisSwitch)
{
case 0://stisk RED buttonu
//Serial.println("switch RED just pressed");
counter++;
if(counter>=MAX_PRUTOK)counter=MAX_PRUTOK; //Serial.println (counter/4096.0*100,4);
/* pocita cas mezi stisky tlacitka
if (allow){
Serial.print("Cas mezi stisky: ");Serial.println(millis()-timerRED);
allow=false; timerRED = millis();
}
allow = true; 
timerRED = millis();*/
break;
case 1://stisk BLACK buttonu
//Serial.println("switch BLACK just pressed"); 
if (counter <= 0)counter = 0;
else counter--; 
//Serial.println (counter/4096.0*100,4);
break;
}

if(prevcounter!=counter){
  if(counter == 0)digitalWrite(8, HIGH);
  else digitalWrite(8, LOW);
  //Serial.println (counter);
  //lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("set: "+String(counter)+" "+"l/m"+" ");
  lcd.setCursor(0,1);
  lcd.print(String(DACinput)+" "+String(DACinput/4095.0*5.0)+"V"+" ");
/*if(justreleased){
  DACinput = count*4096/100-1;
  setDAC(DACinput);  
  }*/
  prevcounter=counter;
  
}

unsigned long currentMillis = millis();
if((currentMillis - previousMillis) > 250UL){
    nacteno = analogRead(DACoutputReadPin);
  if(prevnacteno!=nacteno){
    sprintf(str, "%.2f", nacteno/1023.0*5.0);

    lcd.setCursor(0,2);
    lcd.print("read: "+String(nacteno/1023.0*100)+" "+"l/m"+" ");
    lcd.setCursor(0,3);
    lcd.print(String(nacteno)+" "+String(nacteno/1023.0*5.0)+"V"+" ");


    prevnacteno=nacteno;  
  }
  if(digitalRead(pripojeno) == LOW){
      lcd.clear();
      lcd.setCursor(6,0);
      lcd.print("PRIPOJTE");
      lcd.setCursor(4,2);
      lcd.print("KONTROLER !!!");
    } 



previousMillis = currentMillis;
}






}//end loop()

void check_switches()
{
  static byte previousstate[NUMBUTTONS];
  static byte currentstate[NUMBUTTONS];
  static long lasttime;
  byte index;
  if (millis() < lasttime) {
// we wrapped around, lets just try again
lasttime = millis();
}
if ((lasttime + DEBOUNCE) > millis()) {
// not enough time has passed to debounce
return;
}
// ok we have waited DEBOUNCE milliseconds, lets reset the timer
lasttime = millis();
for (index = 0; index < NUMBUTTONS; index++) {
justpressed[index] = 0; //when we start, we clear out the "just" indicators
justreleased[index] = 0;
currentstate[index] = digitalRead(buttons[index]); //read the button
if (currentstate[index] == previousstate[index]) {
  if ((pressed[index] == LOW) && (currentstate[index] == LOW)) {
// just pressed
justpressed[index] = 1;
}
else if ((pressed[index] == HIGH) && (currentstate[index] == HIGH)) {
justreleased[index] = 1; // just released
}
pressed[index] = !currentstate[index]; //remember, digital HIGH means NOT pressed
}
previousstate[index] = currentstate[index]; //keep a running tally of the buttons
}
}

byte thisSwitch_justPressed() {
  byte thisSwitch = 255;
check_switches(); //check the switches & get the current state
for (byte i = 0; i < NUMBUTTONS; i++) {
  current_keystate[i]=justpressed[i];
  if (current_keystate[i] != previous_keystate[i]) {
    if (current_keystate[i]) thisSwitch=i;
  }
  previous_keystate[i]=current_keystate[i];
}
return thisSwitch;
}

void setDAC(word value){
  //Serial.println("setDAC________________________");
  byte data = 0;
  digitalWrite(10, LOW); delay(100) ;//povoli zapis na DAC
  data = highByte(value); //bity 15-8
  data = 0b00001111 & data; //vynulování nejvyšších 4 bitů
  data = 0b00110000 | data; //konfigurace 4 MSB pro DAC 0=DACA, 0=buffered, 1=1x, 1=output enabled
  SPI.transfer(data); //poslaní 1. 8 bitů
  //Serial.println(data, BIN);
  data = lowByte(value);//bity 0-7
  SPI.transfer(data); //poslání zbylých bitů
  //Serial.println(data, BIN);
  digitalWrite(10, HIGH); //zakaze zapis na DAC
  //Serial.println("________________________END");
}

void SPI_init(){
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  pinMode(10, OUTPUT);
  setDAC(0);
//Serial.println("Inicializace: 0 l/min");
//Serial.print(analogRead(DACoutputReadPin)*100.0/1024.0);Serial.println(" l/min z DAC");
}
