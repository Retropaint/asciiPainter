#include "agnos.hpp"

void agnos::setESCDELAY(int delay) {
	return;
}

void agnos::closeWin() {
	return;
}

void agnos::renameWindow(std::string name) {
	PDC_set_title(name.c_str());
}
