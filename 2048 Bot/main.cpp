#include "Bot.h"
using namespace std;

int main(int argc, char* argv[]) {
	// Use parameters of evo3
	Bot bot = Bot(0.00239234, 278, 867, 195, 119, 0.00196464);
	bot.Run(100);
	return 0;
}