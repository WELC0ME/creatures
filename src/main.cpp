#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>
#include <ctime>

const int BOARD_WIDTH = 45;
const int BOARD_HEIGHT = 25;
const int SQUARE_SIZE = 20;
const int BORDER = 1;
const int RESULTS_LENGTH = 10;

const int BOTS_COUNT = 64;
const int FOOD_COUNT = 150;
const int POISON_COUNT = 50;
const int GENOM_LENGTH = 64;
const int MIN_ALIVE = 8;

const int POISON = 1;
const int WALL = 2;
const int BOT = 3;
const int FOOD = 4;
const int EMPTY = 5;

typedef struct {
  int x;
  int y;
} Point;

typedef struct {
  Point position;
  int hp;
  int genom[GENOM_LENGTH];
  int rotation = 0;
  int command = 0;
} Bot;

const Point LEFT_CORNER = Point { 50, 50 };
const Point RESULTS_CORNER = Point { 1050, 50 };
const Point GENERATION_CORNER = Point { 50, 600 };
const Point TURN_CORNER = Point { 1050, 600 };
const Point RECORD_CORNER = Point { 1200, 600 };

const int RESULT_STEP = 50;
const Point SHIFTS[8] = {
  Point { -1, -1 },
  Point {  0, -1 },
  Point {  1, -1 },
  Point {  1,  0 },
  Point {  1,  1 },
  Point {  0,  1 },
  Point { -1,  1 },
  Point { -1,  0 },
};

int board[BOARD_HEIGHT][BOARD_WIDTH];
Bot bots[BOTS_COUNT];
int results[RESULTS_LENGTH];
int generation = 0;
int turn = 0;
int record = 0;

int get (Point* _point) {
  return board[_point->y][_point->x];
}

void set (Point* _point, int _value) {
  board[_point->y][_point->x] = _value;
}

void getEmpty (Point* _point) {
  while (true) {
    _point->x = rand() % BOARD_WIDTH; 
    _point->y = rand() % BOARD_HEIGHT;
    if (get(_point) == EMPTY)
      break;
  }
}

void placeItem (int _count, int _value) {
  Point cell;
  for (int i = 0; i < _count; i++) {
    getEmpty(&cell);
    set(&cell, _value);
    if (_value == BOT) {
      bots[i].position.x = cell.x;
      bots[i].position.y = cell.y;
    }
  }
}

void init () {
  for (int i = 0; i < BOARD_HEIGHT; i++)
    for (int k = 0; k < BOARD_WIDTH; k++)
      if (i == 0 || k == 0 || i == BOARD_HEIGHT - 1 || k == BOARD_WIDTH - 1)
        board[i][k] = WALL;
      else
        board[i][k] = EMPTY;

  Point walls[12] = {
    Point { 10,  1 },
    Point { 10,  2 },
    Point { 10,  3 },
    Point { 10,  4 },
    Point { 10,  5 },
    Point { 10,  6 },
    Point { 20, 10 },
    Point { 20, 11 },
    Point { 20, 12 },
    Point { 20, 13 },
    Point { 20, 14 },
    Point { 20, 15 },
  };
  for (int i = 0; i < 12; i++)
    set(&walls[i], WALL);

  placeItem(FOOD_COUNT, FOOD);
  placeItem(POISON_COUNT, POISON);
  placeItem(BOTS_COUNT, BOT);

  for (int i = 0; i < BOTS_COUNT; i++) {
    bots[i].hp = 10;
    bots[i].command = 0;
    bots[i].rotation = 0;
  }

  for (int i = RESULTS_LENGTH - 1; i > 0; i--)
    results[i] = results[i - 1];
  results[0] = turn;

  if (turn > record)  
    record = turn;
  turn = 0;
  generation++;
}

void randomGenom () {
  for (int i = 0; i < BOTS_COUNT; i++)
    for (int k = 0; k < GENOM_LENGTH; k++)
      bots[i].genom[k] = rand() % 64;
}

void mutateGenom () {
  int goodGenom[MIN_ALIVE][GENOM_LENGTH];
  int counter = 0;

  for (int i = 0; i < BOTS_COUNT; i++)
    if (bots[i].hp > 0) {
      for (int k = 0; k < GENOM_LENGTH; k++)
        goodGenom[counter][k] = bots[i].genom[k];
      counter++;
    }

  int children = BOTS_COUNT / MIN_ALIVE;
  for (int i = 0; i < MIN_ALIVE; i++) {
    for (int k = 0; k < children; k++)
      for (int j = 0; j < GENOM_LENGTH; j++)
        bots[i * children + k].genom[j] = goodGenom[i][j];
      
    for (int k = 0; k < rand() % 4; k++)
      bots[(i + 1) * children - 1].genom[
        rand() % GENOM_LENGTH] = rand() % 64;
  }

  for (int i = MIN_ALIVE * children; i < BOTS_COUNT; i++)
    for (int k = 0; k < GENOM_LENGTH; k++)
      bots[i].genom[k] = goodGenom[0][k];
  
}

void getNextPosition (Point* _point, int _index, int _command) {
  Point shift = SHIFTS[(bots[_index].rotation + _command) % 8];
  _point->x = bots[_index].position.x + shift.x;
  _point->y = bots[_index].position.y + shift.y;
}

void gameStep () {
  int alive = 0;
  for (int i = 0; i < BOTS_COUNT; i++) {
    if (bots[i].hp <= 0)
      continue;

    alive++;
    int target = 0;
    for (int k = 0; k < 10; k++) {
      int current = bots[i].genom[ bots[i].command ];
      Point nextPosition;
      getNextPosition(&nextPosition, i, current);
      target = get(&nextPosition);

      if (current >= 0 && current <= 7) {
        if (target == EMPTY || target == FOOD || target == POISON) {
          set(&bots[i].position, EMPTY);
          set(&nextPosition, BOT);
          bots[i].position.x = nextPosition.x;
          bots[i].position.y = nextPosition.y;

          if (target == FOOD) {
            bots[i].hp += 10;
            placeItem(1, FOOD);
          }
          else if (target == POISON) {
            bots[i].hp = 0;
            set(&bots[i].position, EMPTY);
            placeItem(1, POISON);
          }
        }      
        break;

      } else if (current >= 8 && current <= 15) {
        if (target == FOOD) {
          bots[i].hp += 10;
          set(&nextPosition, EMPTY);
          placeItem(1, FOOD);
        }
        else if (target == POISON) {
          set(&nextPosition, FOOD);
        }

        break;

      } else if (current >= 16 && current <= 23) {
      } else if (current >= 24 && current <= 31) {
        bots[i].rotation = (bots[i].rotation + current % 8) % 8;
        target = 1;

      } else if (current >= 32 && current <= 63) {
        target = current;
      }
      bots[i].command = (bots[i].command + target) % GENOM_LENGTH;
    }

    bots[i].command = (bots[i].command + target) % GENOM_LENGTH;
    bots[i].hp--;
    if (bots[i].hp <= 0)
      set(&bots[i].position, EMPTY);
  }

  if (alive <= MIN_ALIVE) {
    mutateGenom();
    init();
  } else {
    turn++;
  }
}

int main() {
  srand(time(0));
  sf::VideoMode mode = sf::VideoMode::getFullscreenModes()[0];
  sf::RenderWindow window(mode, "", sf::Style::Fullscreen);
  window.setFramerateLimit(60);

  sf::Event event;
  sf::RectangleShape cell;
  sf::Font font;
  sf::Text text;
  sf::Color grey(128, 128, 128);

  cell.setSize(sf::Vector2f(SQUARE_SIZE, SQUARE_SIZE));
  cell.setOutlineColor(sf::Color::White);
  cell.setOutlineThickness(BORDER);

  font.loadFromFile("fonts/font.ttf");
  text.setFont(font);
  text.setFillColor(sf::Color::White);
  text.setCharacterSize(12);

  init();
  randomGenom();
  bool active = false;

  while (window.isOpen()) {

    while (window.pollEvent(event)) {
      switch (event.type) {

        case sf::Event::KeyPressed:
          switch (event.key.code) {
            
            case sf::Keyboard::Q:
              window.close();
              break;
            
            case sf::Keyboard::S:
              if (!active)
                gameStep();
              break;
            
            case sf::Keyboard::E:
              active = !active;
              break;
          }
          break;

        default:
          break;

      }
    }

    if (active)
      gameStep();
    
    window.clear();

    for (int i = 0; i < BOARD_HEIGHT; i++)
      for (int k = 0; k < BOARD_WIDTH; k++) {
        if (board[i][k] == EMPTY)
          cell.setFillColor(sf::Color::Black);
        else if (board[i][k] == FOOD)
          cell.setFillColor(sf::Color::Green);
        else if (board[i][k] == POISON)
          cell.setFillColor(sf::Color::Red);
        else if (board[i][k] == BOT)
          cell.setFillColor(sf::Color::Blue);
        else if (board[i][k] == WALL)
          cell.setFillColor(grey);
        cell.setPosition(
          LEFT_CORNER.x + k * (SQUARE_SIZE + BORDER),
          LEFT_CORNER.y + i * (SQUARE_SIZE + BORDER)
        );
        window.draw(cell);
      }
    
    for (int i = 0; i < BOTS_COUNT; i++) {
      if (bots[i].hp <= 0)
        continue;
      
      text.setString(std::to_string( bots[i].hp ));
      text.setPosition(
        2 + LEFT_CORNER.x + bots[i].position.x * (SQUARE_SIZE + BORDER),
        2 + LEFT_CORNER.y + bots[i].position.y * (SQUARE_SIZE + BORDER)
      );
      window.draw(text);
    }

    for (int i = 0; i < RESULTS_LENGTH; i++) {
      text.setString(std::to_string( results[i] ));
      text.setPosition(
        RESULTS_CORNER.x,
        RESULTS_CORNER.y + i * RESULT_STEP
      );
      window.draw(text);
    }

    text.setString(std::to_string( generation ));
    text.setPosition(
      GENERATION_CORNER.x,
      GENERATION_CORNER.y
    );
    window.draw(text);

    text.setString(std::to_string( turn ));
    text.setPosition(
      TURN_CORNER.x,
      TURN_CORNER.y
    );
    window.draw(text);

    text.setString(std::to_string( record ));
    text.setPosition(
      RECORD_CORNER.x,
      RECORD_CORNER.y
    );
    window.draw(text);

    window.display();
  }

  return 0;
}
