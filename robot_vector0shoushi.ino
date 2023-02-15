/*
   paj7620u2
   SDA-------A4
   SCL-------A5

   舵机 ------2

   oled
   MOSI-------9
   CLK-------10
   DC--------11
   CS--------12
   RESET------13

*/




#include "eyes.h"
#include "paj7620.h"

#include <SPI.h>
#include <Adafruit_GFX.h>


#include <Servo.h>

//#include <Wire.h>
//#include <SoftwareWire.h>
//#include <Adafruit_SSD1306_Soft.h>

#include <Adafruit_SSD1306.h>

Servo servo_2;
#define display_SCREEN_WIDTH 128
#define display_SCREEN_HEIGHT 64
#define display_OLED_MOSI 9
#define display_OLED_CLK 10
#define display_OLED_DC 11
#define display_OLED_CS 12
#define display_OLED_RESET 13
Adafruit_SSD1306 display(display_SCREEN_WIDTH, display_SCREEN_HEIGHT, display_OLED_MOSI, display_OLED_CLK, display_OLED_DC, display_OLED_RESET, display_OLED_CS);


#define GES_REACTION_TIME 500  // You can adjust the reaction time according to the actual circumstance.
#define GES_ENTRY_TIME 800     // When you want to recognize the Forward/Backward gestures, your gestures' reaction time must less than GES_ENTRY_TIME(0.8s).
#define GES_QUIT_TIME 1000
uint8_t data = 0, data1 = 0, error;
void setup() {
  // put your setup code here, to run once:
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  servo_2.attach(2);
  Serial.begin(115200);


  uint8_t error = 0;
  Serial.println("\nPAJ7620U2 TEST DEMO: Recognize 9 gestures.");

  error = paj7620Init();  // initialize Paj7620 registers
  if (error) {
    Serial.print("INIT ERROR,CODE:");
    Serial.println(error);
  } else {
    Serial.println("INIT OK");
  }
  Serial.println("Please input your gestures:\n");


  Serial.println("Booting");

  // if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) { // Address 0x3D for 128x64
  if (!display.begin(SSD1306_SWITCHCAPVCC)) {  // Address 0x3D for 128x64

    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();

  // Clear the buffer
  display.clearDisplay();
  display.display();
}

unsigned char readkey(void) {

  /*
    还得加需求，输出增加sg90舵机。还有那个手势传感器paj7620u2的九种手势，
    左、右分别对应原来的数字8口和9口的按键应该，
    前、后、上、下都对应数字10口的按键，目的就是要符合原来代码那个动画。
    然后手势顺时针、逆时针舵机分别顺时针、逆时针速度稍慢地转正负30度来回三次。手势摇摆不使用。
  */
  unsigned char ret = 0;
  error = paj7620ReadReg(0x43, 1, &data);                                                   // 获取手势值
  if (digitalRead(8) == 0 | data == GES_FORWARD_FLAG | data == GES_BACKWARD_FLAG) ret = 1;  //前 后
  if (digitalRead(9) == 0 | data == GES_LEFT_FLAG | data == GES_UP_FLAG) ret += 2;          //左 上
  if (digitalRead(10) == 0 | data == GES_RIGHT_FLAG | data == GES_DOWN_FLAG) ret += 4;      //abajo 右 下
  if (digitalRead(11) == 0) ret += 8;                                                       //
  if (data == GES_CLOCKWISE_FLAG)                                                           //顺时针
  {
    servo_2.write(180);
  }
  if (data == GES_COUNT_CLOCKWISE_FLAG)  //逆时针
  {
    servo_2.write(0);
  }
  return (ret);
}

int xp = 16;
int mood = 1;

void loop() {
  int n;
  static int xd = 0;
  static int espera = 0;
  static int step = 0;
  int x1, x2;



  if (espera > 0) {
    espera--;
    delay(1);
  } else {
    x1 = xd + (xp > 16 ? (16 + 2 * (xp - 16)) : xp);
    x2 = 64 + xd + (xp < 16 ? (-16 + (xp * 2)) : xp);
    switch (step) {
      case 0:
        display.clearDisplay();  // Clear the display buffer
        if (xp < 6) {
          display.drawBitmap(x1, 8, peyes[mood][2][0], 32, 32, WHITE);
          display.drawBitmap(x2, 8, peyes[mood][1][1], 32, 32, WHITE);
        } else if (xp < 26) {
          display.drawBitmap(x1, 8, peyes[mood][0][0], 32, 32, WHITE);
          display.drawBitmap(x2, 8, peyes[mood][0][1], 32, 32, WHITE);
        } else {
          display.drawBitmap(x1, 8, peyes[mood][1][0], 32, 32, WHITE);
          display.drawBitmap(x2, 8, peyes[mood][2][1], 32, 32, WHITE);
        }
        display.display();
        espera = random(250, 1000);
        n = random(0, 7);
        if (n == 6) {
          step = 1;
        } else {
          step = 2;
        }
        break;
      case 1:
        display.clearDisplay();  // Clear the display buffer
        display.drawBitmap(x1, 8, eye0, 32, 32, WHITE);
        display.drawBitmap(x2, 8, eye0, 32, 32, WHITE);
        display.display();
        espera = 100;
        step = 0;
        break;
      case 2:
        n = random(0, 10);
        if (n < 5) xd--;
        if (n > 5) xd++;
        if (xd < -4) xd = -3;
        if (xd > 4) xd = 3;
        espera = 0;
        step = 0;
        break;
    }
  }
  //n=0;
  n = readkey();
  if (n == 2) xp = (xp <= 0 ? 0 : xp - 1);
  if (n == 4) xp = (xp >= 32 ? 32 : xp + 1);
  if (n == 1) {
    mood = (mood >= 5 ? 0 : mood + 1);
    do {
    } while (readkey() != 0);
  }
  if (n != 0) {
    espera = 0;
    step = 0;
  }
}
