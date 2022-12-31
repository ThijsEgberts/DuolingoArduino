#include "SPI.h"
#include "TFT_22_ILI9225.h"
#include <Servo.h>
#include <ezButton.h>
#include <EEPROM.h>
#include <SPI.h>

const int TFT_RST = 12;
const int TFT_RS = 11;
const int TFT_CS = 13; // SS
const int TFT_SDI = 10; // MOSI
const int TFT_CLK = 9;  // SCK
const int TFT_LED = 0;   // 0 if wired to +5V directly

const int TFT_BRIGHTNESS = 200; // Initial brightness of TFT backlight (optional)

// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED);

//servos
Servo rServo;
Servo lServo;

//keep track of time
unsigned long greenTimer = 0;
unsigned long redTimer = 0;

//leds
const int redLed = A1;
const int greenLed = A0;

//buttons
const int button1P = 4;
const int button2P = 3;
const int button3P = 2;
const int button4P = 7;

//initialize buttons
ezButton button1(button1P);
ezButton button2(button2P);
ezButton button3(button3P);
ezButton button4(button4P);


// Variables and constants
int greenLedState;
int redLedState;

bool displayOn = true;

int button1Down;
int button2Down;
int button3Down;
int button4Down;

const unsigned int HIGHSCORE_MEMORY_ADDR = 0; //the eeprom memory adress where the highscore is stored
int highScore;
int currentScore = 0;
int currentState = 0;
int questionNr = 1;
int correctAnswer;

//fake loop iteration variables
int correctIter = 0;
int incorrectIter = 0;

//keep track of the printing
//initially everything is true
bool drawMainMenu = true;
bool drawQuestion = true;
bool drawCorrectAns = true;
bool drawIncorrectAns = true;
bool drawPlayAgain = true;

//questions
struct questionTemplate {
  String sentence;
  String correctAnswer;
  String incorrectAnswer1;
  String incorrectAnswer2;
  String incorrectAnswer3;
};

const int nrOfQuestions = 11;

questionTemplate questions[nrOfQuestions];
questionTemplate* currentQuestion;//the current question

/* 
 * the happy owl bitmap image
 */
static const uint8_t PROGMEM happyOwl[] = {
0x0,0xf,0x0,0x0,0x0,0x0,0x0,0xff,0x0,0xf,
0x0,0xf,0x0,0x0,0x0,0x0,0x0,0xff,0x0,0xf,
0x0,0xf,0x0,0x0,0x0,0x0,0x0,0xff,0x0,0xf,
0x0,0xf,0x0,0x0,0x0,0x0,0x0,0xff,0x0,0xf,
0xf,0xf0,0xff,0xff,0xff,0xff,0xff,0x0,0xff,0xf,
0xf,0xf0,0xff,0xff,0xff,0xff,0xff,0x0,0xff,0xf,
0xf,0xf0,0xff,0xff,0xff,0xff,0xff,0x0,0xff,0xf,
0xf,0xf0,0xff,0xff,0xff,0xff,0xff,0x0,0xff,0xf,
0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,
0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,
0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,
0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,
0xf0,0xf0,0x0,0xf,0xff,0xff,0x0,0x0,0xf0,0xff,
0xf0,0xf0,0x0,0xf,0xff,0xff,0x0,0x0,0xf0,0xff,
0xf0,0xf0,0x0,0xf,0xff,0xff,0x0,0x0,0xf0,0xff,
0xf0,0xf0,0x0,0xf,0xff,0xff,0x0,0x0,0xf0,0xff,
0xf0,0xf,0xff,0xf0,0xff,0xf0,0xff,0xff,0x0,0xff,
0xf0,0xf,0xff,0xf0,0xff,0xf0,0xff,0xff,0x0,0xff,
0xf0,0xf,0xff,0xf0,0xff,0xf0,0xff,0xff,0x0,0xff,
0xf0,0xf,0xff,0xf0,0xff,0xf0,0xff,0xff,0x0,0xff,
0xf0,0xff,0x0,0xff,0xf,0xf,0xf0,0xf,0xf0,0xff,
0xf0,0xff,0x0,0xff,0xf,0xf,0xf0,0xf,0xf0,0xff,
0xf0,0xff,0x0,0xff,0xf,0xf,0xf0,0xf,0xf0,0xff,
0xf0,0xff,0x0,0xff,0xf,0xf,0xf0,0xf,0xf0,0xff,
0xf0,0xf0,0xf,0xf,0xf,0xf,0x0,0xf0,0xf0,0xff,
0xf0,0xf0,0xf,0xf,0xf,0xf,0x0,0xf0,0xf0,0xff,
0xf0,0xf0,0xf,0xf,0xf,0xf,0x0,0xf0,0xf0,0xff,
0xf0,0xf0,0xf,0xf,0xf,0xf,0x0,0xf0,0xf0,0xff,
0xf0,0xf0,0x0,0xf,0xf,0xf,0x0,0x0,0xf0,0xff,
0xf0,0xf0,0x0,0xf,0xf,0xf,0x0,0x0,0xf0,0xff,
0xf0,0xf0,0x0,0xf,0xf,0xf,0x0,0x0,0xf0,0xff,
0xf0,0xf0,0x0,0xf,0xf,0xf,0x0,0x0,0xf0,0xff,
0xf0,0xff,0x0,0xff,0x0,0xf,0xf0,0xf,0xf0,0xff,
0xf0,0xff,0x0,0xff,0x0,0xf,0xf0,0xf,0xf0,0xff,
0xf0,0xff,0x0,0xff,0x0,0xf,0xf0,0xf,0xf0,0xff,
0xf0,0xff,0x0,0xff,0x0,0xf,0xf0,0xf,0xf0,0xff,
0xff,0xf,0xff,0xf0,0xff,0xf0,0xff,0xff,0xf,0xff,
0xff,0xf,0xff,0xf0,0xff,0xf0,0xff,0xff,0xf,0xff,
0xff,0xf,0xff,0xf0,0xff,0xf0,0xff,0xff,0xf,0xff,
0xff,0xf,0xff,0xf0,0xff,0xf0,0xff,0xff,0xf,0xff,
0xf0,0xf0,0x0,0xf,0xf,0xf,0x0,0x0,0xf0,0xff,
0xf0,0xf0,0x0,0xf,0xf,0xf,0x0,0x0,0xf0,0xff,
0xf0,0xf0,0x0,0xf,0xf,0xf,0x0,0x0,0xf0,0xff,
0xf0,0xf0,0x0,0xf,0xf,0xf,0x0,0x0,0xf0,0xff,
0xf0,0xff,0xff,0xff,0xf0,0xff,0xf,0xff,0xf0,0xff,
0xf0,0xff,0xff,0xff,0xf0,0xff,0xf,0xff,0xf0,0xff,
0xf0,0xff,0xff,0xff,0xf0,0xff,0xf,0xff,0xf0,0xff,
0xf0,0xff,0xff,0xff,0xf0,0xff,0xf,0xff,0xf0,0xff,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xff,0xf,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xff,0xf,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xff,0xf,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xff,0xf,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xff,0xf,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xff,0xf,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xff,0xf,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xff,0xf,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xff,0xf0,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xff,0xf0,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xff,0xf0,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xff,0xf0,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xff,0xf0,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xff,0xf0,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xff,0xf0,
0xf0,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xff,0xf0,
0xff,0xf,0xff,0xff,0xff,0xff,0xf0,0xff,0xff,0xf0,
0xff,0xf,0xff,0xff,0xff,0xff,0xf0,0xff,0xff,0xf0,
0xff,0xf,0xff,0xff,0xff,0xff,0xf0,0xff,0xff,0xf0,
0xff,0xf,0xff,0xff,0xff,0xff,0xf0,0xff,0xff,0xf0,
0xff,0xf,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xf0,
0xff,0xf,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xf0,
0xff,0xf,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xf0,
0xff,0xf,0xff,0xff,0xff,0xff,0xff,0xf,0xff,0xf0,
0xff,0xf0,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xf0,
0xff,0xf0,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xf0,
0xff,0xf0,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xf0,
0xff,0xf0,0xff,0xff,0xff,0xff,0xff,0xf0,0xff,0xf0,
0xff,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0x0,0xf0,
0xff,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0x0,0xf0,
0xff,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0x0,0xf0,
0xff,0xff,0x0,0xff,0xff,0xff,0xff,0xff,0x0,0xf0,
0xff,0xff,0xff,0x0,0xf,0xff,0xff,0xff,0xff,0xf,
0xff,0xff,0xff,0x0,0xf,0xff,0xff,0xff,0xff,0xf,
0xff,0xff,0xff,0x0,0xf,0xff,0xff,0xff,0xff,0xf,
0xff,0xff,0xff,0x0,0xf,0xff,0xff,0xff,0xff,0xf,
0xff,0xff,0xf0,0xff,0xf0,0x0,0x0,0x0,0xff,0xf,
0xff,0xff,0xf0,0xff,0xf0,0x0,0x0,0x0,0xff,0xf,
0xff,0xff,0xf0,0xff,0xf0,0x0,0x0,0x0,0xff,0xf,
0xff,0xff,0xf0,0xff,0xf0,0x0,0x0,0x0,0xff,0xf,
0xff,0xff,0xf0,0xf0,0xf,0xff,0xf,0xf0,0xff,0xf,
0xff,0xff,0xf0,0xf0,0xf,0xff,0xf,0xf0,0xff,0xf,
0xff,0xff,0xf0,0xf0,0xf,0xff,0xf,0xf0,0xff,0xf,
0xff,0xff,0xf0,0xf0,0xf,0xff,0xf,0xf0,0xff,0xf,
0xff,0xff,0xf0,0x0,0xf,0x0,0xff,0xff,0xf,0xf,
0xff,0xff,0xf0,0x0,0xf,0x0,0xff,0xff,0xf,0xf,
0xff,0xff,0xf0,0x0,0xf,0x0,0xff,0xff,0xf,0xf,
0xff,0xff,0xf0,0x0,0xf,0x0,0xff,0xff,0xf,0xf,
0xff,0xff,0xff,0xff,0x0,0xf,0xff,0xff,0xf0,0xf,
0xff,0xff,0xff,0xff,0x0,0xf,0xff,0xff,0xf0,0xf,
0xff,0xff,0xff,0xff,0x0,0xf,0xff,0xff,0xf0,0xf,
0xff,0xff,0xff,0xff,0x0,0xf,0xff,0xff,0xf0,0xf
};


// Setup
void setup() {
  initQuestions();

  rServo.attach(5);
  lServo.attach(6);

  tft.begin();
  tft.setOrientation(3); //@TODO set to 1
  tft.setBacklight(HIGH);

  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);

  greenLedState = LOW;
  redLedState = LOW;

  //set button debounce times to 50 ms
  button1.setDebounceTime(50);
  button2.setDebounceTime(50);
  button3.setDebounceTime(50);
  button4.setDebounceTime(50);

  Serial.begin(9600);

  //@TODO read highscore from memory
  //EEPROM.update(HIGHSCORE_MEMORY_ADDR, 0);
  highScore = EEPROM.read(HIGHSCORE_MEMORY_ADDR);
}

// Loop
void loop() {

  //if the display is on, do all the game stuff.
  //if (displayOn) {

    //update button debounces
    button1.loop();
    button2.loop();
    button3.loop();
    button4.loop();
    
    //update button down states
    button1Down = button1.getState();
    button2Down = button2.getState(); 
    button3Down = button3.getState();
    button4Down = button4.getState();

    //main loop
    switch (currentState) {
      case 0:
        mainMenu();
        break;
      case 1:
        questionAsked();
        break;
      case 2:
        waitForAnswer();
        break;
      case 3:
        questionCorrect();
        break;
      case 4:
        questionIncorrect();
        break;
      case 5:
        nextQuestion();
        break;
    }

    updateLeds();
}

/*
 * Main menu
 * displays highScore
 * promts to do questions
 * state code 0
 */
void mainMenu() {
  //if play -> questionAsked
  if (drawMainMenu) {
    tft.fillRectangle(0, 0, tft.maxX(), tft.maxY(), COLOR_GREEN);//background

    tft.setBackgroundColor(COLOR_GREEN);
    tft.setFont(Terminal12x16);
    tft.drawText(70, 20, "DUOLINGO", COLOR_WHITE);

    tft.setFont(Terminal6x8);
    tft.drawText(80, 70, "High score:", COLOR_WHITE);
    tft.drawText(110, 80, String(highScore));

    tft.drawText(5, 140, "To start a game, press any button", COLOR_BLACK);

    questionNr = 0;//reset the counter for the questions

    drawMainMenu = false;
  }

  if (button1.isPressed() || button2.isPressed() || button3.isPressed() || button4.isPressed()) {
    drawMainMenu = true;//now we can go back again if needed
    currentState = 1;
  }
  //Serial.println("out of main menu");
}

/*
 * Question asked state
 * pick a new question and setup and display it
 * state code 1
 */
void questionAsked() {
  //generate question
  //->waitforAnswer
  //@TODO do the question generation logic here
  long randInt = random(nrOfQuestions);
  //Serial.println(String(randInt));
  currentQuestion = &questions[randInt];//give the adress of the question  

  questionNr++;

  currentState = 2;//transition to the next state
}

/*
 * Wait for an answer
 * state code 2
 */
void waitForAnswer() {
  //if correct -> questionCorrect
  //if incorrect -> questionIncorrect
  //if back -> mainMenu
  if (drawQuestion) {
    tft.fillRectangle(0, 0, tft.maxX(), tft.maxY(), COLOR_GREEN);//background

    tft.setFont(Terminal12x16);
    tft.drawText(50, 30, "Question " + String(questionNr) + ":", COLOR_WHITE);

    tft.setFont(Terminal6x8);
    tft.drawText(160, 5, "Score: " + String(currentScore), COLOR_BLACK);
    tft.drawText(40, 70, currentQuestion->sentence, COLOR_BLACK);

    //draw the possible answers
    int questionX = 50;
    int questionY = 90;

    //do the random questionOrdering
    // long randInt = random(4);
    switch (random(24)) {
      //correct in 1
      case 0:
        correctAnswer = 0;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer3);
      break;
      case 1:
        correctAnswer = 0;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer3);
      break;
      case 2:
        correctAnswer = 0;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer1);
      break;
      case 3:
        correctAnswer = 0;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer2);
      break;
      case 4:
        correctAnswer = 0;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer1);
      break;
      case 5:
        correctAnswer = 0;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer2);
      break;

      //correct in 2
      case 6:
        correctAnswer = 1;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer3);
      break;
      case 7:
        correctAnswer = 1;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer3);
      break;
      case 8:
        correctAnswer = 1;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer1);
      break;
      case 9:
        correctAnswer = 1;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer2);
      break;
      case 10:
        correctAnswer = 1;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer1);
      break;
      case 11:
        correctAnswer = 1;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer2);
      break;

      //correct in 3
      case 12:
        correctAnswer = 2;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer3);
      break;
      case 13:
        correctAnswer = 2;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer3);
      break;
      case 14:
        correctAnswer = 2;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer1);
      break;
      case 15:
        correctAnswer = 2;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer2);
      break;
      case 16:
        correctAnswer = 2;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer1);
      break;
      case 17:
        correctAnswer = 2;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer2);
      break;

      //correct in 4
      case 18:
        correctAnswer = 3;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->correctAnswer);
      break;
      case 19:
        correctAnswer = 3;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->correctAnswer);
      break;
      case 20:
        correctAnswer = 3;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->correctAnswer);
      break;
      case 21:
        correctAnswer = 3;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->correctAnswer);
      break;
      case 22:
        correctAnswer = 3;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->correctAnswer);
      break;
      case 23:
        correctAnswer = 3;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer3);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->correctAnswer);
      break;

      default:
        correctAnswer = 0;
        tft.drawText(questionX, questionY, "1> " + currentQuestion->correctAnswer);
        tft.drawText(questionX, questionY + 15, "2> " + currentQuestion->incorrectAnswer1);
        tft.drawText(questionX, questionY + 30, "3> " + currentQuestion->incorrectAnswer2);
        tft.drawText(questionX, questionY + 45, "4> " + currentQuestion->incorrectAnswer3);
      break;
    }
    

    drawQuestion = false;
  }

  //answering logic
  if (button1.isPressed()) {
    if (correctAnswer == 0) {
      currentState = 3;
    } else {
      currentState = 4;
    }
    drawQuestion = true;
  }
  if (button2.isPressed()) {
    if (correctAnswer == 1) {
      currentState = 3;
    } else {
      currentState = 4;
    }
    drawQuestion = true;
  }
  if (button3.isPressed()) {
    if (correctAnswer == 2) {
      currentState = 3;
    } else {
      currentState = 4;
    }
    drawQuestion = true;
  }
  if (button4.isPressed()) {
    if (correctAnswer == 3) {
      currentState = 3;
    } else {
      currentState = 4;
    }
    drawQuestion = true;
  }
}

/*
 * Question correct state
 * show the correct answer by turing it green
 * wave the wings
 * turn the eyes green for a bit
 * state code 3
 */
void questionCorrect() {
  //do the animations and stuff
  //update score
  //-> nextQuestion
  
  if (drawCorrectAns) {
    tft.fillRectangle(0, 0, tft.maxX(), tft.maxY(), COLOR_GREEN);//background

    tft.setFont(Terminal12x16);
    tft.drawText(50, 20, "Congrats!", COLOR_WHITE);
    tft.setFont(Terminal6x8);
    tft.drawText(15, 60, "That was the correct answer :)", COLOR_WHITE);

    //@TODO draw happy owl image
    //drawHappyOwlImg(40, 80, COLOR_WHITE, COLOR_BLACK);
    tft.drawBitmap(tft.maxX() / 2 - 40, 75, happyOwl, 80, 100, COLOR_GREEN, COLOR_BLACK);

    //update the highScore and score
    currentScore += 1;
    if (currentScore > highScore) {
      highScore = currentScore;
    }

    drawCorrectAns = false;
  }
  //set servo's?

  //blink the green LED
  //make sure to loop an even amount because elsewise it won't be in the same state at the beginning and in the end
  if (correctIter < 16) {
    if (millis() > greenTimer + 200) {
      greenTimer = millis();
      toggleGreen();
      correctIter++;
    }
  } else {
    drawCorrectAns = true;
    currentState = 5;
    correctIter = 0;
  }
}

/*
 * Question incorrect state
 * show the correct answer by turning it green
 * turn the eyes red for a bit
 * state code 4
 */
void questionIncorrect() {
  //do the animations
  //-> nextQuestion

  if (drawIncorrectAns) {
    tft.fillRectangle(0, 0, tft.maxX(), tft.maxY(), COLOR_GREEN);//background

    tft.setFont(Terminal12x16);
    tft.drawText(tft.maxX() / 2 - 18, 20, "Ahw", COLOR_WHITE);
    tft.setFont(Terminal6x8);
    tft.drawText(15, 45, "That answer was incorrect :(", COLOR_WHITE);
    tft.drawText(5, 65, "The correct answer was: " + currentQuestion->correctAnswer);

    //@TODO draw sad owl image
    //tft.drawBitmap();
    //tft.drawBitmap(40, 40, happyOwl, 80, 100);
    tft.drawBitmap(tft.maxX() / 2 - 40, 80, happyOwl, 80, 100, COLOR_GREEN, COLOR_BLACK);
    drawIncorrectAns = false;
  }

  //blink the red LED
  //make sure to loop an even amount because elsewise it won't be in the same state at the beginning and in the end
  if (incorrectIter < 20) {
    if (millis() > redTimer + 150) {
      redTimer = millis();
      toggleRed();
      incorrectIter++;
    }
  } else {
    drawIncorrectAns = true;
    currentState = 5;
    incorrectIter = 0;
  }
}

/*
 * Next question state
 * after a question has been answered, promt the user wether they want to play again or not
 * state code 5
 */
void nextQuestion() {
  //if yes -> questionAsked
  //if no -> mainMenu
  if (drawPlayAgain) {
    tft.fillRectangle(0, 0, tft.maxX(), tft.maxY(), COLOR_GREEN);//background

    tft.setFont(Terminal12x16);
    tft.drawText(30, 20, "Play again?", COLOR_WHITE);
    tft.setFont(Terminal6x8);
    
    tft.drawText(60, 110, "HighScore: " + String(highScore), COLOR_BLACK);
    tft.drawText(60, 125, "Current score: " + String(currentScore), COLOR_BLACK);

    tft.drawText(70, 65, "1> yes", COLOR_WHITE);
    tft.drawText(70, 80, "2> no", COLOR_WHITE);

    drawPlayAgain = false;
  }

  if (button1.isPressed()) {
    drawPlayAgain = true;
    currentState = 1;
  }
  if (button2.isPressed()) {
    //EEPROM.update(HIGHSCORE_MEMORY_ADDR, highScore);//updates the permanent memory if the highscore has changed since the last write

    drawPlayAgain = true;
    currentScore = 0;
    currentState = 0;
  }
}


//////// Auxiliary methods //////////////

//initialize the question array
//don't add more questions than there are entries in the array (nrOfQuestions)
void initQuestions() {
  questions[0] = {"Test0", "correct", "incorrect1", "incorrect2", "incorrect3"};
  questions[1] = {"Test1", "correct", "incorrect1", "incorrect2", "incorrect3"};
  questions[2] = {"Test2", "correct", "incorrect1", "incorrect2", "incorrect3"};
  questions[3] = {"Test3", "correct", "incorrect1", "incorrect2", "incorrect3"};
  questions[4] = {"Test4", "correct", "incorrect1", "incorrect2", "incorrect3"};
  questions[5] = {"Test5", "correct", "incorrect1", "incorrect2", "incorrect3"};
  questions[6] = {"Test6", "correct", "incorrect1", "incorrect2", "incorrect3"};
  questions[7] = {"Test7", "correct", "incorrect1", "incorrect2", "incorrect3"};
  questions[8] = {"Test8", "correct", "incorrect1", "incorrect2", "incorrect3"};
  questions[9] = {"Test9", "correct", "incorrect1", "incorrect2", "incorrect3"};
  questions[10] = {"Test10", "correct", "incorrect1", "incorrect2", "incorrect3"};
}

//toggle the green leds
void toggleGreen() {
  greenLedState = !greenLedState;
}

//toggle the red leds
void toggleRed() {
  redLedState = !redLedState;
}

//update the leds
void updateLeds() {
  digitalWrite(greenLed, greenLedState);
  digitalWrite(redLed, redLedState);
}

//draw the buttons are currently pressed
void drawButtonDown(int x, int y) {
  tft.drawText(x, y, String(button1Down));
  tft.drawText(x, y + 15, String(button2Down));
  tft.drawText(x, y + 30, String(button3Down));
  tft.drawText(x, y + 45, String(button4Down));

}
//helper method to move the servos in sync
void moveServos(int pos) {
  rServo.write(pos);
  lServo.write(pos);
}

//helper method to move the servos in oposite directions
void moveServosOpposite(int pos) {
  rServo.write(pos);
  lServo.write(180 - pos);
}

// void drawHighSpeedBitmap(uint16_t x1, uint16_t y1, uint16_t** bitmap, int16_t w, int16_t h) {
//     _setWindow(x1, y1, x1+w-1, y1+h-1, L2R_TopDown);
//     startWrite();
//     SPI_DC_HIGH();
//     SPI_CS_LOW();
//     for (uint16_t y = 0; y < h; y++) {
// #ifdef HSPI_WRITE_PIXELS
//         if (_clk < 0) {
//             HSPI_WRITE_PIXELS(bitmap[y], w * sizeof(uint16_t));
//             continue;
//         }
// #endif
//         for (uint16_t x = 0; x < w; x++) {
//             _spiWrite16(bitmap[y][x]);
//         }
//     }
//     SPI_CS_HIGH();
//     endWrite();
// }


