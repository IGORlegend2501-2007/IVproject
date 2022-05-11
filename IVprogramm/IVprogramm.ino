  /*
  Обозначения:
    *data[2] - поворот гироскопом вперёд и назад
    *pixy.getBlocks[0] - мяч
    *
  Задачи:
    *подключение pixy ✅
    *подключение MPU-5060 ✅
    *логика модель pixy 90%
    *логика модель MPU-5060 ✅
    *подключение к CM-530 ✅
    *логика движений 50%
    *(доп)запоминание объектов
    *расчет врата-мяч
    *расмотрение ворот
  Доп. материал:
    *работа с гироскопом: https://alexgyver.ru/arduino-mpu6050/#Получение_сырых_данных
    *подключение pixy: https://medium.com/jungletronics/arduino-meets-pixy-36138c474912
    *pixy: 
    *подключение MPU-5060: https://stemsnab.ru/blogs/uroki/peredacha-dannyh-na-cm530-cherez-rxtx-porty
    *порты: https://vk.com/away.php?to=https%3A%2F%2Farduinomaster.ru%2Fdatchiki-arduino%2Fpodklyuchenie-spi-arduino%2F&cc_key=   
*/
#include <SPI.h>  
#include <Pixy.h>
#include "Wire.h"

static int leftGoat = -100,
           rightGoat = 100,
           bottomGoat = -100;
static int zFallForward = 12000,
           zFallBack = -12000;
static int i = 0;
int wait = 20;
int ziroPosGiro;
uint16_t blocks;
int16_t data[7];  
char buf[32]; 
const int MPU_addr = 0x68;

/*Массивы для контроля CM-530*/
const uint8_t  moveFoward[6]  = {0xAA, 0x01, 0x0E,
                                 0xAA, 0x00, 0x1E};
const uint8_t* moveFoward_ptr = moveFoward;
const uint8_t  moveLeft[6]    = {0xAA, 0x04, 0x5E,
                                 0xAA, 0x00, 0x1E};
const uint8_t* moveLeft_ptr   = moveLeft;
const uint8_t  moveRight[6]   = {0xAA, 0x08, 0x1E,
                                 0xAA, 0x00, 0x1E};
const uint8_t* moveRight_ptr  = moveRight;
const uint8_t  moveBack[6]    = {0xAA, 0x02, 0x7E, 
                                 0xAA, 0x00, 0x1E};
const uint8_t* moveBack_ptr   = moveBack;

// Slide to left and right
const uint8_t  slideLeft[12]  = {0xAA, 0x24, 0x5E, 
                                 0xAA, 0x00, 0x1E};
const uint8_t* slideLeft_ptr  = slideLeft;
const uint8_t  slideRight[12] = {0xAA, 0x28, 0x1E, 
                                 0xAA, 0x00, 0x1E};
const uint8_t* slideRight_ptr = slideRight;

// Look down and up
const uint8_t  wakeUpFromBack[6]   = {0xAA, 0x40, 0x1E, 
                                 0xAA, 0x00, 0x1E};
const uint8_t* wakeUpFromBack_ptr   = wakeUpFromBack;
const uint8_t  wakeUpFromForward[6] = {0xAA, 0x10, 0x1E, 
                                 0xAA, 0x00, 0x1E};
const uint8_t* wakeUpFromForward_ptr = wakeUpFromForward;


// Kick action
const uint8_t  kickAction[12] = {0xAA, 0x00, 0x0F, 
                                 0xAA, 0x00, 0x1E};
const uint8_t* kickAction_ptr = kickAction;
/*конец массивов*/

// FSM States for robot
#define ROBO_FSM_FIND     0 // State for finding the ball
#define ROBO_FSM_WALK     1 // State for walking towards the ball
#define ROBO_FSM_POSITION 2 // State for aligning robot with ball
#define ROBO_FSM_KICK     3 // State for kicking the ball

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
  delay(2000);
  
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);     
  Wire.endTransmission(true);
  
  Serial.begin(57600);
  Serial.begin(57600);
  Serial.print("Starting...\n");
  
  pixy.init();

  getData();
  ziroPosGiro = data[2];
  
  delay(2000);
}
void loop() {
  getData();
  if(isFallBack()){
    WakeUpFromBack();
  }
  else if(isFallForward()){
    WakeUpFromForward();
  }
  else{
    coords();        
    if(canKick()){//нужно показание ворот
      KickBall();
    }
    else if(isBallForward()){
      GoForward();
    }
    else if(isBallLeft()){
      SlideLeft();
    }
    else if(isBallLeftForward()){
      RotateLeft();
      delay(1000);
      GoForward();
    }
    else if(isBallRight()){
      SlideRight();
    }
    else if(isBallRightForward()){
      RotateRight();
      delay(1000);
      GoForward();
    }
    else{
      searchBall();
    }
  }
  delay(wait);
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
  return ball.x >= leftGoat && ball.x <= rightGoat && ball.y <= bottomGoat ? true : false;
}
bool isBallForward(){
  return ball.x >= leftGoat && ball.x <= rightGoat && ball.y >= bottomGoat ? true : false;
}
bool isBallLeft(){
  return ball.x <= leftGoat && ball.y <= bottomGoat ? true : false;
}
bool isBallLeftForward(){
  return ball.x <= leftGoat && ball.y >= bottomGoat ? true : false;
}
bool isBallRight(){
  return ball.x >= rightGoat && ball.y <= bottomGoat ? true : false;
}
bool isBallRightForward(){
  return ball.x >= rightGoat && ball.y >= bottomGoat ? true : false;
}
bool isGoatForward(){
  return false;
}
bool isGoatLeft(){
  return false;
}
bool isGoatRight(){
  return false;
}
void searchBall(){
  //поворачивать шеей
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
  return data[2] <= zFallBack ? true : false;
} 
bool isFallForward(){
  return data[2] >= zFallForward ? true : false;
}
void printGiro(){
  Serial.println(data[2]);
}
/*__________Движения________*/
void WakeUpFromBack(){
  //Serial.println("fall Back");
  Serial.write(wakeUpFromBack_ptr, 3);
  delay(210);
  Serial.write(wakeUpFromBack_ptr, 3);
}
void WakeUpFromForward(){
  Serial.println("fall Forward");
  Serial.write(wakeUpFromForward_ptr, 3);
  delay(210);
  Serial.write(wakeUpFromForward_ptr, 3);
}
void GoForward(){
  Serial.write(moveFoward_ptr, 3);
  delay(210);
  Serial.write(moveFoward_ptr+3, 3);
}
void GoBack()
{
  Serial.write(moveBack_ptr, 3);
  delay(210);
  Serial.write(moveBack_ptr+3, 3);
}
void RotateRight(){
  Serial.write(moveRight_ptr, 3);
  delay(210);
  Serial.write(moveRight_ptr+3, 3);
}
void RotateLeft(){
  Serial.write(moveLeft_ptr, 3);
  delay(210);
  Serial.write(moveLeft_ptr+3, 3);
}
void KickBall(){
  Serial.write(kickAction_ptr, 3);
  delay(210);
  Serial.write(kickAction_ptr+3, 3);
}
void SlideLeft()
{
  Serial.write(slideLeft_ptr, 3);
  delay(210);
  Serial.write(slideLeft_ptr+3, 3);
}
void SlideRight()
{
  Serial.write(slideRight_ptr, 3);
  delay(210);
  Serial.write(slideRight_ptr+3, 3);
}
