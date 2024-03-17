/*
Creator: Nhat Nguyen
Date: March 5, 2024
Schematic 
- Arduino Uno
- Keypad:
  4 col: 8 7 6 5
  3 row: 4 3 2
- LCD:
  SDA: -> A4
  SCL: -> A5
- RFID:
  MISO: -> 12
  MOSI: -> 11
  SDA (SS): -> 10
  SCK: -> 13
  RST: -> 9
*/
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

//Total User in the System
#define UserCount 3

// Keypad
#define ROWs 4
#define COLs 3

// MFRC522
#define SS_PIN 10
#define RST_PIN 9

enum State {IDLES, WELCOME, INSERT_RFID, RFID_SCAN, PWD, UNLOCKED};
enum State state; 

// Global variable
String PWD_ip;  //store input passcode
bool IDLE_flag = false;

int PWDtried_count = 0;

// Keypad
int holdDelay = 100;
int n = 3;
int keypadState = 0;
char keyx;

char keys [ROWs][COLs] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
// setup Keypad
byte rowPin[ROWs] = {8,7,6,5};
byte colPin[COLs] = {4,3,2};
Keypad keypad = Keypad(makeKeymap(keys), rowPin, colPin, ROWs, COLs);

// setup MFRC522
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

// setup LCD
LiquidCrystal_I2C lcd (0x3F,16,2);

// Class
class User {
  private:
    String username;
    String userUID;
    String userPWD;
  public:
    User(){};
    User(String name, String UID, String PWD){
      username = name;
      userUID = UID;
      userPWD = PWD;
    };
    String getUsername(){return username;}
    String getUserUID(){return userUID;}
    String getUserPWD(){return userPWD;}
};
/*
class Admin: public User{
  private:
    String adPWD;
  public:
    Admin(){};
    Admin(String name, String UID, String PWD, String ad_PWD):adPWD(ad_PWD) {
      User(name,UID,PWD);
    }
    String getAdPWD(){return adPWD;}
};
*/
// User in system
User users[] = { User("user1", "3F932126", "1234"),
                 User("user2", "B1E4CA73", "1111"),
                 User("Boss", "1A83D580", "2213")};
User *currentUser = nullptr;

int main(void){
  init();
// Serial
  Serial.begin(9600);
  Serial.println("Program begin.");
// LCD
  lcd.init();
  lcd.noBacklight();
  lcd.clear();
// MFRC522
  SPI.begin();
  mfrc522.PCD_Init();

  while(true){
    state = StateCheck();
    switch (state){
      case IDLES:
        lcd.noBacklight();
        lcd.clear();
        break;
      case WELCOME:
        lcd.clear();
        lcd.backlight();
        lcd.setCursor(0,0);
        lcd.print("Welcome");
        break;
      case INSERT_RFID:
        lcd.setCursor(0,0);
        lcd.print("Scan Your Card");
        break;
      case RFID_SCAN:
        lcd.clear();
        break;
      case PWD:
        lcd.setCursor(0,0);
        lcd.print("Enter Passcode");        
        break;
      case UNLOCKED:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Welcome ");
        lcd.print(currentUser->getUsername());
        break;

      default:
        state = IDLES;
        break;
    }
  }
  return 0;
}
//Main Functions

/* Dont use this
char keypadScan(){
  char temp = keypad.getKey();
  if ((int)keypad.getState() == PRESSED){
    if(temp != NULL){
      keyx = temp;
    }
  }
  if ((int)keypad.getState() == HOLD){
    keypadState++;
    keypadState = constrain(keypadState, 1, n-1);
    delay(holdDelay);
  }
  if ((int)keypad.getState() == RELEASED){
    keyx += keypadState;
    keypadState = 0;
    return keyx;
  }
}
*/
// State Machine
enum State StateCheck(){  //check state of the system
  // IDLES
  if (state == IDLES){
    char temp1 = pressedKey();
    for(int i = 0; i<ROWs; i++){  //press any key 
      for(int j = 0 ; j<COLs; j++){
        if (temp1 == keys[i][j]){
          return WELCOME; 
        }
      }
    }
  }
  // WELCOME
  if (state == WELCOME){
    // if key '*' pressed -> state = RFID
    char temp = pressedKey(); //check for key
    

    if (temp == '*'){
      // state = RFID;
      return INSERT_RFID;
    }
    // if (IDLE_flag){
    //   _delay_ms(3000);
    //   IDLE_flag = false;
    //   return IDLES;
    // }
    
  }
  // INSERT_RFID
  if (state == INSERT_RFID){
    if ((! mfrc522.PICC_IsNewCardPresent()) || (! mfrc522.PICC_ReadCardSerial())) { //If a new PICC placed to RFID reader continue
      return INSERT_RFID;
    } 
    else {
      return RFID_SCAN;
    }
  }
  // RFID
  if (state == RFID_SCAN){
    // if check UID == true -> state = PWD
    // else display invalid message, delay 3 sec -> state = WELCOME
    if (checkUID()){
      lcd.setCursor(0,0);
      lcd.print("Valid Card");
      _delay_ms(2000);
      // state = PWD;
      return PWD;
    } else {
      lcd.setCursor(0,0);
      lcd.print("Invalid Card");
      _delay_ms(2000);
      // state = WELCOME;
      return WELCOME;
    }
  }
  // PWD
  if (state == PWD){
      if (checkPWD()) {
        lcd.setCursor(0,1);
        lcd.print("Access Granted");
        _delay_ms(1500);  
        return UNLOCKED;
      } else {
        if (PWDtried_count > 2) {
          PWDtried_count = 1;
          lcd.clear();
          lcd.setCursor(1,0);
          lcd.print("Access Denied");
          _delay_ms(1500);  // wait 1.5s
          lcd.clear();
          return IDLES;
        } else {
          PWDtried_count++;
          lcd.setCursor(0,1);
          lcd.print("Incorrect");
          _delay_ms(1500);  // wait 1.5s
          lcd.clear();
        }
        
      }
    // if PWD correct && tried < 3 -> state = UNLOCKED
    // else display error message, delay 3 secs -> state == WELCOME
  }
  // UNLOCKED
  if (state == UNLOCKED){
    _delay_ms(5000);
    return IDLES;
  }
}
// RFID
String getUID(){  //get UID from user
  // Getting ready for Reading PICCs
  String tempo = "";
  // Serial.print("Before: ");
  Serial.println(tempo);
  for ( uint8_t i = 0; i < 4; i++) { // The MIFARE PICCs that we use have 4 byte UID
    tempo.concat(String(mfrc522.uid.uidByte[i], HEX)); // Adds the 4 bytes in a single String variable
  }
  tempo.toUpperCase();
  mfrc522.PICC_HaltA(); // Stop reading

  // Serial.print("After: ");
  // Serial.println(tempo);
  return tempo;  
}
bool checkUID(){  
  // check UID with user and admin 
  // return true if match
  // false if not match
  String UID_temp = getUID();
  for(int i = 0; i< UserCount;i++){
    if (UID_temp == users[i].getUserUID()){
      currentUser = &users[i];
      // Serial.print("true: ");
      // Serial.println(UID_temp);
      return true;
    } 
  }
  return false;
}

char pressedKey(){
  char temp;
  while(1){
    temp = keypad.getKey();
    if (temp != NO_KEY){ //return only if there key is press
      // IDLE_flag = false;
      break;
    } else {
      // IDLE_flag = true;
      // return;
    }
  }
  return temp;
}
// PWD
String getPWD(){
  lcd.setCursor (0,1);
  int index = 0;
  String PWD_temp = "";
  while(true){
    char pressed = pressedKey();
    if (pressed == '#'){
      index = 0;
      break; 
    } else {
      PWD_temp+=pressed;
      lcd.setCursor(index,1);
      lcd.print("*");
      index++;
    }
  }
  return PWD_temp;
}
bool checkPWD(){  //check passcode
  PWD_ip = getPWD();
  if (PWD_ip == currentUser->getUserPWD()){
    return true;
  }
  return false;
}
