#ifndef POINT_H
#define POINT_H
#include <cmath>


// Point Class
class Point
{
public: 
    int x, y; // x,y coordinates

    // Constructor(x,y)
    Point(int x, int y)
    {
        this -> x = x;
        this -> y = y;       
    }

    // Default Constructor
    Point()
    {
        x = 0;
        y = 0;
    }

    Point(const Point& point)
    {
        this->x = point.x;
        this->y = point.y;
    }

    Point& operator=(const Point& point)
    {
        this->x = point.x;
        this->y = point.y;
        return *this;
    }

    double magnitude()
    {
        return sqrt(pow(x, 2) + pow(y, 2));
    }

    // Overload == operator for comparison
    bool operator==(const Point& otherP) const
    {
        return (x == otherP.x && y == otherP.y);
    }

    bool operator!=(const Point& point) const
    {
        return !(*this == point);
    }

    Point operator-(const Point& otherP)
    {
        return Point(x - otherP.x, y - otherP.y);
    }

    void operator<<(const Point& otherP)
    {
        std::cout << "\nX: " << x << "Y: " << y;
    }
}; 

namespace std 
{
    template<>
    struct hash<Point>
    {
        size_t operator()(const Point& point) const
        {
            return std::hash<int>()(point.x) ^ (std::hash<int>()(point.y) << 1);
        }
    };
}


#endif// end Point

