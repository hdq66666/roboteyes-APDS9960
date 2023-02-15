
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, 2, 3, OLED_RESET);

#include "eyes.h"

#include "PAJ7620U2.h"
unsigned short Gesture_Data;

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false, false);
  // put your setup code here, to run once:
  pinMode(8, INPUT_PULLUP);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, INPUT_PULLUP);

  Serial.begin(115200);
  uint8_t i;
  Serial.print("\nGesture Sensor Test Program ...\n");
  delayMicroseconds(800);
  Wire.begin();
  delayMicroseconds(10);
  while (!PAJ7620U2_init()) {
    Serial.print("\nGesture Sensor Error\n");
    delay(500);
  }
  Serial.print("\nGesture Sensor OK\n");
  I2C_writeByte(PAJ_BANK_SELECT, 0);  //Select Bank 0
  for (i = 0; i < Gesture_Array_SIZE; i++) {
    I2C_writeByte(Init_Gesture_Array[i][0], Init_Gesture_Array[i][1]);  //Gesture register initializes
  }

  Serial.println("Booting");

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
  Gesture_Data = 0;
  Gesture_Data = I2C_readU16(PAJ_INT_FLAG1);
  if (Gesture_Data) {
    switch (Gesture_Data) {
      case PAJ_UP:
        Serial.print("Up\r\n");
        ret += 2;  //9  向上
        break;
      case PAJ_DOWN:
        Serial.print("Down\r\n");
        ret += 4;  //10  向下
        break;

      case PAJ_LEFT:
        Serial.print("Left\r\n");
        ret += 2;  //9    向左
        break;
      case PAJ_RIGHT:
        Serial.print("Right\r\n");
        ret += 4;  //10   向右
        break;

      case PAJ_FORWARD:
        Serial.print("Forward\r\n");
        ret = 1;  //8  向前
        break;
      case PAJ_BACKWARD:
        Serial.print("Backward\r\n");
        ret += 8;  //11  向后
        break;

      case PAJ_CLOCKWISE: Serial.print("Clockwise\r\n"); break;
      case PAJ_COUNT_CLOCKWISE: Serial.print("AntiClockwise\r\n"); break;
      case PAJ_WAVE: Serial.print("Wave\r\n"); break;
      default: break;
    }
    if (digitalRead(8) == 0) ret = 1;    //izquierda 左
    if (digitalRead(9) == 0) ret += 2;   //arriba 上
    if (digitalRead(10) == 0) ret += 4;  //abajo 右
    if (digitalRead(11) == 0) ret += 8;  //fuego ？
    return (ret);
  }
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
