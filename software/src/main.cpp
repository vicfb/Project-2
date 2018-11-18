#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Bounce2.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define POT 14
#define BUTTON 8

enum class GameState{GS_Start, GS_Pause, GS_Playing, GS_GameOver}; //States of the state machine

GameState Current_page = GameState::GS_Start;//Initial State 

// Instantiate a Bounce object
Bounce debouncer = Bounce(); 

float ypos = 0;//ball starts at the top
float level = 1;//smallest speed
uint8_t xpos = random(display.width());//randomly selects the position on y

//variables to keep the score
uint8_t score = 0;
uint8_t previous_score = 0;

//size of the line to catch the ball
const uint8_t line_size = 20;

//flags
bool ballDown = false;
bool game_over = false;


//Displays the score on the top right
void Score(uint8_t score){
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.print("Score: ");
  display.setCursor(110, 0);
  display.print(score);
}

//generates a circle at a random place and moves it down
void moveCircle(uint8_t xpos){
  display.setCursor(75, 0);
  Score(score);
  for (int16_t i=0; i<3; i+=1) {
    display.drawCircle(xpos, ypos, i, WHITE);
  }
  ypos = ypos+level;
  
  display.drawCircle(xpos, ypos, 1, BLACK);
  
  if(ypos > display.height()){
    ypos = 0;
    level = level + 0.5;
    ballDown = true; 
  }
  
}

//draws the line
void drawLine(uint8_t pot_value) {  
    display.drawLine(pot_value, display.height()-1, line_size+pot_value, display.height()-1, WHITE);
    display.display();

}

//moves the line with the pot
void moveLine(uint16_t line_position){
  uint16_t pot_value = line_position; 
  
  drawLine(pot_value);
  display.clearDisplay();

}

//Page shown at the start of the game
void Start() {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(30, 2);
  display.clearDisplay();
  display.println("CATCH");
  display.setCursor(15, 19);
  display.println("THE BALL");
  display.setTextSize(1);
  display.setCursor(2,40);
  display.println("Press button to Begin");
  display.display();
  
}

//Page shown when the game is paused
void Pause() {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(35, display.height()/2);
  display.clearDisplay();
  display.print("PAUSE");
  display.display();
}

//Function that handles the game
void Playing(){
  uint16_t line_position = analogRead(POT);
  uint16_t beginning_of_line;
  uint16_t end_of_line;
      
  line_position = map<int, int, int, int, int>(line_position, 0, 1023, 0, (display.width()-line_size));
  
  beginning_of_line = line_position;
  end_of_line = beginning_of_line + line_size;

  moveCircle(xpos);
  moveLine(line_position);
  
  
  if((xpos>=beginning_of_line) && (xpos<=end_of_line) && (ballDown)){
    display.setCursor(80, 0);
    score++;
    ballDown = false;
    //Serial.print(score);
    xpos = random(display.width());
    
  }
    else if(ballDown){
      ballDown = false;
      game_over = true;
      xpos = random(display.width());
      previous_score = score;
      score = 0;
      level = 1;
    }

}

//Page shown at the end of the game
void GameOver(uint8_t previous_score){
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.clearDisplay();
  display.println("GAME OVER");
  display.setTextSize(1);
  display.setCursor(0, 30);
  display.print("Your score is: ");
  display.setCursor(90,30);
  display.println(previous_score);
  display.setCursor(0, 45);
  display.println("Press the button");
  display.print("to start again");
  display.display();
}

void setup()   {
  Serial.begin(9600);

  pinMode(POT, INPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);  // initialize with the I2C addr 0x3C
  // init done
  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);

  display.clearDisplay();

  //attach the debouncer to the button
  debouncer.attach(BUTTON);

}


void loop() {
  debouncer.update(); //Update the Bouncer

  //Switch of states of the state machine occurs when the button is pressed or the game is over.
  switch(Current_page){
    
    case GameState::GS_Start:
      Start();
      if(debouncer.fell()){
        Current_page = GameState::GS_Playing;
      }
      break;

    case GameState::GS_Playing:
      Playing();
      if(debouncer.fell()){
        Current_page = GameState::GS_Pause;
      }
      else if (game_over){
        Current_page = GameState::GS_GameOver;
      }
      break;
    
    case GameState::GS_Pause:
      Pause();
      if (debouncer.fell()){
        Current_page = GameState::GS_Playing;
      }
      break;
    
    case GameState::GS_GameOver:
      GameOver(previous_score);
      if(debouncer.fell()){
        game_over = false;
        Current_page = GameState::GS_Playing;
      }
      break;
  }
  

}