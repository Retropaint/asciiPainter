#include "agnos.hpp"

void agnos::setESCDELAY(int delay) {
	ESCDELAY = delay;
}

void agnos::closeWin() {
	endwin();
}

void agnos::renameWindow(std::string name) {
	printf("\033]0;%s\007", name.c_str());
	fflush(stdout);
}
