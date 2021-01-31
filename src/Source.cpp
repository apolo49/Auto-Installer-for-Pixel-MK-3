#include "headers/Main.h"
#ifdef _WIN32
#define strdup _strdup
#endif

int main() {
	return Main::Begin();
}