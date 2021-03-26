/*  ### 수분 자동 공급 IoT 화분 ###
 *  1. WiFi 연결을 통해 수분의 데이터를 기록하고 웹 브라우저 또는 전용 서버를 통해 데이터를 확인 할 수 있는 코드입니다.
 *  2. WiFi 연결이 끊겨도 수분의 자동 공급 기능은 문제없이 작동합니다.
 *  3. 사용하고자 하는 OLED 종류나 OLED 드라이버가 따로 있다면 해당 부분의 코드를 수정해 주세요.
 *  4. 아래 코드를 (#A 부분) 테스트 해 보신 후 펌프를 작동하게 할 센서 값을 적절하게 변경하여 사용하시면 됩니다.  
 *  5. '[파일] - [환경설정]' 메뉴 최 하위의 [추가 보드 매니저 URLs] 부분에 'http://arduino.esp8266.com/stable/package_esp8266com_index.json' 입력
 */

/*
  * 1. This is a code that allows you to record minutes of data through a WiFi connection and check the data through a web browser or a dedicated server.
  * 2. Even if the WiFi connection is disconnected, the automatic irrigation system works without any problems.
  * 3. If there is a separate OLED type or OLED driver you want to use, please modify the code of the corresponding part.
  * 4. After testing the code below (#A part), you can change the sensor value to operate the pump appropriately and use it.
  * 5. Enter 'http://arduino.esp8266.com/stable/package_esp8266com_index.json' in the [Additional Board Manager URLs] section at the bottom of the '[File] - [Preferences]' menu.
*/

#include <ESP8266WiFi.h>           // 정확한 라이브러리를 설치 후에도 이 부분에서 에러가 나면, ESP8266 환경설정 추가. 보드를 D1 mini (혹은 ESP8266 계열) 로 선택했는지 확인합니다.
#include <WiFiUdp.h>
#include "mySSID.h"
#include "Images.h"
#include "SSD1306Wire.h"           // I2C 용 OLED 라이브러리를 사용합니다.
#include "DHT.h"

#define DHTPIN D5     // what digital pin we're connected to.
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

SSD1306Wire  display(0x3c, D2, D1);  // esp8266-01 과 OLED를 연결할 경우, GPIO (SDA, SCK) 포트 번호
//SSD1306Wire  display(0x3c, 4, 5);  // D1 mini 와 OLED를 연결할 경우, GPIO (SDA, SCL) 포트 번호

String apiKey = thingSpeak;  // ThingSpeak의 Write API Key 입력 (사이트 가입 후 받은 API Key 를 직접 입력) 
// mySSID.h 파일로 가서 접속하려는 WiFi SSID 와 Password 로 변경해주세요! 
char ssid[] = mySSID;  // 또는 이 곳에 WiFi SSID 를 직접 입력하셔도 됩니다.
char pass[] = myPASS;  // WiFi Password 입력.
const char* server = "api.thingspeak.com";
WiFiClient client;    // client 시작하기

int motorA1 =  D6;   // L9110S 모듈 관련 변수 선언
int motorA2  = D7;   // L9110S 모듈 관련 변수 선언
int sensorVal = A0;


DHT dht(DHTPIN, DHTTYPE);
float humidity = 0.;
float temperature = 0.;

void setup(){
    display.init();    // 초기화
    display.clear();   // 기존 표시되었던 내용을 제거 (초기화) 합니다. 
    display.flipScreenVertically();  // OLED 상하 플립 (뒤집기).
    display.drawRect(0, 0, 128, 64);  //  사각형 테두리를 그립니다.
    display.setFont(ArialMT_Plain_16);  // ArialMT_Plain 폰트는 10, 16, 24 세 가지 크기로 설정하실 수 있습니다.
//    display.setTextAlignment(TEXT_ALIGN_CENTER);   // LEFT , RIGHT 등의 글자 정렬 옵션이 있습니다.  
    display.drawString(5, 10, "Auto Flower Pot");    // (5, 10) 의 좌표에 "...." 내용을 표시합니다.)    
    display.setFont(ArialMT_Plain_10);  // ArialMT_Plain 폰트는 10, 16, 24 세 가지 크기로 설정하실 수 있습니다.  
           
    display.drawString(11, 48, "(c) happybono 2021");         
    display.display();  // 이 함수를 사용해야 바로 위의 drawString(....) 함수로 표시한 내용이 OLED 화면으로 최종적으로 출력됩니다.
    delay(3500);        // 3.5 초 대기합니다.
    
    Serial.begin(9600);
    WiFi.begin(ssid, pass);   // 위에서 입력해 둔 ID 와 Password 을 이용해서 Wi-Fi 에 접속 시도 합니다.
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print("*");
      display.clear();
      display.drawString(11, 48, "Connecting to WiFi...");               
      display.display(); 
      delay(6000);
      break;
    }
    pinMode( motorA1 , OUTPUT); 
    pinMode( motorA2 , OUTPUT);   
    dht.begin();
    display.clear();
}

void loop () {
//    Serial.print("Asensor = ");
//    Serial.println(analogRead(A0));  // 0 (건조) ~ 100 (습함) 값 출력    
       humidity = dht.readHumidity();
       // Read temperature as Fahrenheit (isFahrenheit = true)
     temperature = dht.readTemperature();
    sensorVal = map(analogRead(A0), 450, 1023, 100, 0);  // 토양센서 측정범위 확인 후 0 ~ 100 사이의 값으로 맵핑 값을 저장합니다.
    display.drawXbm(0, 0, flowerPot_W, flowerPot_H, flowerPot);
    display.setFont(ArialMT_Plain_10);  // ArialMT_Plain 폰트는 10, 16, 24 세 가지 크기로 설정하실 수 있습니다.  
    display.drawString(50, 17, String(temperature) + "°C"); 
    display.drawString(93, 17, String(humidity) + "%"); 
    display.setFont(ArialMT_Plain_16);  // ArialMT_Plain 폰트는 10, 16, 24 세 가지 크기로 설정하실 수 있습니다.
    display.drawString(50, 0, "Pot :");
    display.drawString(90, 0, String(sensorVal));  

    if (sensorVal < 30) {   //  자신의 화분 상태와 화분의 종류에 따라, 물 펌프가 작동 되고자 하는 값으로 변경하세요.
      display.setFont(ArialMT_Plain_16);  // ArialMT_Plain 폰트는 10, 16, 24 세 가지 크기로 설정하실 수 있습니다.
//      Serial.print("The flower pot is very dry.");
//      Serial.print("Start Watering...");
      digitalWrite(motorA1, HIGH);
      digitalWrite(motorA2, LOW);
      display.drawString(50, 30, "Pot Dry!");
      display.drawString(50, 45, "Pumping...");
      display.display();
      delay(10);    
    } else {
      digitalWrite(motorA1, LOW);
      digitalWrite(motorA2, LOW);
//      Serial.print("There is enough moisture in the flowerpot.");
//      Serial.print("Watering Stopped...")
      display.drawString(50, 30, "Pot OK!");
      display.drawString(50, 45, "Sleeping...");
      display.display();
      delay(10);
    }
    display.clear();

    // 아래 내용은 ThingSpeak 에서 데이터를 받아서 처리하기 위한 정해진 (요구하는) 형식이기 때문에, 오타가 발생하지 않도록 하는 것이 좋습니다.
    if (client.connect(server,80))   //   "184.106.153.149" 또는 api.thingspeak.com 으로 접속을 시도합니다.
    {  
       String sendData = apiKey+"&field1="+String(sensorVal)+"&field2="+String(temperature)+"&field3="+String(humidity)+"\r\n\r\n";      
       //Serial.println(sendData);
       client.print("POST /update HTTP/1.1\n");
       client.print("Host: api.thingspeak.com\n");
       client.print("Connection: close\n");
       client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
       client.print("Content-Type: application/x-www-form-urlencoded\n");
       client.print("Content-Length: ");
       client.print(sendData.length());
       client.print("\n\n");
       client.print(sendData);
       delay(10);
    }
}
