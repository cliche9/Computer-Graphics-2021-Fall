//
//  main.cpp
//  lab_2
//
//  Created by abc_mac on 2021/10/2.
//

#define GL_SILENCE_DEPRECATION
#include <iostream>
#include <GLUT/glut.h>
#include <cmath>
#include <cstdlib>
#include <vector>
using namespace std;

int SCR_WIDTH = 640, SCR_HERIGHT = 480;

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
    float u1;
    float u2;
    bool outOfSquare;
    
    Line(Point from, Point to): from(from), to(to), u1(0), u2(1), outOfSquare(true) {}
    static unsigned int numberOfLines;
    static void drawAll();
};

struct Rectangle {
    Point leftUp;
    Point rightDown;
    
    Rectangle() {}
    Rectangle(Point lu, Point rd): leftUp(lu), rightDown(rd) {}
};

enum {
    LINE, WINDOW
} currentShape = LINE;


bool isClipping = false;
bool isDrawing = false;
unsigned int Line::numberOfLines = 0;
vector<Line> lines;
Rectangle window;

static void initializer(void);
static void displayCallback(void);
static void menuCallback(int value);
static void mouseCallback(int button, int state, int x, int y);
static void mouseMotionCallback(int x, int y);
static void keyboardCallback(unsigned char key, int x, int y);
static void reshapeCallback(int width, int height);
// draw shape: line or window
static void drawWindow();
// Liang-Barskey
static void LB_lineClip();

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    // 颜色: RGB显示, 刷新: 单缓存
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    // 窗口位置
    glutInitWindowPosition(SCR_WIDTH / 2, SCR_HERIGHT / 2);
    glutInitWindowSize(SCR_WIDTH, SCR_HERIGHT);
    // 创建窗口, 设置标题
    glutCreateWindow("Lab_2: Liang-Barsky clipping");
    // 初始化
    initializer();
    // 画图展示函数
    glutDisplayFunc(displayCallback);
    glutCreateMenu(menuCallback);
    glutAddMenuEntry("DrawLine", 1);
    glutAddMenuEntry("DrawRectangle", 2);
    glutAddMenuEntry("ClearScreen", 3);
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
    glPointSize(3.0f);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, SCR_WIDTH, 0, SCR_HERIGHT);
}

void displayCallback(void) {
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLineWidth(2.5f);
    // draw shape
    if (!isDrawing)
        LB_lineClip();
    Line::drawAll();
    drawWindow();
    // 交换缓存区
    glutSwapBuffers();
}

void menuCallback(int value) {
    switch (value) {
        case 1:
            currentShape = LINE;
            break;
        case 2:
            currentShape = WINDOW;
            break;
        case 3: {
            glClear(GL_COLOR_BUFFER_BIT);
            glutSwapBuffers();
            lines.clear();
            Line::numberOfLines = 0;
            window.leftUp.x = window.rightDown.x = window.leftUp.y = window.rightDown.y = 0;
            isClipping = false;
            break;
        }
        default:
            exit(0);
    }
}

void mouseCallback(int button, int state, int x, int y) {
    y = SCR_HERIGHT - y;
    if (button == GLUT_LEFT_BUTTON) {
        if (currentShape == LINE) {
            ++Line::numberOfLines;
            Line curLine(Point(x, y), Point(x, y));
            lines.emplace_back(curLine);
            if (state == GLUT_DOWN) {
                cout << "Button_Down: from = [" << x << ", " << y << "]\n";
                isDrawing = true;
            } else if (state == GLUT_UP) {
                // 画线结束, 判断是否交换from和to, 保证from.x <= to.x
                if (lines[Line::numberOfLines - 1].from.x > x) {
                    lines[Line::numberOfLines - 1].to = lines[Line::numberOfLines - 1].from;
                    lines[Line::numberOfLines - 1].from.x = x;
                    lines[Line::numberOfLines - 1].from.y = y;
                } else {
                    lines[Line::numberOfLines - 1].to.x = x;
                    lines[Line::numberOfLines - 1].to.y = y;
                }
                cout << "Button_Up: to = [" << x << ", " << y << "]\n";
                isDrawing = false;
            }
        } else if (currentShape == WINDOW) {
            if (state == GLUT_DOWN) {
                window.leftUp.x = window.rightDown.x = x;
                window.leftUp.y = window.rightDown.x = y;
                cout << "Button_Down: LeftUp = [" << x << ", " << y << "]\n";
                isDrawing = true;
            } else if (state == GLUT_UP) {
                // 画框结束, 保证leftUp.x <= rightDown.x && leftUp.y >= rightDown.y
                window.rightDown.x = x;
                window.rightDown.y = y;
                if (window.leftUp.x > window.rightDown.x)
                    swap(window.leftUp.x, window.rightDown.x);
                if (window.leftUp.y < window.rightDown.y)
                    swap(window.leftUp.y, window.rightDown.y);
                
                cout << "Button_Up: RightDown = [" << x << ", " << y << "]\n";
                isDrawing = false;
                isClipping = true;
            }
        }
        // 必须吗？
        glutPostRedisplay();
    }
}

void mouseMotionCallback(int x, int y) {
    y = SCR_HERIGHT - y;
    if (isDrawing) {
        if (currentShape == LINE) {
            lines[Line::numberOfLines - 1].to.x = x;
            lines[Line::numberOfLines - 1].to.y = y;
        } else if (currentShape == WINDOW) {
            window.rightDown.x = x;
            window.rightDown.y = y;
        }
        glutPostRedisplay();
    }
}

void keyboardCallback(unsigned char key, int x, int y) {
    if (key == 'w')
        currentShape = WINDOW;
    else if (key == 'l')
        currentShape = LINE;
}

void reshapeCallback(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width, height);
    gluOrtho2D(0, width, 0, height);
    
    SCR_WIDTH = width;
    SCR_HERIGHT = height;
}

void drawWindow() {
    glColor3f(1.0f, 0.0f, 0.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glRectf(window.leftUp.x, window.leftUp.y, window.rightDown.x, window.rightDown.y);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
}

void Line::drawAll() {
    glBegin(GL_LINES);
    for (auto const &line : lines) {
        int deltaX = line.to.x - line.from.x, deltaY = line.to.y - line.from.y;
        float u1 = line.u1, u2 = line.u2;
        if (!isClipping || line.outOfSquare) {
            glColor3f(0.0f, 0.0f, 0.8f);
            glVertex2f(line.from.x, line.from.y);
            glVertex2f(line.to.x, line.to.y);
        } else {
            glColor3f(0.0f, 0.0f, 0.8f);
            glVertex2f(line.from.x, line.from.y);
            glVertex2f(line.from.x + u1 * deltaX, line.from.y + u1 * deltaY);
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex2f(line.from.x + u1 * deltaX, line.from.y + u1 * deltaY);
            glVertex2f(line.from.x + u2 * deltaX, line.from.y + u2 * deltaY);
            glColor3f(0.0f, 0.0f, 0.8f);
            glVertex2f(line.from.x + u2 * deltaX, line.from.y + u2 * deltaY);
            glVertex2f(line.to.x, line.to.y);
        }
    }
    glEnd();
}

void LB_lineClip() {
    float xl = window.leftUp.x, xr = window.rightDown.x;
    float yb = window.rightDown.y, yt = window.leftUp.y;
    
    for (auto &line : lines) {
        line.outOfSquare = false;
        float u1 = 0, u2 = 1;
        float deltaX = line.to.x - line.from.x, deltaY = line.to.y - line.from.y;
        float p[] = {-deltaX, deltaX, -deltaY, deltaY};
        float q[] = {line.from.x - xl, xr - line.from.x, line.from.y - yb, yt - line.from.y};
        // u * p[k] <= q[k]
        for (int i = 0; i < 4; i++) {
            if (p[i] < 0)
                u1 = max(u1, q[i] / p[i]);
            else if (p[i] > 0)
                u2 = min(u2, q[i] / p[i]);
            else if (p[i] == 0 && q[i] < 0) {
                // 线段在框外
                line.outOfSquare = true;
                break;
            }
        }
        if (u1 > u2)
            // 线段在框外
            line.outOfSquare = true;
        else {
            line.u1 = u1;
            line.u2 = u2;
        }
    }
}
