#ifndef FRUIT_H
#define FRUIT_H
#include "Node.h"
#include "Snake.h"

//class Snake;

class Fruit : public Node
{
public:
    std::string color;
    int value;
   
    Fruit()
    {
        pos.x = 0;
        pos.y = 0;
        value = 1;
        tag = u8"\b\U0001F349";
        isAlive = true;
    }

    std::string get_color() override
    {
        return color;
    }

    void handle_collision(Snake& snake) override
    {
        snake.tail_credit += snake.points_mulitiplyer * value;
        isAlive = false;
    }
};

class Banana : public Fruit
{
public:

    Banana()
    {
        color = "\033[93m"; // Bright Yellow
        tag = u8"\b\U0001F34C";
        value = 5;
        isAlive = true;
    }
};

class Cherry : public Fruit
{
public:
    Cherry()
    {
        tag = u8"\b\U0001F352";
        value = 10;
        isAlive = true;
    }
};

class Apple : public Fruit
{
public:
    int slow_down_factor;
    int iteration;
    int prev_roll = 3;

    Apple()
    {
        color = "\033[32m";
        tag = u8"\b\U0001F34F";
        value = 20;
        isAlive = true;
        slow_down_factor = 3;
        iteration = 1;
        dir = get_random_int(3);
    }

    void move() override
    {
        if (iteration % slow_down_factor == 0)
        {
            iteration = 1;
            int roll = get_random_int(200);
            if (roll == 1)
                roll = get_random_int(9);
            
            switch (roll)
            {
            case 1:
                dir = 72;
                break;
            case 2:
                dir = 80;
                break;
            case 3:
                dir = 77;
                break;
            case 4:
                dir = 75;
                break;
            case 6:
                dir = 7277;
                break;
            case 7:
                dir = 7275;
                break;
            case 8:
                dir = 8075;
                break;
            case 9:
                dir = 8077;
                break;
            }

            Point test_pos(pos);
            Util::move_point(test_pos, dir);

            // Check for collision with boarder
            if (test_pos.x <= 1 || test_pos.x >= constant::X - 1 || 
                test_pos.y <= 1 || test_pos.y >= constant::Y - 1)
                dir = Util::get_opposite_dir(dir);

            Util::move_point(pos, dir);
        }
        else
            iteration++;
    }
};

class Gem : public Fruit
{
public:

    Gem()
    {
        value = 50;
        tag = u8"\b\U0001F48E";
        isAlive = true;
    }
};

class Orange : public Fruit
{
public :

    Orange()
    {
        tag = u8"\b\U0001F34A";
        value = 2;
        isAlive = true;
    }
};

class Strawberry : public Fruit
{
public:
    Strawberry()
    {
        tag = u8"\b\U0001F353";
        value = 15;
        isAlive = true;
    }
};

class Burger : public Fruit
{
public:
    Burger()
    {
        tag = u8"\b\U0001F354";
        value = 8;
        isAlive = true;
    }
};

class Mushroom : public Fruit
{
public:
    Mushroom()
    {
        tag = u8"\b\U0001F344";
        value = get_random_int(20);
        isAlive = true;
    }
};

class Chocolate : public Fruit
{
public:
    Chocolate()
    {
        tag = u8"\b\U0001F36B";
        value = 2;
        isAlive = true;
    }
};

class MeatOnBone : public Fruit
{
public:
    MeatOnBone()
    {
        tag = u8"\b\U0001F356";
        value = 5;
        isAlive = true;
    }
};

class FrenchFries : public Fruit
{
public:
    FrenchFries()
    {
        tag = u8"\b\U0001F35F";
        value = 3;
        isAlive = true;
    }
};

class Donut : public Fruit
{
public:
    Donut()
    {
        tag = u8"\b\U0001F369";
        value = 4;
        isAlive = true;
    }
};

class Cake : public Fruit
{
public:
    Cake()
    {
        tag = u8"\b\U0001F370";
        value = 7;
        isAlive = true;
    }
};

class MoneyBag : public Fruit
{
public:
    MoneyBag()
    {
        tag = u8"\b\U0001F4B0";
        value = 5;
        isAlive = true;
    }
};

class Dollar : public Fruit
{
public:
    Dollar()
    {
        tag = u8"\b\U0001F4B2";
        value = 5;
        isAlive = true;
    }
};

class Amphora : public Fruit
{
public:
    Amphora()
    {
        tag = u8"\b\U0001F3FA";
        value = 5;
        isAlive = true;
    }
};

class Trophy : public Fruit
{
public:
    Trophy()
    {
        tag = u8"\b\U0001F3C6";
        value = 8;
        isAlive = true;
    }
};




#endif

