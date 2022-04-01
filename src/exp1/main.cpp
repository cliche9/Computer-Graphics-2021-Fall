//
//  main.cpp
//  exp1
//
//  Created by abc_mac on 2021/9/18.
//

#define GL_SILENCE_DEPRECATION
#include <iostream>
#include <GLUT/glut.h>
#include <cmath>
#include <cstdlib>
using namespace std;

struct Point {
    int x;
    int y;
    Point(int x = 0, int y = 0) {
        this->x = x;
        this->y = y;
    }
};

enum {
    DDA, Bresenham, CIRCLE
} currentShape = DDA;
bool isDrawing = false;
Point lineFrom;
Point lineTo;
Point center;
float radius;

int SCR_WIDTH = 640, SCR_HERIGHT = 480;

static void initializer(void);
static void displayCallback(void);
static void menuCallback(int value);
static void mouseCallback(int button, int state, int x, int y);
static void mouseMotionCallback(int x, int y);
static void keyboardCallback(unsigned char key, int x, int y);
static void reshapeCallback(int width, int height);
// draw pixel
static void drawPixel(int x, int y);
// simple line
static void simpleLine();
// dda
static void dda();
// bresenham
static void bresenham();
// draw 8 points
static void draw8pixels(int x, int y);
// midPoint circle
static void midPointCircle();
// draw shape: line or circle
static void drawShape();

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    // 颜色: RGB显示, 刷新: 单缓存
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    // 窗口位置
    glutInitWindowPosition(SCR_WIDTH / 2, SCR_HERIGHT / 2);
    glutInitWindowSize(SCR_WIDTH, SCR_HERIGHT);
    // 创建窗口, 设置标题
    glutCreateWindow("exp1: Stroke and line");
    // 初始化
    initializer();
    // 画图展示函数
    glutDisplayFunc(displayCallback);
    glutCreateMenu(menuCallback);
    glutAddMenuEntry("DDA", 1);
    glutAddMenuEntry("Bresenham", 2);
    glutAddMenuEntry("Circle", 3);
    glutAddMenuEntry("Exit", 4);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(mouseMotionCallback);
    glutKeyboardFunc(keyboardCallback);
    glutReshapeFunc(reshapeCallback);
    
    glutMainLoop();
    
    return 0;
}

void initializer(void) {
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, SCR_WIDTH, 0, SCR_HERIGHT);
}

void displayCallback(void) {
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // draw shape
    glColor3f(1.0f, 0.0f, 0.0f);
    glPointSize(4.0f);
    glBegin(GL_POINTS);
    drawShape();
    glEnd();
    glFlush();
    // 交换缓存区
    glutSwapBuffers();
}

void menuCallback(int value) {
    switch (value) {
        case 1:
            currentShape = DDA;
            break;
        case 2:
            currentShape = Bresenham;
            break;
        case 3:
            currentShape = CIRCLE;
            break;
        default:
            exit(0);
    }
}

void mouseCallback(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (currentShape == DDA || currentShape == Bresenham) {
            if (state == GLUT_DOWN) {
                lineFrom.x = lineTo.x = x;
                lineFrom.y = lineTo.y = SCR_HERIGHT - y;
                cout << "Button_Down: [" << lineFrom.x << ", " << lineFrom.y << "]\n";
                isDrawing = true;
            } else if (state == GLUT_UP) {
                lineTo.x = x;
                lineTo.y = SCR_HERIGHT - y;
                cout << "Button_Up: [" << lineTo.x << ", " << lineTo.y << "]\n";
                isDrawing = false;
            }
        } else if (currentShape == CIRCLE) {
            if (state == GLUT_DOWN) {
                center.x = x;
                center.y = SCR_HERIGHT - y;
                radius = 0;
                cout << "Button_Down: Center = [" << center.x << ", " << center.y << "]\nRadius = " << radius << endl;
                isDrawing = true;
            } else if (state == GLUT_UP) {
                radius = sqrt(pow(x - center.x, 2) + pow(SCR_HERIGHT - y - center.y, 2));
                isDrawing = false;
            }
        }
        // 必须吗？
        glutPostRedisplay();
    }
}

void mouseMotionCallback(int x, int y) {
    if (isDrawing) {
        if (currentShape == DDA || currentShape == Bresenham) {
            lineTo.x = x;
            lineTo.y = SCR_HERIGHT - y;
        } else if (currentShape == CIRCLE) {
            radius = sqrt(pow(x - center.x, 2) + pow(SCR_HERIGHT - y - center.y, 2));
            cout << "Cursor = [" << x << ", " << SCR_HERIGHT - y << "]\nRadius = " << radius << endl;
        }
        glutPostRedisplay();
    }
}

void keyboardCallback(unsigned char key, int x, int y) {
    if (key == 'l')
        currentShape = Bresenham;
    else if (key == 'c')
        currentShape = CIRCLE;
}

void reshapeCallback(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width, height);
    gluOrtho2D(0, width, 0, height);
    
    SCR_WIDTH = width;
    SCR_HERIGHT = height;
}

void drawPixel(int x, int y) {
    glVertex2f(x, y);
}

void simpleLine() {
    int x = lineFrom.x;
    int y = (lineFrom.y > lineTo.y) ? lineTo.y : lineFrom.y;
    
    while (y <= lineTo.y) {
        drawPixel(x, y);
        y++;
    }
}

void dda() {
    if (lineFrom.x == lineTo.x) {
        simpleLine();
        return;
    }
    
    int numberOfPoints = 0;
    // 讨论斜率, 自增方向为增长慢的方向
    if (abs(lineTo.x - lineFrom.x) >= abs(lineTo.y - lineFrom.y))
        numberOfPoints = abs(lineTo.x - lineFrom.x) + 1;
    else
        numberOfPoints = abs(lineTo.y - lineFrom.y) + 1;
    
    float dx = (lineTo.x - lineFrom.x + 0.0f) / numberOfPoints;
    float dy = (lineTo.y - lineFrom.y + 0.0f) / numberOfPoints;
    float x = lineFrom.x;
    float y = lineFrom.y;
    
    for (int i = 0; i < numberOfPoints; i++) {
        drawPixel(x, y);
        x += dx;
        y += dy;
    }
}

void bresenham() {
    if (lineFrom.x == lineTo.x) {
        simpleLine();
        return;
    }
    
    Point from(lineFrom.x, lineFrom.y);
    Point to(lineTo.x, lineTo.y);
    int dx = to.x - from.x;
    int dy = to.y - from.y;
    
    // 讨论斜率, 自增方向为增长慢的方向
    // x自增, 判断斜率正负
    if (abs(dx) >= abs(dy)) {
        if (dx < 0) {
            swap(from, to);
        }
        int x = from.x;
        int y = from.y;
        dx = to.x - x;
        dy = to.y - y;
        if (dy < 0) {
            // 斜率为负
            int e = dx;
            while (x <= to.x) {
                drawPixel(x, y);
                x++;
                e += 2 * dy;
                if (e <= 0) {
                    y--;
                    e += 2 * dx;
                }
            }
        } else {
            // 斜率为正
            int e = -dx;
            while (x <= to.x) {
                drawPixel(x, y);
                x++;
                e += 2 * dy;
                if (e >= 0) {
                    y++;
                    e -= 2 * dx;
                }
            }
        }
    }
    // y自增, 判断斜率正负
    else {
        if (dy < 0)
            swap(from, to);
            
        int x = from.x;
        int y = from.y;
        dx = to.x - x;
        dy = to.y - y;
        if (dx < 0) {
            // 斜率为负
            int e = dy;
            while (y <= to.y) {
                drawPixel(x, y);
                y++;
                e += 2 * dx;
                if (e <= 0) {
                    x--;
                    e += 2 * dy;
                }
            }
        } else {
            // 斜率为正
            int e = -dy;
            while (y <= to.y) {
                drawPixel(x, y);
                y++;
                e += 2 * dx;
                if (e >= 0) {
                    x++;
                    e -= 2 * dy;
                }
            }
        }
    }
}

void draw8pixels(int x, int y) {
    int x_offset[] = {1, -1, 1, -1};
    int y_offset[] = {1, 1, -1, -1};
    
    for (int i = 0; i < 4; i++) {
        drawPixel(x_offset[i] * x + center.x, y_offset[i] * y + center.y);
        drawPixel(y_offset[i] * y + center.x, x_offset[i] * x + center.y);
    }
}

void midPointCircle() {
    if (radius == 0) {
        drawPixel(center.x, center.y);
        return;
    }
    int x = 0, y = radius;
    float d = 1.25 - radius;
    
    while (x <= y) {
        draw8pixels(x, y);
        if (d < 0)
            d += 2 * x + 3;
        else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

void drawShape() {
    if (currentShape == DDA)
        dda();
    else if (currentShape == Bresenham)
        bresenham();
    else if (currentShape == CIRCLE)
        midPointCircle();
}
