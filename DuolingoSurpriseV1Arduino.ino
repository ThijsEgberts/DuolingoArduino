#include "SPI.h"
#include "TFT_22_ILI9225.h"
#include <Servo.h>
#include <ezButton.h>

#define TFT_RST 12
#define TFT_RS  11
#define TFT_CS  13  // SS
#define TFT_SDI 10  // MOSI
#define TFT_CLK 9  // SCK
#define TFT_LED 0   // 0 if wired to +5V directly

#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)

// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED);

//servos
Servo rServo;
Servo lServo;

//leds
#define redLed A0
#define greenLed A1

//buttons
#define button1Pin 4
#define button2Pin 3
#define button3Pin 2
#define button4Pin 7

//initialize buttons
ezButton button1(button1Pin);
ezButton button2(button2Pin);
ezButton button3(button3Pin);
ezButton button4(button4Pin);


// Variables and constants
int greenLedState;
int redLedState;

int button1State;
int button2State;
int button3State;
int button4State;

int button1Down;
int button2Down;
int button3Down;
int button4Down;

//vars
int highScore = 0;
int currentState = 0;
int questionNr = 1;

//questions
// struct questionTemplate {
//   String sentence;
//   String correctAnswer;
//   String incorrectAnswer1;
//   String incorrectAnswer2;
//   String incorrectAnswer3;
// };

#define nrOfQuestions 2
//questionTemplate questions[nrOfQuestions];
//questionTemplate currentQuestion;//the current question
/*
 * Tux black/white image in 180x220 converted using Ardafruit bitmap converter
 * https://github.com/ehubin/Adafruit-GFX-Library/tree/master/Img2Code
 */

// Setup
void setup() {
  //initQuestions();

  rServo.attach(5);
  lServo.attach(6);

  tft.begin();
  tft.setFont(Terminal6x8);
  tft.setOrientation(1);
  tft.fillRectangle(0, 0, tft.maxX(), tft.maxY(), COLOR_GREEN);//background

  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);

  greenLedState = LOW;
  redLedState = HIGH;

  //set button debounce times to 50 ms
  button1.setDebounceTime(50);
  button2.setDebounceTime(50);
  button3.setDebounceTime(50);
  button4.setDebounceTime(50);

  // pinMode(button1, INPUT);
  // pinMode(button2, INPUT);
  // pinMode(button3, INPUT);
  // pinMode(button4, INPUT);

  Serial.begin(9600);
}

// Loop
void loop() {
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

  // if (button1.isPressed()) {
  //   toggleGreen();
  // }
  // if (button2.isPressed()) {
  //   toggleRed();
  // }
  // if (button1State == HIGH) {
  //   digitalWrite(redLed, LOW);
  //   digitalWrite(greenLed, HIGH);
  // } else {
  //   digitalWrite(redLed, HIGH);
  //   digitalWrite(greenLed, LOW);
  // }

  Serial.println("preloop");
  //main loop
  switch (currentState) {
    case 0:
      Serial.println("main menu");
      mainMenu();
      break;
    case 1:
      Serial.println("questionAsked");
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

  Serial.println("hi");

  updateLeds();
  /*
  tft.drawRectangle(0, 0, tft.maxX() - 1, tft.maxY() - 1, COLOR_WHITE);
  tft.setFont(Terminal6x8);
  tft.drawText(10, 10, "hello!");
  delay(1000);
  
  tft.clear();
  tft.drawText(10, 20, "clear");
  delay(1000);

  tft.drawText(10, 30, "text small");
  tft.setBackgroundColor(COLOR_YELLOW);
  tft.setFont(Terminal12x16);
  tft.drawText(90, 30, "BIG", COLOR_RED);
  tft.setBackgroundColor(COLOR_BLACK);
  tft.setFont(Terminal6x8);
  delay(1000);

  tft.drawText(10, 40, "setBacklight off");
  delay(500);
  tft.setBacklight(LOW);
  delay(500);
  tft.setBacklight(HIGH);
  tft.drawText(10, 50, "setBacklight on");
  delay(1000);

  tft.drawRectangle(10, 10, 110, 110, COLOR_BLUE);
  tft.drawText(10, 60, "rectangle");
  delay(1000);

  tft.fillRectangle(20, 20, 120, 120, COLOR_RED);
  tft.drawText(10, 70, "solidRectangle");
  delay(1000);

  tft.drawCircle(80, 80, 50, COLOR_YELLOW);
  tft.drawText(10, 80, "circle");
  delay(1000);

  tft.fillCircle(90, 90, 30, COLOR_GREEN);
  tft.drawText(10, 90, "solidCircle");
  delay(1000);

  tft.drawLine(0, 0, tft.maxX() - 1, tft.maxY() - 1, COLOR_CYAN);
  tft.drawText(10, 100, "line");
  delay(1000);

  for (uint8_t i = 0; i < 127; i++)
    tft.drawPixel(random(tft.maxX()), random(tft.maxY()), random(0xffff));
  tft.drawText(10, 110, "point");
  delay(1000);

  for (uint8_t i = 0; i < 4; i++) {
    tft.clear();
    tft.setOrientation(i);
    tft.drawRectangle(0, 0, tft.maxX() - 1, tft.maxY() - 1, COLOR_WHITE);
    tft.drawText(10, 10, "setOrientation (" + String("0123").substring(i, i + 1) + ")");
    tft.drawRectangle(10, 20, 50, 60, COLOR_GREEN);
    tft.drawCircle(70, 80, 10, COLOR_BLUE);
    tft.drawLine(30, 40, 70, 80, COLOR_YELLOW);
    delay(1000);
  }
  
  tft.setOrientation(0);
  tft.clear();
  tft.setFont(Terminal12x16);
  tft.setBackgroundColor(COLOR_YELLOW);
  tft.drawText(10, 40, "bye!", COLOR_RED);
  tft.setBackgroundColor(COLOR_BLACK);
  tft.setFont(Terminal6x8);
  delay(1000);
  
  tft.drawText(10, 60, "off");
  delay(1000);
  
  tft.setBacklight(false);
  tft.setDisplay(false);
  
  while(true);
  */
}

/*
 * Main menu
 * displays highScore
 * promts to do questions
 * state code 0
 */
void mainMenu() {
  //if play -> questionAsked
  tft.setBackgroundColor(COLOR_GREEN);
  tft.setFont(Terminal12x16);
  tft.drawText(70, 20, "DUOLINGO", COLOR_WHITE);

  tft.setFont(Terminal6x8);
  tft.drawText(80, 70, "High score:", COLOR_WHITE);
  tft.drawText(110, 80, String(highScore));

  tft.drawText(5, 140, "To start a game, press any button", COLOR_BLACK);

  if (button1.isPressed() || button2.isPressed() || button3.isPressed() || button4.isPressed()) {
    currentState = 1;
  }
  Serial.println("out of main menu");
}

/*
 * Question asked state
 * pick a new question and setup and display it
 * state code 1
 */
void questionAsked() {
  //generate question
  //->waitforAnswer
  //currentQuestion = questions[0];

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

  tft.drawChar(30, 30, 'h');
  // tft.setFont(Terminal12x16);
  // tft.drawText(70, 20, "Question " + String(questionNr) + ":", COLOR_WHITE);

  // tft.setFont(Terminal6x8);
  // tft.drawText(10, 50, currentQuestion.sentence);

  // //@TODO question ordering logic

  // //draw the possible answers
  // int questionX = 40;
  // int questionY = 80;
  // tft.drawText(questionX, questionY, "1> " + currentQuestion.correctAnswer);
  // tft.drawText(questionX, questionY + 15, "2> " + currentQuestion.incorrectAnswer1);
  // tft.drawText(questionX, questionY + 30, "3> " + currentQuestion.incorrectAnswer2);
  // tft.drawText(questionX, questionY + 45, "4> " + currentQuestion.incorrectAnswer3);

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
}

/*
 * Next question state
 * after a question has been answered, promt the user wether they want to play again or not
 * state code 5
 */
void nextQuestion() {
  //if yes -> questionAsked
  //if no -> mainMenu
}


//////// Auxiliary methods //////////////

//initialize the question array
// void initQuestions() {
//   questions[0] = {"Test0", "correct", "incorrect1", "incorrect2", "incorrect3"};
//   questions[1] = {"Test1", "correct", "incorrect1", "incorrect2", "incorrect3"};
//   questions[2] = {"Test2", "correct", "incorrect1", "incorrect2", "incorrect3"};
// }

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

//draw the current state of the buttons
void drawButtonStates(int x, int y) {
  tft.drawText(x, y, String(button1State));
  tft.drawText(x, y + 15, String(button2State));
  tft.drawText(x, y + 30, String(button3State));
  tft.drawText(x, y + 45, String(button4State));

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


