#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
//#include <regex.h>

#ifdef _WIN32
#define WINVER 0x0500
#include <windows.h>
#endif

//#define START_COMBO 'c'
#define STOP_MACRO 'x'

#define GAMEWINDOW "Notepad"
//#define GAMEWINDOW "Artix Game Launcher"
#define CDSLACK 0


enum macroType {
  COMBO,
  KEYSEQ
};

typedef struct {
  char name[32];
  double cooldownSec;
  clock_t activated;
  //bool active;
} skill;

typedef struct {
  char name[32];
  skill skills[6];
} class;

typedef struct {
  int skills[10];
  size_t nCombo;
  char regKey; // Key to start the macro
} combo;

typedef struct {
  char key;
  enum macroType type;
} macro;

typedef struct {
  class* class;
  combo* combos[10];
  size_t nCombos;
  macro* macros[20];
  size_t nMacros;
  bool active;
  HWND* gameWin;
  HWND* consoleWin;
} bot;

bool getWindows(bot* bot) {
  printf("Searching for window: %s\n", GAMEWINDOW);

  // Get first window on desktop
  HWND firstwindow = FindWindowEx(NULL, NULL, NULL, NULL);
  HWND window = firstwindow;
  TCHAR windowtext[MAX_PATH];
   
  // We need to get the console title in case we
  // accidentally match the search word with it
  // instead of the intended target window.
  TCHAR consoletitle[MAX_PATH];
  GetConsoleTitle(consoletitle, MAX_PATH);
   
  while(1)
  {
    //fprintf(stderr, ".");
     
    // Check window title for a match
    GetWindowText(window, windowtext, MAX_PATH);
    if (strstr(windowtext, GAMEWINDOW) != NULL && strcmp(windowtext, consoletitle) != 0) {
      break;
    }
     
    // Get next window
    window = FindWindowEx(NULL, window, NULL, NULL);
    if (window == NULL || window == firstwindow)
    {
      fprintf(stderr, "Window not found\n");
      return 0;
    }
  }

  fprintf(stderr, "Window found: %s\n", windowtext);
  HWND console = GetActiveWindow();
  memcpy(bot->gameWin, &window, sizeof(window));
  memcpy(bot->consoleWin, &console, sizeof(console));
  return 1;
}

class* initClass() {
  class* c = malloc(sizeof(class));
  return c;
}

void freeBot(bot* bot) {
  // Free class
  free(bot->class);

  // Free combos
  for (int i = 0; i < bot->nCombos; i++) {
    free(bot->combos[i]);
  }

  // Free macros
  for (int i = 0; i < bot->nMacros; i++) {
    free(bot->macros[i]);
  }

  free(bot->gameWin);
  free(bot->consoleWin);
  free(bot);
}

bot* initBot(class* class) {
  bot* b = malloc(sizeof(bot));
  b->class = class;
  b->nCombos = 0;
  b->nMacros = 0;
  b->active = false;
  b->gameWin = malloc(sizeof(HWND));
  b->consoleWin = malloc(sizeof(HWND));

  if (!getWindows(b)) {
    freeBot(b);
    return NULL;
  }

  return b;
}

/**
skill* initSkill() {
  skill* s = malloc(sizeof(skill));
  s->cooldownSec = -1;
  return s;
}
**/

macro* initMacro(enum macroType type, char key) {
  macro* m = malloc(sizeof(macro));
  m->type = type;
  m->key = key;
  return m;
}

combo* initCombo() {
  combo* c = malloc(sizeof(combo));
  c->nCombo = 0;
  char regKey = '\0';
  return c;
}

bool isActive(skill* skill) {
  clock_t cd = CLOCKS_PER_SEC * (skill->cooldownSec + CDSLACK);
  return (clock() - skill->activated) < cd;
}

void pressKey(char key) {
  INPUT ip;
  // Set up a generic keyboard event.
  ip.type = INPUT_KEYBOARD;
  ip.ki.wScan = 0; // hardware scan code for key
  ip.ki.time = 0;
  ip.ki.dwExtraInfo = 0;

  // Key press
  char hex[2];
  sprintf(hex, "%x", key);
  ip.ki.wVk = *hex; // virtual-key code for key
  ip.ki.dwFlags = 0; // 0 for key press
  SendInput(1, &ip, sizeof(INPUT));

  // Release the key
  ip.ki.dwFlags = KEYEVENTF_KEYUP;
  SendInput(1, &ip, sizeof(INPUT));
}

void useSkill(class* class, int skill) {
  while (isActive(&(class->skills[skill-1]))) {
    // Do nothing
  }

  class->skills[skill-1].activated = clock();
  printf("%d ", skill);
  pressKey(skill);
}

int findCombo(bot* bot, char key) {
  for (int i = 0; i < bot->nCombos; i++) {
    if (bot->combos[i]->regKey == key) return i;
  }
  return -1;
}

void startCombo(bot* bot, char key) {
  int indexCombo = findCombo(bot, key);
  if (indexCombo < 0) {
    printf("No combo on key %c\n", key);
    return;
  }
  combo* c = bot->combos[indexCombo];
  for (int i = 0; i < c->nCombo; i++) {
    useSkill(bot->class, c->skills[i]);
  }
  printf(".\n");
}

macro* findMacro(bot* bot, char key) {
  for (int i = 0; i < bot->nMacros; i++) {
    if (bot->macros[i]->key == key) return bot->macros[i];
  }
  return NULL;
}

void useMacro(bot* bot, macro* macro) {
  SetForegroundWindow(*(bot->gameWin));
  switch (macro->type) {
    case COMBO:
      startCombo(bot, macro->key);
      break;
    case KEYSEQ:
      break;
    default:
      break;
  }
  SetForegroundWindow(*(bot->consoleWin));
}

bool startsWith(const char* str, const char* p) {
  return strncmp(str, p, strlen(p)) == 0;
  size_t lenp = strlen(p), lenstr = strlen(str);
  return lenstr < lenp ? false : memcmp(str, p, lenp) == 0;
  //return strncmp(a, b, strlen(b)) == 0;
}

char* substring(char* str, int startIndex, int length) {
  size_t len;
  if (length < 0) len = strlen(str)-1-startIndex;
  else len = length;
  char* subs = malloc(sizeof(len+1));
  strncpy(subs, str + startIndex, len);
  subs[len] = '\0';
  return subs;
}

int firstOf(char* str, char c) {
  for (int i = 0; i < strlen(str); i++){
    if (str[i] == c) return i;
  }
  return -1;
}

void parseClassName(class* class, char* str) {
  int d = firstOf(str, ' ');
  strncpy(class->name, str + d, strlen(str)-d);
  class->name[strlen(str)-d] = '\0';
}

void parseCombo(bot* bot, char* comboStr) {
  combo* combo = initCombo();
  combo->nCombo = 0;

  char c;
  combo->regKey = comboStr[1];
  for (int i = firstOf(comboStr, ' '); i < strlen(comboStr) - 1; i++) {
    c = comboStr[i];
    if (c == '>' || c == ' ') continue;
    combo->skills[combo->nCombo++] = atoi(&c);
  }
  if (combo->nCombo > 0) {
    bot->combos[bot->nCombos++] = combo;
    bot->macros[bot->nMacros++] = initMacro(COMBO, combo->regKey);
  }
}

void parseSkill(class* class, char* str) {
      int sn = atoi(&str[1]);
      char* seconds = str + firstOf(str, ' ') + 1;

      skill skill;
      skill.activated = false;
      skill.cooldownSec = atof(seconds);
      memcpy(&(class->skills[sn]), &skill, sizeof(skill));
}

bot* readFromFile(char* fileName) {
  FILE* file = fopen(fileName, "r");

  if (!file) {
    printf("Can't find file %s\n", fileName);
    return NULL;
  }

  class* class = initClass();
  bot* bot = initBot(class);

  char line[128];
  while (fgets(line, sizeof(line), file)) {
    if (strcmp(line, "") == 0 || line[0] == '#') continue;
    if (startsWith(line, "name: ")) {
      parseClassName(class, line);
    }
    else if (startsWith(line, "@")) {
      parseSkill(class, line);
    }
    else if (startsWith(line, ":")) {
      parseCombo(bot, line);
    }

  }
  fclose(file);
  
  printf("Class: %s\n", bot->class->name);
  printf("Parsed %d macros\n", bot->nMacros);

  return bot;
}

char getInput() {
  return getch();
}

bot* initialize(char* fileName) {
  bot* bot = readFromFile(fileName);
  return bot;
}

void run(bot* bot) {
  char input = ' ';
  while(1) {
    bot->active = false;
    input = getInput();
    switch(input) {
      case STOP_MACRO:
	bot->active = false;
	break;
      case 'q':
	return;
      default:
	macro* macro = findMacro(bot, input);
	if (macro != NULL) {
	  printf("Macro found for key: %c\n", input);
	  bot->active = true;
	  useMacro(bot, macro);
	}
	break;
    }
  }
}

int main(int argc, char** argv) {
  if (argc < 2) {
    printf("Give class file as an argument\n");
    return 1;
  }

  char* fileName = argv[1];
  bot* bot = initialize(fileName);

  if (bot != NULL) {
    run(bot);
  }

  freeBot(bot);
  printf("Exited the program\n");

  return 0;
}

