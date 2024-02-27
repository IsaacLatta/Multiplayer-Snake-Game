#ifndef SNAKE_H
#define SNAKE_H
#include <vector>
#include "Util.h"
#include <stdexcept>
#include <cstdlib>
#include "Timer.h"


// Snake Class
class Snake : public Node
{
 

private:
	std::vector <Point> body;
	int dir = 80;
	int frame = 1;
	std::string head_tag;
	std::string powerup;
	const static int DEFAULT = 4;
	
	std::mutex dir_mutex;
	
	Timer powerup_timer;

	void add_to_tail(Point& point)
	{
		tail_credit--;
		score++;
		body.push_back(point);
	}

	void reduce_tail()
	{
		tail_credit++;
		score--;
		body.pop_back();
	}

	// Move body of snake, following the head
	void move_body(Point& old_head)
	{
		for (int i = body.size() - 1; i > 0; i--)
		{
			body[i] = body[i - 1];
		}

		body[0] = old_head;
	}

	void check_for_powerups()
	{
		if (hasPowerup && powerup_timer.is_expired())
		{
			stop_powerup();
		}
	}

	// Move snake in direction
	void move_snake()
	{
		if (frame % 2 == 0)
		{
			frame = 1;
			Point old_head = pos; // Store old head of snake for body movement

			switch (dir)
			{
			case 72: // Up
				pos.x--;
				break;
			case 80: // Down
				pos.x++;
				break;
			case 75: // Left
				pos.y--;
				break;
			case 77: // Right
				pos.y++;
				break;
			}

			// Check for collision with body
			if (Util::search(body, pos) && powerup != "Invincible")
			{
				remove_life();
				return;
			}

			move_body(old_head); // Move snake body
		}
		else
			frame++;
	}

public:
	int score = 0;
	bool hasPowerup;
	int tail_credit = 0;
	int lives = 0;
	int speed = 1;

	int points_mulitiplyer = 1;
	SOCKET sock;
	std::string name;

	Snake()
	{
		dir = 80;
		body.reserve(1000);
	}

	Snake(SOCKET sock)
	{
		this->sock = sock;
		body.reserve(1000);
	}

	Snake(const Snake& other_snake) 
	{
		isAlive.store(other_snake.isAlive.load());
		pos = other_snake.pos;
		body = other_snake.body;
		dir = other_snake.dir;
		score = other_snake.score;
		color = other_snake.color;
		sock = other_snake.sock;
		tag = other_snake.tag;
		body.reserve(1000);
	}

	std::string get_head_tag()
	{
		return head_tag;
	}

	std::string get_color() override
	{
		if (powerup == "Invincible")
			return Util::get_random_color();
		else if (powerup == "Clone")
			return Util::clone_animation();
		return color;
	}

	std::string get_tag(Point& point) override
	{
		if (point == get_head())
			return head_tag;
		else
		{
			if (powerup == "DoublePoints")
				return Util::double_points_animation();
			if (powerup == "Destructor")
				return Util::microbe_animation();
			if (powerup == "SpeedBoost")
				return Util::speed_boost_animation();
			if (powerup == "Invisible")
				return Util::invisible_animation();
			if (powerup == "Shield")
				return Util::trident_animation();
			if (powerup == "ForceField")
				return Util::boxing_glove_animation();
			return tag;
		}
	}

	bool check_collision(Point& point, int id) override
	{
		if (!isAlive)
			return false;

		if (pos == point && this->id != id)
			return true;

		for (auto& member : body)
		{
			if (member == point)
				return true;
		}
		
		return false;
	}

	void initialize(std::string color, int pos_y, std::string name)
	{
		this->name = name;
		pos.y = pos_y;
		pos.x = 1;
		body.resize(DEFAULT);
		dir = 80; // Down
		isAlive = true;
		head_tag = Util::get_head_snake();
		this->color = Util::get_snake_color(head_tag);
		tag = u8"\U000025B8";
		powerup = "None";
		hasPowerup = false;

		for (int i = 0; i < body.size(); i++)
		{
			body[i].x =1;
			body[i].y = pos.y - (i + 1);
		}
	}

	void respawn(int pos_y)
	{
		pos.y = pos_y;
		pos.x = 1;
		dir = 80;
		isAlive = true;
		if (lives < 0)
		{
			lives = 0;
		}
		score = 0;
		powerup = "None";
		tail_credit = 0;
		body.resize(DEFAULT);

		for (int i = 0; i < body.size(); i++)
		{
			body[i].x = 1;
			body[i].y = pos.y - (i + 1);
		}
	}

	std::vector<Point> get_body()
	{
		return body;
	}

	Point get_head()
	{
		return pos;
	}

	std::string get_powerup()
	{
		return powerup;
	}

	std::string get_body_tag()
	{
		return tag;
	}

	void set_dir(int dir)
	{
		std::lock_guard <std::mutex> lock(dir_mutex);
		this -> dir = dir;
	}

	int get_dir()
	{
		std::lock_guard <std::mutex> lock(dir_mutex);
		return dir;
	}

	void setSocket(SOCKET sock)
	{
		this->sock = sock;
	}

	void set_powerup(std::string ability, int duration_ms)
	{
		hasPowerup = true;
		powerup = ability;
		powerup_timer.start_timer_ms(duration_ms);
	}

	void stop_powerup()
	{
		hasPowerup = false;
		powerup = "None";
		speed = 1;
		points_mulitiplyer = 1;
		invisible = false;
	}

	void move() override
	{
		Point old_tail;

		check_for_powerups();
		for (int i = 0; i < speed; i++)
		{
			old_tail = body[body.size() - 1];
			move_snake();
			if (tail_credit > 0)
			{
				add_to_tail(old_tail);
			}
			
			if(tail_credit < 0)
			{
				if (body.size() < 4)
				{
					remove_life();
				}
				else
				{
					reduce_tail();
				}
			}
		}
	}

	Snake& operator=(Snake& other_snake)
	{
		if (this != &other_snake)
		{
			isAlive.store(other_snake.isAlive.load());
			pos = other_snake.pos;
			body = other_snake.body;
			dir = other_snake.dir;
			score = other_snake.score;
			color = other_snake.color;
			sock = other_snake.sock;
			tag = other_snake.tag;
		}
		return*this;
	}

	void remove_life()
	{
		if (powerup == "Invincible")
			return;
		if (powerup == "ForceField")
		{
			stop_powerup();
			return;
		}

		lives = lives - 1;
		if (lives < 0)
			isAlive = false;
		else
		{
			int temp_score = score;
			respawn(Util::get_random_int(constant::Y));
			score = temp_score;
		}
	}

	//Specific to the tornado mine
	void tornado_scramble()
	{
		pos.x = Util::get_random_int(constant::X - 2);
		pos.y = Util::get_random_int(constant::Y - 20);

		for (auto& member : body)
		{
			member.x = Util::get_random_int(constant::X - 2);
			member.y = Util::get_random_int(constant::Y - 2);
		}
	}

};

#endif // SNAKE_H

