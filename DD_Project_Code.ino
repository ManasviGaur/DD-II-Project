//Code for DD-II project "SNAKE GAME" by Manasvi Gaur [2K19/EC/104] and Lovish Arya [2K19/EC/102]

#include "LedControl.h"

struct Pin {
  static const short JS_pinX = A2;
  static const short JS_pinY = A3;
  static const short JS_pinVCC = 15;
  static const short JS_pinGND = 14;

  static const short potentiometer = A5;
  
  static const short LED_clk = 10;
  static const short LED_cs  = 11;
  static const short LED_din = 12;
};

const short intensity = 6;

const short LED_OutputSpeed = 5;

const short initialSnakeLength = 3;


void setup() {
  start();
  calibrateJST_();
  DisplaySnakeLED_Output();
}


void loop() {
  generateFood();
  scanJST_();
  calculateSnake(); 
  handleGameStates();

}


LedControl LED_screen(Pin::LED_din, Pin::LED_clk, Pin::LED_cs, 1);

struct Point {
  int row_number = 0, column_number = 0;
  Point(int row_number = 0, int column_number = 0): row_number(row_number), column_number(column_number) {}
};

struct object_coordinate {
  int x = 0, y = 0;
  object_coordinate(int x = 0, int y = 0): x(x), y(y) {}
};

bool win = false;
bool gameOver = false;

Point snake;

Point fruit(-1, -1);

object_coordinate JST_Base(500, 500);


int snakeLength = initialSnakeLength;
int snakeSpeed = 1;
int snakeDirection = 0;

const short up     = 1;
const short right  = 2;
const short down   = 3;
const short left   = 4;

const int JST_Threshold = 160;

const float logarithmity = 0.4;

int gameMatrix[8][8] = {};

void generateFood() {
  if (fruit.row_number == -1 || fruit.column_number == -1) {
    if (snakeLength >= 64) {
      win = true;
      return;
    }

    do {
      fruit.column_number = random(8);
      fruit.row_number = random(8);
    } while (gameMatrix[fruit.row_number][fruit.column_number] > 0);
  }
}


void scanJST_() {
  int previousDirection = snakeDirection;
  long timestamp = millis();

  while (millis() < timestamp + snakeSpeed) {
    float raw = mapf(analogRead(Pin::potentiometer), 0, 1023, 0, 1);
    snakeSpeed = mapf(pow(raw, 3.5), 0, 1, 10, 1000);
    if (snakeSpeed == 0) snakeSpeed = 1;

    analogRead(Pin::JS_pinY) < JST_Base.y - JST_Threshold ? snakeDirection = up    : 0;
    analogRead(Pin::JS_pinY) > JST_Base.y + JST_Threshold ? snakeDirection = down  : 0;
    analogRead(Pin::JS_pinX) < JST_Base.x - JST_Threshold ? snakeDirection = left  : 0;
    analogRead(Pin::JS_pinX) > JST_Base.x + JST_Threshold ? snakeDirection = right : 0;

    snakeDirection + 2 == previousDirection && previousDirection != 0 ? snakeDirection = previousDirection : 0;
    snakeDirection - 2 == previousDirection && previousDirection != 0 ? snakeDirection = previousDirection : 0;

    LED_screen.setLed(0, fruit.row_number, fruit.column_number, millis() % 100 < 50 ? 1 : 0);
  }
}


void calculateSnake() {
  switch (snakeDirection) {
    case up:
      snake.row_number--;
      LED_EdgeCase();
      LED_screen.setLed(0, snake.row_number, snake.column_number, 1);
      break;

    case right:
      snake.column_number++;
      LED_EdgeCase();
      LED_screen.setLed(0, snake.row_number, snake.column_number, 1);
      break;

    case down:
      snake.row_number++;
      LED_EdgeCase();
      LED_screen.setLed(0, snake.row_number, snake.column_number, 1);
      break;

    case left:
      snake.column_number--;
      LED_EdgeCase();
      LED_screen.setLed(0, snake.row_number, snake.column_number, 1);
      break;

    default:
      return;
  }

  if (gameMatrix[snake.row_number][snake.column_number] > 1 && snakeDirection != 0) {
    gameOver = true;
    return;
  }

  if (snake.row_number == fruit.row_number && snake.column_number == fruit.column_number) {
    fruit.row_number = -1;
    fruit.column_number = -1;

    snakeLength++;

    for (int row_number = 0; row_number < 8; row_number++) {
      for (int column_number = 0; column_number < 8; column_number++) {
        if (gameMatrix[row_number][column_number] > 0 ) {
          gameMatrix[row_number][column_number]++;
        }
      }
    }
  }

  gameMatrix[snake.row_number][snake.column_number] = snakeLength + 1;

  for (int row_number = 0; row_number < 8; row_number++) {
    for (int column_number = 0; column_number < 8; column_number++) {
      if (gameMatrix[row_number][column_number] > 0 ) {
        gameMatrix[row_number][column_number]--;
      }

      LED_screen.setLed(0, row_number, column_number, gameMatrix[row_number][column_number] == 0 ? 0 : 1);
    }
  }
}

void LED_EdgeCase() {
  snake.column_number < 0 ? snake.column_number += 8 : 0;
  snake.column_number > 7 ? snake.column_number -= 8 : 0;
  snake.row_number < 0 ? snake.row_number += 8 : 0;
  snake.row_number > 7 ? snake.row_number -= 8 : 0;
}


void handleGameStates() {
  if (gameOver || win) {
    unrollSnake();

    DisplayScoreLED_Output(snakeLength - initialSnakeLength);

    if (gameOver) DisplayGameOverLED_Output();
    else if (win) DisplayWinLED_Output();

    win = false;
    gameOver = false;
    snake.row_number = random(8);
    snake.column_number = random(8);
    fruit.row_number = -1;
    fruit.column_number = -1;
    snakeLength = initialSnakeLength;
    snakeDirection = 0;
    memset(gameMatrix, 0, sizeof(gameMatrix[0][0]) * 8 * 8);
    LED_screen.clearDisplay(0);
  }
}


void unrollSnake() {
  LED_screen.setLed(0, fruit.row_number, fruit.column_number, 0);

  delay(800);
  
  for (int i = 0; i < 5; i++) {
    for (int row_number = 0; row_number < 8; row_number++) {
      for (int column_number = 0; column_number < 8; column_number++) {
        LED_screen.setLed(0, row_number, column_number, gameMatrix[row_number][column_number] == 0 ? 1 : 0);
      }
    }

    delay(20);

    for (int row_number = 0; row_number < 8; row_number++) {
      for (int column_number = 0; column_number < 8; column_number++) {
        LED_screen.setLed(0, row_number, column_number, gameMatrix[row_number][column_number] == 0 ? 0 : 1);
      }
    }

    delay(50);

  }


  delay(600);

  for (int i = 1; i <= snakeLength; i++) {
    for (int row_number = 0; row_number < 8; row_number++) {
      for (int column_number = 0; column_number < 8; column_number++) {
        if (gameMatrix[row_number][column_number] == i) {
          LED_screen.setLed(0, row_number, column_number, 0);
          delay(100);
        }
      }
    }
  }
}


void calibrateJST_() {
  object_coordinate values;

  for (int i = 0; i < 10; i++) {
    values.x += analogRead(Pin::JS_pinX);
    values.y += analogRead(Pin::JS_pinY);
  }

  JST_Base.x = values.x / 10;
  JST_Base.y = values.y / 10;
}


void start() {
  pinMode(Pin::JS_pinVCC, OUTPUT);
  digitalWrite(Pin::JS_pinVCC, HIGH);

  pinMode(Pin::JS_pinGND, OUTPUT);
  digitalWrite(Pin::JS_pinGND, LOW);

  LED_screen.shutdown(0, false);
  LED_screen.setIntensity(0, intensity);
  LED_screen.clearDisplay(0);

  randomSeed(analogRead(A5));
  snake.row_number = random(8);
  snake.column_number = random(8);
}


const PROGMEM bool snakeLED_Output[8][56] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const PROGMEM bool gameOverLED_Output[8][90] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const PROGMEM bool scoreLED_Output[8][58] = {
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

const PROGMEM bool digits[][8][8] = {
  {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 1, 1, 0, 1, 1, 1, 0},
    {0, 1, 1, 1, 0, 1, 1, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0}
  },
  {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 1, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0}
  },
  {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 0, 0, 0, 0, 1, 1, 0},
    {0, 0, 0, 0, 1, 1, 0, 0},
    {0, 0, 1, 1, 0, 0, 0, 0},
    {0, 1, 1, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0}
  },
  {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 0, 0, 0, 0, 1, 1, 0},
    {0, 0, 0, 1, 1, 1, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0}
  },
  {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 1, 0, 0},
    {0, 0, 0, 1, 1, 1, 0, 0},
    {0, 0, 1, 0, 1, 1, 0, 0},
    {0, 1, 0, 0, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 1, 1, 0, 0},
    {0, 0, 0, 0, 1, 1, 0, 0}
  },
  {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 0, 0, 1, 1, 0},
    {0, 0, 0, 0, 0, 1, 1, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0}
  },
  {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 1, 1, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0}
  },
  {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 0, 0, 0, 1, 1, 0, 0},
    {0, 0, 0, 0, 1, 1, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0}
  },
  {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0}
  },
  {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 1, 0},
    {0, 0, 0, 0, 0, 1, 1, 0},
    {0, 1, 1, 0, 0, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 0, 0}
  }
};

void DisplaySnakeLED_Output() {
  [&] {
    for (int d = 0; d < sizeof(snakeLED_Output[0]) - 7; d++) {
      for (int column_number = 0; column_number < 8; column_number++) {
        delay(LED_OutputSpeed);
        for (int row_number = 0; row_number < 8; row_number++) {
          LED_screen.setLed(0, row_number, column_number, pgm_read_byte(&(snakeLED_Output[row_number][column_number + d])));
        }
      }

      if (analogRead(Pin::JS_pinY) < JST_Base.y - JST_Threshold
              || analogRead(Pin::JS_pinY) > JST_Base.y + JST_Threshold
              || analogRead(Pin::JS_pinX) < JST_Base.x - JST_Threshold
              || analogRead(Pin::JS_pinX) > JST_Base.x + JST_Threshold) {
        return;
      }
    }
  }();

  LED_screen.clearDisplay(0);

  while (analogRead(Pin::JS_pinY) < JST_Base.y - JST_Threshold
          || analogRead(Pin::JS_pinY) > JST_Base.y + JST_Threshold
          || analogRead(Pin::JS_pinX) < JST_Base.x - JST_Threshold
          || analogRead(Pin::JS_pinX) > JST_Base.x + JST_Threshold) {}

}


void DisplayGameOverLED_Output() {
  [&] {
    for (int d = 0; d < sizeof(gameOverLED_Output[0]) - 7; d++) {
      for (int column_number = 0; column_number < 8; column_number++) {
        delay(LED_OutputSpeed);
        for (int row_number = 0; row_number < 8; row_number++) {
          LED_screen.setLed(0, row_number, column_number, pgm_read_byte(&(gameOverLED_Output[row_number][column_number + d])));
        }
      }

      if (analogRead(Pin::JS_pinY) < JST_Base.y - JST_Threshold
              || analogRead(Pin::JS_pinY) > JST_Base.y + JST_Threshold
              || analogRead(Pin::JS_pinX) < JST_Base.x - JST_Threshold
              || analogRead(Pin::JS_pinX) > JST_Base.x + JST_Threshold) {
        return; 
      }
    }
  }();

  LED_screen.clearDisplay(0);

  while (analogRead(Pin::JS_pinY) < JST_Base.y - JST_Threshold
          || analogRead(Pin::JS_pinY) > JST_Base.y + JST_Threshold
          || analogRead(Pin::JS_pinX) < JST_Base.x - JST_Threshold
          || analogRead(Pin::JS_pinX) > JST_Base.x + JST_Threshold) {}

}

void DisplayWinLED_Output() {
}

void DisplayScoreLED_Output(int score) {
  if (score < 0 || score > 99) return;

  
  int second = score % 10;
  int first = (score / 10) % 10;

  [&] {
    for (int d = 0; d < sizeof(scoreLED_Output[0]) + 2 * sizeof(digits[0][0]); d++) {
      for (int column_number = 0; column_number < 8; column_number++) {
        delay(LED_OutputSpeed);
        for (int row_number = 0; row_number < 8; row_number++) {
          if (d <= sizeof(scoreLED_Output[0]) - 8) {
            LED_screen.setLed(0, row_number, column_number, pgm_read_byte(&(scoreLED_Output[row_number][column_number + d])));
          }

          int c = column_number + d - sizeof(scoreLED_Output[0]) + 6;

          if (score < 10) c += 8;

          if (c >= 0 && c < 8) {
            if (first > 0) LED_screen.setLed(0, row_number, column_number, pgm_read_byte(&(digits[first][row_number][c])));
          } else {
            c -= 8;
            if (c >= 0 && c < 8) {
              LED_screen.setLed(0, row_number, column_number, pgm_read_byte(&(digits[second][row_number][c])));
            }
          }
        }
      }

      if (analogRead(Pin::JS_pinY) < JST_Base.y - JST_Threshold
              || analogRead(Pin::JS_pinY) > JST_Base.y + JST_Threshold
              || analogRead(Pin::JS_pinX) < JST_Base.x - JST_Threshold
              || analogRead(Pin::JS_pinX) > JST_Base.x + JST_Threshold) {
        return;
      }
    }
  }();

  LED_screen.clearDisplay(0);

}

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
