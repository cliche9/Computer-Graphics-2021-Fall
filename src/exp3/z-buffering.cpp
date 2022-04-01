//
//  z-buffering.cpp
//  Z-buffering
//
//  Created by abc_mac on 2021/10/29.
//
#define GL_SILENCE_DEPRECATION
#include "z-buffering.hpp"
#include <iostream>
#include <list>
using namespace std;

// global variables
float zBufferFrame[SCR_WIDTH][SCR_HEIGHT];
vector<Triangle> triangles;
int Triangle::numberOfTriangles = 0;
vector<list<ActiveEdge> > newEdgeTable;
list<ActiveEdge> activeEdgeTable;

static void initializer(void);
static void displayCallback(void);
static void menuCallback(int value);
// draw pixels
static void drawPixel(int x, int y);
static void drawPixel(int x, int y, const Color &color);
static void initNewEdgeTable(const Triangle &triangle);
static void processOneLine(const int y, Triangle &triangle);
// check point left line
static bool isLeftLine(const Line &line, GLfloat x, GLfloat y);
// check point in triangle
static bool isInTriangle(const vector<Line> &edges, GLfloat x, GLfloat y);
// z-buffering
static void zBuffer(void);
// anti-aliasing
static void antiAliasing(void);

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    // 颜色: RGB显示, 刷新: 双缓存
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    // 窗口位置
    glutInitWindowPosition(SCR_WIDTH / 2, SCR_HEIGHT / 2);
    glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
    // 创建窗口, 设置标题
    glutCreateWindow("exp3: Z-Buffering");
    // 初始化
    initializer();

    glutCreateMenu(menuCallback);
    glutAddMenuEntry("Drawtriangle", 1);
    glutAddMenuEntry("Antialiasing", 2);
    glutAddMenuEntry("ClearScreen", 3);
    glutAddMenuEntry("Exit", 4);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    // 画图展示函数
    glutDisplayFunc(displayCallback);
    
    glutMainLoop();
    
    return 0;
}

void initializer(void) {
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glPointSize(3.0f);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(0, SCR_WIDTH, 0, SCR_HEIGHT);
    // 初始化zbufer
    for (int i = 0; i < SCR_WIDTH; i++)
        for (int j = 0; j < SCR_HEIGHT; j++)
            zBufferFrame[i][j] = INT_MIN;
}

void displayCallback(void) {
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLineWidth(2.5f);
}

void menuCallback(int value) {
    switch (value) {
        case 1:
            glutPostRedisplay();
            zBuffer();
            glutSwapBuffers();
            break;
        case 2:
            glutPostRedisplay();
            antiAliasing();
            glutSwapBuffers();
            break;
        case 3: {
            glutPostRedisplay();
            glutSwapBuffers();
            for (int i = 0; i < SCR_WIDTH; i++)
                for (int j = 0; j < SCR_HEIGHT; j++)
                    zBufferFrame[i][j] = INT_MIN;
            break;
        }
        default:
            exit(0);
    }
}

void drawPixel(int x, int y) {
    glVertex2f(x, y);
}

void drawPixel(int x, int y, const Color &color) {
    glColor3f(1 - color.r, 1 - color.g, 1 - color.b);
    glVertex2f(x, y);
}

void initNewEdgeTable(const Triangle &triangle) {
    newEdgeTable.clear();
    newEdgeTable.resize(triangle.ymax - triangle.ymin + 1);
    float deltaX = 0;
    
    for (auto &edge : triangle.edges) {
        if (edge.from.y == edge.to.y)
            continue;
        deltaX = (edge.from.x - edge.to.x + 0.0) / (edge.from.y - edge.to.y);
        if (edge.from.y < edge.to.y)
            newEdgeTable[edge.from.y - triangle.ymin].emplace_back(edge.from.x, deltaX, edge.to.y);
        else
            newEdgeTable[edge.to.y - triangle.ymin].emplace_back(edge.to.x, deltaX, edge.from.y);
    }
}

void processOneLine(const int y, Triangle &triangle) {
    auto front = activeEdgeTable.begin(), from = front, to = front;
    ++to;
    vector<list<ActiveEdge>::iterator> readyToRemove;
    
    while (to != activeEdgeTable.end()) {
        for (int x = from->currentX; x < to->currentX; x++) {
            if (zBufferFrame[x][y] < triangle.plane.zInPlane(x, y)) {
                drawPixel(x, y);
                zBufferFrame[x][y] = triangle.plane.zInPlane(x, y);
            }
        }
        if (to->ymax == y)
            readyToRemove.push_back(to);
        else
            to->currentX += to->incrementX;
        ++from;
        ++to;
    }
    // 删除/更新front()
    if (front->ymax == y)
        activeEdgeTable.pop_front();
    else
        front->currentX += front->incrementX;
    // 删除使用完的边
    for (auto &iter : readyToRemove)
        activeEdgeTable.erase(iter);
}

bool isLeftLine(const Line &edge, GLfloat x, GLfloat y) {
    return (edge.to.x - edge.from.x) * (y - edge.from.y) > (edge.to.y - edge.from.y) * (x - edge.from.x);
}

bool isInTriangle(const vector<Line> &edges, GLfloat x, GLfloat y) {
    bool res = true;
    for (auto &edge : edges)
        res = res && isLeftLine(edge, x, y);
    return res;
}

void zBuffer() {
    // 初始化三角形
    /*
    triangles.emplace_back(Point(120, 180, 100), Point(120, 60, 100), Point(600, 120, 300), Color(1, 0, 0));
    triangles.emplace_back(Point(600, 180, 100), Point(600, 60, 100), Point(120, 120, 300), Color(1, 0, 0));
    */
    triangles.emplace_back(Point(120, 180, 100), Point(60, 340, 100), Point(600, 400, 800), Color(1, 0, 0));
    triangles.emplace_back(Point(100, 130, 100), Point(30, 300, 400), Point(420, 460, 700), Color(0, 1, 0));
    triangles.emplace_back(Point(450, 220, 600), Point(0, 360, 200), Point(420, 460, 100), Color(0, 0, 1));
    Triangle::numberOfTriangles = 3;
    
    for (auto &triangle : triangles) {
        // 获取多边形的ymin和ymax
        // 初始化新边表
        initNewEdgeTable(triangle);
        activeEdgeTable.clear();
        
        glBegin(GL_POINTS);
        glColor3f(triangle.color.r, triangle.color.g, triangle.color.b);
        for (int i = triangle.ymin; i <= triangle.ymax; i++) {
            for (auto &edge : newEdgeTable[i - triangle.ymin])
                activeEdgeTable.push_back(edge);
            // 考虑自相交, sort使其有序
            activeEdgeTable.sort();
            // 处理一行扫描, 更新AET
            processOneLine(i, triangle);
        }
        glEnd();
    }
    
    triangles.clear();
    Triangle::numberOfTriangles = 0;
}

void antiAliasing(){
    const int subsize = 3 * 3;
    pair<float, float> offsets[subsize];
    offsets[0] = make_pair(-0.33, 0.33);
    offsets[1] = make_pair(0.0, 0.33);
    offsets[2] = make_pair(0.33,0.33);
    offsets[3] = make_pair(-0.33, 0);
    offsets[4] = make_pair(0.0,0.0);
    offsets[5] = make_pair(0.33, 0.0);
    offsets[6] = make_pair(-0.33, -0.33);
    offsets[7] = make_pair(0.0, -0.33);
    offsets[8] = make_pair(0.33, -0.33);
    // 初始化三角形
    vector<Line> edges;
    edges.emplace_back(Point(120, 180), Point(600, 400));
    edges.emplace_back(Point(600, 400), Point(60, 340));
    edges.emplace_back(Point(60, 340), Point(120, 180));
    Color color(0, 1, 1);
    int xLeft = SCR_WIDTH, xRight = 0;
    int yBottom = SCR_HEIGHT, yTop = 0;

    glBegin(GL_TRIANGLES);
    for (auto& edge : edges) {
        glVertex2f(edge.from.x, edge.from.y);
        xLeft = min(edge.from.x, xLeft);
        xRight = max(edge.from.x, xRight);
        yBottom = min(edge.from.y, yBottom);
        yTop = max(edge.from.y, yTop);
    }
    glEnd();

    glBegin(GL_POINTS);
    for (int x = xLeft; x <= xRight; x++) {
        for (int y = yBottom; y <= yTop; y++) {
            int inCount = 0;
            for (const auto& offset : offsets) {
                if (isInTriangle(edges, x + offset.first, y + offset.second))
                    inCount++;
                Color color_now = color * ((inCount + 0.0) / subsize);
                drawPixel(x, y, color_now);
            }
        }
    }
    glEnd();
}
