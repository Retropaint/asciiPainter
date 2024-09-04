#include "agnos/agnos.hpp"
#include "main.hpp"
#include <curses.h>
#include <fstream>
#include <iostream> 
#include <vector>
#include <cmath>
#include <cstring>
#include <string>

using std::string;
using std::vector;

int input[] = {'k','l','j','h','r','w','q','c','i','1','2','3','4','5','6','7','8','0','W','u','f'};
vector<string> ascii, colorCoords;
vector<action> actions;
string filename, savedFilename, message;
struct vec2 cursor(0, 0);
bool programRunning = true;

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
	std::fstream ifs("config.txt"); 
	raw.assign((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

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

void edit(char k, int x = cursor.x, int y = cursor.y, int shouldRecord = true, bool changeColor = colorMode) {
	// if this new char is beyond the current x and y that the content would allow, fill the remaining gaps with whitespaces
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
		struct action newAction(x, y, (changeColor ? colorCoords : ascii).at(y)[x], k, fillMode, changeColor);
		actions.push_back(newAction);
	}

	(changeColor ? colorCoords : ascii).at(y)[x] = k;
}

void floodFill(int x, int y, char key, char toReplace) {
	if(
		key == toReplace    ||
		(int)toReplace == 0 ||
		(int)key == 0       ||
		x == -1             ||
		y == -1
	) return;

	edit(key, x, y, true);

	// recursive
	auto& content = (colorMode) ? colorCoords : ascii;
	int s = content.size();
	if(content.at(y)[x-1] == toReplace)             floodFill(x-1, y, key, toReplace);
	if(content.at(y)[x+1] == toReplace)             floodFill(x+1, y, key, toReplace);
	if(y+1 < s && content.at(y+1)[x] == toReplace)  floodFill(x, y+1, key, toReplace);
	if(y-1 > -1 && content.at(y-1)[x] == toReplace) floodFill(x, y-1, key, toReplace);
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
	
	const bool IS_DUMMY = LAST_ACTION.pos.x == -1;
	if(!IS_DUMMY) edit(editChar, LAST_ACTION.pos.x, LAST_ACTION.pos.y, false, LAST_ACTION.wasColor);

	const bool ACTIONS_LEFT = actions.size() > 1;
    if(ACTIONS_LEFT) actions.pop_back();

	// keep calling undo for flood-filled chars (stops at dummy action)
	if(LAST_ACTION.wasFill) undo();
}

bool isOutOfBounds(int x, int y) {
	return y > ascii.size()-1 || x > ascii.at(y).length();
}

void tryRepeat(int x, int y, bool isColor) {
	if(isOutOfBounds(x, y)) {
		fillNullErr:
			message = "Nothing to repeat!";
			return;
	}

	char c = (colorMode ? colorCoords : ascii).at(cursor.y)[cursor.x];
	if((int)c == NULL_CHAR || c == ' ') goto fillNullErr;

	repeatModeChar = c;
}

void tryFloodFill(int x, int y, bool isColor) {
	if(
		isOutOfBounds(x, y)      ||
		(int)ascii.at(y)[x] == 0 || 
		(isColor && ascii.at(y)[x] == ' ')
	) {
		message = "Can't fill here!";
		return;
	}

	message = "Enter key to fill";
	fillMode = true;
}

void getInput() {
	int k = getch();
	if(k == ESC) {
		insertMode = false;
		repeatModeChar = NULLCHAR;
		return;
	}
	if(insertMode && !isArrowKey(k)) {
		edit(k);
		return;
	}
	if(fillMode) {
		char toReplace = (colorMode ? colorCoords : ascii).at(cursor.y)[cursor.x];
		// add dummy action, to separate simultaneous flood-fills
		struct action floodAction(-1, -1, toReplace, k, false, colorMode);
		actions.push_back(floodAction);

		floodFill(cursor.x, cursor.y, (char)k, toReplace);
		fillMode = false;
		return;
	}

	// in ASCII mode, enter any key that's not in config (including color keys)
	if(!colorMode && !isInputKey(k) && k != ESC) {
		edit((char)k);
		return;
	}

	// navigation
	if(k == input[MOVEUP]    || k == KEY_UP)    cursor.y = fmax(cursor.y-1, 0);
	if(k == input[MOVERIGHT] || k == KEY_RIGHT) cursor.x++;
	if(k == input[MOVEDOWN]  || k == KEY_DOWN)  cursor.y++;
	if(k == input[MOVELEFT]  || k == KEY_LEFT)  cursor.x = fmax(cursor.x-1, 0);
	
	if(k == input[UNDO])    undo();
	if(k == input[QUIT])    programRunning = false;
	if(k == input[SAVENEW]) save(false);
	if(k == input[SAVE])    save(true);

	if(k == input[FLOODFILL]) tryFloodFill(cursor.x, cursor.y, colorMode);
		
	if(k == input[ASCIICOLOR]) {
		colorMode = !colorMode; 
		repeatModeChar = NULLCHAR;
	}
	if(k == input[INSERT]) {
		colorMode = false;
		insertMode = !insertMode;
	}
	if(k == input[REPEAT]) {
		if (repeatModeChar != NULLCHAR) repeatModeChar = NULLCHAR;
		else tryRepeat(cursor.x, cursor.y, k);
	}
		
	if(colorMode) checkColorKeys(k);	

	if(repeatModeChar != NULLCHAR) edit((char)repeatModeChar);
}

void displayStatus() {
	attron(COLOR_PAIR(8));

	string mode =     (colorMode)           ? "COLOR" : "ASCII";
	string insert =   (insertMode)          ? " INSERT" : "";
	string saved =    (savedFilename != "") ? " SAVED AS " + savedFilename : "";
	string isRepeat = (repeatModeChar != NULLCHAR) ? " REPEAT" : "";

	string finalStr = mode + isRepeat + insert + saved;
	mvaddstr(LINES-1, 0, finalStr.c_str());

	// custom message on top of status
	mvaddstr(LINES-2, 0, message.c_str());
	message = "";

	savedFilename = "";
	standend();
}

int main(int argc, char **argv) {
	if(validateFile(argc, argv) == false) return 0;

	if(argc == 2) loadAscii(filename, &ascii, &colorCoords);
	else {
		ascii.push_back(" ");
		colorCoords.push_back("0");
	}

	loadConfig();

	initscr();
	agnos::renameWindow("asciiPainter");
	start_color();
	use_default_colors();
	noecho();
	cbreak();
	agnos::setESCDELAY(0);
	keypad(stdscr, true);
	init_pair(1, COLOR_BLACK,   -1);
	init_pair(2, COLOR_RED,     -1);
	init_pair(3, COLOR_GREEN,   -1);
	init_pair(4, COLOR_YELLOW,  -1);
	init_pair(5, COLOR_BLUE,    -1);
	init_pair(6, COLOR_MAGENTA, -1);
	init_pair(7, COLOR_CYAN,    -1);
	init_pair(8, COLOR_WHITE,   -1);
	init_pair(9, -1,            -1);

	// call these for first frame, since consequent draws happen after getInput() 
	draw(0, 0, &ascii, &colorCoords);
	displayStatus();
	move(0, 0);

	while(programRunning) {
		getInput();
		erase();
		draw(0, 0, &ascii, &colorCoords);
		displayStatus();
		move(cursor.y, cursor.x);
	}

	agnos::closeWin();
}
