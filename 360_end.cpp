/*BLINKER，小爱同学等库函数*/
#define BLINKER_PRINT Serial
#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT
#include <EEPROM.h>
#include <SPI.h>
#include <Blinker.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <Ticker.h>
#include <EEPROM.h>
#define mySerial Serial

Ticker Close_Root_Task;
Servo myServo;
/**********************/

/************************配置******************************/
char auth[] = ""; //点灯密钥
char ssid[] = "";       // WiFi名称
char pswd[] = "";     // WiFi密码

const char *AP_ssid = "";         //要创建的热点名
const char *AP_password = ""; //创建的热点密码

#define RST_PIN 5                 // 配置rst针脚
#define SS_PIN 4                  // 配置ss针脚
MFRC522 mfrc522(SS_PIN, RST_PIN); // 创建新的RFID实例

/***************BLINKER新建组件对象*****************/
BlinkerButton Button1("door");
BlinkerButton Button2("on");
BlinkerButton Button3("off");
BlinkerNumber Number1("num-abc");

int counter = 0;
/*****************************************************/

/*************************RFID数据**************************/
bool read_a_card_flag = 0; //卡片刷入标志位
bool Door_Open_Flag = 0;   //开门标志位
bool Root_Model = 0;       // 管理模式标志位

#define RST_PIN 5 // 配置rst针脚
#define SS_PIN 4  // 配置ss针脚
MFRC522 rfid(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key;

byte nuidPICC[4];
#define Max_User_Sum 10                                        //最大用户数，不包括管理员
byte UserUid[Max_User_Sum * 4 + 4] = {0xXX, 0xXX, 0xXX, 0xXX}; //用于存放用户的ID,前4个字节不变为管理员ID卡
byte User_Sum = 1;                                             //当前用户的总数

void printHex(byte *buffer, byte bufferSize) //用户ID转换类型
{
    for (byte i = 0; i < bufferSize; i++)
    {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

/*****************************************************/

/*********网页服务，网页代码***********/
ESP8266WebServer server(80);
String html =
    //"<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>4706</title></head><body> <a href=\"./pin?door=on\"> <input type=\"button\"style=\"width: 1000px;height:500px;font-size:200px;\"value=\"&#x5F00;&#x95E8;\"></a><br /><br /><br /></body></html>";
    "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><title>4706</title></head><body> <button onclick=\"window.location.href='./pin?door=on'\"style=\"width:1000px;height:500px;font-size:200px;\">&#x5F00;&#x95E8;</button><br /></body></html>";
//注意，为达到精确控制，开门，开灯关灯过程中，CPU都会被占用，请勿重复按开门，否则会开两次

/*****************************开门****************************/
void door()
{
  for (int i = 0; i < 200; i++)
  { //拉门，产生PWM，一个单位为0.0105秒，让舵机顺时针转
    digitalWrite(0, HIGH);
    delayMicroseconds(500);
    digitalWrite(0, LOW);
    delayMicroseconds(2000);
    delay(10);
  }

  for (int i = 0; i < 50; i++)
  { //产生PWM，50为持续时间，大约为0.575秒，让舵机停止
    digitalWrite(0, HIGH);
    delayMicroseconds(1500);
    digitalWrite(0, LOW);
    delay(10);
  }

  for (int i = 0; i < 190; i++)
  { //门恢复，产生PWM，一个单位为0.0125秒，让舵机逆时针转
    digitalWrite(0, HIGH);
    delayMicroseconds(2500);
    digitalWrite(0, LOW);
    delay(10);
  }

  for (int i = 0; i < 50; i++)
  { //产生PWM，50为持续时间，大约为0.575秒，让舵机停止
    digitalWrite(0, HIGH);
    delayMicroseconds(1500);
    digitalWrite(0, LOW);
    delay(10);
  }
    Door_Open_Flag = 0;
}
/************************************************************/

/****蜂鸣器长响一声****/
void beer_one()
{
    digitalWrite(16, LOW);
    delay(500);
    digitalWrite(16, HIGH);
}
/*********************/

/****蜂鸣器长响两声****/
void beer_two()
{
    digitalWrite(16, LOW);
    delay(150);
    digitalWrite(16, HIGH);
    delay(150);
    digitalWrite(16, LOW);
    delay(150);
    digitalWrite(16, HIGH);
}
/*********************/

/****蜂鸣器响三声****/
void beer_three()
{
    digitalWrite(16, LOW);
    delay(150);
    digitalWrite(16, HIGH);
    delay(150);
    digitalWrite(16, LOW);
    delay(150);
    digitalWrite(16, HIGH);
    delay(150);
    digitalWrite(16, LOW);
    delay(150);
    digitalWrite(16, HIGH);
}
/******************/

/******************************************网页传送回来的代码判断处理****************************/
void pin()
{
    if (server.arg("door") == "on")
    {
        server.send(200, "text/html", html);
        beer_one();
        door();
    }
}
const int led = 13;

void handleRoot()
{
    server.send(200, "text/html", html);
}

void handleNotFound()
{
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
}
/*************************************************************************************************/

/**************BLINKER按下按键即会执行该函数***************/
void button1_callback(const String &state)
{
    BLINKER_LOG("get button state: ", state);
    beer_one();
    door();
}
/********************************************************/

/*BLINKER如果未绑定的组件被触发，则会执行其中内容*/
void dataRead(const String &data)
{
    BLINKER_LOG("Blinker readString: ", data);
    counter++;
    Number1.print(counter);
}
/**********************************************/
/***小爱电源类回调***/
void miotPowerState(const String &state)
{
    BLINKER_LOG("need set power state: ", state);

    if (state == BLINKER_CMD_ON)
    {
        BlinkerMIOT.powerState("on");
        BlinkerMIOT.print();
        door();
    }
    else if (state == BLINKER_CMD_OFF)
    {
        BlinkerMIOT.powerState("off");
        BlinkerMIOT.print();
        door();
    }
}
/*******************/

/**********************小爱模式回调*******************/
void miotMode(uint8_t mode)
{
    BLINKER_LOG("need set mode: ", mode);

    BlinkerMIOT.mode(mode);
    BlinkerMIOT.print();
    if (mode == BLINKER_CMD_MIOT_DAY)
    {
        // Your mode function
    }
    else if (mode == BLINKER_CMD_MIOT_NIGHT)
    {
        // Your mode function
    }
    else if (mode == BLINKER_CMD_MIOT_COLOR)
    {
        // Your mode function
    }
    else if (mode == BLINKER_CMD_MIOT_WARMTH)
    {
        // Your mode function
    }
    else if (mode == BLINKER_CMD_MIOT_TV)
    {
        beer_one();
        door();
    }
    else if (mode == BLINKER_CMD_MIOT_READING)
    {
        // Your mode function
    }
    else if (mode == BLINKER_CMD_MIOT_COMPUTER)
    {
        // Your mode function
    }

    //    wsMode = mode;
}
/*******************/

/***********************RFID读卡自定义函数***********************/

void RFID_init() //初始化读卡
{
    SPI.begin();     // Init SPI bus
    rfid.PCD_Init(); // Init MFRC522
    Serial.println("初始化读卡");
}

void RFID_read() //读卡并判断
{
    read_a_card_flag = 0; //读卡标志位0

    if (rfid.PICC_IsNewCardPresent())
    {
        if (rfid.PICC_ReadCardSerial())
        {
            read_a_card_flag = 1; //读卡标志位置1 有卡片刷入
            Serial.print(F("ID: "));
            printHex(rfid.uid.uidByte, rfid.uid.size);
            rfid.PICC_HaltA();      //停止 PICC
            rfid.PCD_StopCrypto1(); //停止加密PCD
        }
    }

    if (1 == read_a_card_flag) //有卡片刷入
    {
        uint8_t tempValidation = 0;
        tempValidation = Validation_ID(User_Sum, UserUid); //验证用户ID 返回0不通过 返回1通过 返回2管理员卡

        if (0 == Root_Model) //不处于管理模式
        {

            if (0 != tempValidation) //不为0通过
            {
                Door_Open_Flag = 1; //开门
                beer_one();         // led亮一次
                Serial.println(F("  wellcome "));
            }
            else
            {
                beer_three(); // led亮三次
                Serial.println(F("  no "));
            }
        }

        if (0 == Root_Model && 2 == tempValidation) //不处于管理模式&&有管理员卡片刷入>>开启卡片写入
        {
            Root_Model = 1;                                  //管理模式打开
            tempValidation = 3;                              //卡片标志位为3，跳过关闭管理模式的判断条件
            Close_Root_Task.once_scheduled(300, Close_Root); //添加完用户后5分钟自动关闭管理员模式
            beer_two();
            Serial.println(F("root_open"));
        }

        if (1 == Root_Model && 2 == tempValidation) //处于管理模式&&有管理员卡片刷入>>关闭卡片写入
        {
            Root_Model = 0; //管理模式关闭
            Close_Root();   //关闭管理员模式
            Serial.println(F("root_write"));
            Serial.println(F("root_close"));
        }

        if (1 == Root_Model && 0 == tempValidation) //处于管理模式&&有不通过卡片刷入>>添加新用户
        {
            Add_User(User_Sum, UserUid, rfid.uid.uidByte);
            Close_Root_Task.once_scheduled(300, Close_Root); //添加完用户后5分钟自动关闭管理员模式
            User_Sum++;                                      //用户数量增加
            if (Max_User_Sum < User_Sum)                     //增加用户数量未超过最大数量
            {
                User_Sum = Max_User_Sum; //用户数量为用户总数
            }
            Serial.printf("add_new_user   user_sum %d\n", User_Sum);
        }
    }

    if (1 == Door_Open_Flag) //开门
    {
        door();
    }
}

//验证ID
//返回1 通过
//返回2 管理员卡
uint8_t Validation_ID(byte user_sum, const byte user_uid[])
{
    if (Max_User_Sum <= user_sum)
    {
        user_sum = Max_User_Sum;
    }
    if (1 > user_sum)
    {
        user_sum = 1;
    }

    for (uint8_t i = 0; i <= user_sum; i++)
    {
        if (user_uid[i * 4 + 0] == rfid.uid.uidByte[0])
        {
            if (user_uid[i * 4 + 1] == rfid.uid.uidByte[1] &&
                user_uid[i * 4 + 2] == rfid.uid.uidByte[2] &&
                user_uid[i * 4 + 3] == rfid.uid.uidByte[3])
            {
                if (0 == i)
                {
                    return 2;
                }

                return 1; //验证通过
            }
        }
    }
    return 0;
}

void Close_Root() //关闭管理员模式，保存数据到flash
{
    EEPROM.begin(3000); // Blinker 已经使用了0-2447
    Root_Model = 0;
    for (uint8_t i = 4; i < Max_User_Sum * 4 + 4; i++)
    {
        EEPROM.write(i - 4 + 2500, UserUid[i]);
    }
    EEPROM.write(Max_User_Sum * 4 + 4 + 2500, User_Sum);
    EEPROM.end();
    Serial.printf("  %d\n", User_Sum);
}

void Add_User(byte user_sum, byte user_uid[], const byte new_user[4]) //添加新用户
{
    if (Max_User_Sum <= user_sum)
    {
        user_sum = Max_User_Sum;
    }
    if (1 > user_sum)
    {
        user_sum = 1;
    }
    user_uid[user_sum * 4 + 0] = new_user[0];
    user_uid[user_sum * 4 + 1] = new_user[1];
    user_uid[user_sum * 4 + 2] = new_user[2];
    user_uid[user_sum * 4 + 3] = new_user[3];
}

/******************************************************/

void setup()
{
    Serial.begin(115200); //波特率
    RFID_init();          //初始化读卡
    delay(500);
    myServo.attach(0, 544, 2400); //初始化舵机

    pinMode(16, OUTPUT);   // led IO
    digitalWrite(16, HIGH); // led IO

    /*********读取EEPROM索引的值********/

    EEPROM.begin(3000); // Blinker 已经使用了0-2447
    for (uint8_t i = 4; i < Max_User_Sum * 4 + 4; i++)
    {
        UserUid[i] = EEPROM.read(i - 4 + 2500);
    }
    User_Sum = EEPROM.read(Max_User_Sum * 4 + 4 + 2500);
    EEPROM.end();
    if (1 > User_Sum)
    {
        User_Sum = 1;
    }
    else if (Max_User_Sum < User_Sum)
    {
        User_Sum = 1;
    }
    Serial.printf("  %d", User_Sum);

    /***********************************/

    /***************初始化网页******************/
    server.on("/", handleRoot);
    server.on("/pin", HTTP_GET, pin);
    server.on("/inline", []()
              { server.send(200, "text/plain", "this works as well"); });
    server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP server started");
    /******************************************/

    /******************初始化blinker********************/
#if defined(BLINKER_PRINT)
    BLINKER_DEBUG.stream(BLINKER_PRINT);
#endif

    Blinker.begin(auth, ssid, pswd);
    Blinker.attachData(dataRead);
    Button1.attach(button1_callback);
    // Button2.attach(button2_callback);
    // Button3.attach(button3_callback);
    BlinkerMIOT.attachPowerState(miotPowerState); //小爱调节电源
    BlinkerMIOT.attachMode(miotMode);
    Blinker.setTimezone(8.0);
    /****************************************************/

    /***********初始化AP设置**********/
    WiFi.begin();
    Serial.println();
    Serial.print("Setting soft-AP ... ");

    IPAddress softLocal(192, 168, 1, 1);    // IP
    IPAddress softGateway(192, 168, 1, 1);  //网关
    IPAddress softSubnet(255, 255, 255, 0); //子网掩码
    WiFi.softAPConfig(softLocal, softGateway, softSubnet);
    WiFi.softAP(AP_ssid, AP_password);

    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    /**********************************/
}
void loop()
{
    Blinker.run();
    server.handleClient();
    RFID_read(); //读卡并做分析处理
}
