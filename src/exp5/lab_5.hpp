//
//  exp5.hpp
//  exp5
//
//  Created by abc_mac on 2021/11/19.
//

#ifndef exp5_hpp
#define exp5_hpp

#define GL_SILENCE_DEPRECATION
#define SCR_WIDTH 800
#define SCR_HEIGHT 600
#define GLUT_ESCAPE 27
#include <GLUT/glut.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <fstream>
#include <cmath>
using namespace std;

constexpr auto PI = 3.1415926536;

struct Vertex;
struct HalfEdge;
struct Face;

vector<Vertex> newVertices;
vector<HalfEdge *> newEdges;
vector<Face> newFaces;

struct Vertex{
    float x;
    float y;
    float z;
    
    HalfEdge *outEdge;              // one of the half-edges emantating from the vertex
    
    Vertex(float x, float y, float z, HalfEdge *out = nullptr): x(x), y(y), z(z), outEdge(nullptr) {}
};

struct HalfEdge{
    Vertex *endVertex;              // vertex at the end of the half-edge
    HalfEdge *pair;                 // oppositely oriented adjacent half-edge
    Face *face;                     // face the half-edge borders
    HalfEdge *next;                 // next half-edge around the face
    
    HalfEdge(Vertex *to = nullptr, HalfEdge *pair = nullptr, Face *face = nullptr, HalfEdge *next = nullptr): endVertex(to), pair(pair), face(face), next(next) {}
};

struct Face{
    HalfEdge *aroundEdge;           // one of the half-edges bordering the face;
    
    Face(HalfEdge *edge = nullptr): aroundEdge(edge) {}
};

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

class Model {
public:
    Model(): numberOfVertices(0), numberOfFaces(0), numberOfEdges(0) {}
    
    ~Model() {
        if (!edges.empty()) {
            for (auto e : edges)
                if (e != nullptr)
                    delete e;
        }
    }
    
    void loadOff(const string &filePath) {
        this->clear();              // 清空model
        ifstream fin(filePath);     // 打开文件流
        if (!fin.is_open()) {
            cerr << "Opening OFF file failed.";
            exit(1);
        }
        string tag;
        fin >> tag;
        if (tag != "OFF") {
            cerr << "This is not a .off file.";
            exit(1);
        }
        fin >> numberOfVertices >> numberOfFaces >> numberOfEdges;
        
        // 读取顶点
        float x, y, z;
        for (int i = 0; i < numberOfVertices; i++) {
            fin >> x >> y >> z;
            vertices.emplace_back(x, y, z);
        }
        // 读取面
        int halfEgdeCount;
        map<pair<int, int>, HalfEdge *> mapping;
        for (int i = 0; i < numberOfFaces; i++) {
            fin >> halfEgdeCount;
            // 建立面
            faces.emplace_back(nullptr);
            // 建立面相关的边
            int vertexNumber[halfEgdeCount];                // 当前面的顶点编号
            int edgeStart = (int)edges.size();              // 新增边在所有边中的起始位置
            for (int j = 0; j < halfEgdeCount; j++) {       // 接收当前面的顶点
                fin >> vertexNumber[j];
                edges.emplace_back(new HalfEdge());         // 在接受顶点时建立HalfEdge
                vertices[vertexNumber[j]].outEdge = edges[edgeStart + j];
            }
            
            for (int j = 0; j < halfEgdeCount; j++)         // 建立顶点<u, v> --> HalfEgde *的映射
                mapping[make_pair(vertexNumber[j], vertexNumber[(j + 1) % halfEgdeCount])] = edges[edgeStart + j];
            
            // 对于每个边, 更新edges的信息 { endVertex, face, next, pair }
            for (int j = 0; j < halfEgdeCount; j++) {
                edges[edgeStart + j]->endVertex = &vertices[vertexNumber[(j + 1) % halfEgdeCount]];
                edges[edgeStart + j]->face = &faces[i];
                edges[edgeStart + j]->next = edges[(edgeStart + j + 1) - (j == halfEgdeCount - 1 ? halfEgdeCount : 0)];
                edges[edgeStart + j]->pair = nullptr;
                pair<int, int> key = make_pair(vertexNumber[(j + 1) % halfEgdeCount], vertexNumber[j]);
                if (mapping.count(key)) {
                    edges[edgeStart + j]->pair = mapping[key];
                    mapping[key]->pair = edges[edgeStart + j];
                }
            }
            // 将面与HalfEdge连接起来
            faces[i].aroundEdge = edges[edgeStart];
        }
        
        fin.close();                // 关闭文件流
    }
    
    void draw() {
        // 遍历所有面, 绘制整个model
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_POINT_SMOOTH, GL_NICEST);
        for (int i = 0; i < numberOfFaces; i++) {
            glBegin(GL_LINE_LOOP);
            glColor3f(1.0f, 1.0f, 1.0f);
            HalfEdge *edge = faces[i].aroundEdge;
            // 对每个面遍历所有包围边
            do {
                glVertex3f(edge->endVertex->x, edge->endVertex->y, edge->endVertex->z);
                edge = edge->next;
            } while (edge != faces[i].aroundEdge);
            glEnd();
        }
    }
    
    void loopSubdivision() {
        newVertices.clear();
        newFaces.clear();
        newEdges.clear();
        HalfEdge *current;
        
        int n = 0;                          // 内部顶点周围顶点数
        float p_sumx, p_sumy, p_sumz;       // 周围顶点坐标和
        float new_x, new_y, new_z;          // 新顶点的坐标
        float beta;                         // 参数beta
        // 保存更新后旧顶点的位置, 记录每个边的起始点, 便于后面更新面
        unordered_map<HalfEdge *, int> originVertices;
        for (int i = 0; i < vertices.size(); i++) {
            current = vertices[i].outEdge;
            
            n = p_sumx = p_sumy = p_sumz = 0;
            do {
                n++;
                p_sumx += current->endVertex->x;
                p_sumy += current->endVertex->y;
                p_sumz += current->endVertex->z;
                originVertices[current] = int(newVertices.size());
                current = current->pair->next;
            } while (current != vertices[i].outEdge);
            
            beta = 1.0 / n * (0.625 - pow(0.375 + 0.25 * cos(2 * PI / n), 2));
            new_x = (1 - n * beta) * vertices[i].x + beta * p_sumx;
            new_y = (1 - n * beta) * vertices[i].y + beta * p_sumy;
            new_z = (1 - n * beta) * vertices[i].z + beta * p_sumz;
            
            newVertices.emplace_back(new_x, new_y, new_z);
        }
        
        // 生成新顶点, 记录每个边的中点 --- 即新点
        unordered_map<HalfEdge *, int> centerVertices;
        unordered_map<HalfEdge *, bool> existingVertices;
        for (auto &edge : edges) {
            float sum_x0 = edge->pair->endVertex->x + edge->endVertex->x;
            float sum_y0 = edge->pair->endVertex->y + edge->endVertex->y;
            float sum_z0 = edge->pair->endVertex->z + edge->endVertex->z;
            float sum_x1 = edge->next->endVertex->x + edge->pair->next->endVertex->x;
            float sum_y1 = edge->next->endVertex->y + edge->pair->next->endVertex->y;
            float sum_z1 = edge->next->endVertex->z + edge->pair->next->endVertex->z;
            
            if (existingVertices.count(edge)) {
                centerVertices[edge] = centerVertices[edge->pair];
                continue;
            }
            newVertices.emplace_back(0.375 * sum_x0 + 0.125 * sum_x1, 0.375 * sum_y0 + 0.125 * sum_y1, 0.375 * sum_z0 + 0.125 * sum_z1);
            // 记录下新顶点
            centerVertices[edge] = int(newVertices.size()) - 1;
            existingVertices[edge] = true;
            existingVertices[edge->pair] = true;
        }
        
        // 生成新面, 连接新顶点和更新后的旧顶点
        unordered_map<int, vector<int> > verticesAroundFace;
        for (auto &face : faces) {
            HalfEdge *edgesAround[3];
            int indexOfCenter[3];
            edgesAround[0] = face.aroundEdge;
            edgesAround[1] = face.aroundEdge->next;
            edgesAround[2] = face.aroundEdge->next->next;
            for (int i = 0; i < 3; i++)
                indexOfCenter[i] = centerVertices[edgesAround[i]];
            
            int pos = 0;
            for (int i = 0; i < 3; i++) {
                pos = int(newFaces.size());
                newFaces.emplace_back(nullptr);
                verticesAroundFace[pos].emplace_back(originVertices[edgesAround[i]]);
                verticesAroundFace[pos].emplace_back(centerVertices[edgesAround[i]]);
                verticesAroundFace[pos].emplace_back(centerVertices[edgesAround[(i + 2) % 3]]);
            }
            pos++;
            newFaces.emplace_back(nullptr);
            for (int i = 0; i < 3; i++)
                verticesAroundFace[pos].emplace_back(centerVertices[edgesAround[i]]);
        }
        
        // 新边, 已经对每个面记录了包围的点, 保存在verticesAroundFace里面
        map<pair<int, int>, HalfEdge *> mapping;
        int count = 0;
        for (int i = 0; i < newFaces.size(); i++) {
            auto start = newEdges.size();
            // 建立新面的三条环绕新边
            int n = int(verticesAroundFace[i].size());
            for (int j = 0; j < n; j++) {
                // 新建边
                HalfEdge *e = new HalfEdge(&newVertices[verticesAroundFace[i][(j + 1) % n]], nullptr, &newFaces[i], nullptr);
                // 将新边加入映射
                mapping[make_pair(verticesAroundFace[i][j], verticesAroundFace[i][(j + 1) % n])] = e;
                // 更新vertices
                newVertices[verticesAroundFace[i][j]].outEdge = e;
                // 设置pair
                // 当前边为<u, v>
                // 查找key = <v, u>
                pair<int, int> key(verticesAroundFace[i][(j + 1) % n], verticesAroundFace[i][j]);
                if (mapping.count(key)) {
                    count++;
                    e->pair = mapping[key];
                    mapping[key]->pair = e;
                }
                newEdges.emplace_back(e);
            }
            // 遍历环绕新边, 设置next
            for (int j = 0; j < n; j++)
                newEdges[start + j]->next = newEdges[start + (j + 1) % n];
            newFaces[i].aroundEdge = newEdges[start];
        }
        /*
        for (auto &m : mapping)
            cout << "<" << m.first.first << ", " << m.first.second << ">\n";
        cout << count << " pairs\n";
        */
        this->clear();
        vertices = newVertices;
        edges = newEdges;
        faces = newFaces;
        numberOfVertices = int(vertices.size());
        numberOfEdges = int(edges.size());
        numberOfFaces = int(faces.size());
    }
    
    void colorify(int index) {
        if (vertices.empty())
            return;
        
        float x, y, z;
        x = vertices[index].x;
        y = vertices[index].y;
        z = vertices[index].z;

        glEnable(GL_LINE_SMOOTH);
        glHint(GL_POINT_SMOOTH, GL_NICEST);
        
        
        // 相邻点标红、相邻边标蓝
        HalfEdge *edge = vertices[index].outEdge;
        do {
            // 蓝边
            glBegin(GL_LINE_LOOP);
            glColor3f(0.0f, 0.0f, 1.0f);
            glVertex3f(x, y, z);
            glVertex3f(edge->endVertex->x, edge->endVertex->y, edge->endVertex->z);
            glEnd();
            // 红点
            glBegin(GL_POINTS);
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex3f(edge->endVertex->x, edge->endVertex->y, edge->endVertex->z);
            glEnd();
            
            edge = edge->pair->next;
        } while (edge != vertices[index].outEdge);
        
        // 当前点标绿
        glBegin(GL_POINTS);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(x, y, z);
        glEnd();
    }
    
    void clear() {
        if (!vertices.empty())
            vertices.clear();
        if (!edges.empty()) {
            for (auto e : edges)
                if (e != nullptr)
                    delete e;
            edges.clear();
        }
        if (!faces.empty())
            faces.clear();
        numberOfVertices = numberOfFaces = numberOfEdges = 0;
    }
    
private:
    vector<Vertex> vertices;
    vector<HalfEdge *> edges;
    vector<Face> faces;
    int numberOfVertices;
    int numberOfFaces;
    int numberOfEdges;
};

#endif /* exp5_hpp */
