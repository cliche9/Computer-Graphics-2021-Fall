//
//  z-buffering.hpp
//  Z-buffering
//
//  Created by abc_mac on 2021/10/29.
//

#ifndef z_buffering_hpp
#define z_buffering_hpp

#define GL_SILENCE_DEPRECATION
#define SCR_WIDTH 640
#define SCR_HEIGHT 480
#include <GLUT/glut.h>
#include <vector>
#include <algorithm>
using namespace std;

struct Color {
    GLfloat r;
    GLfloat g;
    GLfloat b;
    
    Color(const GLfloat r = 0.0f, const GLfloat g = 0.0f, const GLfloat b = 0.0f): r(r), g(g), b(b) {}
    Color(const Color &color) {
        this->r = color.r;
        this->g = color.g;
        this->b = color.b;
    }
    Color operator * (const GLfloat times) {
        return Color(r * times, g * times, b * times);
    }
};

struct Point {
    int x;
    int y;
    int z;
    Point(int x = 0, int y = 0, int z = 0) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    Point(const Point &p) {
        this->x = p.x;
        this->y = p.y;
        this->z = p.z;
    }
};

struct Line {
    Point from;
    Point to;
    
    Line(Point from, Point to): from(from), to(to) {}
};

struct Plane {
    // Ax + By + Cz + D = 0
    float a;
    float b;
    float c;
    float d;
    
    Plane() {
        this->a = 0;
        this->b = 0;
        this->c = 0;
        this->d = 0;
    }
    
    Plane(const Point &p1, const Point &p2, const Point &p3) {
        float determinant = p1.x * (p2.y * p3.z - p3.y * p2.z) - p1.y * (p2.x * p3.z - p3.x * p2.z) + p1.z * (p2.x * p3.y - p3.x * p2.y);
        this->a = (p2.y - p1.y) * (p3.z - p1.z) - (p2.z - p1.z) * (p3.y - p1.y) / determinant;
        this->b = (p3.x - p1.x) * (p2.z - p1.z) - (p2.x - p1.x) * (p3.z - p1.z) / determinant;
        this->c = (p2.x - p1.x) * (p3.y - p1.y) - (p3.x - p1.x) * (p2.y - p1.y) / determinant;
        this->d = -(a * p1.x + b * p1.y + c * p1.z);
    }
    
    float zInPlane(float x, float y) {
        return -(d + b * y + a * x) / c;
    }
};

struct ActiveEdge {
    float currentX;
    float incrementX;
    int ymax;
    
    ActiveEdge(int x = 0, float deltaX = 0, int ymax = 0): currentX(x), incrementX(deltaX), ymax(ymax) {}
    
    bool operator<(const ActiveEdge &e) const {
        return this->currentX < e.currentX;
    }
};

struct Triangle {
    vector<Line> edges;
    Color color;
    Plane plane;
    int ymin;
    int ymax;
    static int numberOfTriangles;
    
    Triangle() {
        this->color.r = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
        this->color.g = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
        this->color.b = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
        this->ymin = INT_MAX;
        this->ymax = -1;
    }
    
    Triangle(const Point &p1, const Point &p2, const Point &p3): plane(p1, p2, p3) {
        this->color.r = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
        this->color.g = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
        this->color.b = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
        this->ymin = min({p1.y, p2.y, p3.y});
        this->ymax = max({p1.y, p2.y, p3.y});
        this->edges.emplace_back(Point(p1), Point(p2));
        this->edges.emplace_back(Point(p2), Point(p3));
        this->edges.emplace_back(Point(p3), Point(p1));
    }
    
    Triangle(const Point &p1, const Point &p2, const Point &p3, const Color &color): color(color), plane(p1, p2, p3) {
        this->ymin = min({p1.y, p2.y, p3.y});
        this->ymax = max({p1.y, p2.y, p3.y});
        this->edges.emplace_back(Point(p1), Point(p2));
        this->edges.emplace_back(Point(p2), Point(p3));
        this->edges.emplace_back(Point(p3), Point(p1));
    }
};

#endif /* z_buffering_hpp */
