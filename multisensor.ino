```cpp
// Include Libraries
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <DHT_U.h>
#include <EEPROM.h>

// Define constants
#define time_out_test 10000           // time out for each test
#define time_interval_test 2000       // time between tests
#define time_warmup_gas_sensor 20000  // time to warm up gas sensor
#define time_screen_off 30000         // time to turn off screen

//Define Pins and EEPROM addresses
# define pin_M 4
# define pin_PLUS 7
# define pin_MINUS 6
# define pin_LED 5
# define pin_DHT 3
# define pin_BUZZER 2
# define pin_GAS_SENSOR A3
#define address_threshold_temp 0    // increments of 1 [0-50] mapped from 0 C to 50 C with increments of "T_increment"
#define address_threshold_gas 1     // increments of 1 [0-200] mapped from 0 to 1000 with increments of "G_increment"

// Others
#define DHTTYPE DHT11
#define T_increment 1
#define G_increment 5

// Create  objects
LiquidCrystal_I2C lcd(0x27,16,2);   // Create LCD in the i2c address (my LCD ys 16x2)
DHT dht(pin_DHT, DHTTYPE);	        // Create dht


// Global variables
float temp, gas_level;
int t_threshold=EEPROM.read(address_threshold_temp)*T_increment;
int gas_threshold=EEPROM.read(address_threshold_gas)*G_increment;
int display_mode=0;
bool screen_on,alarm_on,disabling_alarm,lcd_always_on=0;
bool buzzer_sound_on, led_light_on=1;

void setup() 
{
  Serial.begin(9600);

  lcd.init();                     // INITIALIZE LCD
  dht.begin();	                  // INITIALIZE DHT
  setup_gas_sensor();

  // Setup Thresholds
  // EEPROM.update(address_threshold_temp, 40/T_increment);	    // First Setup Ever for T threshold 
  // EEPROM.update(address_threshold_gas, 300/G_increment);	    // First Setup Ever for Gas threshold 
  t_threshold=EEPROM.read(address_threshold_temp)*T_increment;  
  gas_threshold=EEPROM.read(address_threshold_temp)*T_increment;

  pinMode(pin_M, INPUT_PULLUP);
  pinMode(pin_PLUS, INPUT_PULLUP);
  pinMode(pin_MINUS, INPUT_PULLUP);
  pinMode(pin_LED, OUTPUT);
  pinMode(pin_BUZZER, OUTPUT);
  
  if (!digitalRead(pin_M))          // If M is pressed on startup, it runs the tests
    test_devices();
  
  screen_on=1;                      // Start device with on for time_screen_off seconds
}

void loop() 
{
}

void setup_gas_sensor()
{
  lcd.backlight();  //  TURN ON LCD BACKLIGHT
  lcd.print("Warming up");
  lcd.setCursor(0, 1);
  lcd.print("Gas Sensor");
  delay(time_warmup_gas_sensor);  // WARM UP GAS SENSOR
  lcd.clear();
  lcd.noBacklight();
}

void test_devices()
{
  Serial.println("Begin Testing...");   
  lcd.backlight();  //  TURN ON LCD BACKLIGHT
  lcd.clear();
  lcd.print("TESTING...");
  delay(time_interval_test); 

  test_lcd();             // TESTING LCD
  test_button(pin_M);     // TESTING BUTTONS
  test_button(pin_PLUS);
  test_button(pin_MINUS);
  test_led();             // TESTING LED
  test_buzzer();          // TESTING BUZZER
  test_dht();             // TESTING DHT
  test_gas_sensor();      // TESTING GAS SENSOR
  test_eemprom();         // TESTING EEPROM (PEND)
  
  // END OF TESTS
  lcd.clear();
  lcd.print("END OF TESTS");
  delay(time_interval_test);
  lcd.clear();
  lcd.noBacklight();
  Serial.println("End of Test");   
}

void test_lcd()
{
  lcd.clear();
  lcd.print("Testing LCD...");
  delay(time_interval_test); 
  lcd.clear();
  lcd.print("HELLO WORLD");  
  delay(time_interval_test);
  lcd.setCursor(0, 1);
  lcd.print("LCD OK");  
  delay(time_interval_test);
}

void test_button(int pin)
{
  bool test_result=0;  // To store result of the test
  String buttonName = "Hello String";
  switch (pin)
    {
      case 4:
        buttonName="M";
        break;
      case 7:
        buttonName="+";
        break;
      case 6:
        buttonName="-";
        break;
      default:
        buttonName="ERROR";
        break;
    }
  lcd.clear();
  lcd.print("Press ");
  lcd.print(buttonName);
  lcd.print("...");
  unsigned long time_test_started=millis();
  while (test_result==0 && (millis()-time_test_started < time_out_test)) // Wait for button or time_out
  {
    if (!digitalRead(pin))
      test_result=1;    
  } 
  lcd.setCursor(0, 1);
  if (test_result==0)             // Test not OK or time out
    lcd.print("Button NOT OK");  
  else                            // Test OK
    lcd.print("Button OK");  
  delay(time_interval_test);   
}

void test_led()
{
  lcd.clear();
  lcd.print("Testing LED...");
  delay(time_interval_test); 
  lcd.clear();
  lcd.print("LED ON");  
  digitalWrite(pin_LED, HIGH);
  delay(time_interval_test);
  lcd.setCursor(0, 1);
  lcd.print("LED OFF");  
  digitalWrite(pin_LED, LOW);
  delay(time_interval_test);
}

void test_buzzer()
{
  lcd.clear();
  lcd.print("Testing Buzzer..");
  delay(time_interval_test);
  lcd.clear();
  lcd.print("Buzzer ON");
  tone(pin_BUZZER,440,time_interval_test*2);  // 2xTimes to test the "noTone" function
  delay(time_interval_test); 
  lcd.setCursor(0, 1);
  lcd.print("Buzzer OFF");
  noTone(pin_BUZZER);
  delay(time_interval_test);  
}

void test_dht()
{
  lcd.clear();
  lcd.print("Testing DHT...");
  delay(time_interval_test);
  lcd.clear();
  lcd.print("Reading Temp");
  delay(time_interval_test); 
  temp=dht.readTemperature();
  lcd.setCursor(0, 1);
  if (isnan(temp)) 	                // DHT Working OK?
    lcd.print("DHT NOT OK");   
  else
  {
    lcd.print("Temp: ");
    lcd.print(temp);
    lcd.print(" C");
  }
  delay(time_interval_test);
}

void test_gas_sensor()
{
  lcd.clear();
  lcd.print("Testing Gas S...");
  delay(time_interval_test);
  lcd.clear();
  lcd.print("Reading Gas Lvl");
  delay(time_interval_test); 
  gas_level=analogRead(pin_GAS_SENSOR);
  lcd.setCursor(0, 1);
  lcd.print("Gas Lvl: ");
  lcd.print(gas_level);
  delay(time_interval_test);
}

void test_eemprom()
{
  lcd.clear();
  lcd.print("Testing EEPROM");
  delay(time_interval_test);
  lcd.clear();
  lcd.print("Reading EEMPROM");
  t_threshold=EEPROM.read(address_threshold_temp)*T_increment;  
  gas_threshold=EEPROM.read(address_threshold_gas)*G_increment;
  delay(time_interval_test); 
  lcd.clear();
  lcd.print("T Thresh: ");
  lcd.print(t_threshold);
  lcd.setCursor(0, 1);
  lcd.print("G Thresh: ");
  lcd.print(gas_threshold);
  delay(time_interval_test);
}
```
