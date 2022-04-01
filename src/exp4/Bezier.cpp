//
//  Bezier.cpp
//  exp4
//
//  Created by abc_mac on 2021/11/5.
//

#include "Bezier.hpp"
#include <iostream>
using namespace std;

enum {  // 当前绘制曲线类型
    CLEAR, BEZIER, BSPLINE
} currentCurve = CLEAR;

enum {  // 当前对控制点的操作类型
    IDLE, INSERT, MOVE, REMOVE
} currentStatus = IDLE;

// global variables
vector<Point> controlPoints;
auto movingPoint = controlPoints.end();
auto inRangePoint = controlPoints.end();
/*
int indexOfMovingPoint = -1;
int indexOfInRangePoint = -1;
*/
Color colorOfBezierLine, colorOfBspline, colorOfControlLine;

static void initializer(void);
static void displayCallback(void);
static void menuCallback(int value);
static void mouseCallback(int button, int state, int x, int y);
static void mouseMotionCallback(int x, int y);
static void mousePassiveMotionCallback(int x, int y);
static void keyboardCallback(unsigned char key, int x, int y);
static void reshapeCallback(int width, int height);
// deCasteljau
static void deCasteljau();
// deBoor
static void deBoor();
// draw one pixel
static void drawPixel(GLfloat x, GLfloat y);
// 绘制控制点和线
static void drawControlLine();

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    // 颜色: RGB显示, 刷新: 双缓存
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    // 窗口位置
    glutInitWindowPosition(SCR_WIDTH / 2, SCR_HEIGHT / 2);
    glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
    // 创建窗口, 设置标题
    glutCreateWindow("exp3: Scan And Fill");
    // 初始化
    initializer();

    glutCreateMenu(menuCallback);
    glutAddMenuEntry("Beizer", 1);
    glutAddMenuEntry("B-spline", 2);
    glutAddMenuEntry("Insert", 3);
    glutAddMenuEntry("Move", 4);
    glutAddMenuEntry("Remove", 5);
    glutAddMenuEntry("ClearScreen", 6);
    glutAddMenuEntry("Exit", 7);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(mouseMotionCallback);
    glutPassiveMotionFunc(mousePassiveMotionCallback);
    glutKeyboardFunc(keyboardCallback);
    // 画图展示函数
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    
    glutMainLoop();
    
    return 0;
}

void initializer(void) {
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glPointSize(3.0f);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, SCR_WIDTH, 0, SCR_HEIGHT);
}

void displayCallback(void) {
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glPointSize(3.0f);
    glLineWidth(3.0f);
    // 绘制控制点和对应的连线
    drawControlLine();
    // 绘制对应的曲线
    if (currentCurve == BEZIER)
        deCasteljau();
    else if (currentCurve == BSPLINE)
        deBoor();
    // 交换缓冲区
    glutSwapBuffers();
}

void menuCallback(int value) {
    switch (value) {
        case 1: {
            currentCurve = BEZIER;
            currentStatus = IDLE;
            break;
        }
        case 2: {
            currentCurve = BSPLINE;
            currentStatus = IDLE;
            break;
        }
        case 3:
            currentStatus = INSERT;
            break;
        case 4:
            currentStatus = MOVE;
            break;
        case 5:
            currentStatus = REMOVE;
            break;
        case 6: {
            glClear(GL_COLOR_BUFFER_BIT);
            glutSwapBuffers();
            currentCurve = CLEAR;
            currentStatus = IDLE;
            controlPoints.clear();
            movingPoint = controlPoints.end();
            inRangePoint = controlPoints.end();
            break;
        }
        default:
            exit(0);
    }
}

void mouseCallback(int button, int state, int x, int y) {
    y = SCR_HEIGHT - y;
    
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            switch (currentStatus) {
                case INSERT: {
                    controlPoints.emplace_back(x, y);
                    cout << "插入控制点: [" << x << ", " << y << "]\n";
                    glutPostRedisplay();
                    break;
                }
                case MOVE: {
                    if (inRangePoint != controlPoints.end()) {
                        movingPoint = inRangePoint;
                    }
                    /*
                    if (indexOfInRangePoint != -1) {
                        indexOfMovingPoint = indexOfInRangePoint;
                    }
                     */
                    break;
                }
                case REMOVE: {
                    if (inRangePoint != controlPoints.end()) {
                        cout << "删除控制点: [" << inRangePoint->x << ", " << inRangePoint->y << "]\n";
                        controlPoints.erase(inRangePoint);
                        inRangePoint = controlPoints.end();
                        glutPostRedisplay();
                    }
                    /*
                    if (indexOfInRangePoint != -1) {
                        cout << "删除控制点: [" << controlPoints[indexOfInRangePoint].x << ", " << controlPoints[indexOfInRangePoint].y << "]\n";
                        controlPoints.erase(controlPoints.begin() + indexOfInRangePoint);
                        indexOfInRangePoint = -1;
                    }
                     */
                    break;
                }
                default:
                    break;
            }
        } else if (state == GLUT_UP && currentStatus == MOVE) {
            movingPoint = controlPoints.end();
        }
    }
    cout << "Button_Left = [" << x << ", " << y << "]\n";
    glutPostRedisplay();
}

void mouseMotionCallback(int x, int y) {
    y = SCR_HEIGHT - y;
    
    if (currentStatus == MOVE) {
        if (movingPoint != controlPoints.end()) {
            movingPoint->moveTo(x, y);
            glutPostRedisplay();
        }
    }
}

void mousePassiveMotionCallback(int x, int y) {
    y = SCR_HEIGHT - y;
    
    bool handMouseOn = false;
    if (currentStatus == MOVE || currentStatus == REMOVE) {
        // MOVE/REMOVE时, 鼠标接近某个控制点时, 自动变为手指
        for (auto iter = controlPoints.begin(); iter != controlPoints.end(); ++iter) {
            if (iter->isInControlRange(x, y)) {
                handMouseOn = true;
                inRangePoint = iter;
                break;
            }
        }
        if (handMouseOn)
            glutSetCursor(GLUT_CURSOR_INFO);
        else
            glutSetCursor(GLUT_CURSOR_INHERIT);
    }
}

void keyboardCallback(unsigned char key, int x, int y) {
    switch (key) {
        case '0': {
            currentCurve = BEZIER;
            currentStatus = IDLE;
            break;
        }
        case '1': {
            currentCurve = BSPLINE;
            currentStatus = IDLE;
            break;
        }
        case 'i':
            currentStatus = INSERT;
            break;
        case 'm':
            currentStatus = MOVE;
            break;
        case 'r':
            currentStatus = REMOVE;
            break;
        case 'c': {
            glClear(GL_COLOR_BUFFER_BIT);
            glutSwapBuffers();
            currentCurve = CLEAR;
            currentStatus = IDLE;
            break;
        }
        case GLUT_ESCAPE:
            exit(0);
    }
}

void reshapeCallback(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width, height);
    gluOrtho2D(0, width, 0, height);
}

void deCasteljau() {
    if (controlPoints.size() < 2)
        return;
    
    vector<Point> currentPoints(controlPoints);
    vector<Point> bezierPoints;
    unsigned long n = controlPoints.size();
    
    for (float t = 0.0f; t <= 1.0f; t += 0.005f / n) {
        for (int i = 1; i < n; i++) {
            // 递推, 逐渐缩小控制点规模, 每走一轮，控制点个数 - 1
            for (int j = 0; j < n - i; j++) {
                // 在当前控制点规模下绘制曲线
                if (i == 1) {
                    // 第一轮必须是最初的控制点, 需要用到controlPoints
                    currentPoints[j].x = (1 - t) * controlPoints[j].x + t * controlPoints[j + 1].x;
                    currentPoints[j].y = (1 - t) * controlPoints[j].y + t * controlPoints[j + 1].y;
                    continue;
                } else {
                    // 递推, 每次都用上一轮的点更新当前点, 直到推到贝塞尔曲线上的点
                    currentPoints[j].x = (1 - t) * currentPoints[j].x + t * currentPoints[j + 1].x;
                    currentPoints[j].y = (1 - t) * currentPoints[j].y + t * currentPoints[j + 1].y;
                }
            }
        }
        // cout << "控制点数量=" << controlpoint.size() << endl;一直是点的数目，因为之前直接存在这个里面了，但是有用的只有下标为1的才有用
        // 递推完成, currentPoints[0]中存放的是贝塞尔曲线上的点
        bezierPoints.emplace_back(currentPoints[0]);
    }
    
    glBegin(GL_LINE_STRIP);
    glColor3f(colorOfBezierLine.r, colorOfBezierLine.g, colorOfBezierLine.b);
    for (auto &p : bezierPoints)
        drawPixel(p.x, p.y);
    glEnd();
}

void deBoor() {
    vector<Point> currentPoints(controlPoints);
    vector<Point> deBoorPoints;
    int n = (int)controlPoints.size() - 1;
    int p = 3;
    float t[100000];
    
    // 准均匀B样条
    for (int i = 0; i < p; i++)
        t[i] = 0;
    for (int i = p; i < n + 1; i++)
        t[i] = t[i - 1] + 1.0 / (n + 1 - p + 1);
    for (int i = n + 1; i <= n + p; i++)
        t[i] = 1;
    
    for (int j = p - 1; j <= n; j++) {
        for (float u = t[j]; u <= t[j + 1]; u += 0.001 / n) {
            for (int r = 1; r <= p - 1; r++) {
                for (int i = j; i >= j - p + r + 1; i--) {
                    float x1 = u - t[i];
                    float x2 = t[i + p - r] - t[i];
                    float y1 = t[i + p - r] - u;
                    float y2 = t[i + p - r] - t[i];
                    
                    float c1, c2;
                    if (x1 == 0.0f && x2 == 0.0f)
                        c1 = 0;
                    else
                        c1 = x1 / x2;
                    if (y1 == 0.0f && y2 == 0.0f)
                        c2 = 0;
                    else
                        c2 = y1 / y2;
                    
                    if (r == 1) {
                        currentPoints[i].x = controlPoints[i].x * c1 + controlPoints[i - 1].x * c2;
                        currentPoints[i].y = controlPoints[i].y * c1 + controlPoints[i - 1].y * c2;
                        continue;
                    } else {
                        currentPoints[i].x = currentPoints[i].x * c1 + currentPoints[i - 1].x * c2;
                        currentPoints[i].y = currentPoints[i].y * c1 + currentPoints[i - 1].y * c2;
                    }
                }
            }
            deBoorPoints.emplace_back(currentPoints[j]);
        }
    }
    
    glBegin(GL_LINE_STRIP);
    glColor3f(colorOfBspline.r, colorOfBspline.g, colorOfBspline.b);
    for (auto &p : deBoorPoints)
        drawPixel(p.x, p.y);
    glEnd();
}

void drawPixel(GLfloat x, GLfloat y) {
    glVertex2f(x, y);
}

void drawControlLine() {
    glBegin(GL_LINE_STRIP);
    glColor3f(colorOfControlLine.r, colorOfControlLine.g, colorOfControlLine.b);
    for (auto &p : controlPoints)
        drawPixel(p.x, p.y);
    glEnd();
}
