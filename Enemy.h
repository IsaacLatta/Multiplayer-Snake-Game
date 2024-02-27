#ifndef ENEMY_H
#define ENEMY_H
#include "Node.h"
#include "Util.h"
#include <algorithm>
#include "Fruit.h"
#include "Powerup.h"

class Enemy : public Node
{
public:
	std::weak_ptr <Node> target;
	std::atomic <bool> hasTarget;
	bool isFriend = false;
	int prev_dir = 80;
	int frame = 1;
	
	Enemy(){}

	virtual void set_target(std::shared_ptr<Node>& node)
	{
		hasTarget = true;
		target = node;
	}

	virtual void calculate_dir()
	{
		if (std::shared_ptr<Node> node_to_target = target.lock())
		{
			int primary_dir = 0, secondary_dir = 0, tertiary_dir = 0;

			if (!node_to_target->isAlive)
			{
				hasTarget = false;
				return;
			}

			Point ahead_pos(node_to_target->get_pos());
			Util::move_point(ahead_pos, node_to_target->dir);
			Util::move_point(ahead_pos, node_to_target->dir);

			int delta_x = ahead_pos.x - pos.x; // Vertical delta
			int delta_y = ahead_pos.y - pos.y; // Horizontal delta

			// Determine the primary, secondary, and tertiary directions based on the deltas.
			if (abs(delta_y) > abs(delta_x)) // Strong bias for horizontal movement since y is horizontal
			{
				primary_dir = (delta_y < 0) ? 75 : 77; // 75 is left, 77 is right
				secondary_dir = (delta_x < 0) ? 72 : 80; // 72 is up, 80 is down
				tertiary_dir = Util::get_opposite_dir(secondary_dir);
			}
			else // Strong bias for vertical movement since x is vertical
			{
				primary_dir = (delta_x < 0) ? 72 : 80; // 72 is up, 80 is down
				secondary_dir = (delta_y < 0) ? 75 : 77; // 75 is left, 77 is right
				tertiary_dir = Util::get_opposite_dir(secondary_dir);
			}

			// Check if the primary direction is not the opposite of the current direction
			if (primary_dir != Util::get_opposite_dir(prev_dir))
			{
				dir = primary_dir;
			}
			// If it is, choose the secondary direction
			else if (secondary_dir != Util::get_opposite_dir(prev_dir))
			{
				dir = secondary_dir;
			}
			// If secondary is also not possible, choose the tertiary direction
			else
			{
				dir = tertiary_dir;
			}

			prev_dir = dir; // Update previous direction
		}
		else
		{
			hasTarget = false;
			dir = prev_dir;
		}
	}

	virtual void kill()
	{
		isAlive = false;
	}

	void handle_collision(Snake& snake) override
	{
		if (snake.get_powerup() == "Invincible")
			kill();
		else
		{
			if (snake.get_powerup() == "ForecField")
				kill();
			snake.remove_life();
		}
	}

};

class Zombie : public Enemy
{
public:
	
	Zombie()
	{
		dir = 80;
		prev_dir = 80;
		tag = u8"\b\U0001F9DF";
		color = "\033[0m";
		frame = 1;
	}

	void move() override
	{
		//std::lock_guard <std::mutex> lock(pos_mutex);
		if (frame % 5 == 0)
		{
			frame = 1;
			calculate_dir();
			Util::move_point(pos, dir);
		}
		else
			frame++;
	}

	void calculate_dir() override
	{
		int dir_1;

		if (std::shared_ptr<Node> node_to_target = target.lock())
		{
			if (!node_to_target->isAlive)
			{
				hasTarget = false;
				dir = prev_dir;
				return;
			}

			if (node_to_target->invisible)
			{
				if (Util::get_random_int(20) == 1)
				{
					dir = Util::get_random_dir();
					prev_dir = dir;
				}
				else
					dir = prev_dir;
				return;
			}

			int delta_x = node_to_target->get_pos().x - pos.x;
			int delta_y = node_to_target->get_pos().y - pos.y;
			
			if (abs(delta_x) > abs(delta_y))
			{
				if (delta_x < 0)
					dir_1 = 72;
				else
					dir_1 = 80;

			}
			else if (abs(delta_x) < abs(delta_y))
			{
				if (delta_y < 0)
					dir_1 = 75;
				else
					dir_1 = 77;
			}
			else
				dir_1 = prev_dir;

			dir = dir_1;
		}
		else
		{
			hasTarget = false;
			dir = prev_dir;	
		}
	}
};


class Fireball : public Enemy
{
public:

	int delta_x = 0, delta_y = 0;
	int frame = 1;

	Fireball()
	{
		tag = u8"\b\U0001F525";
	}

	Fireball(const Point& point)
	{
		tag = u8"\b\U0001F525";
		pos = point;
	}

	Fireball(const Fireball& fireball)
	{
		tag = u8"\b\U0001F525";
		isAlive = fireball.isAlive.load();
		pos = fireball.pos;
		delta_x = fireball.delta_x;
		delta_y = fireball.delta_y;
		dir = fireball.dir.load();
	}

	void set_delta(int x, int y)
	{
		this->delta_x = x;
		this->delta_y = y;
	}

	void move() override
	{
		//std::lock_guard <std::mutex> lock(pos_mutex);
		if (frame % 3 == 0)
		{
			Util::move_point(pos, dir);
			Util::move_point(pos, dir);
			if (pos.x <= 0 || pos.x >= constant::X - 1 
			 || pos.y <= 0 || pos.y >= constant::Y - 1)
				isAlive = false;
		}
		else
			frame++;
	}

	std::string get_tag(Point& point) override
	{
		return tag;
	}

	/*
	void handle_collision(Snake& snake)
	{
		if (snake.get_powerup() == "Invincible" || snake.get_powerup() == "ForceField")
			isAlive = false;
		else
		{
			snake.remove_life();
		}
	}
	*/
};

class Alien : public Enemy
{
public:
	std::vector <Point> body;
	std::string head_tag;

	Alien()
	{
		head_tag = u8"\b\U0001F47D";
		tag = u8"\033[38;5;118m\U000025B8";
		body.resize(4);
		color = "\033[38;5;118m";
		

		for (int i = 0; i < body.size(); i++)
		{
			body[i].x = pos.x;
			body[i].y = pos.y - (i + 1);
		}
	}

	Alien(Point& point)
	{
		head_tag = u8"\b\U0001F47D";
		tag = u8"\033[38;5;118m\U000025B8";
		body.resize(4);
		color = "\033[38;5;118m";
		pos = point;
		
		for (int i = 0; i < body.size(); i++)
		{
			body[i].x = pos.x;
			body[i].y = pos.y - (i + 1);
		}
	}

	Alien(const Alien& alien)
	{
		head_tag = u8"\b\U0001F47D";
		tag = u8"\033[38;5;118m\U000025B8";
		body = alien.body;
		pos = alien.pos;
		target = alien.target;
		isAlive = alien.isAlive.load();
	}

	std::string get_tag(Point& point) override
	{
		if (pos == point)
			return head_tag;
		else
			return tag;
	}

	void move() override
	{
		if (frame % 3 == 0)
		{
			frame = 1;
			Point old_head(pos);
			calculate_dir();
			Util::move_point(pos, dir);
			move_body(old_head);
		}
		else
			frame++;
	}

	bool check_collision(Point& point, int id) override
	{
		if (!isAlive)
			return false;

		if (this->id == id)
			return false;

		if (pos == point)
			return true;

		for (auto& member : body)
		{
			if (member == point)
				return true;
		}

		return false;
	}

	void move_body(Point& old_head)
	{
		for (int i = body.size() - 1; i > 0; i--)
		{
			body[i] = body[i - 1];
		}

		body[0] = old_head;
	}
};

class UFO : public Enemy
{
public:
	std::vector <std::shared_ptr<Node>> nodes;
	int fireballs_to_spawn = 0;

	UFO()
	{
		tag = u8"\b\U0001F6F8";
		nodes.reserve(200);
	}

	void move() override
	{
		if (std::shared_ptr<Node> node = target.lock())
		{
			if (!node->isAlive)
			{
				hasTarget = false;
				dir = prev_dir;
				return;
			}

			calculate_dir();
			if (Util::get_random_int(20) == 1)
			{
				if (!reuse_fireball())
				{
					Fireball fireball(pos);
					fireball.dir = dir.load();
					nodes.push_back(std::make_shared<Fireball>(std::move(fireball)));
				}
			}

			if (Util::get_random_int(250) == 1)
			{
				if (!reuse_alien())
				{
					Alien alien(pos);
					alien.set_target(node);
					nodes.push_back(std::make_shared<Alien>(std::move(alien)));
				}
			}

			for (auto& node : nodes)
			{
				node->move();
			}

			if (Util::get_random_int(50) == 1)
			{
				dir = Util::get_random_dir();

				Point test_pos(pos);
				Util::move_point(test_pos, dir);
				Util::move_point(test_pos, dir);
				Util::move_point(test_pos, dir);
				if (test_pos.x <= 0 || test_pos.x >= constant::X - 1 || test_pos.y <= 0 || test_pos.y >= constant::Y - 1)
					return;

				Util::move_point(pos, dir);
				Util::move_point(pos, dir);
				Util::move_point(pos, dir);
			}
		}
		else
		{
			hasTarget = false;
			dir = prev_dir;
		}
	}
	

	bool check_collision(Point& point, int id) override
	{
		if (!isAlive)
			return false;

		if (this->id == id)
			return false;

		if (pos == point)
			return true;

		for (auto& node : nodes)
		{
			if (node->check_collision(point, -1) && node->isAlive)
				return true;
		}
		return false;
	}
		
	std::string get_tag(Point& point) override
	{
		if (pos == point)
			return tag;

		for (auto& node : nodes)
		{
			if (node->check_collision(point, -1) && node->isAlive)
				return node->get_tag(point);
		}
	}

	void kill() override
	{
		isAlive = false;
		for (auto& node : nodes)
		{
			node->isAlive = false;
			node->set_pos(pos);
		}
	}

	void handle_collision(Snake& snake) override
	{
		// Collides with fireball or alien
		for (auto& node : nodes)
		{
			Point snake_head = snake.get_pos();
			if (node->check_collision(snake_head, -1) && node->isAlive)
			{
				node->handle_collision(snake);
				return;
			}
		}

		// Collides with UFO
		if (pos == snake.get_head())
		{
			if (snake.get_powerup() == "Invincible")
			{
				kill();
			}
			else
			{
				if (snake.get_powerup() == "Shield")
					kill();
				snake.remove_life();
			}
		}

	}

	bool reuse_fireball()
	{
		for (auto& node : nodes)
		{
			auto fireball_ptr = std::dynamic_pointer_cast<Fireball>(node);
			if (fireball_ptr)
			{
				if (!fireball_ptr->isAlive)
				{
					fireball_ptr->dir = dir.load();
					fireball_ptr->set_pos(pos);
					fireball_ptr->isAlive = true;
					return true;
				}
			}
		}
		return false;
	}

	bool reuse_alien()
	{
		for (auto& node : nodes)
		{
			auto alien_ptr = std::dynamic_pointer_cast<Alien>(node);
			if (alien_ptr)
			{
				if (!alien_ptr->isAlive)
				{
					alien_ptr->isAlive = true;
					alien_ptr->set_pos(pos);
					return true;
				}
			}
		}
		return false;
	}

};

class Ghost : public Enemy
{
private:
	int slow_down_factor;
	int iteration;
	int animation_it;
	std::string old_color;

public:
	Ghost()
	{
		tag = u8"\b\U0001F47B";
		old_color = "\033[31m";
		isAlive = true;
		iteration = 1;
		dir = 2;
		slow_down_factor = 4;
		animation_it = 1;
		hasTarget = false;
		frame = 1;
	}

	void move() override
	{
		if (frame % 6 == 0)
		{
			frame = 1;
			if (std::shared_ptr<Node> node = target.lock())
			{
				if (!node->isAlive)
				{
					hasTarget = false;
					dir = prev_dir;
					return;
				}

				if (node->invisible)
				{
					if (Util::get_random_int(20) == 1)
					{
						dir = Util::get_random_dir();
						prev_dir = dir;
					}
					else
						dir = prev_dir;
					return;
				}

				int move_x = 1, move_y = 1;
				int delta_x = node->get_pos().x - pos.x;
				int delta_y = node->get_pos().y - pos.y;
				
				// Intentionally avoids the player
				if (delta_y == 0)
				{
					if (delta_y < 0)
						pos.y--;
					else
						pos.y++;
					return;
				}

				// Intentionally avoids the player
				if (delta_x == 0)
				{
					if (delta_x < 0)
						pos.x--;
					else
						pos.x++;
					return;
				}
				
				int divisor = 3; // Cut the distance the ghost will travel by 3 in each direction
				delta_x /= divisor;
				delta_y /= divisor;

				if (delta_x < 0)
					move_x = -1;
				if (delta_y < 0)
					move_y = -1;

				for (int i = 0; i < abs(delta_x); i++)
				{
					pos.x += move_x;
				}
				for (int i = 0; i < abs(delta_y); i++)
				{
					pos.y += move_y;
				}
			}
			else
				hasTarget = false;
		}
		else
			frame++;
	}
};

class Bear : public Enemy
{
private:
	bool isAwake;
	bool bearIsDead;
	std::string head_tag;
	std::vector <std::shared_ptr<Fruit>> food;
	std::vector <Point> body;

	void move_body(Point& old_head)
	{
		for (int i = body.size() - 1; i > 0; i--)
		{
			body[i] = body[i - 1];
		}

		body[0] = old_head;
	}

	std::shared_ptr<Fruit> get_food()
	{
		std::shared_ptr<Fruit> food;
		int x = Util::get_random_int(6);

		switch (x)
		{
		case 1:
			food = std::make_shared <Cake>();
			return food;
		case 2:
			food = std::make_shared <Donut>();
			return food;
		case 3:
			food = std::make_shared <FrenchFries>();
			return food;
		case 4:
			food = std::make_shared <MeatOnBone>();
			return food;
		case 5:
			food = std::make_shared <Chocolate>();
			return food;
		default:
			food = std::make_shared <Burger>();
			return food;
		}
	}

public:
	Bear()
	{
		isAwake = false;
		bearIsDead = false;
		isAlive = true;
		hasTarget = false;
		head_tag = u8"\b\U0001F43B";
		tag = u8"\033[38;5;130m\U000025B8";
		body.resize(10);
		pos.x = Util::get_random_int(constant::X);
		pos.y = Util::get_random_int(constant::Y);
		frame = 1;
	
		for (int i = 0; i < body.size(); i++)
		{
			body[i].x = pos.x;
			body[i].y = pos.y - (i + 1);

			std::shared_ptr<Fruit> item_1 = get_food();
			std::shared_ptr<Fruit> item_2 = get_food();
			Point point_1(pos.x - 1, pos.y - (i + (i + 1)));
			item_1->set_pos(point_1);

			Point point_2(pos.x + 1, pos.y - (i + (i + 1)));
			item_2->set_pos(point_2);
			
			food.push_back(item_1);
			food.push_back(item_2);
		}
	}

	void move() override
	{
		if (isAwake && hasTarget)
		{
			if (frame % 3 == 0)
			{
				frame = 1;
				calculate_dir();
				Point old_head(pos);
				Util::move_point(pos, dir);
				move_body(old_head);
			}
			else
				frame++;
		}
	}

	bool check_collision(Point& point, int id) override
	{
		if (!isAlive)
			return false;

		if (this->id == id)
			return false;

		if ((Util::search(body, point)|| point == pos ) && !bearIsDead )
		{
			return true;
		}
		
		for (auto& item : food)
		{
			if (item->check_collision(point, -1) && item->isAlive)
			{
				return true;
			}
		}
		
		return false;
	}

	// Reset bear and its food
	void kill() override
	{
		isAlive = false;
		food.clear();
		pos.x = Util::get_random_int(constant::X);
		pos.y = Util::get_random_int(constant::Y);

		for (int i = 0; i < body.size(); i++)
		{
			body[i].x = pos.x;
			body[i].y = pos.y - (i + 1);

			std::shared_ptr<Fruit> item_1 = get_food();
			std::shared_ptr<Fruit> item_2 = get_food();

			Point point_1(pos.x - 1, pos.y - (i + (i + 1)));
			item_1->set_pos(point_1);
			
			Point point_2(pos.x + 1, pos.y - (i + (i + 1)));
			item_2->set_pos(point_2);
			
			food.push_back(item_1);
			food.push_back(item_2);
		}
	}

	void handle_collision(Snake& snake) override
	{
		bool fruit_left = false;;
		for (auto& item : food)
		{
			if (item->isAlive)
				fruit_left = true;
			Point snake_head = snake.get_pos();
			if (item->check_collision(snake_head, -1))
			{
				if(!snake.invisible)
					isAwake = true;
				item->handle_collision(snake);
				item->isAlive = false;
			}
		}
		
		if (!fruit_left && bearIsDead)
			kill();

		if ((pos == snake.get_head() || Util::search(body, snake.get_head())) && !bearIsDead)
		{
			if (snake.get_powerup() == "Invincible")
			{
				bearIsDead = true;
			}
			else if (!isAwake)
			{
				isAwake = true;
			}
			else 
			{ 
				if (snake.get_powerup() == "ForceField")
					bearIsDead = true;
				snake.remove_life();
			}
		}
	}

	std::string get_tag(Point& point) override
	{
		if (pos == point && !bearIsDead)
			return head_tag;

		if (Util::search(body, point))
		{
			return tag;
		}

		for (auto& item : food)
		{
			if (item->check_collision(point, -1))
			{
				return item->get_tag(point);
			}
		}
		
		return "";
	}

};


class Dragon : public Enemy
{
private:
	bool isAwake;
	bool dragonIsDead;
	std::string head_tag;
	std::vector <std::shared_ptr<Fruit>> treasure;
	std::vector <Point> body;
	std::vector <std::shared_ptr<Fireball>> fireballs;
	std::shared_ptr <Trident> shield;

	void move_body(Point& old_head)
	{
		for (int i = body.size() - 1; i > 0; i--)
		{
			body[i] = body[i - 1];
		}

		body[0] = old_head;
	}

	std::shared_ptr<Fruit> get_treasure()
	{
		std::shared_ptr<Fruit> item;
		int x = Util::get_random_int(100);
		if (x > 30)
		{
			item = std::make_shared<MoneyBag>();
			return item;
		}
		else if (x > 12)
		{
			item = std::make_shared <Amphora>();
			return item;
		}
		else if (x > 2)
		{
			item = std::make_shared <Trophy>();
			return item;
		}
		else
		{
			item = std::make_shared <Gem>();
			return item;
		}
	}

	bool reuse_fireball()
	{
		for (auto& fireball : fireballs)
		{
			if (!fireball->isAlive)
			{
				fireball->dir = dir.load();
				fireball->set_pos(pos);
				fireball->isAlive = true;
				return true;
			}
		}
		return false;
	}

public:
	Dragon()
	{
		isAwake = false;
		dragonIsDead = false;
		isAlive = true;
		hasTarget = false;
		head_tag = u8"\b\U0001F432";
		tag = u8"\033[92m\U000025C8";
		body.resize(20);
		pos.x = Util::get_random_int(constant::X);
		pos.y = Util::get_random_int(constant::Y);
		fireballs.reserve(100);
		treasure.reserve(100);
		frame = 1;
		shield = std::make_shared<Trident>();
		Point shield_spawn_point(Util::get_random_int(constant::X), Util::get_random_int(constant::Y));
		shield->set_pos(shield_spawn_point);
		
		for (int i = 0; i < body.size(); i++)
		{
			body[i].x = pos.x;
			body[i].y = pos.y - (i + 1);

			if (i < body.size() - 10)
			{
				for (int j = 1; j < 2; j++)
				{
					std::shared_ptr<Fruit> item = get_treasure();
					Point spawn_point(pos.x - j, pos.y - (i + (i + 1)));
					item->set_pos(spawn_point);
					
					treasure.push_back(item);
				}
				for (int j = 1; j < 2; j++)
				{
					std::shared_ptr<Fruit> item = get_treasure();
					Point spawn_point(pos.x + j, pos.y - (i + (i + 1)));
					item->set_pos(spawn_point);
					
					treasure.push_back(item);
				}
			}
		}
	}

	void move() override
	{
		if (isAwake && hasTarget)
		{
			if (frame % 3 == 0)
			{
				frame = 1;
				calculate_dir();
				Point old_head(pos);
				Util::move_point(pos, dir);
				move_body(old_head);

				if (isAwake && !dragonIsDead)
				{
					if (Util::get_random_int(5) == 1)
					{
						if (!reuse_fireball())
						{
							Fireball fireball(pos);
							calculate_dir();
							fireball.dir = dir.load();
							fireballs.push_back(std::make_shared<Fireball>(std::move(fireball)));
						}
					}
				}
			}
			else
				frame++;
		}
		else
			hasTarget = false;

		if (fireballs.size() > 0)
		{
			for (auto& fireball : fireballs)
			{
				fireball->move();
			}
		}
	}

	bool check_collision(Point& point, int id) override
	{
		if (!isAlive)
			return false;

		if (this->id == id)
			return false;

		if (shield->check_collision(point, -1))
			return true;

		if ((Util::search(body, point) || point == pos) && !dragonIsDead)
		{
			return true;
		}

		for (auto& item : treasure)
		{
			if (item->check_collision(point, -1) && item->isAlive)
			{
				return true;
			}
		}

		for (auto& fireball : fireballs)
		{
			if (fireball->check_collision(point, -1) && fireball->isAlive)
				return true;
		}

		return false;
	}

	void handle_collision(Snake& snake) override
	{
		bool treasure_left = false;
		Point snake_head = snake.get_pos();
		if (shield->check_collision(snake_head, -1))
			shield->handle_collision(snake);

		for (auto& item : treasure)
		{
			if (item->isAlive)
				treasure_left = true;

			if (item->check_collision(snake_head, -1))
			{
				if (!snake.invisible)
					isAwake = true;
				item->handle_collision(snake);
				item->isAlive = false;
			}
		}

		if (!treasure_left && dragonIsDead) // Reset dragon
			kill();

		for (auto& fireball : fireballs)
		{
			if (fireball->check_collision(snake_head, -1))
				fireball->handle_collision(snake);
		}

		if ((pos == snake.get_head() || Util::search(body, snake.get_head())) && !dragonIsDead)
		{
			if (snake.get_powerup() == "Invincible")
			{
				dragonIsDead = true;
			}
			else
			{
				if (snake.get_powerup() == "ForecField")
					dragonIsDead = true;

				isAwake = true;
				snake.remove_life();
			}
		}
	}

	std::string get_tag(Point& point) override
	{
		if (pos == point && !dragonIsDead)
			return head_tag;

		if (shield->check_collision(point, -1))
			return shield->get_tag(point);

		if (Util::search(body, point))
		{
			return tag;
		}

		for (auto& item : treasure)
		{
			if (item->check_collision(point, -1))
			{
				return item->get_tag(point);
			}
		}

		for (auto& fireball : fireballs)
		{
			if (fireball->check_collision(point, -1))
			{
				return fireball->get_tag(point);
			}
		}

		return "";
	}

	void kill()
	{
		isAlive = false;
		pos.x = Util::get_random_int(constant::X);
		pos.y = Util::get_random_int(constant::Y);
		Point shield_spawn_point(Util::get_random_int(constant::X), Util::get_random_int(constant::Y));
		shield->set_pos(shield_spawn_point);

		treasure.clear();
		for (int i = 0; i < body.size(); i++)
		{
			body[i].x = pos.x;
			body[i].y = pos.y - (i + 1);

			if (i < body.size() - 10)
			{
				for (int j = 1; j < 2; j++) // Above Dragon
				{
					std::shared_ptr<Fruit> item = get_treasure();
					Point reset_point(pos.x - j, pos.y - (i + (i + 1)));
					
					item->set_pos(reset_point);
					treasure.push_back(item);
				}
				for (int j = 1; j < 2; j++) // Below Dragon
				{
					std::shared_ptr<Fruit> item = get_treasure();
					Point reset_point(pos.x + j, pos.y - (i + (i + 1)));
					
					item->set_pos(reset_point);
					treasure.push_back(item);
				}
			}
		}

		for (auto& fireball : fireballs) // Reset fireball position
		{
			fireball->isAlive = false;
			fireball->set_pos(pos);
		}

	}	
};

class GravityGolem : public Enemy
{
private:
	std::vector <Point> body;
	std::string head_tag;

	void move_body(Point& old_head)
	{
		for (int i = body.size() - 1; i > 0; i--)
		{
			body[i] = body[i - 1];
		}

		body[0] = old_head;
	}

public:
	GravityGolem()
	{
		isAlive = true;
		hasTarget = false;

		head_tag = u8"\b\U0001F31A";
		tag = u8"\033[35m\U000025C9";
		body.resize(10);
		pos.x = Util::get_random_int(constant::X);
		pos.y = Util::get_random_int(constant::Y) - 10;
		frame = 1;

		for (int i = 0; i < body.size(); i++)
		{
			body[i].x = pos.x;
			body[i].y = pos.y - (i + 1);
		}
	}

	void move() override
	{
		if (!isAlive)
			return; 
		if (frame % 6 == 0)
		{
			frame = 1;
			calculate_dir();
			Point old_head(pos);
			Util::move_point(pos, dir);
			move_body(old_head);
			if (std::shared_ptr<Node> node = target.lock())
			{
				if (std::shared_ptr<Snake> snake = std::dynamic_pointer_cast<Snake>(node))
				{
					if (snake->get_powerup() == "Invincible")
						return;
				}
				Point displacement = node->get_pos() - pos;
				if (displacement.magnitude() < 10 && isAlive)
				{
					if (displacement.magnitude() < 7)
					{
						if (displacement.magnitude() < 3)
						{
							node->move_pos(Util::get_opposite_dir(dir));
						}
						node->move_pos(Util::get_opposite_dir(dir));
					}
					else if(Util::get_random_int(4) == 1)
						node->move_pos(Util::get_opposite_dir(dir));
				}
			}
		}
		else
			frame++;
	}

	std::string get_tag(Point& point)
	{
		if (pos == point)
		{
			return head_tag;
		}
		else
			return tag;
	}

	bool check_collision(Point& point, int id) override
	{
		if (!isAlive)
			return false;

		if (this->id == id)
			return false;

		if (pos == point)
			return true;

		for (auto& member : body)
		{
			if (member == point)
				return true;
		}
		
		return false;
	}
};

class CloneSnake : public Enemy
{
private:
	bool hasSnake;
	std::string head_tag;
	std::vector <Point> body;
	std::weak_ptr<Snake> snake;
	Timer timer;

	void move_body(Point& old_head)
	{
		for (int i = body.size() - 1; i > 0; i--)
		{
			body[i] = body[i - 1];
		}

		body[0] = old_head;
	}

public:
	int lifeTime_ms = 10000;

	CloneSnake()
	{
		isAlive = true;
		isFriend = true;
		hasSnake = false;
		head_tag = u8"\b\U0001F445";
		tag = u8"\U000025C9";
		frame = 1;
		dir = 80;

		pos.x = Util::get_random_int(constant::X);
		pos.y = Util::get_random_int(constant::Y);
		body.resize(6);

		for (int i = 0; i < body.size(); i++)
		{
			body[i].x = pos.x;
			body[i].y = pos.y - (i + 1);
		}
		timer.start_timer_ms(lifeTime_ms);
	}

	void calculate_dir()
	{
		if (std::shared_ptr<Node> node_to_target = target.lock())
		{
			int primary_dir = 0, secondary_dir = 0, tertiary_dir = 0;

			if (!node_to_target->isAlive)
			{
				hasTarget = false;
				return;
			}

			int delta_x = node_to_target->get_pos().x - pos.x; // Vertical delta
			int delta_y = node_to_target->get_pos().y - pos.y; // Horizontal delta
			
			// Determine the primary, secondary, and tertiary directions based on the deltas.
			if (abs(delta_y) > abs(delta_x)) // Strong bias for horizontal movement since y is horizontal
			{
				primary_dir = (delta_y < 0) ? 75 : 77; // 75 is left, 77 is right
				secondary_dir = (delta_x < 0) ? 72 : 80; // 72 is up, 80 is down
				tertiary_dir = Util::get_opposite_dir(secondary_dir);
			}
			else // Strong bias for vertical movement since x is vertical
			{
				primary_dir = (delta_x < 0) ? 72 : 80; // 72 is up, 80 is down
				secondary_dir = (delta_y < 0) ? 75 : 77; // 75 is left, 77 is right
				tertiary_dir = Util::get_opposite_dir(secondary_dir);
			}

			// Check if the primary direction is not the opposite of the current direction
			if (primary_dir != Util::get_opposite_dir(prev_dir))
			{
				dir = primary_dir;
			}
			// If it is, choose the secondary direction
			else if (secondary_dir != Util::get_opposite_dir(prev_dir))
			{
				dir = secondary_dir;
			}
			// If secondary is also not possible, choose the tertiary direction
			else
			{
				dir = tertiary_dir;
			}

			prev_dir = dir; // Update previous direction
		}
		else
		{
			hasTarget = false;
			dir = prev_dir;
		}
	}
	
	void move() override
	{
		if (timer.is_expired())
			isAlive = false;
		if (frame % 3 == 0)
		{
			if (std::shared_ptr<Node> my_target = target.lock())
			{
				if (std::shared_ptr<Snake> my_snake = snake.lock())
				{
					if (!my_snake->isAlive)
					{
						isAlive = false;
						return;
					}
					if (pos == my_target->get_pos() && my_target->isAlive && isAlive)
					{
						my_target->handle_collision(*my_snake);
						hasTarget = false;
					}
				}
				else
					hasSnake = false;
			}
			else
				hasTarget = false;

			frame = 1;
			calculate_dir();
			Point old_head = pos;
			Util::move_point(pos, dir);
			move_body(old_head);
		}
		else
			frame++;
	}

	void set_snake(std::shared_ptr<Snake>& snake)
	{
		hasSnake = true;
		head_tag = snake->get_head_tag();
		tag = snake->get_body_tag();
		this->snake = snake;
	}

	std::string get_tag(Point& point)
	{
		if (pos == point)
		{
			return head_tag;
		}
		else
			return tag;
	}

	std::string get_color()
	{
		if (Util::get_random_int(10) == 1)
			return "\033[33m"; // Blue 
		else
			return "\033[94m"; // Yellow
	}

	bool check_collision(Point& point, int id) override
	{
		if (!isAlive)
			return false;

		if (this->id == id)
			return false;

		if (pos == point)
			return true;

		for (auto& member : body)
		{
			if (member == point)
				return true;
		}

		return false;
	}
};

class Monster : public Enemy
{
public:
	Monster()
	{
		tag = u8"\b\U0001F47E";
		isAlive = true;
		frame = 1;
		dir = Util::get_random_dir();
	}

	void move() override
	{
		if (frame % 3 == 0)
		{
			frame = 1;

			if (Util::get_random_int(30) == 1)
			{
				dir = Util::get_opposite_dir(dir);
			}
			
			Point test_pos(pos);
			Util::move_point(test_pos, dir);
			if (test_pos.x <= 0 || test_pos.x >= constant::X - 1 || test_pos.y <= 0 || test_pos.y >= constant::Y - 1)
			{
				dir = Util::get_opposite_dir(dir);
			}
			Util::move_point(pos, dir);
		}
		else
			frame++;
	}
};

#endif
