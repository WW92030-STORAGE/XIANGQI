// Original library and code base - https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <bits/stdc++.h> 
using namespace std;

#include <esp_random.h>
#include <EEPROM.h>

#include "matrices.h"
#include "panels.h"
#include "xiangqi.h"
#include "genetic.h"

vector<vector<vector<bool>>> animation{grid0, grid1, grid2, grid3, grid4, grid5, grid6, grid7, grid8};

// EEPROM

#define EEPROM_SIZE 1
uint8_t flashbyte = 0;

void eeprom_setup() {
  EEPROM.begin(EEPROM_SIZE);
  flashbyte = EEPROM.read(0);
  EEPROM.write(0, (flashbyte != 0) ? 0 : 127);
  EEPROM.commit();
}

// SETUP

SET_LOOP_TASK_STACK_SIZE(32*1024);

void setup() {
  eeprom_setup();
  matrixsetup();
  disp(grid0);
}

int blinkdelay = 0;
int blinkthreshold = 0;

// ANIMATION

void anim_loop(uint16_t color) {
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m * chain; j++) col[i][j] = color;
  }
  delay(100);
  // Serial.println(rand() % 1024);
  disp(grid0);

  blinkdelay++;
  if (blinkdelay >= blinkthreshold) {
    blinkdelay = 0;
    blinkthreshold = (int)(runif(16, 64));
    playAnimation(animation, 20, false);
    playAnimation(animation, 20, true);
    dispstats();
    Serial.printf("BLINK THRESHOLD %ld\n", blinkthreshold);
  }
}

// LOOP

XGame game;
int16_t status = 0;

XAI ai1(1.465682, 0.377270, 1.772698, 0.043397, 0.140934, -1.568957, 0.009095, -0.374828, 0.744469, 1000.000000, 0.062204);
XAI ai2(0.475478, 1.896725, 0.368725, 1.433821, -1.249733, 0.028993, 0.899625, -0.425733, 1.093600, 1000.000000, -0.064280);

std::pair<int, int> src = {0, 0};
std::pair<int, int> vec = {0, 0};
XPiece movingpiece;

void loop() {
  if (flashbyte == 0) {
    anim_loop(matrixpanel->color565(255, 255, 255));
    return;
  }
  render(game, true);
  dispstats();

  auto move = (game.sidetomove) ? ai1.pick(game) : ai2.pick(game);
  dispstats();

  src = move.first;
  vec = move.second;
  movingpiece = game.get(src.first, src.second);
  std::cout << src.first << " " << src.second << " > " << vec.first << " " << vec.second << "\n";

  game.execute(src, vec);
  game.sidetomove = !game.sidetomove;

  status = 0;
  if (game.checkmate()) status = (game.sidetomove) ? -2 : 2;
  if (game.stalemate()) status = (game.sidetomove) ? -1 : 1;

  if (game.checkmate() || game.stalemate()) {
    game.reset();
    status = 0;
    delay(900);
    render(game, true);
  }

  delay(100);


}

void render(XGame game, bool useCheckerboard) {
  matrixpanel->clearScreen();
  uint16_t bg = hex565(0x404040);
  if (status == 2) bg = hex565(0x400000);
  else if (status == -2) bg = hex565(0x004040);
  else if (status != 0) bg = 0;
  for (int x = 0; x < 8; x++) {
    for (int y = 0; y < 9; y++) matrixpanel->drawRect(3 * x + 1, 3 * y + 1, 4, 4, bg);
  }

  if (useCheckerboard) {
    matrixpanel->clearScreen();
    for (int x = 0; x < 9; x++) {
      for (int y = 0; y < 10; y++) {
        if ((x + y) & 1) matrixpanel->fillRect(3 * x, 3 * y, 3, 3, bg);
      }
    }
  }

  matrixpanel->fillRect(3 * src.first, 3 * (9 - src.second), 3, 3, hex565(0x808080));
  matrixpanel->fillRect(3 * (src.first + vec.first), 3 * (9 - src.second - vec.second), 3, 3, hex565(0x808080));

  for (int x = 0; x < 9; x++) {
    for (int y = 0; y < 10; y++) {
      int yvis = 9 - y;

      XPiece piece = game.get(x, y);
      if (piece.isEmpty()) continue;
      int16_t color = piece.getColor() ? hex565(0xFF0000) : hex565(0x00FFFF);

      if (piece.isPawn()) matrixpanel->drawPixel(3 * x + 1, 3 * yvis + 1, color);
      if (piece.isKing()) matrixpanel->drawRect(3 * x, 3 * yvis, 3, 3, color);
      if (piece.isAdvisor()) {
        matrixpanel->drawPixel(3 * x, 3 * yvis, color);
        matrixpanel->drawPixel(3 * x + 2, 3 * yvis, color);
        matrixpanel->drawPixel(3 * x + 2, 3 * yvis + 2, color);
        matrixpanel->drawPixel(3 * x, 3 * yvis + 2, color);
      }
      if (piece.isElephant()) {
        matrixpanel->drawPixel(3 * x, 3 * yvis, color);
        matrixpanel->drawPixel(3 * x + 2, 3 * yvis, color);
        matrixpanel->drawPixel(3 * x + 2, 3 * yvis + 2, color);
        matrixpanel->drawPixel(3 * x, 3 * yvis + 2, color);
        matrixpanel->drawPixel(3 * x + 1, 3 * yvis + 1, color);
      }
      if (piece.isRook()) {
        matrixpanel->drawPixel(3 * x + 1, 3 * yvis, color);
        matrixpanel->drawPixel(3 * x + 1, 3 * yvis + 2, color);
        matrixpanel->drawPixel(3 * x + 2, 3 * yvis + 1, color);
        matrixpanel->drawPixel(3 * x, 3 * yvis + 1, color);
        matrixpanel->drawPixel(3 * x + 1, 3 * yvis + 1, color);
      }
      if (piece.isKnight()) {
        matrixpanel->drawPixel(3 * x, 3 * yvis, color);
        matrixpanel->drawPixel(3 * x + 1, 3 * yvis, color);
        matrixpanel->drawPixel(3 * x + 2, 3 * yvis, color);
        matrixpanel->drawPixel(3 * x, 3 * yvis + 1, color);
        matrixpanel->drawPixel(3 * x, 3 * yvis + 2, color);
      }
      if (piece.isCannon()) {
        matrixpanel->drawPixel(3 * x + 1, 3 * yvis, color);
        matrixpanel->drawPixel(3 * x + 1, 3 * yvis + 2, color);
        matrixpanel->drawPixel(3 * x + 2, 3 * yvis + 1, color);
        matrixpanel->drawPixel(3 * x, 3 * yvis + 1, color);
      }
    }
  }
}