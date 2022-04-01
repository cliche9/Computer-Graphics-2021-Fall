//
//  exp5.cpp
//  exp5
//
//  Created by abc_mac on 2021/11/19.
//

#include "exp5.hpp"
#include "arcball.hpp"

const string modelPath = "/Users/abc_mac/Code/CG_2021Fall/exp5/exp5/models/";
const string modelFiles[] = {"pyramid.off", "cube.off", "bunny.off", "bumpy.off"};
Model model;
bool isColorified = false;
ArcBallT arcBall(SCR_WIDTH, SCR_HEIGHT);
ArcBallT *ArcBall = &arcBall;

static void initializer(void);
static void displayCallback(void);
static void menuCallback(int value);
static void mouseCallback(int button, int state, int x, int y);
static void mouseMotionCallback(int x, int y);
static void keyboardCallback(unsigned char key, int x, int y);
static void reshapeCallback(int width, int height);

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    // 颜色: RGB显示, 刷新: 双缓存
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    // 窗口位置
    glutInitWindowPosition(SCR_WIDTH / 2, SCR_HEIGHT / 2);
    glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
    // 创建窗口, 设置标题
    glutCreateWindow("exp5: Half-Edge Structure");
    // 初始化
    initializer();

    glutCreateMenu(menuCallback);
    glutAddMenuEntry("Pyramid", 0);
    glutAddMenuEntry("Cube", 1);
    glutAddMenuEntry("Bunny", 2);
    glutAddMenuEntry("Bumpy", 3);
    glutAddMenuEntry("Loop Subdivision", 4);
    glutAddMenuEntry("Colorify", 5);
    glutAddMenuEntry("ClearScreen", 6);
    glutAddMenuEntry("Exit", 7);
    glutAttachMenu(GLUT_MIDDLE_BUTTON);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(mouseMotionCallback);
    glutKeyboardFunc(keyboardCallback);
    // 画图展示函数
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);

    glutMainLoop();
    
    return 0;
}

void initializer(void) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glShadeModel(GL_FLAT);
}

void displayCallback(void) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    // 设置视点位置
    gluLookAt(0.0f, 0.0f, 5.0f,
              0.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);
    // 设置模型位置, 平移
    glTranslatef(0.0f, 0.0f, -3.0f);
    // 缩放设置
    glScalef(ArcBall->zoomRate, ArcBall->zoomRate, ArcBall->zoomRate);
    // 旋转设置
    glMultMatrixf(ArcBall->Transform.M);
    // 绘制model
    model.draw();
    if (isColorified)
        model.colorify(4);
    // 交换缓冲区
    glutSwapBuffers();
}

void menuCallback(int value) {
    isColorified = false;
    switch (value) {
        case 0:
        case 1:
        case 2:
        case 3:
            model.loadOff(modelPath + modelFiles[value]);
            glutPostRedisplay();
            break;
        case 4:
            model.loopSubdivision();
            glutPostRedisplay();
            break;
        case 5:
            isColorified = true;
            glutPostRedisplay();
            break;
        case 6:
            model.clear();
            glutPostRedisplay();
            break;
        default:
            exit(0);
    }
}

void mouseCallback(int button, int state, int x, int y) {
    if(button == GLUT_LEFT_BUTTON && state==GLUT_DOWN){
        ArcBall->isClicked = true;
        mouseMotionCallback(x, y);
    }
    else if(button == GLUT_LEFT_BUTTON && state==GLUT_UP)
        ArcBall->isClicked = false;
    else if(button == GLUT_RIGHT_BUTTON && state==GLUT_DOWN){
        ArcBall->isRClicked = true;
        mouseMotionCallback(x, y);
    }
    else if(button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
        ArcBall->isRClicked = false;
    ArcBall->upstate();
    glutPostRedisplay();
}

void mouseMotionCallback(int x, int y) {
    ArcBall->MousePt.s.X = x;
    ArcBall->MousePt.s.Y = y;
    ArcBall->upstate();
    glutPostRedisplay();
}

void keyboardCallback(unsigned char key, int x, int y) {
    if (key == GLUT_ESCAPE)
        exit(0);
}

void reshapeCallback(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1, 1, -1, 1, 1.5, 20);
    glMatrixMode(GL_MODELVIEW);
    ArcBall->setBounds((GLfloat)width, (GLfloat)height);
}
