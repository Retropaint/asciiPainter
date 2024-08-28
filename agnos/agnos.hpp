#include <curses.h>
#include <string>

namespace agnos {
	void setESCDELAY(int delay);
	void closeWin();
	void renameWindow(std::string name);
}
