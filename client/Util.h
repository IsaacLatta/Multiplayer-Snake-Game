#ifndef UTIL_H
#define UTIL_H

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include "Windows.h"
#include <algorithm>
#include <random>

class Util
{
public: 
	
	// Generic search
	template <typename T>
	static bool search(std::vector <T>& v, T object)
	{
		bool ret = false;
		auto it = std::find(v.begin(), v.end(), object);

		return it != v.end();
	}

	// Search and return
	template <typename T>
	static T* search_ret(std::vector <T>& v, T& object)
	{
		for (int i = 0; i < v.size(); i++)
		{
			if (v[i] == object)
			{
				return &v[i];
			}
		}
		return nullptr;
	}

	// Gets opposite direction
	static int opposite_dir(int& prev_dir)
	{
		int ret = 0;

		switch (prev_dir)
		{
		case 72: // Prev was Up
			ret = 80; // Ret down
			break;
		case 80: // Prev was Down
			ret = 72; // Ret up
			break;
		case 75: // Prev was left
			ret = 77; // Ret right
			break;
		case 77: // Prev was Right 
			ret = 75; // Ret Left
			break;
		}
		return ret;
	}

};

struct Element
{
	char tag;
	std::string color;

	Element()
	{
		tag = ' ';
		color = "\033[0m";
	}

};

struct User
{
	std::string username = "XXX";
	int score = 0;

	User()
	{}

	User(std::string& username, int& score)
	{
		this->username = username;
		this->score = score;
	}
};


#endif 

