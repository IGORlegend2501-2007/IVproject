/*
  Обозначения:
    *data[2] - поворот гироскопом вперёд и назад
    *pixy.getBlocks[0] - мяч
    *
  Задачи:
    *подключение pixy ✅
    *подключение MPU-5060 ✅
    *логика модель pixy
    *логика модель MPU-5060 ✅
    *подключение к CM-530 
    *логика движений 
    *(доп)запоминание объектов
    *расчет врата-мяч
  Доп. материал:
    *работа с гироскопом: https://alexgyver.ru/arduino-mpu6050/#Получение_сырых_данных
    *подключение pixy: https://medium.com/jungletronics/arduino-meets-pixy-36138c474912
    *pixy: 
    *подключение MPU-5060: 
    *порты: https://vk.com/away.php?to=https%3A%2F%2Farduinomaster.ru%2Fdatchiki-arduino%2Fpodklyuchenie-spi-arduino%2F&cc_key=   
*/
#include <SPI.h>  
#include <Pixy.h>
#include "Wire.h"

static int i = 0;
int ziroPosGiro;
uint16_t blocks;
int16_t data[7];  
char buf[32]; 
const int MPU_addr = 0x68;

Pixy pixy;

struct PixyBlockCoords {
  int x;
  int y;
  int width;
  int height;
  int visible;
};

PixyBlockCoords ball;
PixyBlockCoords enemyGoats;

void setup() {  
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);     
  Wire.endTransmission(true);
  
  Serial.begin(9600);
  Serial.print("Starting...\n");
  
  pixy.init();

  getData();
  ziroPosGiro = data[2];
  
  delay(500);
}
void loop() {
  getData();
  if(isFallBack()){
    //Serial.println("Back");
    wakeUpFromBack();
  }
  else if(isFallForward()){
    //Serial.println("Forward");
    wakeUpFromForward();
  }
  else{
    //Serial.println("Good");
    //Serial.println(data[2]);
    if(canKick()){
      kickBall();
    }
    
    coords();
  }
  delay(200);
}
/*__________PIXY____________*/
void coords(){
  blocks = pixy.getBlocks();
  if (blocks)
  {
    ball.visible = true;
    ball.x = pixy.blocks[0].x;
    ball.y = pixy.blocks[0].y;
    ball.width = pixy.blocks[0].width;
    ball.height = pixy.blocks[0].height;
  }  
}
bool canKick(){
  return false;
}
bool isBallForward(){
  return false;
}
bool isBallLeft(){
  return false;
}
bool isBallLeftForward(){
  return false;
}
bool isBallRight(){
  return false;
}
bool isBallRightForward(){
  return false;
}
void searchBall(){
  
}
void printBallCoords(){
  Serial.print("X: ");
  Serial.print(ball.x);
  Serial.print(" Y: ");
  Serial.print(ball.y);
  Serial.print(" Width: ");
  Serial.print(ball.width);
  Serial.print(" Height: ");
  Serial.println(ball.height);
}
/*__________Гироскоп________*/
void getData() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
  for (byte i = 0; i < 7; i++) {
    data[i] = Wire.read() << 8 | Wire.read();
  }
}
bool isFallBack(){
  return data[2] <= -13000 ? true : false;
}
bool isFallForward(){
  return data[2] >= 11000 ? true : false;
}
void printGiro(){
  Serial.println(data[2]);
}
/*__________Движения________*/
void wakeUpFromBack(){
  
}
void wakeUpFromForward(){
  
}
void goForward(){
  
}
void rotateRight(){
  
}
void rotateLeft(){
  
}
void kickBall(){
  
}
