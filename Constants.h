#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace constant
{
	constexpr int X = 25;
	constexpr int Y = 100;
	const std::string RESET_COLOR = "\033[0m";
	const std::string BORDER_COLOR = "\033[92m"; // Bright Green
	//const std::string BORDER_COLOR = "\033[96m"; // Bright Cyan
	//const std::string BORDER_COLOR = "\033[93m"; // Bright Yellow
	//const std::string SCORE_BOARD_COLOR = "\033[1m\033[92m"; // Bold Green
	const std::string SCORE_BOARD_COLOR = "\033[1m\033[93m"; // Bold Yellow
	//const std::string SCORE_BOARD_COLOR = "\033[1m\033[96m"; // Bold Cyan
	std::string WELCOME_MESSAGE = u8"\033[93m\U0001F40D Welcome To the Snake Game! \U0001F40D\n\033[1m\033[96m[*] \033[93mEnter Username: \033[1m\033[96m";
	std::string INVALID_USERNAME_MESSAGE = "\033[91m[!] \033[93mUsername Taken\n\033[96m[*] \033[93m\Enter Username: \033[96m";
	std::string VALID_USERNAME_MESSAGE = "\033[96m[*] \033[93m\Welcome: \033[96m";
}

#endif
