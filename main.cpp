#include <iostream>
#include <string>
#include "CEngine.h"

int main() {
	std::string command = "";
	CEngine* gameEngine = NULL;

	while(1) {
		std::getline(std::cin, command);
		if(command == "lose" || command == "win" || command == "draw" || command == "bad") break;
		else if(command == "turn") {
			int resx, resy;
			gameEngine->playerMove(resx, resy);
			std::cout << "move " << static_cast<char>('a' + resx) << " " << resy + 1 << std::endl; 
		}
		std::string subcommand = command.substr(0, 4);
		if(subcommand == "init") {
			std::string color = command.substr(5, 5);
			EChipColor playerColor = (color == "white") ? EChipColor::White : EChipColor::Black;
			EChipColor opponentColor = (playerColor == EChipColor::White) ? EChipColor::Black : EChipColor::White;
			gameEngine = new CEngine(playerColor, opponentColor);
		}
		else if(subcommand == "move") {
			char cy = command.at(7);
			char cx = command.at(5);
			gameEngine->opponentMove(cx - 'a', cy - '0' - 1);
		}
	}

	return 0;
}
