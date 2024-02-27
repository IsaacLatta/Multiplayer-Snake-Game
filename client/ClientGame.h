#ifndef CLIENTGAME_H
#define CLIENTGAME_H

#include "Client.h"
#include "Util.h"
#include <conio.h>

class ClientGame
{
private:
	Client client;
	std::atomic <bool> new_board;
	std::atomic <bool> shouldQuit = false;
	std::string board;
	std::mutex board_mutex;
	std::mutex print_mutex;

	void print(std::string& str, bool clearScreen)
	{
		std::lock_guard<std::mutex> lock(print_mutex);
		if (shouldQuit)
			return;

		if (clearScreen)
			system("cls");

		std::cout << str;
	}

	void set_board(std::string& new_board)
	{
		std::lock_guard <std::mutex> lock(board_mutex);
		this -> board = new_board;
		this -> new_board = true;
	}

	std::string get_board()
	{
		std::lock_guard <std::mutex> lock(board_mutex);
		return board;
	}

	void print_board_loop()
	{
		std::string temp_board = "";
		while (!shouldQuit)
		{
			if (new_board)
				temp_board = get_board();

			print(temp_board, 1); // Clear and print screen
			Sleep(16.67); // Control frame rate
		}
	}

	void game_over()
	{
		std::lock_guard <std::mutex> lock(print_mutex);
		
		// Receive and print scoreboard
		system("cls");
		std::string message = client.recvMessage();
		std::cout << message; 

		while (true)
		{
			message = client.recvMessage();
			if (message == "$Yes") // New game starting
			{
				system("cls"); // Clear the screen for new game count down timer
			}
			else if (message == "$No") // No new game starting
			{
				shouldQuit = true; // Signal thread shutdown
				client.sendInt(-1); // Respond with -1 to close the get_guest_input loop on the server side
				return;
			}
			else if (message == "$Start") // New game start signal
			{
				break;
			}
			else
			{
				std::cout << message;
			}
		}
	}
	
	void get_board_loop()
	{
		std::string board = "";
		std::string board_chunk = "";

		while (!shouldQuit)
		{
			if (board_chunk.find("$end") != std::string::npos) // All rows of gameboard received
			{
				set_board(board); // Set new gameboard
				board = "";
			}
			else if (board_chunk == "$Gameover")
			{
				game_over(); 
			}
			else if (!(board_chunk == client.RECV_MESSAGE_ERROR))
			{
				new_board = false;
				board = board + board_chunk;
			}
			board_chunk = client.recvMessage();
		}
	}
	
	// Send keyboard input to the server
	void send_input_loop()
	{
		// Default directions
		int dir_caught = 80; // Down
		int dir_to_send = 72; // Up
		try {
			while (!shouldQuit)
			{
				if (_kbhit())
				{
					int ch = _getch();
					if (ch == 224 || ch == 0)
						dir_caught = _getch(); // Get the arrow key code

					if (dir_caught != Util::opposite_dir(dir_to_send))
						dir_to_send = dir_caught;

					client.sendInt(dir_to_send);
				}
			}
		}
		catch (std::exception& e)
		{
			if (shouldQuit)
				return;
			std::cout << e.what();
		}
	}

public:
	void start_game()
	{
		try
		{
			std::string username, welcome_msg, message, server_ip;

			// Set server ip address and connect
			std::cout << "\033[1m\033[96m[*] \033[93mEnter Server IP : \033[1m\033[96m";
			std::cin >> server_ip;
			client.setServer_IP(server_ip);
			std::cout << "\033[1m\033[96m[*] \033[93mConnecting to server ...";
			client.connectToServer();
			std::cout << "\n\033[1m\033[96m[+] \033[93mConnected!\n";

			// Work with server to set username of snake prior to the game starting
			welcome_msg = client.recvMessage();
			system("cls");
			std::cout << welcome_msg;

			while (true)
			{
				std::cin >> username;
				client.sendMessage(username);
				message = client.recvMessage();
				if (message == "$Success") // Username not taken
					break;
				else
					std::cout << message;
			}
			message = client.recvMessage();
			std::cout << message;

			while (true)
			{
				message = client.recvMessage();
				if (message == "$Start")
					break;
				std::cout << message;
			}

			// Start the game
			std::thread get_board_thread(&ClientGame::get_board_loop, this);
			std::thread print_board_thread(&ClientGame::print_board_loop, this);
			std::thread send_input_thread(&ClientGame::send_input_loop, this);

			// Shutdown Game
			print_board_thread.join();
			get_board_thread.join();
			send_input_thread.join();
		}
		catch (std::exception& e)
		{
			shouldQuit = true;
			std::cout << e.what();
		}
		std::cout << "\n\033[1m\033[96m[*] \033[93mGame Ended" << "\033[0m";
	}
};

#endif
