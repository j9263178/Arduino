/*
7 Segment (Common Anode) Display Pin:

   CA1  a   f  CA2 CA3  b
 ___|___|___|___|___|___|____
|    CA1  CA2  CA3  CA4    |
|        a                                  |
|    f       b                              |
|        g                                  |
|    e       c                             |
|        d       dp(Dot)            |
|__________________________|
    |    |      |    |   |      |
    e   d   dp  c   g  CA4

First 74HC595:
                          _______
D3 OUT  Q1-|1 * 16|-VCC 5V
D2 OUT  Q2-|2    15|-Q0 OUT b
 f OUT     Q3-|3    14|-SER data A.PIN 10
 a OUT    Q4-|4    13|-~OE GND
D1 OUT Q5 -|5    12|-RCLK latch A.PIN 9
D4 OUT Q6 -|6    11|-SRCLK clock A.PIN 8
 g OUT Q7   -|7    10|-5V
      GND       -|8____9|-

Second 74HC595:
                        _______
dp OUT Q1-|1 * 16|-VCC 5V
d OUTQ2   -|2     15|-Q0 OUT c
e OUT Q3  -|3     14|-SER data PIN QH'
   OUT   Q4-|4     13|-~OE GND
   OUT   Q5-|5     12|-RCLK latch A.PIN 9
   OUT   Q6-|6     11|-SRCLK clock A.PIN 8
   OUT   Q7-|7     10|-5V
      GND     -|8____9|-
*/

#define number_of_74hc595s 2

unsigned long previousMillis = 0;
unsigned long interval = 10;
int dataPin = 10;
int latchPin = 9;
int clockPin = 8;
int dp = 9;
int a = 4;
int b = 0;
int c = 8;
int d = 10;
int e = 11;
int f = 3;
int g = 7;
int D1 = 5;
int D2 = 2;
int D3 = 1;
int D4 = 6;
//seven segment digits in byte Bpgfedcba
byte display_word[] = {B11000000, B11111001, B10100100, //0, 1, 2
                       B10110000, B10011001, B10010010, //3, 4, 5
                       B10000011, B11111000, B10000000, //6, 7, 8
                       B10011000, B11111111, B10001000, //9, blank, A
                       B10000011, B11000110, B10100001, //b, C, d
                       B10000110, B10001110 //E,F
                      };

int sr_segment_pins[] = {a, b, c, d, e, f, g, dp};
int sr_digits_pins[] = {D1, D2, D3, D4};
//two shift registers digits in bytes [number_of_SR][number_of_digits][number_of_displayset][nodp_dp]
byte sr_word[number_of_74hc595s][sizeof(sr_digits_pins) / 2][sizeof(display_word)][2];

void setup() {
  //set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  Serial.begin(9600);
  //preset sr_word (anode displayer use negtive true logic)
  for (int i = 0; i < number_of_74hc595s ; i++) {
    for (int j = 0; j < sizeof(sr_digits_pins) / 2 ; j++) {
      for (int k = 0; k < sizeof(display_word) ; k++) {
        for (int l = 0; l < 2 ; l++) {
          sr_word[i][j][k][l] = B11111111;
        }
      }
    }
  }
  //initialize sr_word
  for (int i = 0; i < sizeof(display_word); i++) {
    byte read_word = display_word[i];
    for (int j = 0; j < sizeof(sr_digits_pins) / 2; j++) {
      int sr_D_PosW = sr_digits_pins[j] / 8;
      int bit_D_PosW = sr_digits_pins[j] % 8;
      for (int k = 0; k < sizeof(sr_segment_pins) / 2 - 1; k++) {
        int sr_S_PosW = sr_segment_pins[k] / 8;
        int bit_S_PosW = sr_segment_pins[k] % 8;
        for (int l = 0; l < 2; l++) {
          sr_word[sr_D_PosW][j][i][l] = bitWrite(sr_word[sr_D_PosW][j][i][l], bit_D_PosW, 0);
          sr_word[sr_S_PosW][j][i][l] = bitWrite(sr_word[sr_S_PosW][j][i][l], bit_S_PosW, bitRead(read_word, k));
          sr_word[sr_segment_pins[7] / 8][j][i][l] = bitWrite(sr_word[sr_segment_pins[7] / 8][j][i][l], sr_segment_pins[7] % 8, !l);
        }
      }
    }
  }
}

void loop() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
  }
  // display time with ripple blanking function (to tens digit)
  int ripple_blank = int(previousMillis / 100000 % 10); //status of ripple blanking
  displayWrite(0, int(previousMillis / 100000 % 10) + !ripple_blank * 10, 0);
  ripple_blank = ripple_blank + int(previousMillis / 10000 % 10);
  displayWrite(1, int(previousMillis / 10000 % 10 + !ripple_blank * 10), 0);
  displayWrite(2, int(previousMillis / 1000 % 10), 1);
  displayWrite(3, int(previousMillis / 100 % 10), 0);
}

void displayWrite(int digitPos, int digitNum, int digitDp) {
  // take the latchPin low
  digitalWrite(latchPin, LOW);
  // shift out the bits:
  for (int i = number_of_74hc595s - 1; i > -1; i--) {
    shiftOut(dataPin, clockPin, MSBFIRST, sr_word[i][digitPos][digitNum][digitDp]);
  }
  //take the latch pin high
  digitalWrite(latchPin, HIGH);
}
