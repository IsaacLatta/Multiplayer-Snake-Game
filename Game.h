#ifndef GAME_H
#define GAME_H
#include "Server.h"
#include "conio.h"
#include "Enemy.h"
#include "Fruit.h"
//#include "EnemySnake.h"
#include "ScoreTracker.h"
#include "Mine.h"

#include <cmath>
#include <fstream>
#include <cassert>
#include <thread>
#include <map>
#include <queue>
#include <utility>
///	Rows are x, Cols are y	///

// Game Class
class Game
{
private:
	// Initialization of Data Fields
	static const int X = constant::X;
	static const int Y = constant::Y;
	Element board[X][Y] = {};
	
	Server server;
	ScoreTracker scoreBoard;

	std::atomic <bool> isRunning = true;
	std::atomic <bool> all_snakes_dead = false;
	std::atomic <bool> nuke = false;
	
	int fruits_to_spawn = 0, powerups_to_spawn = 0;
	int mines_to_spawn = 0, enemies_to_spawn = 0;

	//std::mutex score_board_mutex;
	std::mutex nodes_mutex;
	std::mutex targets_mutex;
	std::mutex new_player;
	std::mutex gameboard_mutex;
	std::mutex print_mutex;

	std::map <int, std::weak_ptr<Node>> targets;

	std::vector <std::thread> threads;
	
	std::vector <Point> border;

	std::vector <std::weak_ptr<Snake>> snakes;
	std::vector < std::weak_ptr<Fruit>> fruits;
	std::list <std::shared_ptr<Node>> alive_nodes;
	std::queue <std::shared_ptr<Mine>> dead_mines;
	std::queue <std::shared_ptr<Fruit>> dead_fruits;
	std::queue <std::shared_ptr<Powerup>> dead_powerups;
	std::queue <std::shared_ptr<Enemy>> dead_enemies;
	
	

	void respawn(std::shared_ptr<Node>& node, int& counter)
	{
		node->isAlive = true;
		counter--;
		Point node_pos = node->get_pos();
		if (!board[node_pos.x][node_pos.y].isFree)
		{
			Point new_node_pos(Util::get_random_int(X - 2) + 2, Util::get_random_int(Y - 2) + 2);
			node->set_pos(new_node_pos);
		}
		Util::add_node(alive_nodes, node, nodes_mutex);
	}

	template <typename T>
	int respawn_dead_nodes(std::queue<std::shared_ptr<T>>& dead_nodes, int counter)
	{
		while (!dead_nodes.empty() && counter > 0)
		{
			std::shared_ptr<Node> node = std::static_pointer_cast<Node> (dead_nodes.front());
			std::shared_ptr<Enemy> enemy = std::dynamic_pointer_cast<Enemy>(node);
			if (enemy && enemy->isFriend) // Do not respawn the CloneSnake from clone powerup
				continue;

			dead_nodes.pop();
			respawn(node, counter);
		}
		return counter;
	}

	void respawn_nodes()
	{
		fruits_to_spawn = respawn_dead_nodes(dead_fruits, fruits_to_spawn);

		for (int i = 0; i < fruits_to_spawn; i++)
		{
			Util::add_node(alive_nodes, spawn_new_fruit(), nodes_mutex);
		}

		mines_to_spawn = respawn_dead_nodes(dead_mines, mines_to_spawn);

		for (int i = 0; i < mines_to_spawn; i++)
		{
			Util::add_node(alive_nodes, spawn_new_mine(), nodes_mutex);
		}

		powerups_to_spawn = respawn_dead_nodes(dead_powerups, powerups_to_spawn);

		for (int i = 0; i < powerups_to_spawn; i++)
		{
			Util::add_node(alive_nodes, spawn_new_powerup(), nodes_mutex);
		}

		enemies_to_spawn = respawn_dead_nodes(dead_enemies, enemies_to_spawn);

		for (int i = 0; i < enemies_to_spawn; i++)
		{
			Util::add_node(alive_nodes, spawn_new_enemy(), nodes_mutex);
		}

		fruits_to_spawn = 0;
		mines_to_spawn = 0;
		enemies_to_spawn = 0;
		powerups_to_spawn = 0;
	}

	void update_board()
	{
		std::string row_to_send = "";
		
		server.broadcastMessage(scoreBoard.getScoreboard_str()); // send the scoreboard
		for (int i = 0; i < X; i++) // x cord 
		{
			row_to_send = "";
			for (int j = 0; j < Y; j++) // y cord
			{
				Point point(i, j);
				Node node_to_check(i, j);
				std::shared_ptr<Node> node = Util::search_nodes(alive_nodes, node_to_check, nodes_mutex);

				if (Util::search(border, point)) // Check Boarder
				{
					board[i][j].tag = u8"\U00002588";
					board[i][j].color = constant::BORDER_COLOR;
					board[i][j].isFree = false;
				}
				else if (node && node->isAlive) // Check node collision
				{
					board[i][j].tag = node->get_tag(point);
					board[i][j].color = node->get_color();
					board[i][j].isFree = false;
				}
				else // Empty space
				{
					board[i][j].tag = " ";
					board[i][j].color = constant::RESET_COLOR;
					board[i][j].isFree = true;
				}
				row_to_send += board[i][j].color + board[i][j].tag;
			}
			row_to_send = row_to_send + "\n";
			server.broadcastMessage(row_to_send);
		}
		if (all_snakes_dead) // All players have died; game over
		{
			game_over();
		}
		else // Send the row to all players
		{
			row_to_send = "$end";
			server.broadcastMessage(row_to_send);
		}
	}

	void new_game()
	{
		nuke = false;
		system("cls");
		std::string message = "\033[1m\033[96m[*] \033[93mGame Starting in:  ";
		std::cout << message;
		server.broadcastMessage(message);
		for (size_t i = 5; i > 0; i--)
		{
			message = "\033[1m\033[96m\b" + std::to_string(i);
			std::cout << message;
			server.broadcastMessage(message);
			Sleep(1000);
		}
		message = "$Start";
		server.broadcastMessage(message);

		for (auto& node : alive_nodes)
		{
			node->isAlive = false;
		}

		all_snakes_dead = false;
		for (auto& snake_wptr : snakes)
		{
			if (std::shared_ptr<Snake> snake = snake_wptr.lock())
			{
				snake->respawn(Util::get_random_int(constant::Y));
				Util::add_node(alive_nodes, snake, nodes_mutex);
			}
		}

		fruits_to_spawn = 15;
		mines_to_spawn = 5;
		powerups_to_spawn = 5;
		respawn_nodes();
	}

	void game_over()
	{
		std::unique_lock <std::mutex> lock(print_mutex);
		std::unique_lock <std::mutex> np_lock(new_player); // Lock in case newplayer is currently connecting 

		std::string msg_to_send = "";
		char input;
		
		msg_to_send = "$Gameover";
		server.broadcastMessage(msg_to_send); // Send gameover signal to other players

		// Display and send the scoreboard
		system("cls");
		std::string scoreboard = scoreBoard.getFinalScoreboard_str();
		server.broadcastMessage(scoreboard);
		std::cout << scoreboard;

		// Send and display gameover animation
		msg_to_send = "\033[96m[*] ";
		std::cout << msg_to_send;
		server.broadcastMessage(msg_to_send);

		std::string message = "\033[1m\033[93mGAME OVER!";
		for (size_t i = 0; i < message.length(); i++) // Animation
		{
			std::cout << message[i];
			msg_to_send = message[i];
			server.broadcastMessage(msg_to_send);
			Sleep(100);
		}

		msg_to_send = "\n\033[96m[*] \033[93mWaiting on host ...";
		server.broadcastMessage(msg_to_send);
		
		std::cout << "\n\033[96m[*] ";
		message = "\033[93mPlay Again(y/n)? \033[96m";
		for (size_t i = 0; i < message.length(); i++) // Animation
		{
			std::cout << message[i];
			Sleep(100);
		}

		std::cin >> input;
		if (input == 'y' || input == 'Y') // Start new game
		{
			msg_to_send = "$Yes";
			server.broadcastMessage(msg_to_send);
			new_game(); 
			return;
		}
		
		msg_to_send = "$No";
		server.broadcastMessage(msg_to_send); // Tell clients no new game
		isRunning = false;
	}

	// Print Game Board
	void print_board()
	{
		std::lock_guard <std::mutex> lock(print_mutex);
		std::string output = "";

		if (!isRunning)
			return;

		system("cls");
		output += scoreBoard.getScoreboard_str();
		{
			for (int i = 0; i < X; i++)
			{
				for (int j = 0; j < Y; j++)
				{
					output += board[i][j].color + board[i][j].tag;
				}
				output += "\n";
			}
		}
		std::cout << output;
	}
	
	std::shared_ptr <Mine> spawn_new_mine()
	{
		std::shared_ptr <Mine> mine;
		int spawn_x, spawn_y;
		int x = Util::get_random_int(100);

		if (x > 95)
			mine = std::make_shared<Tornado>();
		else if (x > 85)
			mine = std::make_shared <Poop>();
		else if (x > 80)
			mine = std::make_shared <Nuke>();
		else if (x > 70)
			mine = std::make_shared <Microbe>();
		else if (x > 60)
			mine = std::make_shared <ZombieSpawner>();
		else
			mine = std::make_shared <Mine>();
		do
		{
			spawn_x = Util::get_random_int(X - 3) + 2;
			spawn_y = Util::get_random_int(Y - 3) + 2;
		} while (board[spawn_x][spawn_y].isFree && 
			board[spawn_x][spawn_y - 1].isFree && 
			board[spawn_x][spawn_y + 1].isFree); // Check for free position, as well as left and right of position

		Point spawn_point(spawn_x, spawn_y);
		mine->set_pos(spawn_point);
		return mine;
	}

	std::shared_ptr <Enemy> spawn_new_enemy()
	{
		std::shared_ptr <Enemy> enemy;
		int x = Util::get_random_int(100);
		
		if (x > 92)
		{
			enemy = std::make_shared<Dragon>();
			return enemy;
		}
		else if (x > 85)
		{
			enemy = std::make_shared<GravityGolem>();
			return enemy;
		}
		else if (x > 72)
		{
			enemy = std::make_shared<Bear>();
			return enemy;
		}
		if (x > 60)
			enemy = std::make_shared<UFO>();
		else if (x > 40)
			enemy = std::make_shared<Ghost>();
		else
			enemy = std::make_shared<Monster>();	
		do
		{
			Point spawn_point(Util::get_random_int(X - 3) + 2, Util::get_random_int(Y - 3) + 2);
			enemy->set_pos(spawn_point);
		} while (Util::search_nodes(alive_nodes, *enemy, nodes_mutex) && Util::search(border, enemy->get_pos()));
		
		return enemy;
	}

	std::shared_ptr <Fruit> spawn_new_fruit()
	{ 
		int spawn_x, spawn_y;
		std::shared_ptr<Fruit> fruit;
		int x = Util::get_random_int(100);
		if (x > 95)
			fruit = std::make_shared<Apple>();
		else if (x > 92)
			fruit = std::make_shared<Strawberry>();
		else if (x > 87)
			fruit = std::make_shared<Mushroom>();
		else if (x > 85)
			fruit = std::make_shared<Gem>();
		else if (x > 68)
			fruit = std::make_shared<Banana>();
		else if (x > 58)
			fruit = std::make_shared<Cherry>();
		else if (x > 38)
			fruit = std::make_shared<Orange>();
		else
			fruit = std::make_shared<Fruit>();
		do
		{
			spawn_x = Util::get_random_int(X - 3) + 2;
			spawn_y = Util::get_random_int(Y - 3) + 2;


		} while (board[spawn_x][spawn_y].isFree &&
			board[spawn_x][spawn_y - 1].isFree &&
			board[spawn_x][spawn_y + 1].isFree); // Check for free position, as well as left and right of position

		Point spawn_point(spawn_x, spawn_y);
		fruit->set_pos(spawn_point);
		fruits.push_back(fruit);
		return fruit;
	}

	std::shared_ptr <Powerup> spawn_new_powerup()
	{
		int spawn_x, spawn_y, attempts = 0;;
		std::shared_ptr<Powerup> powerup;
		int x = Util::get_random_int(100);
		
		if (x > 85)
			powerup = std::make_shared<BoxingGlove>();
		else if (x > 80)
			powerup = std::make_shared<Portal>();
		else if (x > 75)
			powerup = std::make_shared<Clone>();
		else if (x > 70)
			powerup = std::make_shared<CrystalBall>();
		else if (x > 65)
			powerup = std::make_shared<Trident>();
		else if (x > 55)
			powerup = std::make_shared<TimeBonus>();
		else if(x > 50)
			powerup = std::make_shared<Clover>();
		else if(x > 33)
			powerup = std::make_shared<DoublePoints>();
		else if (x > 30)
			powerup = std::make_shared<Life>();
		else if (x > 10)
			powerup = std::make_shared<SpeedBoost>();
		else
			powerup = std::make_shared<Powerup>();
		do
		{
			spawn_x = Util::get_random_int(X - 3) + 2;
			spawn_y = Util::get_random_int(Y - 3) + 2;
		} while (board[spawn_x][spawn_y].isFree &&
			board[spawn_x][spawn_y - 1].isFree &&
			board[spawn_x][spawn_y + 1].isFree); // Check for free position, as well as left and right of position

		Point spawn_point(spawn_x, spawn_y);
		powerup->set_pos(spawn_point);
		return powerup;
	}
	
	void handle_fruit_collision(std::shared_ptr<Snake>& snake, std::shared_ptr<Node>& node)
	{
		std::shared_ptr<Fruit> fruit = std::dynamic_pointer_cast<Fruit> (node);
		if (fruit == nullptr)
			return;

		node->handle_collision(*snake);

		fruits_to_spawn += Util::get_random_int(2) - 1; // Get number of fruits to spawn(0-2)
		mines_to_spawn += Util::get_random_int(2) - 1; // Get number of mines to spawn(0-2)
		powerups_to_spawn += Util::get_random_int(2) - 1; // Get number of powerups to spawn(0-2)
		
		int x = Util::get_random_int(12); // Odds of spawning enemy 
		if (x == 1)
			enemies_to_spawn += 1;
	}

	void handle_mine_collision(std::shared_ptr<Snake>& snake, std::shared_ptr<Node>& node)
	{
		if (snake->get_powerup() == "Invincible")
		{
			node->isAlive = false;
			return;
		}

		std::shared_ptr<Mine> mine = std::dynamic_pointer_cast<Mine> (node);
		if (mine == nullptr)
			return;
		
		node->handle_collision(*snake);
		if (std::dynamic_pointer_cast<ZombieSpawner>(mine)) 
		{
			// Spawn three zombies
			for (int i = 0; i < 3; i++)
			{
				std::shared_ptr<Zombie> zombie = std::make_shared<Zombie>();
				Point spawn_point(Util::get_random_int(constant::X), Util::get_random_int(constant::Y));
				zombie->set_pos(spawn_point);
				Util::add_node(alive_nodes, zombie, nodes_mutex);
			}
		}
		else if (std::dynamic_pointer_cast<Poop>(mine))
		{
			mines_to_spawn += 10; // Spawn 10 new mines
		}
		else if (std::dynamic_pointer_cast<Nuke>(mine)) // Signal nuke to begin next iteration
				nuke = true;
	}

	void handle_snake_collision(std::shared_ptr<Snake>& snake, std::shared_ptr<Node>& node)
	{
		std::shared_ptr<Snake> snake_2 = std::dynamic_pointer_cast<Snake> (node);
		if (snake_2 == nullptr || snake->id == snake_2->id)
			return;

		if (snake->get_powerup() == "Invincible")
		{
			snake_2->remove_life();
			snake->tail_credit += snake_2->score;
		}
		else
		{
			snake->remove_life();
			if (!snake->isAlive)
			{
				snake_2->tail_credit += snake->score;
			}
		}
	}

	void handle_powerup_collision(std::shared_ptr<Snake>& snake, std::shared_ptr<Node>& node)
	{
		if (std::shared_ptr<Powerup> powerup = std::dynamic_pointer_cast<Powerup>(node))
		{
			if (std::dynamic_pointer_cast<Clone>(node)) 
			{
				if (!snake->hasPowerup)
				{
					// Create clone snake
					std::shared_ptr<CloneSnake> clone = std::make_shared<CloneSnake>();
					clone->set_snake(snake);
					calculate_new_target(*clone);
					snake->set_powerup("Clone", clone->lifeTime_ms);
					Util::add_node(alive_nodes, clone, nodes_mutex);
				}
				node->isAlive = false;
			}
			else
				node->handle_collision(*snake);
		}
	}

	void handle_node_collision(std::shared_ptr<Snake>& snake, std::shared_ptr<Node>& node)
	{
		if (!node->isAlive)
			return;

		if (std::dynamic_pointer_cast<Snake> (node))
			handle_snake_collision(snake, node);
		else if (std::dynamic_pointer_cast<Fruit> (node))
			handle_fruit_collision(snake, node);
		else if (std::dynamic_pointer_cast<Mine> (node))
			handle_mine_collision(snake, node);
		else if (std::dynamic_pointer_cast<Powerup> (node))
			handle_powerup_collision(snake, node);
		else if (std::dynamic_pointer_cast<Enemy> (node))
			node->handle_collision(*snake);
	}

	void check_snake_last_move(std::shared_ptr<Snake>& snake)
	{
		if (Util::search(border, snake->get_head()))
		{
			snake->remove_life();
			return;
		}

		std::shared_ptr <Node> hit_node = Util::search_nodes(alive_nodes, *snake, nodes_mutex);
		if (hit_node)
			handle_node_collision(snake, hit_node);
	}

	void check_node_last_move(std::shared_ptr<Node>& node)
	{
		std::shared_ptr<Snake> snake = std::dynamic_pointer_cast<Snake> (node);
		
		if (snake)
		{
			add_target(snake, snake->id); // Add snake to target list for enemies
			scoreBoard.updateScoreboard(snake->name, snake->score, snake->lives);
			check_snake_last_move(snake);
			return;
		}

		std::shared_ptr<Enemy> enemy = std::dynamic_pointer_cast<Enemy> (node);
		if (enemy && !enemy->hasTarget) // Ensure enemy has target
		{
			calculate_new_target(*enemy);
		}
	}

	// Add dead node to its respective queue
	void handle_dead_node(std::shared_ptr<Node>& node)
	{	
		if (auto fruit = std::dynamic_pointer_cast<Fruit> (node))
			dead_fruits.push(std::move(fruit));
		else if (auto mine = std::dynamic_pointer_cast<Mine> (node))
			dead_mines.push(std::move(mine));
		else if (auto powerup = std::dynamic_pointer_cast<Powerup> (node))
			dead_powerups.push(std::move(powerup));
		else if (auto enemy = std::dynamic_pointer_cast<Enemy> (node))
			dead_enemies.push(std::move(enemy));
	}

	void process_nodes()
	{
		bool snake_still_alive = false;

		if (nuke) 
		{
			mines_to_spawn += 10;
		}

		if (dead_fruits.size() > 30) // Ensure fruits on the board
			fruits_to_spawn += 5;

		for (auto it = alive_nodes.begin(); it != alive_nodes.end();)
		{
			if (!it->get()->isAlive) // Case: dead node
			{
				handle_dead_node(*it);
				it = alive_nodes.erase(it);
				continue;
			}

			if (std::dynamic_pointer_cast<Snake>(*it)) // Case: Snake, check if atleast one snake is still alive
			{
				if (it->get()->isAlive)
				{
					snake_still_alive = true;
				}
			}

			it->get()->move();
			check_node_last_move(*it);
			++it;
		}
		if (snake_still_alive) 
		{
			respawn_nodes();
		}
		else
		{
			all_snakes_dead = true;
		}
	}

	void add_target(std::shared_ptr<Node> node, int id)
	{
		std::lock_guard <std::mutex> lock(targets_mutex);
		std::weak_ptr<Node> target = node;
		targets[id] = target;
	}

	void calculate_new_target(Enemy& enemy)
	{
		std::map <double, std::shared_ptr<Node>> potential_targets;
		if (!enemy.isFriend)
		{
			for (auto& node : targets)
			{
				if (std::shared_ptr <Node> target = node.second.lock())
				{
					if (enemy.id == target->id)
						continue;

					Point test_head = enemy.get_pos();
					Point displacement = target->get_pos() - test_head;
					potential_targets[displacement.magnitude()] = target; // Store target by displacement
				}
			}
		}
		else // Specific code for the clone snake
		{
			for (auto& fruit : fruits)
			{
				if (std::shared_ptr <Node> target = fruit.lock())
				{
					if (target->isAlive)
					{
						Point test_head = enemy.get_pos();
						Point displacement = target->get_pos() - test_head;
						potential_targets[displacement.magnitude()] = target; // Store target by displacement
					}
				}
			}
		}
		if (potential_targets.size() > 0)
		{
			enemy.set_target(potential_targets.begin()->second); // Pick target with smallest displacement
		}
	}
	
	std::string new_player_login(Snake& snake)
	{
		std::string username = "", message = "";

		server.sendMessage(snake.sock, constant::WELCOME_MESSAGE);

		while (isRunning)
		{
			username = server.recvMessage(snake.sock);
			if (username == server.RECV_MESSAGE_ERROR) // Login failed
				return server.RECV_MESSAGE_ERROR;

			if (scoreBoard.playerExists(username)) // Username already taken
			{
				server.sendMessage(snake.sock, constant::INVALID_USERNAME_MESSAGE);
			}
			else // Successful username
			{
				message = "$Success";
				server.sendMessage(snake.sock, message);
				message = constant::VALID_USERNAME_MESSAGE + username + "\n";
				server.sendMessage(snake.sock, message);
				break;
			}
		}
		scoreBoard.createPlayer(username);

		if(!isRunning)
			return server.RECV_MESSAGE_ERROR;

		message = "\033[1m\033[96m[*] \033[93mGame Starting in:  ";
		server.sendMessage(snake.sock, message);
		for (size_t i = 5; i > 0; i--) // Start game count down animation
		{
			message = "\033[1m\033[96m\b" + std::to_string(i);
			server.sendMessage(snake.sock, message);
			Sleep(1000);
		}
		message = "$Start";
		server.sendMessage(snake.sock, message);

		server.allowBroadcast(snake.sock);
		return username;
	}

	// Add new snake to the game
	void add_new_player(Snake& snake)
	{
		std::lock_guard <std::mutex> lock(new_player);

		std::string username = new_player_login(snake); // Initiate login process
		if (username == server.RECV_MESSAGE_ERROR) // Login failed
			return;

		auto snake_ptr = std::make_shared<Snake>(snake);
		std::weak_ptr<Snake> snake_wptr = snake_ptr;
		snake_ptr->initialize(Util::get_random_color(), Util::get_random_int(constant::Y), username); // Generate random spawn position
																									  // near the top of the board
		snakes.push_back(snake_ptr);
		Util::add_node(alive_nodes, snake_ptr, nodes_mutex);
		threads.push_back(std::thread([this, snake_ptr]() { this->get_guest_input(*snake_ptr); })); // Launch thread to receive the players input
	}

	void get_other_players()
	{
		while (isRunning)
		{
			if (!server.pendingConnection()) //If no pending connection, try again
				continue;

			SOCKET clientSock = server.getConnection();
			if (clientSock == INVALID_SOCKET) // Connection failed
			{
				continue;
			}

			Snake snake(clientSock);
			add_new_player(snake);
		}
	}

	void get_input_host(Snake& snake)
	{
		int dir_caught = 80;
		while (isRunning)
		{
			// Check for input
			if (_kbhit())
			{
				int ch = _getch();
				if (ch == 224 || ch == 0) 
					dir_caught = _getch(); 
				
				// Check if the new direction is not the opposite of the current direction
				if (dir_caught != Util::get_opposite_dir(snake.get_dir())) {
					snake.set_dir(dir_caught);
				}
			}
		}
	}

	void get_guest_input(Snake& snake)
	{
		int myInt;

		while (isRunning)
		{
			myInt = server.recvInt(snake.sock);
			if (myInt == -1) // Player disconnected
				break;

			snake.set_dir(myInt);
		}
	}

	void main_game_loop()
	{
		while (isRunning)
		{
			print_board();

			process_nodes();

			update_board();

			Sleep(16.67); // Control frame rate and speed of the game
		}
	}

public:

	// Initializes game components
	Game()
	{
		for (int i = 0; i < X; i++)
		{
			for (int j = 0; j < Y; j++)
			{
				if ((i == 0) || (i == X - 1) || (j == 0) || (j == Y - 1))
				{
					border.push_back(Point(i, j));
				}
			}
		}
		for (int i = 0; i < 50; i++)
		{
			std::shared_ptr<Fruit> fruit = spawn_new_fruit();
			fruit->isAlive = false;
			dead_fruits.push(fruit);

			if (i < 30)
			{
				std::shared_ptr<Powerup> powerup = spawn_new_powerup();
				powerup->isAlive = false;
				dead_powerups.push(powerup);

				std::shared_ptr<Mine> mine = spawn_new_mine();
				mine->isAlive = false;
				dead_mines.push(mine);
			}
			if (i < 10)
			{
				std::shared_ptr<Enemy> enemy = spawn_new_enemy();
				enemy->isAlive = false;
				dead_enemies.push(enemy);
			}
		}
		fruits_to_spawn = 10;
		mines_to_spawn = 3;
		powerups_to_spawn = 3;
		respawn_nodes();
	}

	void start_game()
	{
		std::string username;

		std::cout << constant::WELCOME_MESSAGE;
		std::cin >> username;

		scoreBoard.createPlayer(username);
		std::cout << "\033[1m\033[96m[*] \033[93mGame Starting in:  ";
		for (size_t i = 5; i > 0; i--)
		{
			std::cout << "\033[1m\033[96m\b" + std::to_string(i);
			Sleep(1000);
		}

		auto snake_host_ptr = std::make_shared<Snake>();
		std::weak_ptr<Snake> snake_host_wptr = snake_host_ptr;
		snake_host_ptr->initialize(Util::get_random_color(), 5, username);

		alive_nodes.push_back(snake_host_ptr);
		snakes.push_back(snake_host_wptr); 
		threads.push_back(std::thread([this, snake_host_ptr]() { this->get_input_host(*snake_host_ptr); }));
		threads.push_back(std::thread(&Game::get_other_players, this));
		
		main_game_loop();

		for (auto& thread : threads)
		{
			thread.join();
		}
		std::cout << "\033[1m\033[96m[*] \033[93mGame Ended" << constant::RESET_COLOR;
	}
};
#endif

