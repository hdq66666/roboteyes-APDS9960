/*
 * apds9960
 * SDA-------A4
 * SCL-------A5
 * 
 * 舵机sg90 ---2
 * 
 * oled ssd1306
 * SDA-------sda
 * SCL-------scl
 *
 * 按键
 *切换表情-----8  
 *向左看-------9  
 *向右看-------10
*/   
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int act;
int pos = 90;
#include "eyes.h"
#include "MH_APDS9960.h"
MH_APDS9960 apds;
Servo myservo;
void setup() {
  // put your setup code here, to run once:
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);
  myservo.attach(2);
  Serial.begin(115200);
  Serial.println("Booting");
  myservo.write(pos);
  if (!apds.begin()) {
    Serial.println("failed to initialize device! Please check your wiring.");
  } else Serial.println("Device initialized!");

  //gesture mode will be entered once proximity mode senses something close
  apds.enableProximity(true);
  apds.enableGesture(true);


  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) {  // Address 0x3D for 128x64
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
  unsigned char ret = 0;
  if (digitalRead(8) == 0) ret = 1;    //izquierda
  if (digitalRead(9) == 0) ret += 2;   //arriba
  if (digitalRead(10) == 0) ret += 4;  //abajo
  if (digitalRead(11) == 0) ret += 8;  //fuego
  return (ret);
}

int xp = 16;
int mood = 1;
void gesture() {
  uint8_t gesture = apds.readGesture();
  if (gesture == APDS9960_DOWN) {
    act = 1;
    Serial.println("v");
    delay(15);
    // act = 0;
  }
  if (gesture == APDS9960_UP) {
    act = 1;
    Serial.println("^");
    delay(15);
    // act = 0;
  }
  if (gesture == APDS9960_LEFT) {
    myservo.write(pos = pos - 18);
    act = 4;
    Serial.println("<");
    delay(50);
    // act = 0;
  }
  if (gesture == APDS9960_RIGHT) {
    myservo.write(pos = pos + 18);
    act = 2;
    Serial.println(">");
    delay(50);
    // act = 0;
  }
}
void loop() {
  gesture();
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
  n = 0;
  // n = readkey();
  n = act;
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
