#ifndef MINE_H
#define MINE_H
#include "Node.h"

class Mine : public Node
{
public: 

	Mine()
	{
		pos.x = 0;
		pos.y = 0;
		color = "\033[91m"; // Light Red
		tag = u8"\b\U0001F4A3";
		isAlive = true;
	}

	void handle_collision(Snake& snake) override
	{
		if (!(snake.get_powerup() == "Invincible"))
		{
			snake.remove_life();
		}
		isAlive = false;
	}
};

class Poop : public Mine
{
public:

	Poop()
	{
		tag = u8"\b\U0001F4A9";
		color = "\033[91m"; // Red
		isAlive = true;
	}

	void handle_collision(Snake& snake) override
	{
		isAlive = false;
	}
};

class Nuke : public Mine
{
public: 
	Nuke()
	{
		tag = u8"\b\U0001F9E8";
		isAlive = true;
	}

	std::string get_tag(Point& point) override
	{
		return tag;
	}

	void handle_collision(Snake& snake) override
	{
		isAlive = false;
	}
};

class ZombieSpawner : public Mine
{
public: 
	ZombieSpawner()
	{
		tag = u8"\b\U0001F300";
	}

	void handle_collision(Snake& snake) override
	{
		isAlive = false;
	}
};

class Microbe : public Mine
{
public:
	std::string ability = "Destructor";

	Microbe()
	{
		tag = u8"\b\U0001F9A0";
		isAlive = true;
	}

	void handle_collision(Snake& snake) override
	{
		if (!(snake.get_powerup() == "Invincible" || snake.get_powerup() == "Shield"))
		{
			snake.tail_credit -= snake.get_body().size() - 4;
			snake.set_powerup(ability, 5000);
		}
		isAlive = false;
	}
};

class Tornado : public Mine
{
public:
	Tornado()
	{
		tag = u8"\U0001F32A";
		isAlive = true;
	}

	void handle_collision(Snake& snake) override
	{
		snake.tornado_scramble();
	}
};



#endif