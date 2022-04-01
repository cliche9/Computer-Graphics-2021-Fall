//
//  Bezier.hpp
//  exp4
//
//  Created by abc_mac on 2021/11/5.
//

#ifndef Bezier_hpp
#define Bezier_hpp

#define GL_SILENCE_DEPRECATION
#define GLUT_ESCAPE 27
#define SCR_WIDTH 800
#define SCR_HEIGHT 600
#include <GLUT/glut.h>
#include <vector>
#include <cmath>
using namespace std;

struct Color {
    GLfloat r;
    GLfloat g;
    GLfloat b;
    
    Color() {
        this->r = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
        this->g = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
        this->b = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
    }
    Color(const GLfloat r, const GLfloat g, const GLfloat b): r(r), g(g), b(b) {}
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
    float x;
    float y;
    float z;
    float controlRadius;
    Point(float x = 0, float y = 0, float z = 0) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->controlRadius = 7.0;
    }
    Point(const Point &p) {
        this->x = p.x;
        this->y = p.y;
        this->z = p.z;
        this->controlRadius = p.controlRadius;
    }
    
    bool isInControlRange(float x, float y) {
        return sqrt(pow(x - this->x, 2) + pow(y - this->y, 2)) < this->controlRadius;
    }
    
    void moveTo(float x, float y) {
        this->x = x;
        this->y = y;
    }
};


#endif /* Bezier_hpp */
