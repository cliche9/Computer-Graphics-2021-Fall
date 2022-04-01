//
//  exp3.cpp
//  exp3
//
//  Created by abc_mac on 2021/10/22.
//

#include <list>
#include "exp3.hpp"
using namespace std;

int SCR_WIDTH = 640, SCR_HEIGHT = 480;
int Polygon::numberOfPolygons = 0;
int edgeNumber = 0;
int polyNumber = 0;

enum {
    IDLE, DRAW, SCAN, ZBUFFER
} currentStatus = IDLE;

// global variables
vector<Polygon> polygons;
bool isTracing = false;
vector<list<ActiveEdge> > newEdgeTable;
list<ActiveEdge> activeEdgeTable;


static void initializer(void);
static void displayCallback(void);
static void menuCallback(int value);
static void mouseCallback(int button, int state, int x, int y);
static void mouseMotionCallback(int x, int y);
static void keyboardCallback(unsigned char key, int x, int y);
static void reshapeCallback(int width, int height);
// draw polygons
static void drawAllPolygons();
// scan and fill
static void scanAndFill();
// init NET
static void initNewEdgeTable(const Polygon &polygon);
// fill one line
static void processOneLine(const int y, const Color &color);
// draw one pixel
static void drawPixel(int x, int y, const Color &color);

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    // 颜色: RGB显示, 刷新: 单缓存
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    // 窗口位置
    glutInitWindowPosition(SCR_WIDTH / 2, SCR_HEIGHT / 2);
    glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
    // 创建窗口, 设置标题
    glutCreateWindow("exp3: Scan And Fill");
    // 初始化
    initializer();

    glutCreateMenu(menuCallback);
    glutAddMenuEntry("DrawPolygon", 1);
    glutAddMenuEntry("ScanAndFill", 2);
    glutAddMenuEntry("ClearScreen", 3);
    glutAddMenuEntry("Exit", 4);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glutMouseFunc(mouseCallback);
    glutPassiveMotionFunc(mouseMotionCallback);
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
    glLineWidth(2.5f);
    // 绘制所有的多边形
    drawAllPolygons();
    if (currentStatus == SCAN)
        scanAndFill();
}

void menuCallback(int value) {
    switch (value) {
        case 1:
            currentStatus = DRAW;
            break;
        case 2:
            currentStatus = SCAN;
            break;
        case 3: {
            glClear(GL_COLOR_BUFFER_BIT);
            glFlush();
            polygons.clear();
            polyNumber = 0;
            edgeNumber = 0;
            currentStatus = IDLE;
            isTracing = false;
            break;
        }
        default:
            exit(0);
    }
}

void mouseCallback(int button, int state, int x, int y) {
    y = SCR_HEIGHT - y;
    
    if (currentStatus == DRAW) {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            if (isTracing == false)
                polygons.emplace_back(Polygon());
            isTracing = true;
            polygons[polyNumber].ymin = min(polygons[polyNumber].ymin, y);
            polygons[polyNumber].ymax = max(polygons[polyNumber].ymax, y);
            cout << "Button_Left_Down = [" << x << ", " << y << "]\n";
            if (polygons[polyNumber].edges.empty()) {
                polygons[polyNumber].edges.emplace_back(Point(x, y), Point(x, y));
            }
            else {
                float distance = sqrt(pow(polygons[polyNumber].edges[0].from.x - x, 2) + pow(polygons[polyNumber].edges[0].from.y - y, 2));
                if (distance < 4) {
                    // 结束绘制
                    polygons[polyNumber].edges[edgeNumber].to = polygons[polyNumber].edges[0].from;
                    polyNumber++;
                    Polygon::numberOfPolygons++;
                    edgeNumber = 0;
                    currentStatus = IDLE;
                    isTracing = false;
                } else {
                    // 前一条边to连到下一条边from
                    polygons[polyNumber].edges[edgeNumber].to = Point(x, y);
                    polygons[polyNumber].edges.emplace_back(Point(x, y), Point(x, y));
                    edgeNumber++;
                }
            }
        }
    } else if (currentStatus == SCAN) {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
            scanAndFill();
        currentStatus = IDLE;
    }
}

void mouseMotionCallback(int x, int y) {
    y = SCR_HEIGHT - y;
    
    if (currentStatus == DRAW && isTracing) {
        cout << "Button_Move = [" << x << ", " << y << "]\n";
        polygons[polyNumber].edges[edgeNumber].to.x = x;
        polygons[polyNumber].edges[edgeNumber].to.y = y;
        glutPostRedisplay();
    }
}

void keyboardCallback(unsigned char key, int x, int y) {
    if (key == 'i')
        currentStatus = IDLE;
    else if (key == 'd')
        currentStatus = DRAW;
    else if (key == 's')
        currentStatus = SCAN;
}

void reshapeCallback(int width, int height) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, width, height);
    gluOrtho2D(0, width, 0, height);
    
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

void drawAllPolygons() {
    for (auto &polygon : polygons)
        polygon.drawPolygon();
    glFlush();
}

void scanAndFill() {
    // 对每个多边形单独处理
    for (auto &polygon : polygons) {
        // 获取多边形的ymin和ymax
        // 初始化新边表
        initNewEdgeTable(polygon);
        activeEdgeTable.clear();
        
        for (int i = polygon.ymin; i <= polygon.ymax; i++) {
            for (auto &edge : newEdgeTable[i - polygon.ymin])
                activeEdgeTable.push_back(edge);
            // 考虑自相交, sort使其有序
            activeEdgeTable.sort();
            // 处理一行扫描, 更新AET
            processOneLine(i, polygon.color);
        }
    }
}

void initNewEdgeTable(const Polygon &polygon) {
    newEdgeTable.clear();
    newEdgeTable.resize(polygon.ymax - polygon.ymin + 1);
    float deltaX = 0;
    
    for (auto &edge : polygon.edges) {
        if (edge.from.y == edge.to.y)
            continue;
        deltaX = (edge.from.x - edge.to.x + 0.0) / (edge.from.y - edge.to.y);
        if (edge.from.y < edge.to.y)
            newEdgeTable[edge.from.y - polygon.ymin].emplace_back(edge.from.x, deltaX, edge.to.y);
        else
            newEdgeTable[edge.to.y - polygon.ymin].emplace_back(edge.to.x, deltaX, edge.from.y);
    }
}

void processOneLine(const int y, const Color &color) {
    auto front = activeEdgeTable.begin(), from = front, to = front;
    ++to;
    vector<list<ActiveEdge>::iterator> readyToRemove;
    
    while (to != activeEdgeTable.end()) {
        for (int x = from->currentX; x < to->currentX; x++) {
            drawPixel(x, y, color);
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

void drawPixel(int x, int y, const Color &color) {
    glBegin(GL_POINTS);
    glColor3f(color.r, color.g, color.b);
    glVertex2f(x, y);
    glEnd();
    glFlush();
}
