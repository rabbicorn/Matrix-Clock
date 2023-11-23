// continue@nate.com
// 20220424 2x20 시계 8x8 40개
// 20220424 OTA 추가
// 20220424 초 작은폰트
// 20220427 소스정리
// 20220703 ESP32 소스변환  doit esp32 devkit v1
// 20220706 server ota 적용
// =====================================================================================

//  설정

// =====================================================================================



#define DEBUG           1     // 출력모니터링
#define USE_DHT         1     // 온습도계
#define USE_WIFI        1     // 와이파이
#define USE_CDS         1     // CDS
#define USE_MOTION      1     // 모션센서
#define USE_WEATHER     1     // 오픈웨더맵 현재날씨

// -------------------------------------------------------------------------------------




// =====================================================================================
//  DEBUG
// =====================================================================================

#if DEBUG
#define PRINT(s, x) { Serial.print(F(s)); Serial.println(x); }
#define PRINTS(x) Serial.println(F(x))
#define PRINTX(s, x) { Serial.print(F(s)); Serial.println(x, HEX); }
#else
#define PRINT(s, x)
#define PRINTS(x)
#define PRINTX(s, x)
#endif

// 아래 {}괄호안의 문장들 줄내리면 에러남
#define mprt(x) {P.displayClear();P.displayZoneText(ZONE_LOWER0, x, PA_CENTER, SCROLL_SPEED, 0, PA_PRINT, PA_NO_EFFECT);P.displayAnimate();}
// -------------------------------------------------------------------------------------


// =====================================================================================

//  라이브러리

// =====================================================================================

// 시간
#include <time.h>

// 도트메트릭스
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#if USE_WIFI
// 와이파이 & 웹서버
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#endif

// OTA
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP32httpUpdate.h>


#if USE_DHT
#include "DHT.h"
#endif



#if USE_WEATHER
//#include <ArduinoJson.h>
#endif

#include "Font_Data.h"

// -------------------------------------------------------------------------------------





// =====================================================================================
//  설정 ( * 필요시 수정 하세요 )
// =====================================================================================

// 도트메트릭스 설정
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES  40


#define DATA_PIN  23 //13 //D7
#define CS_PIN    5  //15 // D8
#define CLK_PIN   18 //14 // D5
//esp32
//DIN  GPIO23
//CS  GPIO5
//CLK GPIO18

#if USE_MOTION
// 동작 감지
#define   MOV_PIN   2       //  D4
#endif



#if USE_DHT
// 온도센서 핀
#define   DHTPIN    4       //12      //  D6

// 온도센서 종류

#define   DHTTYPE   DHT22   //  DHT11, DHT21, DHT22 ..

#endif

#define   CDSPIN    34 

#if USE_WIFI

// 와이파이 이름 설정
const char* APNAME   = "espClock_00100";
const char* APPW   = "";
#endif
//OTA
const char* otahost = "ESP32-2x20clock-webota";

// 문자열 가져오는 주소
const char* host = "www.iothub.co.kr";
const uint8_t port = 80;

String url = "/btc_price.php";


// 순차적으로 표시될 문자들
char msg[][100] =
{
  "Continue Clock v1",
  "Knowledge is power.",  //아는 것이 힘이다.
  "No pain no gain.",  //고통 없이는 얻는 것도 없다.
  "Too early to stop.", //멈추기엔 너무 이르다.
  "Time goes now.,"//시간은 지금도 간다.
  "Easier said than done.", //말이란 행동보다 쉬운 법..
  "Asking costs nothing.", //질문은 비용이 들지 않는다.
  "Pain past is pleasure.", //지나간 고통은 쾌락이다.
  "Never too old to learn.", //배움에 늦은 나이는 결코 없다.
  "Better late then never." //늦어도 하지 않는 것보다는 낮다.
};

// -------------------------------------------------------------------------------------









// =====================================================================================

//  변수 초기화

// =====================================================================================



// 디스플레이 변수 설정

#define ZONE_LOWER0   0     // 
#define ZONE_LOWER1   1     // 
#define ZONE_LOWER2   2     // 
#define ZONE_UPPER0   3     // 
#define ZONE_UPPER1   4     // 
#define ZONE_UPPER2   5     // 
#define SCROLL_SPEED  30


#define MAX_MESG      50
#define MAX_MESG1     6
//#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

uint8_t     SPEED_TIME1         = 75;
uint8_t     SPEED_TIME2         = 75;
uint16_t     PAUSE_TIME1         = 0;
uint16_t     PAUSE_TIME2         = 0;

textEffect_t  scrollEffect1 = PA_NO_EFFECT;
textEffect_t  scrollEffect2 = PA_NO_EFFECT;
textPosition_t  txtalign1      = PA_CENTER ;
textPosition_t  txtalign2      = PA_CENTER ;

// -------------------------------------------------------------------------------------



// 시계 설정값

int     dst           =   0;
int     day;

uint16_t h, m, s;
uint8_t dow;
uint8_t month;

char    szTimeL[MAX_MESG1];         //  초
char    szTimeH[MAX_MESG1];         //  시간
char    szDay[MAX_MESG1];         //  일

char    szMesg[MAX_MESG + 1];
char    szMesg1[MAX_MESG + 1];
char    szMesg2[MAX_MESG + 1];
char    szMesg3[MAX_MESG + 1];
char      szMesg4[MAX_MESG + 1];        // 기상청 날씨

// -------------------------------------------------------------------------------------

#if USE_CDS
// cds 변수
int     cds           =   0;
int     deep_night    =   0;
#endif

// -------------------------------------------------------------------------------------
#if USE_MOTION
uint8_t    MOV = 0;
#endif
// -------------------------------------------------------------------------------------

#if USE_WEATHER
float   Temperature;
float   Humidity;
String  Weather_str;
String  weatherDescription;
#endif

// -------------------------------------------------------------------------------------

#if USE_WIFI
// 와이파이&웹서버 설정
String        st;
int           statusCode;
String        content;
int nCount_set = ARRAY_SIZE(msg);
#endif

// -------------------------------------------------------------------------------------

uint32_t   countTime1 = 0;
uint32_t   countTime2 = 0;
uint32_t   countTime3 = 0;
uint32_t   countTime4 = 0;
uint32_t   countTime5 = 0;
uint32_t   countTime6 = 0;
uint32_t   countTime7 = 0;

uint8_t    cycle     = 0;
uint8_t    display   = 0;
bool       flasher   = false;


// 사용자 정의 폰트

uint8_t degC[] = { 6, 3, 3, 56, 68, 68, 68 }; // Deg C
uint8_t deg1[] = { 8, 231, 0, 221, 0, 59, 0, 110, 0};    // 33 - '!' 비
uint8_t deg2[] = { 8, 128, 72, 44, 62, 26, 9, 0, 0};    // 34 - '"' 번개
uint8_t deg3[] = { 8, 0, 0, 56, 124, 124, 124, 56, 0}; // 39 - '''  맑음
uint8_t deg4[] = { 8, 56, 68, 84, 92, 72, 112, 96, 64};    // 40 - '(' 구름
uint8_t deg5[] = { 8, 12, 78, 142, 255, 14, 14, 12, 0};    // 41 - ')' 우산
uint8_t deg6[] = { 8, 4, 78, 228, 65, 19, 57, 16, 0};  // 42 - '*' 눈

// 팩맨 스피리스 설정

const uint8_t F_PMAN1 = 6;
const uint8_t W_PMAN1 = 8;
const uint8_t F_PMAN2 = 6;
const uint8_t W_PMAN2 = 18;

static const uint8_t PROGMEM pacman1[F_PMAN1 * W_PMAN1] =  // gobbling pacman animation
{
  0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c,
};



static const uint8_t PROGMEM pacman2[F_PMAN2 * W_PMAN2] =  // ghost pursued by a pacman
{
  0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe,
};

// -------------------------------------------------------------------------------------




// =====================================================================================

//  함수 초기화

// =====================================================================================
// 소프트웨어 클
//MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
// 하드웨어 클럭
 MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
#if USE_DHT
DHT dht(DHTPIN, DHTTYPE);
#endif

#if USE_WIFI
// 와이파이 접속 및 핫스팟 설정
bool testWifi(void);
void launchWeb(void);
void setupAP(void);

WebServer server(80);
#endif

#if USE_WEATHER
// 오픈웨더맵 현재날씨
void getWeatherData(void);
#endif

// -------------------------------------------------------------------------------------





// =====================================================================================
//  오픈웨더맵 현재날씨
// =====================================================================================

//  https://api.openweathermap.org/data/2.5/onecall?lat=37.653907&lon=127.021770&exclude=hourly,daily&appid=d8032f60d2083e86588cea420f1d9ac9&lang=en&units=metric
//  http://api.openweathermap.org/data/2.5/weather?id=1835848&units=metric&APPID=d8032f60d2083e86588cea420f1d9ac9
//  http://www.iothub.co.kr/clock1/weatherkr.php?se=tempmax
#if USE_WEATHER
void getWeatherData() {
  // 기상청정보
  url = "/clock1/weatherkr.php?se=temp";
  user_msg1(szMesg4);
  float temperature = atof(szMesg4);

  url = "/clock1/weatherkr.php?se=hum";
  user_msg1(szMesg4);
  float humidity = atof(szMesg4);

  url = "/clock1/weatherkr.php?se=sky1";
  user_msg1(szMesg4);
  String weather_str = szMesg4;

  url = "/clock1/weatherkr.php?se=sky2";
  user_msg1(szMesg4);
  String description = szMesg4;

  weatherDescription = description;
  Temperature = temperature;
  Humidity = humidity;
  Weather_str = weather_str;

  Serial.println(Temperature);
  Serial.println(Humidity);
  Serial.println(Weather_str);
  Serial.println(weatherDescription);
}
#endif

// -------------------------------------------------------------------------------------



// =====================================================================================
//
// =====================================================================================

#if USE_DHT
void getHumidit(char *psz)
// Code for reading clock date
{
  float h = dht.readHumidity();
  dtostrf(h, 3, 0, psz);
  strcat(psz, " %");
}
#endif

// -------------------------------------------------------------------------------------



// =====================================================================================
//
// =====================================================================================

#if USE_DHT
void getTemperatur(char *psz)
// Code for reading clock date
{
  float t = dht.readTemperature();
  dtostrf(t, 3, 0, psz);
  strcat(psz, " $");
}
#endif

// -------------------------------------------------------------------------------------





// =====================================================================================
//
// =====================================================================================

char *mon2str(uint8_t mon, char *psz, uint8_t len)
// Get a label from PROGMEM into a char array
{
  static const char str[][4] PROGMEM =
  {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  };

  *psz = '\0';
  mon--;

  if (mon < 12)
  {
    strncpy_P(psz, str[mon], len);
    psz[len] = '\0';
  }

  return (psz);
}

// -------------------------------------------------------------------------------------




// =====================================================================================
//
// =====================================================================================


char *dow2str(uint8_t code, char *psz, uint8_t len)
{
  static const char str[][10] PROGMEM =
  {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
  };

  *psz = '\0';
  code--;

  if (code < 7)
  {
    strncpy_P(psz, str[code], len);
    psz[len] = '\0';
  }
  return (psz);
}


// -------------------------------------------------------------------------------------




// =====================================================================================
//
// =====================================================================================

void getsec(char *psz)
// Code for reading clock date
{
  sprintf(psz, "%02d", s);
}

// -------------------------------------------------------------------------------------




// =====================================================================================
//
// =====================================================================================

void getTime(char *psz, bool f = true)
// Code for reading clock time
{
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  h = p_tm->tm_hour;
  m = p_tm->tm_min;
  s = p_tm->tm_sec;
  sprintf(psz, "%02d%c%02d", h, (f ? ':' : ' '), m);
}

// -------------------------------------------------------------------------------------




// =====================================================================================
//
// =====================================================================================

void getDate(char *psz)
// Code for reading clock date
{
  char  szBuf[10];
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  dow = p_tm->tm_wday + 1;
  day = p_tm->tm_mday;
  month = p_tm->tm_mon + 1;

  sprintf(psz, "%d %s %04d", day, mon2str(month, szBuf, sizeof(szBuf) - 1), (p_tm->tm_year + 1900));
  sprintf(szDay, "%02d", day);
}

// -------------------------------------------------------------------------------------



// =====================================================================================
//
// =====================================================================================

void getTimentp()
{
  // 시계 타임존 설정
  int timezone    = 9;
  configTime(timezone * 3600, dst, "pool.ntp.org", "time.nist.gov");

  while (!time(nullptr)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Time Update");
}

// -------------------------------------------------------------------------------------



// =====================================================================================
//
// =====================================================================================

#if USE_WIFI
bool testWifi(void)
{
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    Serial.print("*");
    c++;
  }

  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}
#endif

// -------------------------------------------------------------------------------------



// =====================================================================================
//
// =====================================================================================

#if USE_WIFI
void launchWeb()
{
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED) Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());

  createWebServer();
  // Start the server

  server.begin();
  Serial.println("Server started");

}
#endif
// -------------------------------------------------------------------------------------



// =====================================================================================
//
// =====================================================================================

#if USE_WIFI
void setupAP(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");

  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");

    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");

      delay(10);
    }
  }
  Serial.println("");

  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    st += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);

  WiFi.softAP(APNAME, APPW);
  Serial.println("softap");

  launchWeb();

  Serial.println("over");
}
#endif
// -------------------------------------------------------------------------------------




// =====================================================================================
//  웹서버 / 와이파이 설정 핫스팟
// =====================================================================================

#if USE_WIFI
void createWebServer()
{
  //  const char*   ssid          = "text";
  //  const char*   pass    = "text";

  server.on("/", []() {
    IPAddress ip = WiFi.softAPIP();
    String ipStr  =   String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    content       =   "<!DOCTYPE HTML>\r\n<html>Hello from Clock at ";
    content       +=  "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
    content       +=  ipStr;
    content       +=  "<p>";
    content       +=  st;
    content       +=  "</p><form method='get' action='setting'><label>Wifi Name: </label><input name='ssid' length=32>PASS : <input name='pass' length=64><input type='submit'></form>";
    content       +=  "<p> help desk : continue@nate.com</p>";
    content       +=  "</html>";
    server.send(200, "text/html", content);
  });

  server.on("/scan", []() {
    //setupAP();
    IPAddress ip = WiFi.softAPIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    content = "<!DOCTYPE HTML>\r\n<html>go back";
    server.send(200, "text/html", content);
  });

  server.on("/setting", []() {
    String qsid = server.arg("ssid");
    String qpass = server.arg("pass");
    if (qsid.length() > 0 && qpass.length() > 0) {
      Serial.println("clearing eeprom");
      for (int i = 0; i < 96; ++i)EEPROM.write(i, 0);
      Serial.println(qsid);
      Serial.println("");
      Serial.println(qpass);
      Serial.println("");
      Serial.println("writing eeprom ssid:");

      for (unsigned int i = 0; i < qsid.length(); ++i)
      {
        EEPROM.write(i, qsid[i]);
        Serial.print("Wrote: ");
        Serial.println(qsid[i]);
      }
      Serial.println("writing eeprom pass:");
      for (unsigned int i = 0; i < qpass.length(); ++i)
      {
        EEPROM.write(32 + i, qpass[i]);
        Serial.print("Wrote: ");
        Serial.println(qpass[i]);
      }
      EEPROM.commit();

      content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
      statusCode = 200;

      ESP.restart();

    } else {

      content = "{\"Error\":\"404 not found\"}";
      statusCode = 404;
      Serial.println("Sending 404");
    }

    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(statusCode, "application/json", content);
  });

}

#endif
// -------------------------------------------------------------------------------------



// =====================================================================================
//  오픈웨더맵 현재 구름
// =====================================================================================

#if USE_WEATHER
void sky1(char *psz)
{
  //getWeatherData();
  String we = "";
  if ( weatherDescription.indexOf("clouds") >= 0 )we = "(";
  if ( weatherDescription.indexOf("haze") >= 0 )we = "(";
  if ( weatherDescription.indexOf("mist") >= 0 )we = "(";
  if ( weatherDescription.indexOf("clear") >= 0 )we = "\'";
  if ( weatherDescription.indexOf("rain") >= 0 )we = ")";
  if ( weatherDescription.indexOf("Snow") >= 0 ) we = "* ";
  if ( weatherDescription.indexOf("Extreme") >= 0 )we = "\" ";

  we = we + " " + weatherDescription;
  we.toCharArray( psz , we.length() + 1 );
}
#endif

// -------------------------------------------------------------------------------------



// =====================================================================================
//  오픈웨더맵 현재온도
// =====================================================================================

#if USE_WEATHER
void sky2(char *psz)
{
  //getWeatherData();
  dtostrf(Temperature, 3, 0, psz);
  strcat(psz, " $ ");
}

#endif

// -------------------------------------------------------------------------------------




// =====================================================================================
//  오픈웨더맵 현재습도
// =====================================================================================

#if USE_WEATHER
void sky3(char *psz)
{
  //getWeatherData();
  dtostrf(Humidity, 3, 0, psz);
  strcat(psz, " %");
}
#endif

// -------------------------------------------------------------------------------------

void nul(char *psz)
{
  String we = "";
  we = "";
  we.toCharArray( psz , we.length() + 1 );
}


// =====================================================================================
//  디스플레이 이펙트 랜덤설정
// =====================================================================================

void eft1()
{
  int rn = 2;
  int rnT = 0;
  rn = random(10, 15);
  rnT = random(1, 3);
  if (cds > 100) {
    SPEED_TIME1 = random(10, 50); //75;
    PAUSE_TIME1 = 1000;//rnT * 1000;
  } else {
    SPEED_TIME1 = random(30, 80); //75;
    PAUSE_TIME1 = 1000;//rnT * 1000;
  }

  if (rn == 0)scrollEffect1 = PA_NO_EFFECT;
  if (rn == 1)scrollEffect1 = PA_PRINT;
  //if (rn == 2)scrollEffect1 = PA_SCROLL_UP;
  //if (rn == 3)scrollEffect1 = PA_SCROLL_DOWN;
  //if (rn == 4)scrollEffect1 = PA_SCROLL_LEFT;
  //if (rn == 5)scrollEffect1 = PA_SCROLL_RIGHT;
  if (rn == 7) {
    scrollEffect1 = PA_SPRITE;
    SPEED_TIME1 = 80;
  }
  //if (rn == 7 ) {
  //  scrollEffect1 = PA_SLICE;
  //  SPEED_TIME1 = 0;
  //}
  //if (rn == 8)scrollEffect1 = PA_MESH;
  //if (rn == 9)scrollEffect1 = PA_FADE;
  if (rn == 10)scrollEffect1 = PA_DISSOLVE;
  if (rn == 11)scrollEffect1 = PA_BLINDS;
  //if (rn == 12) {
  //  scrollEffect1 = PA_RANDOM;
  //  SPEED_TIME1 = 0;
  // }
  if (rn == 12)scrollEffect1 = PA_WIPE;
  if (rn == 13)scrollEffect1 = PA_WIPE_CURSOR;
  //if (rn == 15)scrollEffect1 = PA_SCAN_HORIZ;
  //if (rn == 16)scrollEffect1 = PA_SCAN_HORIZX;
  //if (rn == 17)scrollEffect1 = PA_SCAN_VERT;
  //if (rn == 18)scrollEffect1 = PA_SCAN_VERTX;
  if (rn == 14)scrollEffect1 = PA_OPENING;
  if (rn == 15)scrollEffect1 = PA_OPENING_CURSOR;
  //if (rn == 21)scrollEffect1 = PA_SCROLL_UP_LEFT;
  //if (rn == 22)scrollEffect1 = PA_SCROLL_UP_RIGHT;
  //if (rn == 23)scrollEffect1 = PA_SCROLL_DOWN_LEFT;
  //if (rn == 24)scrollEffect1 = PA_SCROLL_DOWN_RIGHT;
  //if (rn == 25)scrollEffect1 = PA_GROW_UP;
  //if (rn == 26)scrollEffect1 = PA_GROW_DOWN;
  if (scrollEffect1 == scrollEffect2)scrollEffect1 = PA_PRINT;
  int ali1 = random(1, 3);
  if (ali1 == 1)txtalign1 = PA_CENTER;
  if (ali1 == 2)txtalign1 = PA_CENTER;
  if (ali1 == 3)txtalign1 = PA_RIGHT;

}

void eft2()
{
  int rn = 2;
  int rnT = 0;
  rn = random(2, 3);
  rnT = random(1, 3);
  if (cds > 100) {
    SPEED_TIME2 = random(30, 80); //75;
    PAUSE_TIME2 = rnT * 1000;
  } else {
    SPEED_TIME2 = random(30, 120); //75;
    PAUSE_TIME2 = rnT * 1000;
  }

  if (rn == 0)scrollEffect2 = PA_NO_EFFECT;
  if (rn == 1)scrollEffect2 = PA_PRINT;
  //if (rn == 2)scrollEffect2 = PA_SCROLL_UP;
  //if (rn == 3)scrollEffect2 = PA_SCROLL_DOWN;
  if (flasher)scrollEffect2 = PA_SCROLL_LEFT;
  if (!flasher)scrollEffect2 = PA_SCROLL_RIGHT;
  if (rn == 4) {
    scrollEffect2 = PA_SPRITE;
    SPEED_TIME2 = 80;
  }

  if (rn == 5) {
    scrollEffect2 = PA_SLICE;
    SPEED_TIME2 = 0;
  }

  //if (rn == 8)scrollEffect2 = PA_MESH;
  //if (rn == 9)scrollEffect2 = PA_FADE;
  //if (rn == 10)scrollEffect2 = PA_DISSOLVE;
  //if (rn == 11)scrollEffect2 = PA_BLINDS;
  //if (rn == 12) {
  //  scrollEffect2 = PA_RANDOM;
  //  SPEED_TIME2 = 0;
  //}

  //if (rn == 13)scrollEffect2 = PA_WIPE;
  //if (rn == 14)scrollEffect2 = PA_WIPE_CURSOR;
  //if (rn == 15)scrollEffect2 = PA_SCAN_HORIZ;
  //if (rn == 16)scrollEffect2 = PA_SCAN_HORIZX;
  //if (rn == 17)scrollEffect2 = PA_SCAN_VERT;
  //if (rn == 18)scrollEffect2 = PA_SCAN_VERTX;
  //if (rn == 19)scrollEffect2 = PA_OPENING;
  //if (rn == 20)scrollEffect2 = PA_OPENING_CURSOR;
  // if (rn == 8)scrollEffect2 = PA_SCROLL_UP_LEFT;
  // if (rn == 9)scrollEffect2 = PA_SCROLL_UP_RIGHT;
  // if (rn == 10)scrollEffect2 = PA_SCROLL_DOWN_LEFT;
  // if (rn == 11)scrollEffect2 = PA_SCROLL_DOWN_RIGHT;
  //if (rn == 12)scrollEffect2 = PA_GROW_UP;
  //if (rn == 13)scrollEffect2 = PA_GROW_DOWN;
  if (scrollEffect1 == scrollEffect2)scrollEffect2 = PA_PRINT;
  int ali2 = random(1, 3);
  if (ali2 == 1)txtalign2 = PA_CENTER;
  if (ali2 == 2)txtalign2 = PA_CENTER;
  if (ali2 == 3)txtalign2 = PA_RIGHT;
}

// -------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------

void user_msg()
{
  String im = "";
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    PRINTS("connection failed -,.- ");
    return;
  }

  // This will send a string to the server
  PRINTS("msg sending data to server");
  if (client.connected()) {
    //String url = "";
    //url = "/clock1/name.html";
    //url = "/btc_price.php";
    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
  }

  // wait for data to be available
  unsigned long timeout = millis();

  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      PRINTS(">>> Client Timeout !!!");
      client.stop();
      return;
    }
  }

  while (client.available()) {
    char ch = client.read();
    if (ch == '\n') {
      if (client.read() == '~') {
        im = client.readStringUntil('~');
        PRINT("URLstring:", im);
        im.trim();
      }
    }
  }


  im.toCharArray(szMesg3, im.length() + 1 );

  // Close the connection
  PRINTS("msg closing connection ");
  client.stop();
}

// -------------------------------------------------------------------------------------




// -------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------

void Split(String sData)
{
  int nGetIndex = 0 ;
  int nCount = 0;
  String tmp;
  nCount_set = 0;
  while (true)
  {
    nGetIndex = sData.indexOf("|");
    if (-1 != nGetIndex)
    {
      tmp = sData.substring(0, nGetIndex);
      tmp.toCharArray(msg[nCount], tmp.length() + 1 );
      tmp = "";
      PRINT("msg :", msg[nCount] );
      sData = sData.substring(nGetIndex + 1);
    }
    else
    {
      tmp = sData;
      tmp.toCharArray(msg[nCount], tmp.length() + 1 );
      PRINT("", msg[nCount] );
      nCount_set = nCount;
      break;
    }
    ++nCount;
    nCount_set = nCount;
  }
}

// -------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------

void user_msg1(char *psz)
{
  String im = "";
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    PRINTS("connection failed -,.- ");
    return;
  }

  // This will send a string to the server
  PRINTS("msg sending data to server");
  if (client.connected()) {
    //String url = "";
    //url = "/clock1/name.html";
    //url = "/btc_price.php";
    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
  }

  // wait for data to be available
  unsigned long timeout = millis();

  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      PRINTS(">>> Client Timeout !!!");
      client.stop();
      return;
    }
  }

  while (client.available()) {
    char ch = client.read();
    if (ch == '\n') {
      if (client.read() == '~') {
        im = client.readStringUntil('~');
        PRINT("URLstring:", im);
        im.trim();
      }
    }
  }


  im.toCharArray(psz, im.length() + 1 );

  // Close the connection
  PRINTS("msg closing connection ");
  client.stop();
}

// -------------------------------------------------------------------------------------
// OTA WEB Update
// -------------------------------------------------------------------------------------

void webota() {
    // wait for WiFi connection
    if((WiFi.status() == WL_CONNECTED)) {

        t_httpUpdate_return ret = ESPhttpUpdate.update("http://www.iothub.co.kr/clock1/esp32v2.bin");

        switch(ret) {
            case HTTP_UPDATE_FAILED:
                Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                break;

            case HTTP_UPDATE_NO_UPDATES:
                Serial.println("HTTP_UPDATE_NO_UPDATES");
                break;

            case HTTP_UPDATE_OK:
                Serial.println("HTTP_UPDATE_OK");
                break;
        }
    }
}
// -------------------------------------------------------------------------------------




// -------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------
void brit()
{
#if USE_CDS
  if (cds < 150 && cds > 20)
  {
    P.setIntensity(ZONE_LOWER0, 1);
    P.setIntensity(ZONE_UPPER0, 1);
    P.setIntensity(ZONE_LOWER1, 1);
    P.setIntensity(ZONE_UPPER1, 2);
    P.setIntensity(ZONE_LOWER2, 2);
    P.setIntensity(ZONE_UPPER2, 2);
  } else if (cds < 20 and MOV == 0 ) {
    P.displayClear();
    deep_night = 1 ;
    P.displayReset();
  } else if (cds < 20 and MOV == 1 ) {
    for (int i = 0; i < 6; i++)P.setIntensity(i, 0);
  } else {
    // 시계 밝게 조정
    P.setIntensity(ZONE_LOWER0, 2);
    P.setIntensity(ZONE_UPPER0, 2);
    P.setIntensity(ZONE_LOWER1, 2);
    P.setIntensity(ZONE_UPPER1, 5);
    P.setIntensity(ZONE_LOWER2, 5);
    P.setIntensity(ZONE_UPPER2, 5);
  }
#endif
}

// -------------------------------------------------------------------------------------




// -------------------------------------------------------------------------------------
//
// -------------------------------------------------------------------------------------

void cledisplay() {
  P.begin(6);
  P.setInvert(false);
  P.setZone(ZONE_LOWER0, 0, 11);  // 영어명언
  P.setZone(ZONE_LOWER1, 12, 12 ); // 시계 초
  P.setZone(ZONE_LOWER2, 13, 19 ); // 시계 하
  P.setZone(ZONE_UPPER0, 20, 31);   // 날짜등
  P.setZone(ZONE_UPPER1, 32, 32);   // 일자
  P.setZone(ZONE_UPPER2, 33, 39);   // 시계 상

  P.setFont(ZONE_LOWER0, NULL);
  P.setFont(ZONE_UPPER0, NULL);
  P.setFont(ZONE_LOWER1, numeric7Seg); // 초시계 작은글꼴
  P.setFont(ZONE_UPPER1, numeric7Seg);
  P.setFont(ZONE_LOWER2, BigFontLower);
  P.setFont(ZONE_UPPER2, BigFontUpper);

  P.addChar('$', degC);
  P.addChar('!', deg1);    // 33 - '!' 비
  P.addChar('"', deg2);    // 34 - '"' 번개
  P.addChar('\'', deg3); // 39 - '''  맑음
  P.addChar('(', deg4);    // 40 - '(' 구름
  P.addChar(')', deg5);    // 41 - ')' 우산
  P.addChar('*', deg6);  // 42 - '*' 눈

  P.setSpriteData(pacman1, W_PMAN1, F_PMAN1, pacman2, W_PMAN2, F_PMAN2);

  P.displayZoneText(ZONE_LOWER2, szTimeH, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(ZONE_UPPER2, szTimeH, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(ZONE_LOWER1, szTimeL, PA_CENTER, 10, 180, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(ZONE_UPPER1, szDay, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(ZONE_UPPER0, szMesg, PA_CENTER, 70, 1000, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(ZONE_LOWER0, szMesg3, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

}
// =====================================================================================
//  프로그램 초기화
// =====================================================================================

void setup(void)
{
#if DEBUG
  Serial.begin(115200);
  Serial.println("[Clock Start]");
#endif

  P.begin(6);
  P.setInvert(false);
  P.setZone(ZONE_LOWER0, 0, 11);  // 영어명언
  P.setZone(ZONE_LOWER1, 12, 12 ); // 시계 초
  P.setZone(ZONE_LOWER2, 13, 19 ); // 시계 하
  P.setZone(ZONE_UPPER0, 20, 31);   // 날짜등
  P.setZone(ZONE_UPPER1, 32, 32);   // 일자
  P.setZone(ZONE_UPPER2, 33, 39);   // 시계 상

  P.setFont(ZONE_LOWER0, NULL);
  P.setFont(ZONE_UPPER0, NULL);
  P.setFont(ZONE_LOWER1, numeric7Seg); // 초시계 작은글꼴
  P.setFont(ZONE_UPPER1, numeric7Seg);
  P.setFont(ZONE_LOWER2, BigFontLower);
  P.setFont(ZONE_UPPER2, BigFontUpper);

  for ( int q = 0; q < 6; q++) {
    P.setIntensity(q, 0);
  }

  P.setCharSpacing(P.getCharSpacing() * 2);
  P.displayClear();
#if DEBUG
  for ( int q = 0; q < 6; q++)P.displayZoneText(q, "----------------------------------------------------------", PA_CENTER, SCROLL_SPEED, 0, PA_PRINT, PA_NO_EFFECT);
  P.displayAnimate();
  delay(3000);
#endif
  P.displayClear();
  for ( int q = 0; q < 6; q++)P.displayZoneText(q, " ", PA_CENTER, SCROLL_SPEED, 0, PA_PRINT, PA_NO_EFFECT);
  P.displayAnimate();
#if DEBUG
  mprt("Display ok.");
  delay(1000);
#endif
#if USE_WIFI
  WiFi.disconnect();
  EEPROM.begin(512); //Initialasing EEPROM
#if DEBUG
  mprt("wifi Connect..");
  delay(1000);
#endif
  Serial.println("Reading EEPROM ssid");

  String esid;

  for (int i = 0; i < 32; ++i)
  {
    esid += char(EEPROM.read(i));
  }

  Serial.println();
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");

  String epass = "";
  for (int i = 32; i < 96; ++i)
  {
    epass += char(EEPROM.read(i));
  }

  Serial.print("PASS: ");
  Serial.println(epass);
  WiFi.begin(esid.c_str(), epass.c_str());
#endif

  if (testWifi())
  {
    //------------------------------------------------------
    // OTA
    ArduinoOTA.setHostname(otahost);
    ArduinoOTA.onStart([]() { // switch off all the PWMs during upgrade
      // 원하는 프로그램
    });
    ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end
      // 원하는 프로그램
    });
    ArduinoOTA.onError([](ota_error_t error) {
      mprt("OTA ERR ESP RESRT !!!");
      delay(5000);
      (void)error;
      ESP.restart();
    });
    /* setup the OTA server */
    ArduinoOTA.begin();
    Serial.println("OTA Ready");
    //------------------------------------------------------

#if USE_WIFI
    Serial.println("Succesfully Connected!!!");
#if DEBUG
    mprt("wifi Ok.");
    delay(1000);
#endif
    getTimentp();
    getTimentp();
#if DEBUG
    mprt("Loading Time Ok.");
    delay(1000);
#endif
#endif

#if USE_WEATHER
    getWeatherData();
#endif
#if DEBUG
    mprt("Start .. ");
    delay(1000);
#endif
    P.addChar('$', degC);
    P.addChar('!', deg1);    // 33 - '!' 비
    P.addChar('"', deg2);    // 34 - '"' 번개
    P.addChar('\'', deg3); // 39 - '''  맑음
    P.addChar('(', deg4);    // 40 - '(' 구름
    P.addChar(')', deg5);    // 41 - ')' 우산
    P.addChar('*', deg6);  // 42 - '*' 눈

    P.setSpriteData(pacman1, W_PMAN1, F_PMAN1, pacman2, W_PMAN2, F_PMAN2);
    //    P.setSpriteData(3, pacman1, W_PMAN1, F_PMAN1, pacman2, W_PMAN2, F_PMAN2);

    for ( int q = 0; q < 6 ; q++ )P.setIntensity( q , 0 );

    P.setIntensity(ZONE_UPPER2, 1);
    P.setIntensity(ZONE_LOWER2, 1);

    P.displayZoneText(ZONE_LOWER2, szTimeH, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    P.displayZoneText(ZONE_UPPER2, szTimeH, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    P.displayZoneText(ZONE_LOWER1, szTimeL, PA_CENTER, 10, 180, PA_PRINT, PA_NO_EFFECT);
    P.displayZoneText(ZONE_UPPER1, szDay, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
    //P.displayZoneText(ZONE_UPPER0, szMesg, PA_CENTER, 70, 1000, PA_PRINT, PA_NO_EFFECT);
    //P.displayZoneText(ZONE_LOWER0, msg[cycle], PA_CENTER, 120, 0, PA_SCROLL_RIGHT, PA_SCROLL_RIGHT);
    return;

  } else  {

    Serial.println("Turning the HotSpot On");
    P.displayClear();
    P.displayZoneText(ZONE_LOWER0, "HotSpot On", PA_CENTER, SCROLL_SPEED, 0, PA_PRINT, PA_NO_EFFECT);
    P.displayZoneText(ZONE_UPPER0, "192.168.4.1", PA_CENTER, SCROLL_SPEED, 0, PA_PRINT, PA_NO_EFFECT);
    P.displayAnimate();

#if USE_WIFI
    launchWeb();
    setupAP();// Setup HotSpot
    user_msg();
#endif
  }

  Serial.println();
  Serial.println("Waiting.");
  mprt("Waiting.");
  delay(1000);

  while ((WiFi.status() != WL_CONNECTED))
  {
    Serial.print(".");
    delay(100);
    server.handleClient();
  }
}

// -------------------------------------------------------------------------------------



// =====================================================================================
//
// =====================================================================================

void loop(void)
{
  // OTA
  ArduinoOTA.handle();
  cds = analogRead(CDSPIN);

#if USE_MOTION

  MOV = digitalRead(MOV_PIN);
  //   Serial.println(MOV);

  if ( MOV == 1 ) {
    countTime4 = millis();
    P.setTextEffect(ZONE_LOWER1, PA_SCROLL_UP, PA_SCROLL_UP);

  } else {
    countTime4 = 0;
    P.setTextEffect(ZONE_LOWER1, PA_PRINT, PA_NO_EFFECT);
  }
  // 마지막움직임 보정
  if (digitalRead(MOV_PIN))countTime4 = millis();
#endif

  // 1시간마다 시간 갱신
  if ( millis() - countTime6 >= 1000 * 60 * 60 )
  {
    countTime6 = millis();
    getTimentp();
  }

  // 10분마다 일기예보 갱신
#if USE_WEATHER
  if ( millis() - countTime1 >= 1000 * 60 * 10 )
  {
    countTime1 = millis();
    getWeatherData();
  }
#endif

  // 10분마다 서버에서 메시지 가져오기
  if ( millis() - countTime2 >= 1000 * 3  )
  {
    countTime2 = millis();

    if (url != "/btc_price.php")
    {
      url = "/btc_price.php";
    } else {
      //url = "/jango.html";
      url = "/btc_price.php";
    }

    user_msg();
  }
#if USE_CDS
  // 1분 밝기조정
  if ( millis() - countTime5 >= 1000 * 60 )
  {
    countTime5 = millis();
    brit();
    webota();
  }

  if ( deep_night == 0 ) {
#endif

    P.displayAnimate();

    if (P.getZoneStatus(ZONE_UPPER0))
    {
      eft1();
      switch (display)
      {
        case 0:
#if USE_DHT
          P.displayZoneText(ZONE_UPPER0, szMesg, txtalign1, SPEED_TIME1, PAUSE_TIME1, scrollEffect1, scrollEffect1);
          P.setTextEffect(ZONE_UPPER0, scrollEffect1, scrollEffect1);
          nul(szMesg);
          nul(szMesg1);
          nul(szMesg2);

          getHumidit(szMesg1);
          getTemperatur(szMesg2);

          strcat(szMesg, " in:");
          strcat(szMesg, szMesg2);
          strcat(szMesg, szMesg1);
#endif
          display++;
          break;

        case 1:
#if USE_WEATHER
          P.displayZoneText(ZONE_UPPER0, szMesg, txtalign1, SPEED_TIME1, PAUSE_TIME1, scrollEffect1, scrollEffect1);
          P.setTextEffect(ZONE_UPPER0, scrollEffect1, scrollEffect1);
          nul(szMesg);
          nul(szMesg1);
          nul(szMesg2);

          sky1(szMesg);

#endif
          display++;
          break;

        case 2:
#if USE_WEATHER
          P.displayZoneText(ZONE_UPPER0, szMesg, txtalign1, SPEED_TIME1, PAUSE_TIME1, scrollEffect1, scrollEffect1);
          P.setTextEffect(ZONE_UPPER0, scrollEffect1, scrollEffect1);
          nul(szMesg);
          nul(szMesg1);
          nul(szMesg2);
          sky2(szMesg1);
          sky3(szMesg2);
          strcat(szMesg, " out:");
          strcat(szMesg, szMesg1);
          strcat(szMesg, szMesg2);
#endif
          display++;
          break;

        case 3:
          P.displayZoneText(ZONE_UPPER0, szMesg, txtalign1, SPEED_TIME1, PAUSE_TIME1, scrollEffect1, scrollEffect1);
          P.setTextEffect(ZONE_UPPER0, scrollEffect1, scrollEffect1);
          nul(szMesg);
          nul(szMesg1);
          nul(szMesg2);
          getDate(szMesg);
          display++;
          break;

        default:
          P.displayZoneText(ZONE_UPPER0, szMesg, txtalign1, SPEED_TIME1, PAUSE_TIME1, scrollEffect1, scrollEffect1);
          P.setTextEffect(ZONE_UPPER0, scrollEffect1, scrollEffect1);

          nul(szMesg);
          nul(szMesg1);
          nul(szMesg2);
          dow2str(dow, szMesg, MAX_MESG);

          display = 0;
          break;

      }
      P.displayReset(ZONE_UPPER0);

    }

    if (P.getZoneStatus(ZONE_LOWER0))
    {
      //user_msg();
      P.displayZoneText(ZONE_LOWER0, szMesg3, PA_CENTER, 0 , 0, PA_PRINT, PA_NO_EFFECT);
      P.setTextEffect(ZONE_LOWER0, PA_PRINT, PA_NO_EFFECT);

      P.displayReset(ZONE_LOWER0);


      if ( millis() - countTime7 >= 1000 * 60 * 1 )
      {
        countTime7 = millis();
        cledisplay();
        brit();
      }

    }

    if (millis() - countTime3 >= 1000)
    {
      countTime3 = millis();
      getsec(szTimeL);
      getTime(szTimeH, flasher);
      flasher = !flasher;
      P.displayReset(ZONE_LOWER2);
      P.displayReset(ZONE_UPPER2);
      P.displayReset(ZONE_LOWER1);
      P.displayReset(ZONE_UPPER1);

//      P.synchZoneStart();
    }
#if USE_CDS

  }
  deep_night = 0;
#endif
}

// -------------------------------------------------------------------------------------









// =====================================================================================

//

// =====================================================================================
