#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <mutex>
#include <algorithm>
#include <random>
#include <list>
#include "Node.h"


namespace Util
{
	
	bool search(std::vector<Point>& points,const Point& point)
	{
		for (auto& p : points)
		{
			if (point == p)
			{
				return true;
			}
		}
		return false;
	}
	
	template <typename T>
	bool search(std::vector<T>& vec, T a)
	{
		for (auto& v : vec)
		{
			if (a==v)
			{
				return true;
			}
		}
		return false;
	}

	std::shared_ptr<Node> search_nodes(std::list <std::shared_ptr<Node>>& nodes, Node& my_node, std::mutex& m)
	{
		std::lock_guard <std::mutex> lock(m);
		for (auto& node : nodes)
		{
			if (node != nullptr)
			{
				Point node_pos = my_node.get_pos();
				if (node->check_collision(node_pos, my_node.id))
					return node;
			}
		}
		return nullptr;
	}

	void remove_node(std::vector <std::shared_ptr<Node>>& nodes, std::shared_ptr<Node> node, std::mutex& m)
	{
		std::lock_guard <std::mutex> lock(m);
		auto it = std::find(nodes.begin(), nodes.end(), node);
		if (it != nodes.end())
			nodes.erase(it);
	}

	void add_node(std::list <std::shared_ptr<Node>>& nodes, const std::shared_ptr<Node>& node, std::mutex& m)
	{
		std::lock_guard <std::mutex> lock(m);
		nodes.push_back(std::move(node));
	}

	void move_point(Point& point, int direction)
	{
		switch (direction)
		{
		case 72: // Up
			point.x--;
			break;
		case 80: // Down
			point.x++;
			break;
		case 75: // Left
			point.y--;
			break;
		case 77: // Right
			point.y++;
			break;
		case 7275: // Up and left
			point.y--;
			point.x--;
			break;
		case 7277: // Up and right
			point.y++;
			point.x--;
			break;
		case 8077: // Down and right
			point.y++;
			point.x++;
			break;
		case 8075: // Down and left
			point.y--;
			point.x++;
			break;
		}
	}

	// Gets opposite direction
	static int get_opposite_dir(int prev_dir)
	{
		int ret = 0;

		switch (prev_dir)
		{
		case 72: // Prev was Up
			return 80; // Ret down
		case 80: // Prev was Down
			return 72; // Ret up
		case 75: // Prev was left
			return 77; // Ret right
		case 77: // Prev was Right 
			return 75; // Ret Left
		case 7275: // Up and left
			return 8077; // Down and right
		case 7277: // Up and right
			return 8075; // Down and left
		case 8077: // Down and right
			return 7275; // Up and left
		case 8075: // Down and left
			return 7277; // Down and right
		}
		return ret;
	}
	
	int get_random_int(int end)
	{
		static std::default_random_engine engine{ static_cast<long unsigned int>(time(0)) };
		std::uniform_int_distribution <int> distribution_color{ 1,end };
		return distribution_color(engine);
	}

	int get_random_dir()
	{
		int x = get_random_int(4);
		if (x == 1)
			return 80;
		if (x == 2)
			return 72;
		if (x == 3)
			return 77;
		return 75;
	}

	std::string microbe_animation()
	{
		std::string chars = "!*&%$@#**?";
		std::string ret_str = "";

		int x = get_random_int(chars.size() - 1);
		ret_str += chars[x];

		return ret_str;
	}

	std::string double_points_animation()
	{
		if (Util::get_random_int(10) > 9)
			return u8"2";
		else
			return u8"\U000025B8";
	}

	std::string speed_boost_animation()
	{
		return "\033[1m\033[93m+";
	}

	std::string invisible_animation()
	{
		if (get_random_int(30) == 1)
			return u8"\U000025B8";
		else
			return " ";
	}

	std::string trident_animation()
	{
		if (get_random_int(10) > 5)
			return u8"\033[38;5;220m\U000025C8";
		else
			return u8"\U000025C8";
	}

	std::string boxing_glove_animation()
	{
		if (get_random_int(5) > 2)
			return u8"\033[34m\U000025C8";
		else
			return u8"\033[31m\U000025C8";
	}

	std::string clone_animation()
	{
		if (Util::get_random_int(10) == 1)
			return "\033[94m"; // Yellow
		else
			return "\033[33m"; // Blue 
	}

	// Function to generate a random head for the snake based on a wide range of emojis
	std::string get_head_snake() 
	{
		std::string head;
		int x = get_random_int(36);

		switch (x) 
		{	
		case 1: head = u8"\b\U0001F383"; break; // Pumpkin face, orange
		case 2: head = u8"\b\U0001F419"; break; // Octopus
		case 3: head = u8"\b\U0001F438"; break; // Frog face
		case 4: head = u8"\b\U0001F43D"; break; // Pig nose
		case 5: head = u8"\b\U0001F5FF"; break; // Easter island guy
		case 6: head = u8"\b\U0001F620"; break; // Angry face, red
		case 7: head = u8"\b\U0001F601"; break; // Grinning face with smiley eyes, yellow
		case 8: head = u8"\b\U0001F385"; break; // Santa head, red
		case 9: head = u8"\b\U0001F414"; break; // Chicken head, white
		case 10: head = u8"\b\U0001F420"; break; // Tropical fish, colorful
		case 11: head = u8"\b\U0001F428"; break; // Koala head, grey
		case 12: head = u8"\b\U0001F434"; break; // Horse head, brown
		case 13: head = u8"\b\U0001F436"; break; // Dog head, brown
		case 14: head = u8"\b\U0001F437"; break; // Pig head, pink
		case 15: head = u8"\b\U0001F41D"; break; // Eyeball emoji, white
		case 16: head = u8"\b\U0001F43D"; break; // Pig nose, pink
		case 17: head = u8"\b\U0001F444"; break; // Mouth, red
		case 18: head = u8"\b\U0001F474"; break; // Older man, skin tone
		case 19: head = u8"\b\U0001F475"; break; // Older woman, skin tone
		case 20: head = u8"\b\U0001F47C"; break; // Baby angel, light skin tone
		case 21: head = u8"\b\U0001F608"; break; // Purple smiling face with horns, purple
		case 22: head = u8"\b\U0001F31E"; break; // Sun with face, yellow
		case 23: head = u8"\b\U0001F31D"; break; // Quarter moon with face, light blue
		case 24: head = u8"\b\U0001F922"; break; // Nauseated face, green
		case 25: head = u8"\b\U0001F976"; break; // Ice cold face, light blue
		case 26: head = u8"\b\U0001F921"; break; // Clown face, red
		case 27: head = u8"\b\U0001F916"; break; // Robot, grey
		case 28: head = u8"\b\U0001F63E"; break; // Pouting cat, grey
		case 29: head = u8"\b\U0001F9E0"; break; // Brain, pink
		case 30: head = u8"\b\U0001F482"; break; // Queen's guard, red
		case 31: head = u8"\b\U0001F9D5"; break; // Woman with veil, light skin tone
		case 32: head = u8"\b\U0001F42E"; break; // Cow face, white
		case 33: head = u8"\b\U0001F30D"; break; // Globe, blue
		case 34: head = u8"\b\U0001F93F"; break; // Diving mask, blue
		case 35: head = u8"\b\U0001F3A9"; break; // Tophat, black
		case 36: head = u8"\b\U0001F628"; break; // Open mouth face, yellow
		}

		return head;
	}

	// Function to map each head emoji to its synergistic color
	std::string get_snake_color(std::string& head) {
		// Mapping heads to colors using if-else statements
		if (head == u8"\b\U0001F620" || head == u8"\b\U0001F621") return "\033[31m"; // Red
		else if (head == u8"\b\U0001F601" || head == u8"\b\U0001F603" || head == u8"\b\U0001F604") return "\033[33m"; // Yellow
		else if (head == u8"\b\U0001F385" || head == u8"\b\U0001F608" || head == u8"\b\U0001F482") return "\033[31m"; // Red
		else if (head == u8"\b\U0001F414" || head == u8"\b\U0001F42E") return "\033[97m"; // White
		else if (head == u8"\b\U0001F420") return "\033[34m"; // Blue (representing colorful)
		else if (head == u8"\b\U0001F428" || head == u8"\b\U0001F916") return "\033[90m"; // Grey
		else if (head == u8"\b\U0001F434" || head == u8"\b\U0001F436") return "\033[33m"; // Brown (no direct ANSI color, using yellow as approximation)
		else if (head == u8"\b\U0001F437" || head == u8"\b\U0001F43D") return "\033[35m"; // Pink
		else if (head == u8"\b\U0001F41D") return "\033[37m"; // White (for eyeball, assuming white is the primary color)
		else if (head == u8"\b\U0001F444" || head == u8"\b\U0001F9E0") return "\033[31m"; // Red (mouth and brain, with red as a strong color)
		else if (head == u8"\b\U0001F474" || head == u8"\b\U0001F475" || head == u8"\b\U0001F9D5") return "\033[38;5;255m"; // Skin tone (using white as approximation)
		else if (head == u8"\b\U0001F47C") return "\033[97m"; // Light skin tone (white)
		else if (head == u8"\b\U0001F31E" || head == u8"\b\U0001F31D") return "\033[33m"; // Yellow for sun and light blue for moon
		else if (head == u8"\b\U0001F922") return "\033[32m"; // Green for nauseated face
		else if (head == u8"\b\U0001F976" || head == u8"\b\U0001F93F") return "\033[34m"; // Light blue for ice cold face and diving mask
		else if (head == u8"\b\U0001F921") return "\033[31m"; // Red for clown face
		else if (head == u8"\b\U0001F63E") return "\033[90m"; // Grey for pouting cat
		else if (head == u8"\b\U0001F1E8\U0001F1E6") return "\033[31m"; // Red and white for Canada flag (using red)
		else if (head == u8"\b\U0001F1EE\U0001F1F3") return "\033[33m"; // Orange, white, green for India flag (using orange)
		else if (head == u8"\b\U0001F30D") return "\033[34m"; // Blue for globe
		else return "\033[90m"; // Default grey for any unhandled cases
	}

	std::string get_random_color()
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
		case 6: color= "\033[36m"; break; // Cyan
		case 7: color = "\033[37m"; break; // White
		case 8: color = "\033[94m"; break; // Gray
		case 9: color = "\033[93m"; break; // Bright Yellow	
		}
		return color;
	}

	int gcd(int a, int b)
	{
		return b == 0 ? a : gcd(b, a % b);
	}

};

struct Element
{
	std::string tag;
	std::string color;
	std::atomic <bool> isFree;

	Element()
	{
		tag = ' ';
		color = "\033[0m";
	}
};

/*
*/

#endif 

