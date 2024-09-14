#define NULLCHAR 0
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
#define LASTCOLOR 16
#define SAVE 17
#define UNDO 18
#define FLOODFILL 19
#define VISUAL 20
#define YANK 21
#define PASTE 22
#define CUT 23

enum CONTENTMODE {
	ASCII,
	COLORFG,
	COLORBG
};

struct vec2 {
	short int x, y;

	vec2() {
		x = 0;
		y = 0;
	}
	
	vec2(short int _x, short int _y) {
		x = _x;
		y = _y;
	}

	vec2(const struct vec2 &clone) {
		x = clone.x;
		y = clone.y;
	}
};

struct contentChar {
	struct vec2 pos;
	char ascii, colorFg, colorBg;

	contentChar() {
		pos.x = 0;
		pos.y = 0;
		ascii = 0;
		colorFg = 0;
		colorBg = 0;
	}

	contentChar(struct vec2 _pos, char _ascii, char _colorFg, char _colorBg) {
		pos = _pos;
		ascii = _ascii;
		colorFg = _colorFg;
		colorBg = _colorBg;
	}
};

struct action {
	struct vec2 pos;
	char prevVal, nextVal;
	bool repetitive;
	enum CONTENTMODE contentMode;

	action(int _x, int _y, char _prevVal, char _nextVal, bool _repetitive, enum CONTENTMODE _contentMode) {
		pos.x=_x;
		pos.y=_y;
		prevVal=_prevVal;
		nextVal=_nextVal;
		repetitive=_repetitive;
		contentMode=_contentMode;
	}
	action(struct vec2 _pos, char _prevVal, char _nextVal, bool _repetitive, enum CONTENTMODE _contentMode) {
		pos=_pos;
		prevVal=_prevVal;
		nextVal=_nextVal;
		repetitive=_repetitive;
		contentMode=_contentMode;
	}
};

enum SELECTMODE {
	OFF,
	RECT,
	LASSO,
	FREE
};

enum CONTENTMODE contentMode;
bool insertMode;
bool fillMode;
int  repeatModeChar = NULLCHAR;
enum SELECTMODE selectMode;
