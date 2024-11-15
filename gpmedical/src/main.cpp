#include <Arduino.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

#define amount_of_digits 3

char pulses[amount_of_digits]; // array
char pulses_1[amount_of_digits]; // array
byte pulses_size = 3; // bytes almacenados
byte pulses_size_1 = 3; // bytes almacenados

const int ac_in_zero = 2;

const int triac = 12;

bool state = false;

int state_ac_in_zero = 0;

int lastButtonState_asc = 0;

char customKey; // valor de los key presionados

float final_number; // numero de presentación final

float scaling;

float percent;

float millis_p = 25;

float final_time;

float time_curve = 500;

float division;

float aditional_value = 0;

// variables restrictivas -------------------------------------------------------------
int factor_change_pages = 2;
bool pass_value_in_moment_first_page = false;
bool pass_value_in_moment_third_page = false;
bool temporary_if;
int amount_of_numbers_in_time = 0;
int count;

// strings conditionales 
String condition_for_set_time = "set_time";



enum State {START, SET_VALUES, OPERATE};
State currentState = START;

const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {11, 10, 9, 8}; // rows
byte colPins[COLS] = {7, 6, 5, 4}; // collumn

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

LiquidCrystal_I2C lcd(0x27,16,2);  //definir lcd




//----------------------------------------------------------------------




void setup(){

  // define output
  pinMode(triac, OUTPUT);
  pinMode(ac_in_zero, INPUT);

  // define push
  pinMode(3, INPUT_PULLUP);


  // inicializa la comunicación serial
  Serial.begin(9600);

  // Inicializar el LCD
  lcd.init();
  
  //Encender la luz de fondo.
  lcd.backlight();


  lcd.setCursor(2, 0);
  lcd.print("GPNET SYSTEM");
  lcd.setCursor(4, 1);
  lcd.print("WELCOME");
  delay(4000);
  lcd.clear();

  lcd.setCursor(1, 0);
  lcd.print("PRESS D");
  lcd.setCursor(1, 1);
  lcd.print("TO CONTINUE");
  
}

//-------------------------------------------------------------------------------------------------------------------------------------------



  
void loop(){
  customKey = customKeypad.getKey(); // canal para presionar key

  if ((customKey == 'D' ) && (factor_change_pages == 0)){
    see_page_1 :
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("PRESS # VALUE");
    lcd.setCursor(1, 1);
    lcd.print("PRESS A TIME");
    factor_change_pages = 1;
    pass_value_in_moment_first_page = true;
    pass_value_in_moment_third_page = false;
  }
  
  else if ((customKey == 'D') && (factor_change_pages == 2)){
    see_page_3:
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("PRESS C");
    lcd.setCursor(4, 1);
    lcd.print("TO START");
    currentState = START;
    factor_change_pages = 0;
    pass_value_in_moment_first_page = false;
    pass_value_in_moment_third_page = true;
  }
  else if ((customKey == '*') && (pass_value_in_moment_third_page == true)){
      goto see_page_1;
    }
      else if ((customKey == '*') && (pass_value_in_moment_first_page == true)){
      goto see_page_3;
    }



  switch (currentState) {

    case START:

// cambio de página
      if ((customKey == '#') && (pass_value_in_moment_first_page == true)) 
      { for_apply_hz:
        clearData(); 
        lcd.clear();
        change_values();
        currentState = SET_VALUES;
        goto see_page_1; // For see current page 
      }
      else if ((customKey == 'A') && (pass_value_in_moment_first_page == true))
      {
        clearData_1(condition_for_set_time);
        lcd.clear();
        set_time_curve();
        currentState = SET_VALUES;
        goto see_page_1;
      }
      
      else if ( (customKey == 'C') && (pass_value_in_moment_third_page == true)){
        temporary_if = true;
        currentState = SET_VALUES;
      }
      break;


      case SET_VALUES:
      // agrega los valores cuando sea necesario
      currentState = OPERATE;
      break;



    case OPERATE:

  if ((customKey == '#') && (pass_value_in_moment_first_page == true))
  {
    clearData(); // limpiar array
    lcd.clear();
    change_values(); // set
    currentState = SET_VALUES;
    goto see_page_1;
  } 

  else if ((customKey == 'A') && (pass_value_in_moment_first_page == true))
  {
    clearData_1(condition_for_set_time);
    lcd.clear();
    set_time_curve();
    currentState = SET_VALUES;
    goto see_page_1;
  }
  while (pass_value_in_moment_third_page == true){
  if (digitalRead(3) == LOW && final_time < 5){
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("ERROR");
      lcd.setCursor(3, 1);
      lcd.print("SET TIME");
      delay(2500);
      goto see_page_1;
  }
  else if (digitalRead(3) == LOW && final_number < 10){
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("ERROR");
      lcd.setCursor(4, 1);
      lcd.print("SET VALUE");
      delay(2500);
      goto see_page_1;
  }
  if (temporary_if == true){
    lcd.clear();
    lcd.setCursor(4, 0);
    lcd.print("PRESS *");
    lcd.setCursor(3,1);
    lcd.print("TO CANCEL");
    temporary_if = false;
    }
  aditional_value = 0;
  while (digitalRead(3) == LOW && final_time >= 5)
  {
    temporary_if = true;
    division = scaling/(final_time); // change this multiply to add incrementation
    state_ac_in_zero = digitalRead(ac_in_zero);

    if(state_ac_in_zero != lastButtonState_asc){
      if (state_ac_in_zero == LOW){
        state = true;
      }
    }
    lastButtonState_asc = state_ac_in_zero;
    if(state == true){
      function_repetitive();
    }
  }
  if(customKeypad.getKey() == '*'){
      pass_value_in_moment_third_page = false;
      goto see_page_3;
  }
}
  break;
  }
}


//---------------------------------------------------------------------------------------------------------------------------------------------

//Sección para cambiar los valores (Amplitud)
void change_values(){
inic:
  bool moment_pass_to_confirm_result = false;
  pulses_size = 3;

  while (moment_pass_to_confirm_result == false){
      lcd.setCursor(2, 0);
      lcd.print("*SET A");
      lcd.setCursor(0, 1);
      lcd.print("VALUE 0-100:");

    // bloquear el uso de otros caracteres
    again:
    customKey = customKeypad.getKey();
    if (customKey == '#' || customKey == 'B' || customKey == 'A' ){
      goto again;
    }
    else if (customKey == '*'){
    break;
  }
    // almacena y escribe la información proporcionada
    else if (customKey && customKey != 'C' && customKey != 'D'&& pulses_size <= 3 && pulses_size > 0){
    pulses[pulses_size] = customKey; 
    lcd.setCursor(15 - pulses_size ,1); 
    lcd.print(pulses[pulses_size]); 
    pulses_size--;
 }
    else if(customKey == 'D' && pulses_size < 3){
      pulses_size++; 
      pulses[pulses_size] = ' '; 
      lcd.setCursor(15 - pulses_size,1);
      lcd.print(pulses[pulses_size]); 
  }
    else if(customKey == 'C'){ // condición para aceptar la información actual (Confirm)
        pulses_size = 0; // mandar la información que se encuentre sin importar
        moment_pass_to_confirm_result = true; // rompe el while con la información
      }

  }
  // enviar la información 
    delay(1000);
    if (pulses_size == 0){

    // convertir de array a string concatenandolo y luego a int
    String numberString = "";
    for (int i = 4; i > pulses_size; i--) {
      numberString += String(pulses[i]); // concatenar
    }

    final_number = numberString.toInt(); // convertir a int 
    Serial.println("Percentage obtained: " + String(final_number)); 
    scaling = 8300*(final_number/100);
    
    if (final_number < 0 || final_number > 100){
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("ERROR ONLY");
      lcd.setCursor(2, 1);
      lcd.print("VALUES 10-90");
      delay(4000);
      clearData();
      lcd.clear();
      goto inic;
    }
  }
  lcd.clear();
}

//-------------------------------------------------------------------------------------------------------------------------------

void function_repetitive(){
    // 10 hercios dispuesto a ser set
  delaymicroseconds(aditional_value);
  digitalWrite(triac, HIGH);
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print((aditional_value/scaling)*100);
  Serial.println(aditional_value);
  delaymicroseconds(10);
  digitalWrite(triac, LOW);


  aditional_value += division;
  while (digitalRead(3) == LOW && aditional_value >= scaling){
  delaymicroseconds(scaling); 
  digitalWrite(triac, HIGH);
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print((scaling/scaling)*100);
  Serial.println(scaling);
  delaymicroseconds(10);
  digitalWrite(triac, LOW);
  }
}

//--------------------------------------------------------------------------------------------------------------------------------

// Sección para configurar la curva de tiempo 
void set_time_curve(){
  inic:
  bool moment_pass_to_confirm_result = false;
  pulses_size_1 = 3;

  while (moment_pass_to_confirm_result == false ){
      lcd.setCursor(2, 0);
      lcd.print("*SET A");
      lcd.setCursor(0, 1);
      lcd.print("TIME 5-120:");

    // bloquear el uso de otros caracteres
  again:
  customKey = customKeypad.getKey();
  if (customKey == '#' || customKey == 'B' || customKey == 'A' ){
    goto again;
    }
    else if (customKey == '*'){
    break;
  }
    // almacena y escribe la información proporcionada
    else if (customKey && customKey != 'C' && customKey != 'D'&& pulses_size_1 <= 3 && pulses_size_1 > 0){
    pulses_1[pulses_size_1] = customKey; 
    lcd.setCursor(15 - pulses_size_1 ,1); 
    lcd.print(pulses_1[pulses_size_1]);
    pulses_size_1--;


 }else if(customKey == 'D' && pulses_size_1 < 3){
    pulses_size_1++; 
    pulses_1[pulses_size_1] = ' '; 
    lcd.setCursor(15 - pulses_size_1,1);
    lcd.print(pulses_1[pulses_size_1]); 
 }

 else if(customKey == 'C'){ // condición para aceptar la información actual (Confirm)
        pulses_size_1 = 0; // mandar la información que se encuentre sin importar
        moment_pass_to_confirm_result = true; // rompe el while con la información
      }
  }
  // enviar la información 
    delay(1000);
    if (pulses_size_1 == 0 ){

    // convertir de array a string concatenandolo y luego a int
    String numberString_1 = "";
    for (int i = 3; i > pulses_size_1; i--) {
      numberString_1 += String(pulses_1[i]); // concatenar
    }
    
    final_time = numberString_1.toInt(); // convertir a int
    Serial.println("Time obtained: " + String (final_time));
    
    if (final_time < 5 || final_time > 120){
      lcd.clear();
      lcd.setCursor(3, 0);
      lcd.print("ERROR ONLY");
      lcd.setCursor(2, 1);
      lcd.print("VALUES 5-120");
      delay(4000);
      clearData_1(condition_for_set_time);
      lcd.clear();
      goto inic;
    }
  }
  lcd.clear();
}



//--------------------------------------------------------------------------------------------------------------------------------------------


// Sección para limpiar arrays
void clearData(){
  while(pulses_size < 3){
    pulses_size++;
    pulses[pulses_size] = 0; 
  }
  return;
}

void clearData_1(String condition){

  if (condition == "set_time"){
    while(pulses_size_1 < 3){
      pulses_size_1++;
      Serial.println(pulses_1[pulses_size_1]);
      Serial.println("here");
      pulses_1[pulses_size_1] = 0; 
    }
    return;
  }
}