/*
7 Segment (Common Anode) Display Pin:
 
     g     f   5V   a     b
 ___|___|___|___|___|____
|                                       |
|        a                               |
|    f       b                          |
|        g                               |
|    e       c                          |
|        d              dp(Dot) |
|________________________|
    |     |     |      |     |
    e    d   5V   c   dp
 
74HC595:
           _______
 b OUT Q1-|1 *  16|-VCC 5V
 c OUT Q2-|2    15|-Q0 OUT a
 d OUT Q3-|3    14|-SER data A.PIN 10
 e OUT Q4-|4    13|-OE-bar GND
 f OUT Q5-|5    12|-RCLK latch A.PIN 9
 g OUT Q6-|6    11|-SRCLK clock A.PIN 8 
 p OUT Q7-|7    10|-5V
      GND-|8_____9|-
*/

int dataPin = 10;
int latchPin = 9;
int clockPin = 8;
int buttonPin = 7;
//seven segment digits in byte Bpgfedcba
byte dec_digits[] = {B11000000,B11111001,B10100100,//0, 1, 2
                     B10110000,B10011001,B10010010,//3, 4, 5
                     B10000011,B11111000,B10000000,//6, 7, 8
                     B10011000 };                  //9

byte test_digits[] = {B11111110,B11111101,B11111011,//0, 1, 2
                     B11110111,B11101111,B11011111};                  //9
                                          
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers
int numberToDisplay = 9;

void setup() {
  //set pins to output so you can control the shift register and read the button state
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  // take the latchPin low so 
  // the LEDs don't change while you're sending in bits:
  digitalWrite(latchPin, LOW);
  // shift out the bits:
  shiftOut(dataPin, clockPin, MSBFIRST, dec_digits[numberToDisplay]); 
  //take the latch pin high so the LEDs will light up:
  digitalWrite(latchPin, HIGH);
}

void loop() {

  /*delay(15);
  numberToDisplay = (numberToDisplay + 1) % 6;
        // take the latchPin low so 
        // the LEDs don't change while you're sending in bits:
  digitalWrite(latchPin, LOW);
        // shift out the bits:
  shiftOut(dataPin, clockPin, MSBFIRST, test_digits[numberToDisplay]); 
        //take the latch pin high so the LEDs will light up:
  digitalWrite(latchPin, HIGH);*/
  
  // read the state of the switch into a local variable:
  int reading = digitalRead(buttonPin);
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:
  
  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        numberToDisplay = (numberToDisplay + 1) % 10;
        // take the latchPin low so 
        // the LEDs don't change while you're sending in bits:
        digitalWrite(latchPin, LOW);
        // shift out the bits:
        shiftOut(dataPin, clockPin, MSBFIRST, dec_digits[numberToDisplay]); 
        //take the latch pin high so the LEDs will light up:
        digitalWrite(latchPin, HIGH);
      }
    }
  }
  // save the reading.  Next time through the loop,
  // it'll be the lastButtonState:
  lastButtonState = reading;
}
