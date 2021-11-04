//
//  lab_3.hpp
//  lab_3
//
//  Created by abc_mac on 2021/10/22.
//

#ifndef lab_3_hpp
#define lab_3_hpp

#define GL_SILENCE_DEPRECATION
#include <iostream>
#include <GLUT/glut.h>
#include <cmath>
#include <cstdlib>
#include <vector>
using namespace std;

struct Color {
    GLfloat r;
    GLfloat g;
    GLfloat b;
    
    Color(const GLfloat r = 0.0f, const GLfloat g = 0.0f, const GLfloat b = 0.0f): r(r), g(g), b(b) {}
};

struct Point {
    int x;
    int y;
    Point(int x = 0, int y = 0) {
        this->x = x;
        this->y = y;
    }
    Point(const Point &p) {
        this->x = p.x;
        this->y = p.y;
    }
};

struct Line {
    Point from;
    Point to;
    
    Line(Point from, Point to): from(from), to(to) {}
    void draw() {
        glVertex2f(from.x, from.y);
        glVertex2f(to.x, to.y);
    }
};

struct Polygon {
    vector<Line> edges;
    Color color;
    int ymin;
    int ymax;
    static int numberOfPolygons;
        
    Polygon() {
        this->color.r = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
        this->color.g = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
        this->color.b = static_cast<GLfloat>(rand()) / static_cast<GLfloat>(RAND_MAX);
        this->ymin = INT_MAX;
        this->ymax = -1;
    }
    
    void drawPolygon() {
        glBegin(GL_LINE_STRIP);
        glColor3f(color.r, color.g, color.b);
        for (auto &line : edges) {
            line.draw();
        }
        glEnd();
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

#endif /* lab_3_hpp */
