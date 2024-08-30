#include "agnos/agnos.hpp"
#include <curses.h>
#include <fstream>
#include <iostream> 
#include <vector>
#include <cmath>
#include <cstring>
#include <string>
#include <regex>

#define NONE 1
#define ESC 27

// input indices
#define MOVEUP 0
#define MOVERIGHT 1
#define MOVEDOWN 2
#define MOVELEFT 3
#define REPEAT 4
#define SAVENEW 5
#define QUIT 6
#define ASCIICOLOR 7
#define INSERT 8
#define FIRSTCOLOR 9
#define LASTCOLOR 17
#define SAVE 18
#define UNDO 19
#define FLOODFILL 20

using std::string;
using std::vector;

vector<string> ascii;
vector<string> colorCoords;
string filename;
string savedFilename;
int cursorX = 0;
int cursorY = 0;
int repeat = NONE;
bool on = true;
bool isColorMode = false;
bool isInsertMode = false;
bool isFilling = false;
bool saved = false;

int input[] = {'k','l','j','h','r','w','q','c','i','1','2','3','4','5','6','7','8','0','W','u','f'};

// storing actions for the undo feature
struct action {
	short int x, y;
	char prevVal, nextVal;
	bool wasFill, wasColor;

	action(int _x, int _y, char _prevVal, char _nextVal, bool _wasFill, bool _wasColor) {
		x=_x;
		y=_y;
		prevVal=_prevVal;
		nextVal=_nextVal;
		wasFill=_wasFill;
		wasColor=_wasColor;
	}
};
vector<action> actions;

void loadAscii(string filename, vector<string>* ascii, vector<string>* colorCoords) {
	string raw;
	std::fstream ifs(filename); 
	raw.assign((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	char* ptr = std::strtok(&raw[0], "\n");

	// read ascii
	while(ptr[0] != '~') {
		ascii->push_back(ptr);
		ptr = std::strtok(NULL, "\n");
	}

	// discard tilde, and proceed with colorCoords
	ptr = std::strtok(NULL, "\n"); 

	while(ptr != NULL) {
		colorCoords->push_back(ptr);
		ptr = std::strtok(NULL, "\n");
	}
}

void loadConfig() {
	string raw;
	std::fstream ifs("config.txt"); raw.assign((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	char* ptr = std::strtok(&raw[0], "\n");

	int key = 0;
	while(ptr != NULL) {
		// ignore comments and empty lines
		if(ptr[0] == '#' || ptr[0] == ' ') {
			ptr = std::strtok(NULL, "\n");
			continue;
		}

		input[key++] = ptr[0];	
		ptr = std::strtok(NULL, "\n");
	}
}

bool validateFile(int argc, char **argv) {
	if(argc == 1) {
		filename = "newAscii.txt";
		return true;
	}

	filename = argv[1];
	if(filename.find(".txt") == string::npos) {
		printf("Please enter a filename ending with .txt!\n");
		return false;
	}
	std::fstream fileExists(filename);
	if(!fileExists) {
		printf("File doesn't exist!\n");
		return false;
	}

	return true;
}

void getColorCoord(int x, int y, vector<string>* colorCoords) {
	switch(colorCoords->at(y)[x]) {
		case '1': attron(COLOR_PAIR(1)); break;
		case '2': attron(COLOR_PAIR(2)); break;
		case '3': attron(COLOR_PAIR(3)); break;
		case '4': attron(COLOR_PAIR(4)); break;
		case '5': attron(COLOR_PAIR(5)); break;
		case '6': attron(COLOR_PAIR(6)); break;
		case '7': attron(COLOR_PAIR(7)); break;
		case '8': attron(COLOR_PAIR(8)); break;
		default: standend(); break;
	}
}

void draw(int x, int y, vector<string>* ascii, vector<string>* colorCoords) {
	for(int i = 0; i < ascii->size(); i++) {
		for(int cx = 0; cx < ascii->at(i).length(); cx++) {
			getColorCoord(cx, i, colorCoords);
			mvaddch(y+i, fmax(x+cx, 0), ascii->at(i).c_str()[cx]);
		}
	}
}

void save(bool shouldOverride) {
	string asciiStr = "";
	for(int i = 0; i < ascii.size(); i++) {
		asciiStr.append(ascii.at(i) + "\n");
	}
	asciiStr.append("~\n");
	for(int i = 0; i < colorCoords.size(); i++) {
		asciiStr.append(colorCoords.at(i) + "\n");
	}
  
	savedFilename = filename;
	if(!shouldOverride) {
		// keep attempting to save file with name + number
		int attempts = 1;
		savedFilename = filename.substr(0, filename.length() - 4) + "-" + std::to_string(attempts++) + ".txt";
		std::fstream fileExists(savedFilename);
		while(fileExists) {
			savedFilename = filename.substr(0, filename.length() - 4) + "-" + std::to_string(attempts++) + ".txt";
			fileExists.open(savedFilename);
		}
	}

	std::ofstream file(savedFilename);
	file << asciiStr;
}

void edit(char k, int x = cursorX, int y = cursorY, int shouldRecord = true, bool changeColor = isColorMode) {
	// fill in gaps with whitespaces
	if(y > ascii.size()-1) {
		const int REMAINING = y - (ascii.size()-1);
		for(int i = 0; i < REMAINING; i++) {
			ascii.push_back(" ");
			colorCoords.push_back("0");
		}
	}
	if(x > ascii.at(y).length()-1) {
		const unsigned int REMAINING = x - (ascii.at(y).length()-1);
		ascii.at(y).append(REMAINING, ' ');
		colorCoords.at(y).append(REMAINING, '0');
	}

	if(shouldRecord) {
		struct action newAction(x, y, (changeColor ? colorCoords : ascii).at(y)[x], k, isFilling, changeColor);
		actions.push_back(newAction);
	}

	(changeColor ? colorCoords : ascii).at(y)[x] = k;
}

void floodFill(int x, int y, char key, char toReplace) {
	if(
		key == toReplace ||
		(int)toReplace == 0 ||
		(int)key == 0 ||
		x == -1 ||
		y == -1
	) return;

	edit(key, x, y, true);

	// recursive
	auto& content = (isColorMode) ? colorCoords : ascii;
	int s = content.size();
	if(content.at(y)[x-1] == toReplace) 			floodFill(x-1, y, key, toReplace);
	if(content.at(y)[x+1] == toReplace) 			floodFill(x+1, y, key, toReplace);
	if(y+1 < s && content.at(y+1)[x] == toReplace)	floodFill(x, y+1, key, toReplace);
	if(y-1 > -1 && content.at(y-1)[x] == toReplace)	floodFill(x, y-1, key, toReplace);
}

bool isArrowKey(int k) {
	return k == KEY_UP || k == KEY_RIGHT || k == KEY_DOWN || k == KEY_LEFT;
}

bool isInputKey(int k) {
	if(isArrowKey(k)) return true;

	for(int i = 0; i < sizeof(input)-1; i++) {
		if(i >= FIRSTCOLOR && i <= LASTCOLOR) continue;
		if(k == input[i]) return true;
	}
	return false;
}

void checkColorKeys(int k) {
	for(int i = FIRSTCOLOR; i < LASTCOLOR+1; i++) {
		if(k == input[i]) {
			const int NUM_CHAR_OFFSET = 40;
			edit((char)(i + NUM_CHAR_OFFSET));
			break;
		}
	}
}

void undo() {
	const struct action LAST_ACTION = actions.at(actions.size()-1);
	char editChar = LAST_ACTION.prevVal;

	/* 
		to-do: deal with null char directly 
	 	(whitespace might not behave the same across terminals, and undoing this still leaves ascii data as if space was entered) 
	*/
	// replace NULL char with space
	if((int)LAST_ACTION.prevVal == 0) editChar = ' ';
	
	const bool IS_DUMMY = LAST_ACTION.x == -1;
	if(!IS_DUMMY) edit(editChar, LAST_ACTION.x, LAST_ACTION.y, false, LAST_ACTION.wasColor);

	const bool ACTIONS_LEFT = actions.size() > 1;
    if(ACTIONS_LEFT) actions.pop_back();

	// keep calling undo for flood-filled chars (stops at dummy action)
	if(LAST_ACTION.wasFill) undo();
}

void getInput() {
	int k = getch();
	if(k == ESC) {
		isInsertMode = false;
		repeat = NONE;
		return;
	}
	if(isInsertMode && !isArrowKey(k)) {
		edit(k);
		return;
	}
	if(isFilling) {
		char toReplace = (isColorMode ? colorCoords : ascii).at(cursorY)[cursorX];
		// add dummy action, to separate simultaneous flood-fills
		struct action floodAction(-1, -1, toReplace, k, false, isColorMode);
		actions.push_back(floodAction);

		floodFill(cursorX, cursorY, (char)k, toReplace);
		isFilling = false;
		return;
	}

	// in ASCII mode, enter any key that's not in config (including color keys)
	if(!isColorMode && !isInputKey(k) && k != ESC) {
		edit((char)k);
		return;
	}

	// navigation
	if(k == input[MOVEUP]    || k == KEY_UP)    cursorY = fmax(cursorY-1, 0);
	if(k == input[MOVERIGHT] || k == KEY_RIGHT) cursorX++;
	if(k == input[MOVEDOWN]  || k == KEY_DOWN)  cursorY++;
	if(k == input[MOVELEFT]  || k == KEY_LEFT) 	cursorX = fmax(cursorX-1, 0);
	
	if(k == input[UNDO])		undo();
	if(k == input[QUIT])		on = false;
	if(k == input[FLOODFILL])	isFilling = true;
	if(k == input[SAVENEW]) 	save(false);
	if(k == input[SAVE])		save(true);
		
	if(k == input[ASCIICOLOR]) {
		isColorMode = !isColorMode; 
		repeat = NONE;
	}
	if(k == input[INSERT]) {
		isColorMode = false;
		isInsertMode = !isInsertMode;
	}
	if(k == input[REPEAT]) {
		if (repeat != NONE) repeat = NONE;
		else repeat = (isColorMode ? colorCoords : ascii).at(cursorY)[cursorX];
	}
		
	if(isColorMode) checkColorKeys(k);	

	if(repeat != NONE) edit((char)repeat);
}

void displayStatus() {
	attron(COLOR_PAIR(8));

	string mode =		(isColorMode)			? "COLOR" : "ASCII";
	string isRepeat =	(repeat != NONE)		? " REPEAT" : "";
	string insert =		(isInsertMode)			? " INSERT" : "";
	string saved =		(savedFilename != "") 	? " SAVED AS " + savedFilename : "";
	string filling =	(isFilling) 			? " ENTER KEY TO FILL" : "";

	// empty it, so save status disappears
	savedFilename = "";

	string finalStr = mode + isRepeat + insert + saved + filling;
	mvaddstr(LINES-1, 0, finalStr.c_str());
	standend();
}

int main(int argc, char **argv) {
	if(validateFile(argc, argv) == false) return 0;
	initscr();
		
	// configs
	agnos::renameWindow("asciiPainter");
	start_color();
	use_default_colors();
	noecho();
	cbreak();
	agnos::setESCDELAY(0);
	keypad(stdscr, true);
	init_pair(1, COLOR_BLACK, 	-1);
	init_pair(2, COLOR_RED, 	-1);
	init_pair(3, COLOR_GREEN, 	-1);
	init_pair(4, COLOR_YELLOW, 	-1);
	init_pair(5, COLOR_BLUE, 	-1);
	init_pair(6, COLOR_MAGENTA, -1);
	init_pair(7, COLOR_CYAN, 	-1);
	init_pair(8, COLOR_WHITE,	-1);
	init_pair(9, -1,			-1);

	loadConfig();
	
	if(argc == 2) loadAscii(filename, &ascii, &colorCoords);
	else {
		ascii.push_back(" ");
		colorCoords.push_back("0");
	}

	// call these for first frame, since consequent draws happen after getInput() 
	draw(0, 0, &ascii, &colorCoords);
	displayStatus();

	move(0,0);

	while(on) {
		getInput();
		erase();
		draw(0, 0, &ascii, &colorCoords);
		displayStatus();
		move(cursorY, cursorX);
	}

	agnos::closeWin();
}
