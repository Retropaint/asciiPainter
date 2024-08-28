#include "agnos/agnos.hpp"
#include <curses.h>
#include <fstream>
#include <iostream> 
#include <vector>
#include <cmath>
#include <cstring>
#include <string>

#define NOCOLOR -1
#define ESC 27

// input indices
#define MOVEUP 0
#define MOVERIGHT 1
#define MOVEDOWN 2
#define MOVELEFT 3
#define REPEAT 4
#define SAVE 5
#define QUIT 6
#define ASCIICOLOR 7
#define INSERT 8
#define FIRSTCOLOR 9
#define LASTCOLOR 16
#define SAVEOVERRIDE 17
#define UNDO 18

using std::string;
using std::vector;

vector<string> ascii;
vector<string> colorCoords;
string filename;
string savedFilename;
int cursorX = 0;
int cursorY = 0;
int repeat = NOCOLOR;
bool on = true;
bool isColorMode = false;
bool isInsertMode = false;
bool saved = false;

int input[] = {'k','l','j','h','r','w','q','c','i','1','2','3','4','5','6','7','8','W','u'};

// storing actions for the undo feature
struct action {
	int x, y;
	char prevVal, currVal;
	bool isColor;
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

void edit(char k, int x = cursorX, int y = cursorY, bool undid = false, bool changeColor = isColorMode) {
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

	if(undid) {
		const bool ACTIONS_LEFT = actions.size() > 1;
		if(ACTIONS_LEFT) actions.pop_back();
	} else {
		struct action newAction;
		newAction.x = x;
		newAction.y = y;
		newAction.prevVal = (!isColorMode) ? ascii.at(y)[x] : colorCoords.at(y)[x];
		newAction.currVal = k;
		newAction.isColor = isColorMode;
		actions.push_back(newAction);
	}

	if(changeColor) colorCoords.at(y)[x] = k;
	else ascii.at(y)[x] = k;
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

	edit(editChar, LAST_ACTION.x, LAST_ACTION.y, true, LAST_ACTION.isColor);
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

void getInput() {
	int k = getch();
	if(k == ESC) {
		isInsertMode = false;
		repeat = NOCOLOR;
	}
	if(isInsertMode && !isArrowKey(k)) {
		edit(k);
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

	if			(k == input[SAVE]) 					save(false);
	else if	(k == input[SAVEOVERRIDE])	save(true);

	if(k == input[QUIT]) on = false;
		
	if(k == input[ASCIICOLOR]) {
		isColorMode = !isColorMode; 
		repeat = NOCOLOR;
	}
	if(k == input[INSERT]) {
		isColorMode = false;
		isInsertMode = !isInsertMode;
	}
	if(k == input[REPEAT]) {
		if(repeat != NOCOLOR) repeat = NOCOLOR;
		else repeat = (isColorMode)? colorCoords.at(cursorY)[cursorX] : ascii.at(cursorY)[cursorX];
	}
	if(k == input[UNDO]) {
		undo();
	}
	
	if(isColorMode) checkColorKeys(k);	

	if(repeat != NOCOLOR) edit((char)repeat);
}

void displayStatus() {
	attron(COLOR_PAIR(8));

	string mode = (isColorMode) ? "COLOR" : "ASCII";
	string isRepeat = (repeat != NOCOLOR) ? " REPEAT" : "";
	string insert = (isInsertMode) ? " INSERT" : "";
	string saved = (savedFilename != "") ? " SAVED AS " + savedFilename : "";
	savedFilename = "";

	string finalStr = mode + isRepeat + insert + saved;
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
	init_pair(1, COLOR_BLACK, -1);
	init_pair(2, COLOR_RED, -1);
	init_pair(3, COLOR_GREEN, -1);
	init_pair(4, COLOR_YELLOW, -1);
	init_pair(5, COLOR_BLUE, -1);
	init_pair(6, COLOR_MAGENTA, -1);
	init_pair(7, COLOR_CYAN, -1);
	init_pair(8, COLOR_WHITE, -1);

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
