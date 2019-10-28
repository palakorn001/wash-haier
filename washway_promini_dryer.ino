// ***  PROJECT INFO ***
/*
* Demo WASHWAY PROJECT //เครื่องซัก
* VERSION: 1.0 (1/10/19)
* BY     : PALAKORN
* EMAIL  : PALAKORN@IDEAMAKERSHUB.NET
*
* MCU    : ARDUINO(ARDUINO PRO MINI)
*
* Output : RELAY QTY 3
* ค่าส่งออก : Relay(HIGH=ON,LOW=OFF)
*
* commit : รับสัญญานจากตัวเกตเวย์เพื่อสั่งงานรีเลย์เครื่องซักผ้า
* ข้อมูลการ: กด ทดสอบอุปกรณ์
* แก้ไข   : กด รีเซ็ตอุปกรณ์ คือ 4 วินาที
*        : เพิ่มฟังซัน bool check_wash
*/

#include <SPI.h>
#include <LoRa.h>

#include <EEPROM.h>
#include <Scheduler.h>
#define LED 13

//ตั้ง Serial
//ตั้งเวลาโปรแกรม 1,2
String SerialNumber = "2201910009";
int t_pro1 = 25;
int t_pro2 = 5;

String BAND = "433"; // ตั้งค่า ยานความถี่รับส่งข้อมูล
String send_statuswash = "";
int buttonState = 0;
int read_test = 0;
int i = 4;
int test_pin = 2;
int delay1 = 3;
int delay2 = 4;
int delay3 = 5;
int reset_pin = 6;
int check_coin = 9;
int wash1 = 7;
int wash2 = 8;
int program_wash1 = 1;
int program_wash2 = 2;
unsigned long statuswash;
int timecheck = 0;

int statuswash1 = 0;
int startwast = 0;
char HP;
//ตัวแปลเวลา
unsigned long currentMillis;
unsigned long previousMillis = 0;
int Second = 0; //วินาที
int Minute = 0; //นาที
const long interval = 1000;
//ตั้งเวลาอบ1
int set_time = 5;
bool run_time = false;

void software_reboot()
{
    asm volatile("  jmp 0");
}

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    Serial.print("SerialNumber =");
    Serial.println(SerialNumber);

    pinMode(LED, OUTPUT);
    pinMode(reset_pin, INPUT);
    pinMode(test_pin, INPUT);
    pinMode(wash1, INPUT);
    pinMode(delay1, OUTPUT);
    pinMode(delay2, OUTPUT);
    pinMode(delay3, OUTPUT);

    digitalWrite(test_pin, HIGH);
    digitalWrite(reset_pin, HIGH);
    digitalWrite(LED, HIGH);
    check_wash();
    if (!LoRa.begin(BAND.toInt() * 1000000))
    {
        Serial.println("Starting LoRa failed!");
        delay(1000);
        software_reboot();
        while (1)
            ;
    } //if
    Serial.println("Lora1 CONNECTED");
    Scheduler.startLoop(loop1);

} //setup

///////////////////////////////////////////////loop 1 reset
void loop()
{
    //INFO LOOP
    /*อ่านค่า จาก pin reset     
   * ถ้ามีการ กดรีเซ็ตให้นับถอยหลัง 8 วิ
   * เพื่อรีเซ็ตอุปกรณ์
   * 
   * อ่านค่าจาก pin test
   * ถ้ามีการกด test ให้อุปกรณ์ทำงานเพื่อทดสอบระบบ
   */
    currentMillis = millis();
    read_test = digitalRead(test_pin);
    if (read_test == 0)
    {
        relay_start(program_wash1);
        delay(10);
        t_pro1 = 1;
        run_time = true;
    }
    delay(2000);
    // relay_start();
} //loop

///////////////////////////////////////////////loop 2 config
void loop1()
{
    onReceive(LoRa.parsePacket());
    delay(10);
    while (run_time != false)
    {
        if (currentMillis - previousMillis >= interval)
        {
            previousMillis = currentMillis;
            Second = Second + 1;
            if (Second >= 60)
            {
                Second = 0;
                Minute = Minute + 1;
            }
        }

        if (Minute >= set_time)
        {
            relay_start(3);
            Serial.println("finish");
            Minute = 0;
            Second = 0;
            run_time = false;
        }
        Serial.print(Second);
        Serial.print(" : ");
        Serial.print(Minute);
        Serial.print(" : ");
        Serial.println(set_time);

        delay(800);
        if (check_wash() == true)
        {
            relay_start(3);
            run_time = false;
            break;
        }
    }
}

//-------------------------------void-------------------------------------------//
void onReceive(int packetSize)
{
    while (packetSize == 0)
        return; // if there's no packet, return ถ้าไม่มีข้อมูลรีเทล
                //  byte handshake = LoRa.read();
    String incoming = "";
    String data1 = "";
    String data2 = "";
    String data3 = "";
    String data4 = "";
    String logid = "";
    int updatband_delay = 0;
    while (LoRa.available())
    {
        incoming += (char)LoRa.read();
    }
    Serial.println(incoming);
    String L2 = incoming.substring(incoming.indexOf("SN=") + 3);
    data2 = L2.substring(0, L2.indexOf(","));
    Serial.print("data2 =");
    Serial.print(data2);
    Serial.println();

    if (check_wash())
    {
        Serial.println("ready");
        send_statuswash = "1111";
        digitalWrite(delay2, LOW);
        statuswash1 = 1;
        startwast = 0;
        if (data2 == SerialNumber)
        {
            Serial.println("handshake == Address");
            LoRa.beginPacket();
            //           LoRa.write(Address);
            LoRa.print("SN=");
            LoRa.print(SerialNumber);
            LoRa.print(",");
            LoRa.print("ST=");
            LoRa.print(send_statuswash);
            LoRa.print(",");
            LoRa.print("LI=");
            LoRa.print("0000");
            LoRa.print(",");
            LoRa.endPacket();

            String L2 = incoming.substring(incoming.indexOf("SN=") + 3);
            data2 = L2.substring(0, L2.indexOf(","));
            Serial.print("data2 =");
            Serial.print(data2);
            Serial.println();

            String L3 = incoming.substring(incoming.indexOf("LI=") + 3);
            data3 = L3.substring(0, L3.indexOf(","));
            Serial.print("Str_log_id =");
            Serial.print(data3);
            Serial.println();

            String L4 = incoming.substring(incoming.indexOf("RO=") + 3);
            data4 = L4.substring(0, L4.indexOf(","));
            Serial.print("PRO =");
            Serial.print(data4);
            Serial.println();

            if (data2 == SerialNumber)
            {
                startwast = 1;
                Serial.println("start");
                if (startwast == 1 & statuswash1 == 1)
                {
                    if (data4 == "1")
                    {
                        relay_start(program_wash1);
                        set_time = t_pro1;
                    }
                    if (data4 == "2")
                    {
                        relay_start(program_wash2);
                        set_time = t_pro2;
                    }
                }
                //delay(1000);
                for (int i = 0; i < 20; ++i)
                {
                    EEPROM.put(100 + i, data3[i]);
                }
                Serial.println("---------- update_eeprom ----------");
            }
            else
            {
                Serial.println("ready");
                startwast = 0;
            }

            delay(100);
        }
    }
    else
    {
        Serial.println("RUN");
        send_statuswash = "2222";
        statuswash1 = 0;
        startwast = 0;
        if (data2 == SerialNumber)
        {
            Serial.println("Reading EEPROM device");
            for (int i = 100; i < 120; ++i)
            {
                logid += char(EEPROM.read(i));
            }
            Serial.print("logid : ");
            Serial.println(logid);
            String L2 = incoming.substring(incoming.indexOf("SN=") + 3);
            data2 = L2.substring(0, L2.indexOf(","));
            Serial.print("data2 =");
            Serial.print(data2);
            Serial.println();

            String L3 = incoming.substring(incoming.indexOf("LI=") + 3);
            data3 = L3.substring(0, L3.indexOf(","));
            Serial.print("Str_log_id =");
            Serial.print(data3);
            Serial.println();

            String l_log = logid.c_str();
            if (data3 == l_log)
            {
                startwast = 1;
                LoRa.beginPacket();
                LoRa.print("SN=");
                LoRa.print(SerialNumber);
                LoRa.print(",");
                LoRa.print("ST=");
                LoRa.print(send_statuswash);
                LoRa.print(",");
                LoRa.print("LI=");
                LoRa.print(logid);
                LoRa.print(",");
                LoRa.endPacket();
                delay(100);
                if (data4 == "1")
                {
                    set_time = t_pro1;
                    Serial.print("set_time : ")
                        Serial.println(set_time);
                }
                if (data4 == "2")
                {
                    set_time = t_pro2;
                    Serial.print("set_time : ")
                        Serial.println(set_time);
                }
                run_time = true; //เพื่อสั่งให้นับเวลาการซัก
            }
            else
            {
                send_statuswash = "3333";
                LoRa.beginPacket();
                LoRa.print("SN=");
                LoRa.print(SerialNumber);
                LoRa.print(",");
                LoRa.print("ST=");
                LoRa.print(send_statuswash);
                LoRa.print(",");
                LoRa.print("LI=");
                LoRa.print(logid);
                LoRa.print(",");
                LoRa.endPacket();
                delay(100);
            }
        }
    }
}
//--------------------------------------------------------------------------//
void relay_start(int range)
{
    switch (range)
    {
    case 1:
        Serial.print("case");
        Serial.println(range);
        digitalWrite(delay1, LOW);
        delay(100);
        digitalWrite(delay1, HIGH);
        delay(1000);
        digitalWrite(delay1, LOW);
        delay(3000);
        digitalWrite(delay3, HIGH);
        delay(500);
        digitalWrite(delay3, LOW);
        delay(1000);
        break;
    case 2:

        Serial.print("case");
        Serial.println(range);
        digitalWrite(delay1, LOW);
        delay(100);
        digitalWrite(delay1, HIGH);
        delay(1000);
        digitalWrite(delay1, LOW);
        delay(3000);
        digitalWrite(delay3, HIGH);
        delay(500);
        digitalWrite(delay3, LOW);
        delay(500);

        break;
    case 3: //stop
        Serial.print("case");
        Serial.println(range);
        digitalWrite(delay3, HIGH);
        delay(500);
        digitalWrite(delay3, LOW);
        delay(1000);
        digitalWrite(delay1, LOW);
        delay(100);
        digitalWrite(delay1, HIGH);
        delay(1000);
        digitalWrite(delay1, LOW);
        delay(10000);
        break;
    }
}

//FUNCTION CHECK WASH
bool check_wash()
{
    /*CHECK WASH
 * ฟังชันการเช็คสถาณะเครื่องซักผ้า
 * ถ้าสถาณะเครื่องซักผ้า พร้อมใช้งานหรือ ปิดอยู่ ให้ค่าเป็นจริง คือ = 1
 * ถ้าสถาณะเครื่องซักผ้า ไม่พร้อม หรือ เปิดอยู่ ให้ค่าเป็นเท็จ คือ = 0
 */
    statuswash = pulseIn(wash1, HIGH, 1000000);
    float wash = statuswash / 100.00;
    Serial.println(wash);
    if (wash == 0)
    {
        // Serial.println("ready");
        // digitalWrite(delay2, LOW);
        delay(1000);
        return true;
    }
    if (wash >= 2)
    {
        // Serial.println("stop");
        delay(1000);
        return false;
    }
}
