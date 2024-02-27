#ifndef POWERUP_H
#define POWERUP_H
#include "Node.h"
#include "Snake.h"

class Snake;

class Powerup : public Node
{
public:
	int reset_score;
	int duration_ms;
	std::string ability;
	
	Powerup()
	{
		pos.x = 0;
		pos.y = 0;
		reset_score = 15;
		ability = "Invincible";
		tag = u8"\b\U0001F31F";
		color = "Rainbow";
		duration_ms = 12000;
	}

	std::string get_color() override
	{
		std::string color;
		int x = get_random_int(9);

		switch (x)
		{
		case 1: color = "\033[96m"; break; // Bright Cyan
		case 2: color = "\033[31m"; break; // Red
		case 3: color = "\033[93m"; break; // Bright Yellow
		case 4: color = "\033[33m"; break; // Yellow
		case 5: color = "\033[35m"; break; // Magenta
		case 6: color = "\033[36m"; break; // Cyan
		case 7: color = "\033[37m"; break; // White
		case 8: color = "\033[94m"; break; // Gray
		case 9: color = "\033[93m"; break; // Bright Yellow	
		}

		return color;
	}

	void handle_collision(Snake& snake) override
	{
		if(!snake.hasPowerup || snake.get_powerup() == ability)
			snake.set_powerup(ability, duration_ms);
		isAlive = false;
	}
};

class Life : public Powerup
{
public:

	Life()
	{
		reset_score = 0;
		ability = "None";
		tag = u8"\b\U0001F496";
		color = "\033[92m"; // Light Green
	}

	void handle_collision(Snake& snake) override
	{
		snake.lives++;
		isAlive = false;
	}

};

class DoublePoints : public Powerup
{
public:

	DoublePoints()
	{
		tag = u8"\b\U0001F9EC";
		ability = "DoublePoints";
		duration_ms = 10000;
	}

	void handle_collision(Snake& snake) override
	{
		if (!snake.hasPowerup || snake.get_powerup() == ability)
		{
			snake.points_mulitiplyer = 2;
			snake.set_powerup(ability, duration_ms);
		}
		isAlive = false;
	}
};

class Clover : public Powerup
{
public:
	Clover()
	{
		tag = u8"\b\U0001F340";
	}

	void handle_collision(Snake& snake) override
	{
		if (!snake.hasPowerup)
		{
			int x = Util::get_random_int(7);
			isAlive = false;
			switch (x)
			{
			case 1:
				snake.set_powerup("DoublePoints", 10000);
				snake.points_mulitiplyer = 2;
				return;
			case 2:
				snake.invisible = true;
				snake.set_powerup("Invincible", 10000);
				return;
			case 3:
				snake.lives++;
				return;
			case 4:
				snake.set_powerup("Invisible", 10000);
				return;
			case 5:
				snake.set_powerup("ForceField", 10000);
				return;
			case 6:
				snake.set_powerup("Shield", 15000);
				return;
			default:
				snake.set_powerup("SpeedBoost", 200);
				snake.speed = 2;
			}
		}
	}
};

class SpeedBoost : public Powerup
{
public:

	SpeedBoost()
	{
		tag = u8"\b\U0001F680";
		ability = "SpeedBoost";
		isAlive = true;
		duration_ms = 200;
	}

	void handle_collision(Snake& snake) override
	{
		if (!snake.hasPowerup)
		{
			snake.speed = 8;
			snake.set_powerup("SpeedBoost", duration_ms);
		}
		isAlive = false;
	}
};

class CrystalBall : public Powerup
{
public:

	CrystalBall()
	{
		tag = u8"\b\U0001F52E";
		ability = "Invisible";
		isAlive = true;
		duration_ms = 10000;
	}

	void handle_collision(Snake& snake) override
	{
		if (!snake.hasPowerup)
		{
			snake.invisible = true;
			snake.set_powerup(ability, duration_ms);
		}
		isAlive = false;

	}
};

class Clone : public Powerup
{
public:
	Clone()
	{
		tag = u8"\b\U0001F3AD";
		isAlive = true;
		duration_ms = 10000;
		ability = "Clone";
	}
};

class Trident : public Powerup
{
public:
	Trident()
	{
		tag = u8"\b\U0001F531";
		isAlive = true;
		duration_ms = 15000;
		ability = "Shield";
	}
};

class TimeBonus : public Powerup
{
public:

	TimeBonus()
	{
		tag = u8"\b\U000023F0";
		isAlive = true;
		duration_ms = 10000;
	}

	void handle_collision(Snake& snake) override
	{
		if (snake.hasPowerup)
		{
			snake.set_powerup(snake.get_powerup(), duration_ms);
		}
		isAlive = false;
	}

};
#endif

class BoxingGlove : public Powerup
{
public:
	BoxingGlove()
	{
		isAlive = true;
		ability = "ForceField";
		duration_ms = 10000;
		tag = u8"\b\U0001F94A";
	}
};

class Portal : public Powerup
{
public:
	Point other_pos;

	Portal()
	{
		isAlive = true;
		ability = "Teleporting";
		duration_ms = 500;
		tag = u8"\b\U0001F6AA";
		other_pos.x = Util::get_random_int(constant::X);
		other_pos.y = Util::get_random_int(constant::Y);
	}

	bool check_collision(Point& point, int id) override
	{
		if (this->id == id)
			return false;

		if (pos == point || other_pos == point)
			return true;
		
		return false;
	}

	void handle_collision(Snake& snake) override
	{
		if (pos == snake.get_pos() && snake.get_powerup() != "Teleporting")
		{
			snake.set_powerup("Teleporting", duration_ms);
			Point new_pos(other_pos.x, other_pos.y + 1);
			snake.set_pos(new_pos);
		}
		else if (other_pos == snake.get_pos() && snake.get_powerup() != "Teleporting")
		{
			snake.set_powerup("Teleporting", duration_ms);
			Point new_pos(pos.x, pos.y - 1);
			snake.set_pos(new_pos);
		}
	}
};
