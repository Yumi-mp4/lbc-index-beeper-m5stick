#include <Arduino.h>
#include <M5GFX.h>

M5GFX display;

// ===== CONFIG =====
#define BTN_A 11
#define BTN_B 12

#define BG TFT_BLACK
#define FG TFT_CYAN

#define LOGO_W 38
#define LOGO_H 48
#define LOGO_BPR 5

#define BORDER_THICKNESS 2
#define TEXT_CLEAR_H 40
#define TEXT_CLEAR_MARGIN_X BORDER_THICKNESS

// ===== LOGO (PROGMEM) =====
const uint8_t logoBits[] PROGMEM = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,
  0x00,0x00,0x70,0x00,0x00,0x00,0x00,0x78,0x00,0x00,
  0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x38,0x00,0x00,
  0x00,0x0F,0x0B,0xC0,0x00,0x00,0x1F,0xA7,0xE0,0x00,
  0x00,0x0E,0x31,0xE0,0x00,0x00,0x07,0x37,0xC0,0x00,
  0x00,0x00,0x38,0x00,0x00,0x00,0x03,0x7E,0x00,0x00,
  0x00,0x06,0x76,0x00,0x00,0x00,0x07,0x6D,0x80,0x00,
  0x00,0x06,0xAD,0x80,0x00,0x00,0x86,0xAD,0x80,0x40,
  0x01,0x00,0x58,0x00,0x80,0x02,0x00,0x58,0x00,0xC0,
  0x03,0x00,0x58,0x00,0x60,0x06,0x00,0x18,0x00,0x20,
  0x06,0x00,0x18,0x00,0x30,0x0C,0x00,0x18,0x00,0x18,
  0x0C,0x00,0x48,0x00,0x18,0x1C,0x00,0x4F,0xF0,0x18,
  0x1C,0x00,0x7F,0x28,0x1C,0x1C,0x00,0xFF,0xF0,0x1C,
  0x14,0x00,0xF8,0x60,0x1C,0x34,0x01,0x1F,0xE0,0x1C,
  0x34,0x01,0xE1,0xC0,0x14,0x34,0x02,0x00,0x60,0x14,
  0x34,0x02,0x0C,0x80,0x34,0x16,0x02,0x12,0xC0,0x24,
  0x12,0x00,0x0A,0x80,0x2C,0x1B,0x02,0x1D,0x00,0x6C,
  0x19,0x82,0x0E,0x00,0xC8,0x0C,0x82,0x0E,0x01,0x98,
  0x0C,0xC1,0x08,0x03,0x18,0x06,0x71,0x08,0x07,0x30,
  0x06,0x3C,0xFF,0x1C,0x70,0x03,0x0F,0xFF,0xF8,0xE0,
  0x01,0xC3,0xFF,0xE1,0xC0,0x00,0xE0,0x38,0x07,0x00,
  0x00,0x78,0x00,0x1E,0x00,0x00,0x3E,0x00,0x7C,0x00,
  0x00,0x0F,0xFF,0xF0,0x00,0x00,0x01,0xFF,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

// ===== PAGES =====
enum Page { HOME, PRE, TASK, CLR, COIN, D20 };
Page page = HOME;
Page lastPage = (Page)-1;

// ===== TASKS =====
const char* tasks[] = {
  "Find a tree and thank it for the oxygen.",
  "Drink a glass of water slowly.",
  "Observe a bird until it flies away.",
  "Speak only in rhymes for the next 15 minutes.",
  "Write a letter to your future self.",
  "Walk backwards 50 steps.",
  "Compliment the next person you see.",
  "Close your eyes and name five sounds you hear.",
  "Balance a book on your head for one minute.",
  "Draw a circle on the ground and stand in it for a moment.",
  "Hold your breath for a while and take few breaths afterwards.",
  "Acknowledge the weight of your existence.",
  "Find a smooth stone and carry it for the day.",
  "Whisper a secret to the wind.",
  "Stare at a wall until it begins to move.",
  "Sing a song that doesn't exist.",
  "Count your heartbeats until you lose track.",
  "Trace the lines on your palm with a finger."
};
const uint8_t TASK_COUNT = sizeof(tasks) / sizeof(tasks[0]);

// ===== ANIMATION =====
struct Anim {
  const char* t;
  char b[96];
  uint8_t len, i;
  uint32_t tm;
  bool done;
};

char rnd() {
  const char s[] = "@#$%&*+-!?/=";
  return s[random(sizeof(s) - 1)];
}

void animStart(Anim& a, const char* t) {
  a.t = t;
  a.len = min((int)strlen(t), (int)(sizeof(a.b) - 1));
  a.i = 0;
  a.tm = 0;
  a.done = false;
  memset(a.b, ' ', a.len);
  a.b[a.len] = 0;
}

bool animUpdate(Anim& a) {
  if (a.done || millis() - a.tm < 40) return false;
  a.tm = millis();

  for (uint8_t j = 0; j < a.len; j++) {
    if (a.t[j] == ' ') a.b[j] = ' ';
    else if (j < a.i) a.b[j] = a.t[j];
    else a.b[j] = rnd();
  }

  if (++a.i > a.len) {
    memcpy(a.b, a.t, a.len);
    a.done = true;
  }

  return true;
}

// ===== INSTANCES =====
Anim aHome, aMain;

// ===== GENERATORS =====
char d20buf[20];

const char* coin() {
  return random(2) ? "_Yes._" : "_No._";
}

const char* rollD20() {
  snprintf(d20buf, sizeof(d20buf), "_D20: %d._", random(1, 21));
  return d20buf;
}

// ===== DRAW =====
void drawBorders() {
  display.drawRect(0, 0, display.width(), display.height(), FG);
  display.drawRect(1, 1, display.width() - 2, display.height() - 2, FG);
}

void drawLogo() {
  int x = (display.width() - LOGO_W) / 2;
  for (int r = 0; r < LOGO_H; r++)
    for (int c = 0; c < LOGO_W; c++)
      if (pgm_read_byte(&logoBits[r * LOGO_BPR + c / 8]) & (0x80 >> (c % 8)))
        display.drawPixel(x + c, 2 + r, FG);
}

void drawText(const char* t, int centerY) {
  display.setTextColor(FG, BG);
  display.setTextDatum(top_left);

  int maxWidth = display.width() - 30;
  int lineHeight = 16;

  const int MAX_LINES = 5;
  char lines[MAX_LINES][64];
  int lineCount = 0;

  char current[64] = "";
  char word[32];

  int i = 0;
  while (true) {
    int j = 0;
    while (t[i] != ' ' && t[i] != '\0') word[j++] = t[i++];
    word[j] = '\0';

    char test[64];
    if (strlen(current) == 0)
      snprintf(test, sizeof(test), "%s", word);
    else
      snprintf(test, sizeof(test), "%s %s", current, word);

    if (display.textWidth(test) > maxWidth && strlen(current) > 0) {
      if (lineCount < MAX_LINES) strcpy(lines[lineCount++], current);
      strcpy(current, word);
    } else {
      strcpy(current, test);
    }

    if (t[i] == '\0') break;
    i++;
  }

  if (strlen(current) && lineCount < MAX_LINES) {
    strcpy(lines[lineCount++], current);
  }

  int totalHeight = lineCount * lineHeight;
  int y = centerY - totalHeight / 2;

  display.setTextDatum(middle_center);

  for (int k = 0; k < lineCount; k++) {
    display.drawString(lines[k], display.width() / 2, y + lineHeight / 2);
    y += lineHeight;
  }
}

// ===== INPUT =====
bool lastA = false, lastB = false;

void input() {
  bool a = !digitalRead(BTN_A);
  bool b = !digitalRead(BTN_B);

  if (a && !lastA) {
    if (page == PRE) {
      page = TASK;
      animStart(aMain, tasks[random(TASK_COUNT)]);
    } 
    else if (page == TASK) {
      page = CLR;
      animStart(aMain, "_Clear._");
    } 
    else if (page == CLR) {
      page = PRE;
      animStart(aMain, "_Prescript._");
    }
    else if (page == COIN) {
      animStart(aMain, coin());
    }
    else if (page == D20) {
      animStart(aMain, rollD20());
    }
  }

  if (b && !lastB) {
    switch (page) {
      case HOME: page = PRE; break;
      case PRE:  page = COIN; break;
      case COIN: page = D20; break;
      case D20:  page = HOME; break;
      case TASK:
      case CLR:  page = PRE; break;
    }

    if (page == HOME) animStart(aHome, "_Welcome, Proxy._");
    else if (page == PRE) animStart(aMain, "_Prescript._");
    else if (page == COIN) animStart(aMain, "_Hermes._");
    else if (page == D20) animStart(aMain, "_Tyche._");
  }

  lastA = a;
  lastB = b;
}

// ===== RENDER =====
void render() {
  int y = display.height() / 2;

  if (page != lastPage) {
    lastPage = page;
    display.fillScreen(BG);
    drawBorders();
    if (page == HOME) drawLogo();
  }

  Anim* a = (page == HOME) ? &aHome : &aMain;

  if (animUpdate(*a)) {
    display.fillRect(
      TEXT_CLEAR_MARGIN_X,
      y - TEXT_CLEAR_H / 2,
      display.width() - TEXT_CLEAR_MARGIN_X * 2,
      TEXT_CLEAR_H,
      BG
    );
    drawText(a->b, y);
  }
}

// ===== SETUP =====
void setup() {
  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);

  display.begin();
  display.setRotation(1);
  display.setTextSize(2);

  randomSeed(esp_random());

  animStart(aHome, "_Welcome, Proxy._");
}

// ===== LOOP =====
void loop() {
  input();
  render();
}