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
#define LASTCOLOR 17
#define SAVE 18
#define UNDO 19
#define FLOODFILL 20
#define VISUAL 21

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

struct action {
	struct vec2 pos;
	char prevVal, nextVal;
	bool wasFill, wasColor;

	action(int _x, int _y, char _prevVal, char _nextVal, bool _wasFill, bool _wasColor) {
		pos.x=_x;
		pos.y=_y;
		prevVal=_prevVal;
		nextVal=_nextVal;
		wasFill=_wasFill;
		wasColor=_wasColor;
	}
	action(struct vec2 _pos, char _prevVal, char _nextVal, bool _wasFill, bool _wasColor) {
		pos=_pos;
		prevVal=_prevVal;
		nextVal=_nextVal;
		wasFill=_wasFill;
		wasColor=_wasColor;
	}
};

enum SELECTMODE {
	OFF,
	RECT,
	LASSO,
	FREE
};

bool colorMode;
bool insertMode;
bool fillMode;
int  repeatModeChar = NULLCHAR;
enum SELECTMODE selectMode;
