// include the library code:
#include <LiquidCrystal.h> // 液晶屏库
#include <SCoop.h>         // 多线程库
#include <IRremote.h>      // 红外遥控器库
                           // 文档：https://github.com/z3t0/Arduino-IRremote/wiki

// Define pins
// 定义引脚
int PIN_TRIG = 8;
int PIN_ECHO = 9;
int light = 7;
int RECV_PIN = 10;

// Variable declarations
// 声明变量
unsigned int threshold = 64;
bool stateIR = true;
bool stateDetection = false;
int distance;
int counter = 0;
long keyPress;
long keyHold;
bool isKeyHold = false;
bool manualMode = false;
int showMode;

// Define keys
// 保存遥控器数据用于比对
const long keyPower = 0xFFA25D;
const long keyMenu = 0xFFE21D;
const long keyPlus = 0xFF02FD;
const long keyMinus = 0xFF9867;
const long hold = 0xFFFFFFFF;

// Initialize the library with the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
IRrecv irrecv(RECV_PIN);

// Decode IR signals
decode_results results;

// Function declarations
// 定义函数
bool detectDistance() // 检测距离函数，返回距离，单位厘米
{
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  distance = (pulseIn(PIN_ECHO, HIGH) * 17) / 1000;
  if (distance < threshold)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void displayInf() // 更新显示屏函数，无返回值
{
  lcd.home();
  lcd.print("Distance:");
  lcd.print(distance);
  lcd.print("cm   ");
  lcd.setCursor(0, 1);
  lcd.print("Threshold:");
  lcd.print(threshold);
  lcd.print("cm   ");
}

// Multithread
// 多线程
defineTask(taskDisplayInfo);
defineTask(IRremote);
defineTask(toggleLight);
// 这么几个线程就占了好多内存啊...

void IRremote::setup()
{
  irrecv.enableIRIn();
}
void IRremote::loop()
{
  if (irrecv.decode(&results))
  {
    Serial.println(results.value);
    switch (results.value)
    {
    case (hold):
      keyPress = keyHold;
      break;
    case (keyPower):
      keyPress = results.value;
      showMode = 1;
      break;
    case (keyMenu):
      keyPress = results.value;
      counter = 0;
      manualMode = !manualMode;
      Serial.println(manualMode);
      showMode = 2;
      break;
    default:
      keyPress = results.value;
      keyHold = results.value;
      break;
    }
    switch (keyPress)
    {
    case (keyPower):
      stateIR = !stateIR;
      break;
    case (keyPlus):
    if (threshold <= 400) {
      threshold++;
    }
      break;
    case (keyMinus):
    if (threshold > 0) {
      threshold--;
    }
      break;
    default:
      break;
      keyPress = 0;
    }
  }
  else
  {
    keyPress = 0;
    keyHold = 0;
  }
  irrecv.resume();
  delay(240);
}

void toggleLight::setup()
{
  stateDetection = detectDistance();
}
void toggleLight::loop() // 检测各种条件来决定灯是否开关
{
  if (!manualMode)
  {
    while (counter <= 5)
    {
      if (stateDetection == detectDistance())
      {
        counter = counter + 1;
      }
      else
      {
        counter = 0;
        stateDetection = detectDistance();
      }
      sleep(160);
    }
    while (counter > 5)
    {
      if (stateDetection == true && stateIR == true)
      {
        digitalWrite(light, HIGH);
      }
      else
      {
        digitalWrite(light, LOW);
      }
      sleep(720);
      if (stateDetection != detectDistance())
      {
        counter = 0;
        break;
      }
    }
  }
  else
  {
    switch (stateIR)
    {
    case 1:
      digitalWrite(light, HIGH);
      break;
    case 0:
      digitalWrite(light, LOW);
      break;
    }
  }
}

void taskDisplayInfo::setup()
{
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(light, OUTPUT);
  lcd.begin(16, 2);
}
void taskDisplayInfo::loop() // 不断更新屏幕
{
  switch (showMode)
  {
  case 2:
    lcd.clear();
    switch (manualMode)
    {
    case 1:
      lcd.print("Manual Mode");
      break;
    case 0:
      lcd.print("Auto Mode");
      break;
    }
    showMode = 0;
    sleep(960);
    break;
  case 1:
    lcd.clear();
    switch (stateIR)
    {
    case 1:
      lcd.print("On");
      break;
    case 0:
      lcd.print("Off");
      break;
    }
    showMode = 0;
    sleep(960);
    break;
    default:
    break;
  }
detectDistance();
displayInf();
sleep(80);
}

// Multithread start
// 开始运行多线程，无需改动
void setup()
{
  mySCoop.start();
}

void loop()
{
  yield();
}
