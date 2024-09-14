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

int input[] = {'k','l','j','h','r','w','q','c','i','1','2','3','4','5','6','7','8','W','u','f','v','y','p','d'};
vector<string> ascii, colorFg, colorBg;
vector<action> actions;
vector<contentChar> pseudoSelection, selection;
vector<vec2> toBeFilled;
string filename, savedFilename, message;
struct vec2 cursor(0, 0), prevCursor(0, 0);
bool programRunning = true;
char defaultAscii = ' ';
char defaultColorFg = '0';
char defaultColorBg = '7';

void loadAscii(string filename, vector<string>* ascii, vector<string>* colorFg, vector<string>* colorBg) {
	string raw;
	std::fstream ifs(filename); 
	raw.assign((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

	char* ptr = std::strtok(&raw[0], "\n");

	// read ascii
	while(ptr[0] != '~') {
		ascii->push_back(ptr);
		ptr = std::strtok(NULL, "\n");
	}

	// discard tilde, and proceed with colorFg
	ptr = std::strtok(NULL, "\n"); 

	while(ptr[0] != '~') {
		colorFg->push_back(ptr);
		ptr = std::strtok(NULL, "\n");
	}

	// discard tilde, and proceed with colorBg
	ptr = std::strtok(NULL, "\n"); 

	while(ptr != NULL) {
		colorBg->push_back(ptr);
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
		if(ptr[0] == '#' || ptr[0] == defaultAscii) {
			ptr = std::strtok(NULL, "\n");
			continue;
		}

		input[key++] = ptr[0];	
		ptr = std::strtok(NULL, "\n");
	}
}

bool validateFile(int argc, char **argv) {
	

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

void getColorCoord(int x, int y, vector<string>* colorFg, vector<string>* colorBg) {
	int CHAR_INT_OFFSET = 48;
	int fgNum = (colorFg->at(y)[x] - CHAR_INT_OFFSET) * 8;
	int bgNum = (colorBg->at(y)[x] - CHAR_INT_OFFSET);
	attron(COLOR_PAIR(fgNum + bgNum + 1));
}

void draw(int x, int y, vector<string>* ascii, vector<string>* colorFg, vector<string>* colorBg) {
	for(int i = 0; i < ascii->size(); i++) {
		for(int cx = 0; cx < ascii->at(i).length(); cx++) {
			getColorCoord(cx, i, colorFg, colorBg);
			mvaddch(y+i, fmax(x+cx, 0), ascii->at(i).c_str()[cx]);
			standend();
		}
	}
}

void clean() {
	// fill missing colorFg height or width with 0s
	while(colorFg.size() < ascii.size()) {
		colorFg.push_back("0");	
		const int ZEROLENGTH = ascii.at(colorFg.size()).length();
		colorFg.back().append(ZEROLENGTH, defaultColorFg);	
	}

	// replace NULL chars with 0 in colorFg
	// to-do: figure out why this happens at all
	for(int y = 0; y < colorFg.size(); y++) {
		for(int x = 0; x < colorFg.at(y).length(); x++) {
			if((int)colorFg.at(y)[x] == NULLCHAR) colorFg.at(y)[x] = defaultColorFg;
		}
	}
}

void turnContentToRect() {
	unsigned long max = 0;

	// get max width
	for(int i = 0; i < ascii.size(); i++) {
		max = std::max(max, ascii.at(i).length());	
	}

	// fill all lines with max width using whitespace and color 0
	for(int i = 0; i < ascii.size(); i++) {
		while(ascii.at(i).length() < max) {
			ascii.at(i).append(1, defaultAscii);
			colorFg.at(i).append(1, defaultColorFg);
			colorBg.at(i).append(1, defaultColorBg);
		}
	}
}

void trim() {
	// remove lines from bottom (stop if there's any content)
	for(int y = ascii.size()-1; y > 0; y--) {
		bool isEmptyLine = true;
		for(int x = 0; x < ascii.at(y).length(); x++) {
			if(ascii.at(y)[x] != defaultAscii) {
				isEmptyLine = false;
				break;
			}
		}
		if(!isEmptyLine) break;
		else {
			ascii.erase(ascii.begin() + y);
			colorFg.erase(colorFg.begin() + y);
			colorBg.erase(colorBg.begin() + y);
		}
	}

	// remove trailing whitespaces
	for(int y = 0; y < ascii.size(); y++) {
		while(ascii.at(y).back() == defaultAscii) {
			ascii.at(y).pop_back();			
			colorFg.at(y).pop_back();			
			colorBg.at(y).pop_back();			
		}
	}
}

void save(bool shouldOverride) {
	//clean();

	string asciiStr = "";
	for(int i = 0; i < ascii.size(); i++) {
		asciiStr.append(ascii.at(i) + "\n");
	}
	asciiStr.append("~\n");
	for(int i = 0; i < colorFg.size(); i++) {
		asciiStr.append(colorFg.at(i) + "\n");
	}
  	asciiStr.append("~\n");
	for(int i = 0; i < colorBg.size(); i++) {
		asciiStr.append(colorBg.at(i) + "\n");
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

vector<string>& contentOf(enum CONTENTMODE contentMode) {
	switch(contentMode) {
		case ASCII:   return ascii;
		case COLORFG: return colorFg;
		case COLORBG: return colorBg;
	}
}

void edit(char k, int x = cursor.x, int y = cursor.y, enum CONTENTMODE mode = contentMode, int shouldRecord = true, bool repetitive = false) {
	// if this new char is beyond the current x and y that the content would allow, fill the remaining gaps with whitespaces
	while(y > ascii.size()-1) {
		ascii.push_back(" ");
		colorFg.push_back(string() + defaultColorFg);
		colorBg.push_back(string() + defaultColorBg);
	}
	while(x > ascii.at(y).length()-1) {
		ascii.at(y).append(1, defaultAscii);
		colorFg.at(y).append(1, defaultColorFg);
		colorBg.at(y).append(1, defaultColorBg);
	}

	auto& content = contentOf(mode);

	if(shouldRecord) {
		struct action newAction(x, y, content.at(y)[x], k, repetitive, mode);
		actions.push_back(newAction);
	}

	content.at(y)[x] = k;

	// revert color to 0 if entered key was whitespace
	if(mode != ASCII && k == defaultAscii) colorFg.at(y)[x] = defaultColorFg;

	trim();
	turnContentToRect();
}

void checkColorKeys(int k) {
	for(int i = 0; i < (LASTCOLOR+1 - FIRSTCOLOR); i++) {
		const int COLORINPUT = i + FIRSTCOLOR;
		if(k == input[COLORINPUT]) {
			const int NUM_CHAR_OFFSET = 40;

			edit((char)(COLORINPUT + NUM_CHAR_OFFSET - 1), cursor.x, cursor.y, contentMode);
			break;
		}
	}
}

void floodFill(int x, int y, char key, char toReplace, enum CONTENTMODE contentMode, vector<vec2>& toBeFilled, bool isFirst = true) {
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
	auto& content = contentOf(contentMode);
	bool CANDOWN = y+1 < content.size();
	bool CANUP = y-1 > -1;
	// if this point is whitespace, check if touching bounds
	if(ascii.at(y)[x] == defaultAscii) {
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
	if(checkLeft  && content.at(y)[x-1] == toReplace)            
		floodFill(x-1, y, key, toReplace, contentMode, toBeFilled, false);
	if(checkRight && content.at(y)[x+1] == toReplace)            
		floodFill(x+1, y, key, toReplace, contentMode, toBeFilled, false);
	if(checkDown  && CANDOWN && content.at(y+1)[x] == toReplace) 
		floodFill(x, y+1, key, toReplace, contentMode, toBeFilled, false);
	if(checkUp    && CANUP   && content.at(y-1)[x] == toReplace) 
		floodFill(x, y-1, key, toReplace, contentMode, toBeFilled, false);
}

void undo() {
	const struct action LAST_ACTION = actions.at(actions.size()-1);
	char editChar = LAST_ACTION.prevVal;

	/* 
		to-do: deal with null char directly 
	 	(whitespace might not behave the same across terminals, and undoing this still leaves ascii data as if space was entered) 
	*/
	// replace NULL char with space
	if((int)LAST_ACTION.prevVal == 0) editChar = defaultAscii;
	
	const bool IS_DUMMY = LAST_ACTION.pos.x == -1;
	if(!IS_DUMMY) edit(editChar, LAST_ACTION.pos.x, LAST_ACTION.pos.y, LAST_ACTION.contentMode, false);

	const bool ACTIONS_LEFT = actions.size() > 1;
    if(ACTIONS_LEFT) actions.pop_back();

	// keep calling undo for flood-filled chars (stops at dummy action)
	if(LAST_ACTION.repetitive) undo();
}

bool didCursorMove() {
	return cursor.x != prevCursor.x || cursor.y != prevCursor.y;
}

void tryRepeat(int x, int y, enum CONTENTMODE contentMode) {
	if(isOutOfBounds(x, y)) {
		fillNullErr:
			message = "Nothing to repeat!";
			return;
	}

	char c = contentOf(contentMode).at(cursor.y)[cursor.x];
	if((int)c == NULLCHAR || c == defaultAscii) goto fillNullErr;

	repeatModeChar = c;
}

void tryFloodFill(int x, int y, char key, bool isColor) {
	toBeFilled.resize(0);
	string infErr = "This will fill to infinity! Is there a hole?";

	if(
		isOutOfBounds(x, y)      ||
		(int)ascii.at(y)[x] == 0 ||        // don't fill if point is NULL
		(isColor && ascii.at(y)[x] == defaultAscii) // whitespaces shouldn't have a color assigned, so don't accept them
	) {
		message.assign(infErr);
		return;
	}

	char toReplace = contentOf(contentMode).at(cursor.y)[cursor.x];
	floodFill(cursor.x, cursor.y, (char)key, toReplace, contentMode, toBeFilled);

	if(toBeFilled.back().x == -1) {
		message.assign(infErr);
		return;
	}

	message = "Enter key to fill";
	fillMode = true;
}

void getChar(char& asciiChar, char& colorCharFg, char& colorCharBg, int x, int y, char defaultAscii, char defaultFg, char defaultBg) {
	if(!isOutOfBounds(x, y)) {
		asciiChar = ascii.at(y)[x];
		colorCharFg = colorFg.at(y)[x];
		colorCharBg = colorBg.at(y)[x];
	} else {
		asciiChar = defaultAscii; 
		colorCharFg = defaultFg; 
		colorCharBg = defaultBg; 
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

			char asciiChar, colorCharFg, colorCharBg;
			getChar(asciiChar, colorCharFg, colorCharBg, rectCursor.x, rectCursor.y, defaultAscii, defaultColorFg, defaultColorBg);
			selection.push_back(contentChar(rectCursor, asciiChar, colorCharFg, colorCharBg));
		}
	}
}

// dummy actions are used to stop undone repetitive actions from bleeding into the previous one, and to separate sequential repetitive actions
// must be called before the logic that will create repetitive actions
void createDummyAction() {
	struct action dummy(-1, -1, ' ', ' ', false, ASCII);
	actions.push_back(dummy);
}

void yank(bool isCut = false) {
	selection.clear();

	// confirm selection, by copying pseudo to normal
	for(int i = 0; i < pseudoSelection.size(); i++) {
		struct contentChar& s = pseudoSelection[i];
		selection.push_back(contentChar(vec2(s.pos.x, s.pos.y), s.ascii, s.colorFg, s.colorBg));
	}
	pseudoSelection.clear();

	// create dummy if cutting, since it's a repetitive action
	if(isCut) createDummyAction();

	// use initial cursor's position as the origin point of pasted content
	struct vec2 origin(selection[0].pos);
	for(int i = 0; i < selection.size(); i++) {
		struct contentChar& s = selection[i];
		if(isCut) {
			edit(defaultAscii, s.pos.x, s.pos.y, ASCII, true, true);
			edit(defaultColorFg, s.pos.x, s.pos.y, COLORFG, true, true);
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
			edit((char)k, toBeFilled[i].x, toBeFilled[i].y, contentMode, true, true);
		}

		fillMode = false;
		return;
	}

	// in ASCII mode, enter any key that's not in config (including color keys)
	if(contentMode == ASCII && !isInputKey(k) && k != ESC) {
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

			char asciiChar, colorCharFg, colorCharBg;
			getChar(asciiChar, colorCharFg, colorCharBg, cursor.x, cursor.y, defaultAscii, defaultColorFg, defaultColorBg);
			pseudoSelection.push_back(contentChar(cursor, asciiChar, colorCharFg, colorCharBg));
		}
	}
	if(k == input[PASTE]) {
		// pasting is a repetitive action
		createDummyAction();
		for(int i = 0; i < selection.size(); i++) {
			struct vec2 pos;
			pos.x = selection[i].pos.x + cursor.x;
			pos.y = selection[i].pos.y + cursor.y;
			if(pos.x < 0 || pos.y < 0 || selection[i].ascii == defaultAscii) continue;

			edit(selection[i].ascii,   pos.x, pos.y, ASCII, true, true);
			edit(selection[i].colorFg, pos.x, pos.y, COLORFG, true, true);
			edit(selection[i].colorBg, pos.x, pos.y, COLORBG, true, true);
		}
	}

 	if(k == input[FLOODFILL]) tryFloodFill(cursor.x, cursor.y, (char)k, contentMode);
		
	if(k == input[ASCIICOLOR]) {
		switch(contentMode) { 
			case ASCII:   contentMode = COLORFG; break;
			case COLORFG: contentMode = COLORBG; break;
			case COLORBG: contentMode = ASCII;   break;
		}
		repeatModeChar = NULLCHAR;
	}
	if(k == input[INSERT]) {
		contentMode = ASCII;
		insertMode = !insertMode;
	}
	if(k == input[REPEAT]) {
		if (repeatModeChar != NULLCHAR) repeatModeChar = NULLCHAR;
		else tryRepeat(cursor.x, cursor.y, contentMode);
	}

	// check and apply colors if in color modes, and if current point isn't whitespace
	const char CURR = isOutOfBounds(cursor.x, cursor.y) ? 
		defaultAscii : 
		ascii.at(cursor.y)[cursor.x];
	if(contentMode != ASCII && CURR != defaultAscii) checkColorKeys(k);

	if(repeatModeChar != NULLCHAR && didCursorMove()) 
		edit((char)repeatModeChar);

	if(selectMode != OFF) {
		if(didCursorMove()) 
			checkSelection(cursor.x, cursor.y, pseudoSelection);
		if(k == input[YANK] || k == input[CUT]) 
			yank(k == input[CUT]);
	}

	prevCursor.x = cursor.x;
	prevCursor.y = cursor.y;
}

void displayStatus() {
	attron(COLOR_PAIR(0));

	string mode;
	switch(contentMode) {
		case ASCII:   mode = "ASCII";    break;
		case COLORFG: mode = "COLOR_FG"; break;
		case COLORBG: mode = "COLOR_BG"; break;
	}

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

int getColorNum(int num) {
	switch(num) {
		case 0:  return COLOR_WHITE;
		case 1:  return COLOR_RED;
		case 2:  return COLOR_GREEN;
		case 3:  return COLOR_YELLOW;
		case 4:  return COLOR_BLUE;
		case 5:  return COLOR_MAGENTA;
		case 6:  return COLOR_CYAN;
		case 7:  return COLOR_BLACK;
		default: return COLOR_WHITE;
	}
}

void createColorPairs() {
	int MAXCOLORS = 8;
	for(int i = 0; i < MAXCOLORS; i++) {
		for(int j = 0; j < MAXCOLORS; j++) {
			int pairNum = j + i*MAXCOLORS;
			init_pair(pairNum+1, getColorNum(i), getColorNum(j));
		}
	}
}

int main(int argc, char **argv) {
	
	// validate file if provided, otherwise set default name 
	if(argc == 1) {
		filename = "newAscii.txt";
	} else validateFile(argc, argv);

	// load ascii if provided, otherwise start anew
	if(argc == 2) loadAscii(filename, &ascii, &colorFg, &colorBg);
	else {
		ascii.push_back(" ");
		colorFg.push_back("0");
		colorBg.push_back("7");
	}

	// load keybinds (not really supported atm)
	loadConfig();

	// ncurses/pdcurses stuff
	initscr();
	agnos::renameWindow("asciiPainter");
	start_color();
	use_default_colors();
	noecho();
	cbreak();
	agnos::setESCDELAY(0);
	keypad(stdscr, true);
	createColorPairs();	
	
	// call these for first frame, since subsequent draws happen after getInput() 
	draw(0, 0, &ascii, &colorFg, &colorBg);
	displayStatus();
	move(0, 0);

	// main loop
	while(programRunning) {
		// meat of the program; most logic is funneled here 
		getInput();

		// naturally, canvas should be redrawn after input
		erase();
		draw(0, 0, &ascii, &colorFg, &colorBg);

		// statuses at bottom left
		displayStatus();

		// update cursor
		move(cursor.y, cursor.x);

		// show selection, if selectMode is on
		// cursor has to be turned off for this, since selection simulates multiple of them
		if(selectMode != OFF) {
			curs_set(0);
			renderSelection(pseudoSelection);
		} else curs_set(1);
	}

	agnos::closeWin();
}
