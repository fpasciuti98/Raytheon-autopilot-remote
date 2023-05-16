
//Raytheon-Autopilot-Remote
//Two way communication over Raytheon seatalk


#include <RCSwitch.h>

// 0x86 for keystroke
// 0x11 for remote control
// data + inversed data

#define comKEY 0x86 // keystroke
#define comRES 0x87 //response level
#define attREM1 0x11 //attribute remote control Z101
#define attREM2 0x21 //attribute remote control ST600R
#define attAP 0x01 //attribute autopilot ST1000+

int COM = comKEY;
int ATT = attREM1;


#define LAMP_OFF 0x00   //8 non funziona
#define LAMP_ON 0x0C    //7 non funziona
#define PLUS_ONE 0x07   //  funziona solo auto
#define MINUS_ONE 0x05  //  funziona solo auto
#define PLUS_TEN 0x08   //4 funziona solo auto
#define MINUS_TEN 0x06  //3 funziona solo auto
#define STANDBY 0x02    //1 funziona
#define AUTO 0x01       //2 funziona
#define TRACK 0x03      //9 funziona
#define DISP 0x04       //15 non funziona
#define TACK_MINUS 0x21 //5 funzinoa solo auto
#define TACK_PLUS 0x22  //6 funziona solo auto
#define WIND 0x23       //14 non funziona (forse perchè non rileva stazione vento)
#define RESPONSE 0x20   //13 funziona in REM1
#define LONG_PLUS_ONE 0x47 //10 
#define LONG_MINUS_ONE 0x45
#define LONG_MINUS_TEN 0x46
#define LONG_PLUS_TEN 0x48


int ledPin = 13; // LED connected to digital pin 13
int SEATALK_TX_OUT = 6;
int SEATALK_RX_IN = 7;
int buzzer = 12;
RCSwitch mySwitch = RCSwitch();

//--------------------------------SETUP----------------------------------------------------------------------------------------------
void setup()
{
Serial.begin(9600);
pinMode(SEATALK_TX_OUT, OUTPUT); // sets the digital pin as output
pinMode(SEATALK_RX_IN, INPUT); // sets the digital pin as input
pinMode(buzzer,OUTPUT);
pinMode(ledPin, OUTPUT); // sets the digital pin as output
mySwitch.enableReceive(0); // Receiver on inerrupt 0 => that is pin #

int cX;
for ( cX = 0; cX < 3; cX++ ) // power-on flash LED 10 times
{
digitalWrite(ledPin, HIGH); // turn the LED on (HIGH is the voltage level)
digitalWrite(buzzer, HIGH);
delay(300);
digitalWrite(ledPin, LOW); // turn the LED off (LOW is the voltage level)
digitalWrite(buzzer, LOW);
delay(300);
}
}
//-----------------------------------LOOP----------------------------------------------------------------------------------------

void loop ()
{
int cX;
delay(100); // programming delay


if (mySwitch.available()&& mySwitch.getReceivedBitlength()==24 )
{
int value = mySwitch.getReceivedValue(); // get key fob value when key pressed
  
switch(value) 
{

case 17492:
Msg ( MINUS_ONE );
break;

case 20564:
Msg ( PLUS_ONE );
break;

case 21588:
Msg ( MINUS_TEN );
break;

case 5204:
Msg ( PLUS_TEN );
break;
}
mySwitch.resetAvailable();

}
}

//-----------------------------------FUNCTIONS----------------------------------------------------------------------------------------


void Msg (int cData)
{
  digitalWrite(ledPin, HIGH);

  int cError = SendKeystrokeMsg ( AUTO,  LOW );
  cError = SendKeystrokeMsg ( cData, LOW );

  if (cError == HIGH) {
  Serial.println("ERROR");
  digitalWrite(ledPin, LOW);
  for (int i=0;i<5;i++){
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  delay(100);
  }
  
  digitalWrite(ledPin, LOW);
  } else{
  digitalWrite(ledPin, LOW);
  digitalWrite(buzzer, HIGH);
  delay(300);
  digitalWrite(buzzer, LOW);
  } 
}

//==============================================================

int SendKeystrokeMsg ( int cData, int cError )
  {
  int count=0;
  do {
  cError = CheckBus( LOW ); // wait for bus to be idle
  cError = SendByte ( cError, HIGH, COM ); // command: keystroke=86 (è un comando proveniente da tastiera)
  cError = SendByte ( cError, LOW, ATT ); // Attribute Character: remote control= 11 (è la tastiera del telecomando  )
  cError = SendByte ( cError, LOW, cData ); // data:
  cError = SendByte ( cError, LOW,~cData ); // data: inverted data
  digitalWrite(SEATALK_TX_OUT, LOW); 
  delay(10); // LED visible delay
  count++;
  
  } while ( cError == HIGH && count<100); // repeat if message was corrupted
  
  return cError;
}

//==============================================================



int SendByte ( int cError, int cCommand, int cData )
  {

  int cX;
  if ( cError != HIGH ) // if no error from previous
  { 

    cError = SendBit ( cError, HIGH ); // start bit (0V)
    for ( cX = 0; cX < 8; cX++ )
    {
      cError = SendBit ( cError, ~cData & 0x01 ); // LSB data bit    
      cData >>= 1; // shift right
    }
    cError = SendBit ( cError, cCommand ? LOW : HIGH ); // command bit identifies command characters (Se il byte è di comando then LOW, else HIGH)
    cError = SendBit ( cError, LOW ); // stop bit (+12V)

     }
     
  return ( cError );
}


//==============================================================


int SendBit ( int cError, int cBit )
  {

  int cX;
  // this is bit-banged code, it must be adjusted to give 208uS bit times (4800 baud)
  if ( cError != HIGH ) // if no error from previous
  {
  digitalWrite(SEATALK_TX_OUT, cBit); // send bit to output
  for ( cX = 0; cX < 7; cX++ ) // check output bit periodically
  {
  delayMicroseconds(25); // pauses for xx microseconds adjust to match 4800 BAUD
  if ( digitalRead(SEATALK_RX_IN) == !cBit ) // check if output bit is corrupted by another talker
  {
  return ( HIGH ); // return collision error
  }
  }
  return ( LOW ); // return no error, bit sucessfully sent
  }
  else
  {
  return ( HIGH ); // simply return collision error from before
  }
}


//==============================================================



int CheckBus ( int cError ) //esco dalla funzione solamente quando non rilevo nessun segnale per 
  {
  int count=0;
  int cX;
  int val;
  if ( cError != HIGH ) // if no error from previous
  { 
    for ( cX = 0; cX < 255; cX++ ) // assumes output is floating to +12V for ~5mS
    { 
      if ( val=digitalRead(SEATALK_RX_IN) == HIGH) // check if output bit is corrupted by another talker
      {
      cX = 0; // reset count to zero
      count++;
      }
    if (count>10000){
     return (HIGH);
    }
    delayMicroseconds(7); // pauses for 7 microseconds
    }
  return ( LOW );
  }
  else
  {
  return ( HIGH ); // simply return collision error from before
  }
}
