/*
  ======================================================================
  |HWMS🌤️ - Home Weather Monitoring System by https://github.com/Maxdgf|
  =======================================================================
  |Used components:
  |-1) Arduino Nano
  |-2) LCD2004 I2C
  |-3) AHT10 
  |-4) BMP180
  |-6) Buzzer
  |-7) RGB light
  |-8) HC-SR04 ultrasonic module
  |-9) ESP 8266 (ESP 01)
*/

//File -> Preferences and put this link in parameter string 'Additional boards manager URLs': http://arduino.esp8266.com/stable/package_esp8266com_index.json for esp82666 (esp 01) work.

#include <string.h>

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <SFE_BMP180.h>   
#include <Adafruit_AHTX0.h>   
#include <Ultrasonic.h>  

//initializing rgb light and buzzer pins
#define RED_PIN 4
#define GREEN_PIN 5
#define BLUE_PIN 6
#define SOUND_PIN 2

//creating modules objects
LiquidCrystal_I2C lcd(0x27, 20, 4); 
Ultrasonic ultrasonic(11, 12);
SFE_BMP180 pressure; 
Adafruit_AHTX0 aht;

//initializing lcd symbols
const byte gradusSymbol[8] = { 0b01100, 0b10010, 0b10010, 0b01100, 0b00000, 0b00000, 0b00000, 0b00000 };
const byte pressureSymbol[8] = { 0b01100, 0b10010, 0b10010, 0b10010, 0b11110, 0b11110, 0b11110, 0b01100 };
const byte humSymbol[8] = { 0b00000, 0b00100, 0b01110, 0b11111, 0b11111, 0b11111, 0b01110, 0b00000 };
//const byte qualityElement[8] = { 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11111 };
//const byte emptyQualityElementSymbol[8] = { 0b11111, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11111 };
const byte correctSymbol[8] = { 0b00000, 0b00000, 0b01010, 0b01010, 0b10001, 0b01110, 0b00000, 0b00000 };
const byte incorrectSymbol[8] = { 0b00000, 0b01010, 0b01010, 0b00000, 0b01110, 0b10001, 0b00000, 0b00000 };

String temperatureNormColor;
String humidityNormColor;
String pressureNormColor;
String note;

bool isTemperatureNormal;
bool isHumidityNormal;
bool isPressureNormal;

int temperatureMark;
int humidityMark;
int pressureMark;

int weatherQuality;

int distance;

void setup()
{
  Serial.begin(115200);

  lcd.init();                          
  lcd.backlight();
  pressure.begin();  
  aht.begin();

  //creating lcd symbols
  lcd.createChar(1, gradusSymbol);
  lcd.createChar(2, pressureSymbol);
  lcd.createChar(3, humSymbol);
  //lcd.createChar(4, qualityElement);
  lcd.createChar(5, correctSymbol);
  lcd.createChar(6, incorrectSymbol);
  //lcd.createChar(7, emptyQualityElementSymbol);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);  
  pinMode(SOUND_PIN, OUTPUT); 

  printHelloMessage();
  helloSound();
  helloLight();

  delay(5000);         

  lcd.clear();               
}

void loop()
{ 
  //get distance data from ultrasonic module for manage lcd backlight
  distance = ultrasonic.read();
  checkDistanceAndManageDisplayBackight(distance);

  char status;
  double T,P;
  float pressure_parameter;

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  //calculating pressure
  status = pressure.startTemperature();       
  if(status!=0){                             
    delay(status);                          
    status = pressure.getTemperature(T);  
    if(status!=0){    
      status = pressure.startPressure(3); 
      if(status!=0){                            
        delay(status);                   
        status = pressure.getPressure(P,T); 
        if(status!=0){   
          lcd.setCursor(0, 1);                     
          lcd.print("Pressure:"); 
          lcd.setCursor(9, 1);      
          pressure_parameter = P*0.7500637554192,2;     
          lcd.print(String(P*0.7500637554192,2) + "mmHg" + char(2));
        }            
      }     
    } 
  } 

  //get temperature and humidity data
  float temperature_parameter = temp.temperature;
  float humidity_parameter = humidity.relative_humidity;

  String formatted_temperature_parameter = checkTemperaturePositiveOrNegative(temperature_parameter, false);
  String formatted_temperature_parameter_for_other_devices = checkTemperaturePositiveOrNegative(temperature_parameter, true);

  //printing data on lcd
  lcd.setCursor(0, 0);                     
  lcd.print("Temperature:"); 
  lcd.setCursor(12, 0);    
  lcd.print(formatted_temperature_parameter);   
  lcd.setCursor(19, 0);             
  lcd.print("C"); 

  lcd.setCursor(0, 2);
  lcd.print("Humidity:");
  lcd.setCursor(9, 2);
  lcd.print(String(humidity_parameter) + "%" + char(3));

  lcd.setCursor(9, 3);
  lcd.print(char(1));
  lcd.setCursor(10, 3);
  lcd.print("C");
  lcd.setCursor(11, 3);
  lcd.print(checkNormsOfTemperature(temperature_parameter));
  lcd.setCursor(13, 3);
  lcd.print(char(2));
  lcd.setCursor(14, 3);
  lcd.print(checkNormsfPressure(pressure_parameter));
  lcd.setCursor(16, 3);
  lcd.print(char(3));
  lcd.setCursor(17, 3);
  lcd.print(checkNormsOfHumidity(humidity_parameter));

  weatherQuality = calculateWeatherQuality();

  lcd.setCursor(19, 3);
  lcd.print(String(weatherQuality));

  checkWeatherQualityLevel(weatherQuality);

  //drawWeatherQualityBar(char(4), char(7), weatherQuality); optional function (draw a weather quality bar)
  lcd.setCursor(0, 3);
  lcd.print("Quality:");

  configureAndSendWeatherDataToSerialPort(formatted_temperature_parameter_for_other_devices, humidity_parameter, pressure_parameter, note, weatherQuality, temperatureNormColor, humidityNormColor, pressureNormColor);

  delay(500); //update delay
}

//checking norms of temperature
char checkNormsOfTemperature(float temperature) {
  if (temperature <= 28 && temperature > 15) {
    isTemperatureNormal = true;
    temperatureNormColor = "green";
    return char(5);
  } else if (temperature <= 15) {
    isTemperatureNormal = false;
    temperatureNormColor = "yellow";
    return char(6);
  } else if (temperature > 28) { 
    isTemperatureNormal = false;
    temperatureNormColor = "red";
    return char(6);
  }
}

//checking norms of humidity
char checkNormsOfHumidity(float humidity) {
  if (humidity <= 60 && humidity > 30) {
    isHumidityNormal = true;
    humidityNormColor = "green";
    return char(5);
  } else if (humidity <= 30) {
    isHumidityNormal = false;
    humidityNormColor = "yellow";
    return char(6);
  } else if (humidity > 60) {
    isHumidityNormal = false;
    humidityNormColor = "red";
    return char(6);
  }
}

//checking norms of pressure
char checkNormsfPressure(double pressure) {
  if (pressure <= 760 && pressure > 740) { 
    isPressureNormal = true;  
    pressureNormColor = "green";
    return char(5);
  } else if (pressure <= 740) {
    isPressureNormal = false;
    pressureNormColor = "yellow";
    return char(6);
  } else if (pressure > 760) { 
    isPressureNormal = false;
    pressureNormColor = "red";
    return char(6);
  }
}

//calculating weather quality
int calculateWeatherQuality() {
  if (isTemperatureNormal) {
    temperatureMark = 1;
  } else {
    temperatureMark = 0;
  }

  if (isHumidityNormal) {
    humidityMark = 1;
  } else {
    humidityMark = 0;
  }

  if (isPressureNormal) {
    pressureMark = 1;
  } else {
    pressureMark = 0;
  }

  return temperatureMark + humidityMark + pressureMark;
}

//checking weather quality for manage RGB light and configuring "note" data for esp 01
void checkWeatherQualityLevel(int quality) {
  if (quality == 3) {
    note = "All parameters are normal😉!";
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, LOW);
    digitalWrite(RED_PIN, LOW);
  } else if (quality == 2) {
    note = "Parameters are becoming less than normal😐!";
    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, HIGH);
  } else if (quality == 1) {
    note = "Parameters are much lower than the normal😧! think about correcting the situation🫣.";
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(BLUE_PIN, HIGH);
  } else if (quality == 0) {
    note = "Parameters are not comparable with the norms💀!!!";
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(BLUE_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
  }
}

//configure data and send data string to serial port for esp 01 wifi module
void configureAndSendWeatherDataToSerialPort(String temperature, float humidity, double pressure, String note, int quality, String tempNormColor, String humNormColor, String pressNormColor) {
  String data = temperature + "," + String(humidity) + "%," + String(pressure) + "mmHg," + note + "," + quality + "," + tempNormColor + "," + humNormColor + "," + pressNormColor;
  Serial.println(data);
}

//optional function (draw a weather quality bar)
/*void drawWeatherQualityBar(char fullSym, char emptySym, int quality) {
  //printing weather quality
  int fullParts; //filled elements count
  int emptyParts; //empty elements count
  int lastIndex; //last filled element index

  //configuring full and empty elemnts count from weather quality
  if (quality == 3) {
    fullParts = 6;
    emptyParts = 0;
  } else if (quality == 2) {
    fullParts = 4;
    emptyParts = 2;
  } else if (quality == 1) {
    fullParts = 2;
    emptyParts = 4;
  } else if (quality == 0) {
    fullParts = 0;
    emptyParts = 6;
  }

  //draw full elements:
  for (int i = 0; i < fullParts; i++) {
    lcd.setCursor(i, 3);
    lcd.print(fullSym);
    lastIndex = i;
  }

  //draw empty elements
  for (int j = 0; j < emptyParts; j++) {
    lastIndex++; //empty elements drawing starts from last full element index
    lcd.setCursor(lastIndex, 3);
    lcd.print(emptySym);
  }
}*/

//making hello sound for welcome screen
void helloSound() {
  //makes a hello sound whith hello message
  tone(SOUND_PIN, 2000, 100);
  delay(50);
  tone(SOUND_PIN, 3000, 100);
  delay(150); 
  tone(SOUND_PIN, 3000, 300);
  delay(350);
  tone(SOUND_PIN, 3000, 400);
  delay(450);
  tone(SOUND_PIN, 3000, 500);
  delay(550); 
}

//making hello green light blink
void helloLight() {
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, LOW);
  digitalWrite(RED_PIN, LOW);
  delay(250);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
  digitalWrite(RED_PIN, LOW);
  delay(250);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, LOW);
  digitalWrite(RED_PIN, LOW);
  delay(250);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
  digitalWrite(RED_PIN, LOW);
  delay(250);
  digitalWrite(GREEN_PIN, HIGH);
  digitalWrite(BLUE_PIN, LOW);
  digitalWrite(RED_PIN, LOW);
  delay(250);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
  digitalWrite(RED_PIN, LOW);
}

//print hello message
void printHelloMessage() {
  //printing hello message on screen.
  lcd.setCursor(0, 0);
  lcd.print("-------|HWMS|-------");
  lcd.setCursor(0, 1);
  lcd.print("House Weather");
  lcd.setCursor(0, 2);
  lcd.print("Monitoring System");
  lcd.setCursor(0, 3);
  lcd.print("--> by Max");
  lcd.setCursor(11, 3);
  lcd.print(char(1));
  lcd.setCursor(12, 3);
  lcd.print("C");
  lcd.setCursor(14, 3);
  lcd.print(char(2));
  lcd.setCursor(16, 3);
  lcd.print(char(3));
  lcd.setCursor(19, 3);
  lcd.print(char(5));
}

//checking distance data from ultrasonic and manage lcd backlight for optimized lcd screen energy use.
void checkDistanceAndManageDisplayBackight(int distance) {
  if (distance <= 100) {
    lcd.backlight();
  } else if (distance > 100) {
    lcd.noBacklight();
  }
}

//checking temperature value on positive or negative
String checkTemperaturePositiveOrNegative(float temperature, bool fomatForOtherDevices) {
  if (temperature > 0) {
    if (!fomatForOtherDevices) {
      return "+" + String(temperature) + char(1) + "C"; 
    } else {
      return "+" + String(temperature) + "°C"; 
    }  
  } else {
    if (!fomatForOtherDevices) {
      return "-" + String(temperature) + char(1) + "C";
    } else {
      return "-" + String(temperature) + "°C";
    }  
  }
}
