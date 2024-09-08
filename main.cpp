#include "agnos/agnos.hpp"
#include "main.hpp"
#include <curses.h>
#include <fstream>
#include <iostream> 
#include <vector>
#include <cmath>
#include <cstring>
#include <string>
#include <algorithm>

using std::string;
using std::vector;

int input[] = {'k','l','j','h','r','w','q','c','i','1','2','3','4','5','6','7','8','0','W','u','f','v','y','p','d'};
vector<string> ascii, colorCoords;
vector<action> actions;
vector<contentChar> pseudoSelection, selection;
vector<vec2> toBeFilled;
string filename, savedFilename, message;
struct vec2 cursor(0, 0), prevCursor(0, 0);
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

bool isOutOfBounds(int x, int y) {
	return y > ascii.size()-1 || x > ascii.at(y).length()-1 || y < 0 || x < 0;
}

bool isOutOfBounds(struct vec2 pos) {
	return isOutOfBounds(pos.x, pos.y);
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
		case '9': attron(COLOR_PAIR(9)); break;
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

void clean() {
	// fill missing colorCoords height or width with 0s
	while(colorCoords.size() < ascii.size()) {
		colorCoords.push_back("0");	
		colorCoords.back().append(ascii.at(colorCoords.size()), '0');	
	}

	// replace NULL chars with 0 in colorCoords
	// to-do: figure out why this happens at all
	for(int y = 0; y < colorCoords.size(); y++) {
		for(int x = 0; x < colorCoords.at(y).length(); x++) {
			if((int)colorCoords.at(y)[x] == NULLCHAR) colorCoords.at(y)[x] = '0';
		}
	}
}

void turnContentToRect() {
	unsigned long max = 0;

	// get max width
	for(int i = 0; i < ascii.size(); i++) {
		max = std::max(max, ascii.at(i).length());	
	}

	// match all lines with max width using whitespace and color 0
	for(int i = 0; i < ascii.size(); i++) {
		while(ascii.at(i).length() < max) {
			ascii.at(i).append(1, ' ');
			colorCoords.at(i).append(1, '0');
		}
	}
}

void save(bool shouldOverride) {
	clean();
	turnContentToRect();
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



void edit(char k, int x = cursor.x, int y = cursor.y, bool changeColor = colorMode, int shouldRecord = true, bool repetitive = false) {
	// if this new char is beyond the current x and y that the content would allow, fill the remaining gaps with whitespaces
	while(y > ascii.size()-1) {
		ascii.push_back(" ");
		colorCoords.push_back("0");
	}
	while(x > ascii.at(y).length()-1) {
		ascii.at(y).append(1, ' ');
		colorCoords.at(y).append(1, '0');
	}

	auto& content = changeColor ? colorCoords : ascii;

	if(shouldRecord) {
		struct action newAction(x, y, content.at(y)[x], k, repetitive, changeColor);
		actions.push_back(newAction);
	}

	content.at(y)[x] = k;

	// revert color to 0 if entered key was whitespace
	if(!changeColor && k == ' ') colorCoords.at(y)[x] = '0';

	turnContentToRect();
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

void floodFill(int x, int y, char key, char toReplace, bool isColor, vector<vec2>& toBeFilled, bool isFirst = true) {
	if(
		key == toReplace ||
		(int)key == 0    ||
		x == -1          ||
		y == -1          ||
		(toBeFilled.size() > 0 && toBeFilled.back().x == -1)
	) return;

	// if this point is beyond bounds, add dummy to indicate so (and stop the fill)
	if(isOutOfBounds(x, y)) {
		fillBoundsErr:
			toBeFilled.push_back(vec2(-1, -1));
			return;
	}
	auto& content = colorMode ? colorCoords : ascii;
	bool CANDOWN = y+1 < content.size();
	bool CANUP = y-1 > -1;
	// if this point is whitespace, check if touching bounds
	if(content.at(y)[x] == ' ') {
		if((int)content.at(y)[x-1] == NULLCHAR) goto fillBoundsErr;
		if((int)content.at(y)[x+1] == NULLCHAR) goto fillBoundsErr;
		if(CANDOWN && (int)content.at(y+1)[x] == NULLCHAR) goto fillBoundsErr;
		if(CANUP   && (int)content.at(y-1)[x] == NULLCHAR) goto fillBoundsErr;
	}

	toBeFilled.push_back(vec2(x, y));

	// check if to ignore points already in toBeFilled
	bool checkLeft = true, checkRight = true, checkDown = true, checkUp = true;
	for(int i = 0; i < toBeFilled.size(); i++) {
		struct vec2 &pos = toBeFilled[i];
		if(pos.x == x-1 && pos.y == y) checkLeft  = false;
		if(pos.x == x+1 && pos.y == y) checkRight = false;
		if(pos.x == x && pos.y == y+1) checkDown  = false;
		if(pos.x == x && pos.y == y-1) checkUp    = false;
	}
	// recursive
	if(checkLeft  && content.at(y)[x-1] == toReplace)            floodFill(x-1, y, key, toReplace, isColor, toBeFilled, false);
	if(checkRight && content.at(y)[x+1] == toReplace)            floodFill(x+1, y, key, toReplace, isColor, toBeFilled, false);
	if(checkDown  && CANDOWN && content.at(y+1)[x] == toReplace) floodFill(x, y+1, key, toReplace, isColor, toBeFilled, false);
	if(checkUp    && CANUP   && content.at(y-1)[x] == toReplace) floodFill(x, y-1, key, toReplace, isColor, toBeFilled, false);
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
	if(!IS_DUMMY) edit(editChar, LAST_ACTION.pos.x, LAST_ACTION.pos.y, LAST_ACTION.wasColor, false);

	const bool ACTIONS_LEFT = actions.size() > 1;
    if(ACTIONS_LEFT) actions.pop_back();

	// keep calling undo for flood-filled chars (stops at dummy action)
	if(LAST_ACTION.repetitive) undo();
}

bool didCursorMove() {
	return cursor.x != prevCursor.x || cursor.y != prevCursor.y;
}

void tryRepeat(int x, int y, bool isColor) {
	if(isOutOfBounds(x, y)) {
		fillNullErr:
			message = "Nothing to repeat!";
			return;
	}

	char c = (colorMode ? colorCoords : ascii).at(cursor.y)[cursor.x];
	if((int)c == NULLCHAR || c == ' ') goto fillNullErr;

	repeatModeChar = c;
}

void tryFloodFill(int x, int y, char key, bool isColor) {
	toBeFilled.resize(0);
	string infErr = "This will fill to infinity! Is there a hole?";

	if(
		isOutOfBounds(x, y)      ||
		(int)ascii.at(y)[x] == 0 ||        // don't fill if point is NULL
		(isColor && ascii.at(y)[x] == ' ') // whitespaces shouldn't have a color assigned, so don't accept them
	) {
		message.assign(infErr);
		return;
	}

	char toReplace = (isColor ? colorCoords : ascii).at(cursor.y)[cursor.x];
	floodFill(cursor.x, cursor.y, (char)key, toReplace, colorMode, toBeFilled);

	if(toBeFilled.back().x == -1) {
		message.assign(infErr);
		return;
	}

	message = "Enter key to fill";
	fillMode = true;
}

void getChar(char& asciiChar, char& colorChar, int x, int y, char defaultAscii, char defaultColor) {
	if(!isOutOfBounds(x, y)) {
		asciiChar = ascii.at(y)[x];
		colorChar = colorCoords.at(y)[x];
	} else {
		asciiChar = defaultAscii; 
		colorChar = defaultColor; 
	}
}

void checkSelection(int x, int y, vector<contentChar>& selection) {
	if(selectMode == RECT) {
		// resetting the rect is the easiest way to handle backtracking
		selection.resize(1);

		struct vec2 rectCursor(selection.at(0).pos.x, selection.at(0).pos.y);

		// go thru each char horizontally, then go up or down if it's beyond cursor's x until it reaches the cursor
		while(rectCursor.x != cursor.x || rectCursor.y != cursor.y) {
			if(rectCursor.x != cursor.x) rectCursor.x += (cursor.x > rectCursor.x) ? 1 : -1;
			else {
				rectCursor.x = selection.at(0).pos.x;
				rectCursor.y += (cursor.y > rectCursor.y) ? 1 : -1; 
			}

			char asciiChar, colorChar;
			getChar(asciiChar, colorChar, rectCursor.x, rectCursor.y, ' ', '0');
			selection.push_back(contentChar(rectCursor, asciiChar, colorChar));
		}
	}
}

// dummy actions are used to stop undone repetitive actions from bleeding into the previous one, and to separate sequential repetitive actions
// must be called before the logic that will create repetitive actions
void createDummyAction() {
	struct action dummy(-1, -1, ' ', ' ', false, false);
	actions.push_back(dummy);
}

void yank(bool isCut = false) {
	selection.clear();

	// confirm selection, by copying pseudo to normal
	for(int i = 0; i < pseudoSelection.size(); i++) {
		struct contentChar& s = pseudoSelection[i];
		selection.push_back(contentChar(vec2(s.pos.x, s.pos.y), s.ascii, s.color));
	}
	pseudoSelection.clear();

	// create dummy if cutting, since it's a repetitive action
	if(isCut) createDummyAction();

	// use initial cursor's position as the origin point of pasted content
	struct vec2 origin(selection[0].pos);
	for(int i = 0; i < selection.size(); i++) {
		struct contentChar& s = selection[i];
		if(isCut) {
			edit(' ', s.pos.x, s.pos.y, false, true, true);		
			edit('0', s.pos.x, s.pos.y, true, true, true);		
		}
		s.pos.x -= origin.x;
		s.pos.y -= origin.y;
	}
	selectMode = OFF;
}

void getInput() {
	int k = getch();
	if(k == ESC) {
		insertMode = false;
		repeatModeChar = NULLCHAR;
		selectMode = OFF;
		pseudoSelection.clear();
		return;
	}
	if(insertMode && !isArrowKey(k)) {
		edit(k);
		return;
	}
	if(fillMode) {
		createDummyAction();
		for(int i = 0; i < toBeFilled.size(); i++) {
			edit((char)k, toBeFilled[i].x, toBeFilled[i].y, colorMode, true, true);
		}

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
	if(k == input[VISUAL]) {
		if(selectMode == RECT) selectMode = OFF;
		else {
			selectMode = RECT;
			pseudoSelection.clear();

			char asciiChar, colorChar;
			getChar(asciiChar, colorChar, cursor.x, cursor.y, ' ', '0');
			pseudoSelection.push_back(contentChar(cursor, asciiChar, colorChar));
		}
	}
	if(k == input[PASTE]) {
		// pasting is a repetitive action
		createDummyAction();
		for(int i = 0; i < selection.size(); i++) {
			struct vec2 pos;
			pos.x = selection[i].pos.x + cursor.x;
			pos.y = selection[i].pos.y + cursor.y;
			if(pos.x < 0 || pos.y < 0 || selection[i].ascii == ' ') continue;

			edit(selection[i].ascii, pos.x, pos.y, false, true, true);
			edit(selection[i].color, pos.x, pos.y, true, true, true);
		}
	}

	if(k == input[FLOODFILL]) tryFloodFill(cursor.x, cursor.y, (char)k, colorMode);
		
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

	if(repeatModeChar != NULLCHAR && didCursorMove()) edit((char)repeatModeChar);

	if(selectMode != OFF) {
		if(didCursorMove()) checkSelection(cursor.x, cursor.y, pseudoSelection);
		if(k == input[YANK] || k == input[CUT]) yank(k == input[CUT]);
	}

	prevCursor.x = cursor.x;
	prevCursor.y = cursor.y;
}

void displayStatus() {
	attron(COLOR_PAIR(8));

	string mode =     (colorMode)           ? "COLOR" : "ASCII";
	string insert =   (insertMode)          ? " INSERT" : "";
	string saved =    (savedFilename != "") ? " SAVED AS " + savedFilename : "";
	string visual =   (selectMode == RECT)  ? " VISUAL" : "";
	string isRepeat = (repeatModeChar != NULLCHAR) ? " REPEAT" : "";

	string finalStr = mode + isRepeat + insert + visual + saved;
	mvaddstr(LINES-1, 0, finalStr.c_str());

	// custom message on top of status
	mvaddstr(LINES-2, 0, message.c_str());
	message = "";

	savedFilename = "";
	standend();
}

void renderSelection(vector<contentChar>& selection) {
	for(int i = 0; i < selection.size(); i++) {
		struct vec2 sec = selection[i].pos;
		mvchgat(sec.y, sec.x, 1, A_REVERSE, 0, NULL);
	}
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
	init_pair(1, COLOR_BLACK,   COLOR_WHITE);
	init_pair(2, COLOR_RED,     -1);
	init_pair(3, COLOR_GREEN,   -1);
	init_pair(4, COLOR_YELLOW,  -1);
	init_pair(5, COLOR_BLUE,    -1);
	init_pair(6, COLOR_MAGENTA, -1);
	init_pair(7, COLOR_CYAN,    -1);
	init_pair(8, COLOR_WHITE,   -1);

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
		if(selectMode != OFF) {
			curs_set(0);
			renderSelection(pseudoSelection);
		} else curs_set(1);
	}

	agnos::closeWin();
}
