#ifndef NODE_H
#define NODE_H

#include "Point.h"
//#include "Snake.h"
#include "Constants.h"
#include <memory>
#include <string>
#include <atomic>
#include <mutex>

#include <random>

class Snake;

class Node
{

protected:
	Point pos;
	std::string tag;
	std::string color;
	//std::mutex pos_mutex;

public:
	std::atomic <bool> isAlive;
	std::atomic <bool> invisible = false;
	
	int speed;
	std::atomic <int> id;
	std::atomic <int> dir = 0;
	static std::atomic<int> node_count;

	Node()
	{
		speed = 1;
		pos.x = -1;
		pos.y = -1;
		tag = ' ';
		color = "\033[0m";
		isAlive = true;
		id = ++node_count;
	}

	Node(int x, int y)
	{
		speed = 1;
		pos.x = x;
		pos.y = y;
		tag = ' ';
		color = "\033[0m";
		isAlive = true;
		id = ++node_count;
	}

	Node(Node& node)
	{
		tag = node.tag;
		pos = node.pos;
		color = node.color;
		speed = node.speed;
		id = ++node_count;
	}

	Node(Point p)
	{
		pos = p;
		id = ++node_count;
	}

	virtual Node& operator=(Node& node)
	{
		if (this != &node)
		{
			//std::lock_guard <std::mutex> lock(pos_mutex);
			tag = node.tag;
			pos = node.get_pos();
			color = node.color;
			speed = node.speed;
		}
		return*this;
	}

	virtual ~Node() {}

	virtual void move() { }

	virtual void handle_collision(Snake& snake) 
	{
		isAlive = false;
	}

	virtual bool check_collision(Point& point, int id)
	{
		if (!isAlive)
			return false;
		
		return (get_pos() == point && this->id != id);
	}

	virtual std::string get_color()
	{
		return color;
	}

	virtual std::string get_tag(Point& point)
	{
		return tag;
	}

	Point get_pos() 
	{
		//std::lock_guard <std::mutex> lock(pos_mutex);
		return pos;
	}

	void set_pos(Point& point)
	{
		//std::lock_guard <std::mutex> lock(pos_mutex);
		pos = point;
	}

	int get_random_int(int end)
	{
		static std::default_random_engine engine{ static_cast<long unsigned int>(time(0)) };
		std::uniform_int_distribution <int> distribution_color{ 1,end };
		return distribution_color(engine);
	}

	void move_pos(int direction)
	{
		//std::lock_guard<std::mutex> lock(pos_mutex);
		switch (direction)
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
		case 7275: // Up and left
			pos.y--;
			pos.x--;
			break;
		case 7277: // Up and right
			pos.y++;
			pos.x--;
			break;
		case 8077: // Down and right
			pos.y++;
			pos.x++;
			break;
		case 8075: // Down and left
			pos.y--;
			pos.x++;
			break;
		}
	}


}; std::atomic <int> Node::node_count = 0;


struct WeakPtrLessThan
{
	bool operator()(const std::weak_ptr < Node>& a, const std::weak_ptr < Node>& b)
	{
		auto sp_a = a.lock();
		auto sp_b = b.lock();

		if (!sp_a || !sp_b || sp_a == sp_b)
			return false;

		return std::less<std::shared_ptr<Node>>()(sp_a, sp_b);
	}
};



#endif
