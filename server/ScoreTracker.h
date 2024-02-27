#ifndef SCORETRACKER_H
#define SCORETRACKER_H

#include <map>
#include <string>
#include <ostream>
#include <fstream>
#include <iostream>
#include <vector>

struct Player
{
	std::string username;
	int score;
	int lives;

	Player(std::string username)
	{
		this->username = username;
		score = 0;
		lives = 0;
	}
};

class ScoreTracker
{
private:
	std::map <std::string, int> scoreboard;
	std::vector <Player> players;
	std::pair <std::string, int> highestScore;
	std::mutex scoreboard_mutex;
	std::string fileName = "ScoreBoard.txt";
	std::string title = "Scoreboard", border;
	std::string purple = "\033[95m", cyan = "\033[96m", yellow = "\033[93m";
	std::string red = "\033[91m", green = "\033[92m", blue = "\033[94m", brightCyan = "\033[1m\033[96m";
	
	int fieldWidth = 15;

	void sortPlayers()
	{
		size_t maxScoreIndex;
		for (size_t i = 0; i < players.size() - 1; i++)
		{
			maxScoreIndex = i;
			for (size_t j = i + 1; j < players.size(); j++)
			{
				if (players[j].score > players[maxScoreIndex].score)
					maxScoreIndex = j;
			}
			if (i != maxScoreIndex)
				std::swap(players[i], players[maxScoreIndex]);
		}
	}

	void writeScores()
	{
		std::ofstream writeFile(fileName, std::ios::app);
		if (writeFile.is_open())
		{
			for (auto& player : players)
			{
				writeFile << " " << player.username << " " << player.score;
			}
		}
		else
			std::cerr << "[-] Unable to open file: " << fileName;
	}

	void syncScores()
	{
		std::string playerName;
		int maxScore;

		sortPlayers();
		writeScores();

		std::ifstream readFile(fileName);
		if (readFile.is_open())
		{
			while (readFile >> playerName >> maxScore)
			{
				if (maxScore > highestScore.second)
				{
					highestScore.first = playerName;
					highestScore.second = maxScore;
				}
			}
		}
		else
			std::cerr << "[-] Unable to open file: " << fileName;
	}

public:

	ScoreTracker()
	{
		highestScore.first = "Guest";
		highestScore.second = 0;
		border += blue;

		for (size_t i = 0; i < fieldWidth; i++)
		{
			border += '*';
		}
		border += "\n";
	}

	void updateScoreboard(const std::string& name, int score, int lives)
	{
		std::lock_guard<std::mutex> lock(scoreboard_mutex);
		for (auto& player : players)
		{
			if (player.username == name)
			{
				player.score = score;
				player.lives = lives;
			}
		}
	}

	bool playerExists(std::string& username)
	{
		std::lock_guard<std::mutex> lock(scoreboard_mutex);
		for (auto& player : players)
		{
			if (player.username == username)
			{
				return true;
			}
		}
		return false;
	}

	void updateFile()
	{
		std::lock_guard<std::mutex> lock(scoreboard_mutex);
		writeScores();
	}

	void createPlayer(const std::string& username)
	{
		std::lock_guard<std::mutex> lock(scoreboard_mutex);
		Player new_player(username);
		players.push_back(new_player);
	}

	std::string getScoreboard_str()
	{
		std::lock_guard<std::mutex> lock(scoreboard_mutex);
		std::string ret_str = "";

		size_t i = 1;
		for (const auto& player : players)
		{
			ret_str += yellow + player.username + ":" + brightCyan + " " + std::to_string(player.score) + " ";
				
			if (player.lives >= 0)
			{
				ret_str += yellow + "Lives: " + brightCyan  + std::to_string(player.lives) + " ";
			}
			else
			{
				ret_str += u8"\U0001F635"; // Eyes crossed out face
				ret_str += " ";
			}

			if (i % 2 == 0)
				ret_str += "\n";

		}
		ret_str += "\n";
		return ret_str;
	}

	std::string getFinalScoreboard_str()
	{
		std::lock_guard<std::mutex> lock(scoreboard_mutex);
		std::string ret_str = "";

		syncScores();
		
		ret_str +=  border;
		ret_str += red + "Highest Score\n";
		ret_str += border;
		ret_str += red + "By: " + yellow + highestScore.first + "\n";
		ret_str += red + "Score: " + brightCyan + std::to_string(highestScore.second) + "\n";
		ret_str += border;
		ret_str += red + "This Game\n";
		ret_str += border;

		size_t i = 1;
		for (auto& player : players)
		{
			ret_str += red + std::to_string(i) + ". " 
					+ yellow + player.username + ": " + brightCyan + std::to_string(player.score) + "\n";
			i++;
		}
		ret_str += border;

		return ret_str;
	}
};
#endif


