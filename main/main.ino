#include <Adafruit_Sensor.h>
//library for the dht11 sensor
#include <DHT.h>
#include <DHT_U.h>

#define DHTTYPE DHT11 //for the dht 11
#define DHTPIN 7 //digital pin number 7 to read the values

DHT dht(DHTPIN, DHTTYPE); //sensor object

typedef enum
{
  on, off, begining, breach
} sys_state;

//abstract class for the components of the circuit
class SysComponent
{
  protected:
    int pin;
  public:

    //constructor
    SysComponent(int p)
    {
      pin = p;
    }

    //setting input or output
    void setting_inOut(int inOut)
    {
      pinMode(pin, inOut);
    }

    //returning the pin
    int returnPin()
    {
      return pin;
    }

    //virtual function for turning off a component
    virtual void turnOff() {};

    //virtual function for turning on a component
    virtual void turnOn() {};
};

//LED class to manage the LEDs
class LED: public SysComponent
{
  private:
    sys_state state;
  public:

    //constructor
    LED(int pin, sys_state s): SysComponent(pin)
    {
      state = s;
    }

    //turn on the LED - notice the virtual function in SysComponent
    void turnOn()
    {
      digitalWrite(pin, HIGH);
      state = on;
    }

    //turn off the LED - notice the virtual function in SysComponent
    void turnOff()
    {
      digitalWrite(pin, LOW);
      state = off;
    }
};

//buzzer class to manage the buzzer tone and state
class Buzzer: public SysComponent
{
  private:
    int freq;
  public:

    //constructor
    Buzzer(int pin, int f): SysComponent(pin)
    {
      freq = f;
    }

    //change frequency
    void changeFreq(int f)
    {
      freq = f;
    }

    //return the current frequency
    int returnFreq()
    {
      return freq;
    }

    //turn on the buzzer - notice the virtual function in SysComponent
    void turnOn()
    {
      tone(pin, freq);
    }

    //turn off the buzzer - notice the virtual function in SysComponent
    void turnOff()
    {
      noTone(pin);
    }
};

//button class to turn on/off the whole system
class Button: public SysComponent
{
  private:
    sys_state state;
  public:

    //constructor
    Button(int pin, sys_state s): SysComponent(pin)
    {
      state = s;
    }

    //set the system status
    void setStatus(sys_state s)
    {
      state = s;
    }
};

//sensor class to store the temperature and humidity values
class Sensor: public SysComponent
{
  private:
    float temp, hum, thtemp, thhum; //values for temperature, humidity and their respective tresholds
  public:

    //constructor
    Sensor(int pin, float t, float h):SysComponent(pin)
    {
      thtemp = t;
      thhum = h;
    }
    
    //return the maximum allowed temperature
    float returnTempTresh()
    {
      return thtemp;
    }
    
    //return the maximum allowed humidity
    float returnHumTresh()
    {
      return thhum;
    }
    //finding the maximum value of the read temperatures and humidities in a well determined amount of time
    float maximumTemp(int, int);
    float maximumHum(int, int);

    //assigning the maximum values read from the sensor and returning them
    float assignTemp();
    float assignHum();
};

//determine the maximum temperature
float Sensor::maximumTemp(int pin, int count)
{
  //the variable to determine the maximum temperature
  float temp = 0;
  for(int i = 0; i < count; ++i)
  {
    temp = max(temp, dht.readTemperature());
  }
  return temp;
}

//determine the maximum humidity 
float Sensor::maximumHum(int pin, int count)
{
  //the variable to determine the maximum humidity
  float hum = 0;
  for(int i = 0; i < count; ++i)
  {
    hum = max(hum, dht.readHumidity());
  }
  return hum;
}

//return the maximum read temperature from the sensor
float Sensor::assignTemp()
{
  temp = maximumTemp(pin, 100);
  return temp;
}

//return the maximum read humidity from the sensor
float Sensor::assignHum()
{
  hum = maximumHum(pin, 100);
  return hum;
}

//declaring the variables:
sys_state SysState = begining;
LED rgbRed(8, off);
//LED rgbGreen(9, off);
LED rgbBlue(10, off);
LED red(4, off);
LED green(3, off);
Button button(13, off);
Buzzer buzzer(6, 0);
Sensor dht11(7, 30, 60); //pin 7, maximum temp is 30, minimum temp is 15, maximum hum is 60, minimum hum is 20

//changing the state of the system on/off
void changeState()
{
  if(SysState == off)
  {
    SysState = begining;
  }
  else
  {
    SysState = off;
  }
}

void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(9600);
  dht.begin();
  //Serial.println(digitalRead(button.returnPin()));
  
  //setting up the input/output pins
  rgbRed.setting_inOut(OUTPUT);
  rgbBlue.setting_inOut(OUTPUT);
  red.setting_inOut(OUTPUT);
  green.setting_inOut(OUTPUT);
  button.setting_inOut(INPUT);
  buzzer.setting_inOut(OUTPUT);
  dht11.setting_inOut(INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  switch(SysState)
  {
    case off:
      Serial.println(digitalRead(button.returnPin()));
      rgbRed.turnOff();
      rgbBlue.turnOff();
      green.turnOff();
      red.turnOn();
      buzzer.turnOff();
      delay(1000);
      if(digitalRead(button.returnPin()) == HIGH)
      {
        changeState();
        //break;
      }
    break;
    case begining:
      green.turnOn();
      red.turnOff();
      SysState = on;
    break;
    case on:
        green.turnOn();
        red.turnOff();
        delay(200);
        float currentTemp = dht11.assignTemp();
        float currentHum = dht11.assignHum();
        
        //if the tresholds are exceeded we have a breach
        if(currentTemp > dht11.returnTempTresh() || currentHum > dht11.returnHumTresh())
        {
          SysState = breach;
          buzzer.changeFreq(1500);
          //buzzer.turnOn();
          break;
        }
        
        //pressing the button will turn off the system
         if(digitalRead(button.returnPin()) == HIGH)
         {
          changeState();
          break;
         }
         
         Serial.println(digitalRead(button.returnPin()));
         Serial.print(F("Humidity: "));
         Serial.print(currentHum);
         Serial.print(F("%  Temperature: "));
         Serial.println(currentTemp);
         
         delay(5000); //the time in which the temperature of humidity might change
         
         float readTemp = dht.readTemperature();
         float readHum = dht.readHumidity();
         
         //systems response in the event of a temperature/humidity change
         if(abs(currentTemp - readTemp) >= 1)
         {
            rgbBlue.turnOn();
            //rgbRed.turnOff();
            buzzer.changeFreq(1000);
            buzzer.turnOn();
            Serial.println(readTemp);
            delay(500); //time in which the buzzer will reply
            buzzer.turnOff();
            rgbBlue.turnOff();
         }
         if(abs(currentHum - readHum) >= 5)
         {
            rgbRed.turnOn();
            //rgbBlue.turnOff();
            buzzer.changeFreq(700);
            buzzer.turnOn();
            Serial.println(readHum);
            delay(500); //time in which the buzzer will reply
            buzzer.turnOff();
            rgbRed.turnOff();
         }
    break;
    case breach:
      for(int i = 0; i < 10; ++i)
      {
        buzzer.turnOn();
        red.turnOn();
        delay(100);
        buzzer.turnOff();
        red.turnOff();
      }
      SysState = off;
    break;
  }
}
