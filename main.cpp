#define _CRT_SECURE_NO_WARNINGS
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

// stb_image.h needed
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// GLOBAL VARIABLES
int screenWidth = 1024;
int screenHeight = 768;
float camRotateX = 0.0f;
float camRotateY = 0.0f;
float camZoom = 25.0f; 

bool isPaused = false;
float globalTime = 0.0f;

GLuint planetTextureId;
GLuint cubeTextureId;

// STRUCTS 
struct Vector3 { float x, y, z; };
struct Vector2 { float u, v; };
struct Vertex { Vector3 position; Vector2 texCoord; Vector3 normal; };

struct Model {
    std::vector<Vertex> vertices;
    void draw() {
        glBegin(GL_TRIANGLES);
        for (const auto& v : vertices) {
            glNormal3f(v.normal.x, v.normal.y, v.normal.z);
            glTexCoord2f(v.texCoord.u, v.texCoord.v);
            glVertex3f(v.position.x, v.position.y, v.position.z);
        }
        glEnd();
    }
};

Model planetModel;

struct CubeData {
    float orbitRadius;
    float orbitSpeed;
    float orbitAngleOffset;
    float yOffset;
    float selfRotSpeed;
    float selfRotAxisX;
    float selfRotAxisY;
    float size;
};

CubeData cubes[6] = {
    { 1.5f,   80.0f,   0.0f,   0.0f,  60.0f, 1.0f, 0.0f, 0.35f }, 
    { 2.0f,  -60.0f,  60.0f,   0.2f,  40.0f, 0.0f, 1.0f, 0.45f }, 
    { 2.5f,  100.0f, 120.0f,  -0.3f,  80.0f, 1.0f, 1.0f, 0.40f }, 
    { 3.0f,  -70.0f, 180.0f,   0.4f,  50.0f, 0.5f, 1.0f, 0.30f }, 
    { 3.5f,   85.0f, 240.0f,  -0.5f,  70.0f, 1.0f, 0.5f, 0.50f }, 
    { 4.0f,  -50.0f, 300.0f,   0.1f,  90.0f, 0.0f, 1.0f, 0.38f }  
};

// HELPERS
GLuint loadTexture(const char* filename) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
        std::cout << "Successfully loaded: " << filename << std::endl;
    } else {
        std::cerr << "FAILED to load texture: " << filename << std::endl;
    }
    return textureID;
}

bool loadOBJ(const char* path, Model& outModel) {
    std::vector<Vector3> temp_positions;
    std::vector<Vector2> temp_texCoords;
    std::vector<Vector3> temp_normals;
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "FAILED to open OBJ: " << path << std::endl;
        return false;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;
        if (prefix == "v") { Vector3 v; ss >> v.x >> v.y >> v.z; temp_positions.push_back(v); }
        else if (prefix == "vt") { Vector2 vt; ss >> vt.u >> vt.v; temp_texCoords.push_back(vt); }
        else if (prefix == "vn") { Vector3 vn; ss >> vn.x >> vn.y >> vn.z; temp_normals.push_back(vn); }
        else if (prefix == "f") {
            std::string vertexStr;
            for (int i = 0; i < 3; i++) {
                ss >> vertexStr;
                std::replace(vertexStr.begin(), vertexStr.end(), '/', ' ');
                std::stringstream vss(vertexStr);
                int vIdx, vtIdx, vnIdx;
                vss >> vIdx >> vtIdx >> vnIdx;
                Vertex v;
                v.position = temp_positions[vIdx - 1];
                v.texCoord = temp_texCoords[vtIdx - 1];
                v.normal = temp_normals[vnIdx - 1];
                outModel.vertices.push_back(v);
            }
        }
    }
    std::cout << "Successfully loaded OBJ: " << path << std::endl;
    return true;
}

void drawCube(float size) {
    float h = size / 2.0f;
    glBegin(GL_QUADS);
    // Front
    glNormal3f(0.0f, 0.0f, 1.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-h, -h, h); glTexCoord2f(1.0f, 0.0f); glVertex3f(h, -h, h); glTexCoord2f(1.0f, 1.0f); glVertex3f(h, h, h); glTexCoord2f(0.0f, 1.0f); glVertex3f(-h, h, h);
    // Back
    glNormal3f(0.0f, 0.0f, -1.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-h, -h, -h); glTexCoord2f(1.0f, 1.0f); glVertex3f(-h, h, -h); glTexCoord2f(0.0f, 1.0f); glVertex3f(h, h, -h); glTexCoord2f(0.0f, 0.0f); glVertex3f(h, -h, -h);
    // Top
    glNormal3f(0.0f, 1.0f, 0.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-h, h, -h); glTexCoord2f(0.0f, 0.0f); glVertex3f(-h, h, h); glTexCoord2f(1.0f, 0.0f); glVertex3f(h, h, h); glTexCoord2f(1.0f, 1.0f); glVertex3f(h, h, -h);
    // Bottom
    glNormal3f(0.0f, -1.0f, 0.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(-h, -h, -h); glTexCoord2f(0.0f, 1.0f); glVertex3f(h, -h, -h); glTexCoord2f(0.0f, 0.0f); glVertex3f(h, -h, h); glTexCoord2f(1.0f, 0.0f); glVertex3f(-h, -h, h);
    // Right
    glNormal3f(1.0f, 0.0f, 0.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(h, -h, -h); glTexCoord2f(1.0f, 1.0f); glVertex3f(h, h, -h); glTexCoord2f(0.0f, 1.0f); glVertex3f(h, h, h); glTexCoord2f(0.0f, 0.0f); glVertex3f(h, -h, h);
    // Left
    glNormal3f(-1.0f, 0.0f, 0.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-h, -h, -h); glTexCoord2f(1.0f, 0.0f); glVertex3f(-h, -h, h); glTexCoord2f(1.0f, 1.0f); glVertex3f(-h, h, h); glTexCoord2f(0.0f, 1.0f); glVertex3f(-h, h, -h);
    glEnd();
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);

    if (!loadOBJ("planet.obj", planetModel)) std::cout << "Error loading OBJ" << std::endl;
    
    planetTextureId = loadTexture("planet_Quom1200.png"); 
    cubeTextureId = loadTexture("container.png"); 

    // Light Settings
    GLfloat lightAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat lightDiffuse[] = { 1.0f, 1.0f, 0.9f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    
    // Attenuation
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.01f);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Camera Logic
    float camX = camZoom * sin(camRotateY * 0.01745f) * cos(camRotateX * 0.01745f);
    float camY = camZoom * sin(camRotateX * 0.01745f);
    float camZ = camZoom * cos(camRotateY * 0.01745f) * cos(camRotateX * 0.01745f);
    gluLookAt(camX, camY, camZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    // LOGIC: PLANET ORBITING THE UNIVERSE CENTER
    float planetOrbitRadius = 3.0f;
    float planetX = cos(globalTime * 0.3f) * planetOrbitRadius;
    float planetZ = sin(globalTime * 0.3f) * planetOrbitRadius;

    // 1. SET LIGHT POSITION AT THE PLANET
    GLfloat lightPos[] = { planetX, 0.0f, planetZ, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // DRAW PLANET 
    glPushMatrix();
        glTranslatef(planetX, 0.0f, planetZ); 
        
        glBindTexture(GL_TEXTURE_2D, planetTextureId);
        // Emissive material for planet
        GLfloat emissive[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        glMaterialfv(GL_FRONT, GL_EMISSION, emissive);
        
        glRotatef(globalTime * 20.0f, 0.0f, 1.0f, 0.0f); 
        glScalef(0.5f, 0.5f, 0.5f);
        planetModel.draw();
        
        GLfloat noEmission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
    glPopMatrix();

    // 3. DRAW CUBES ORBITING THE MOVING PLANET
    glBindTexture(GL_TEXTURE_2D, cubeTextureId);
    GLfloat matDiff[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    for (int i = 0; i < 6; i++) {
        glPushMatrix();
            // A. Follow Planet
            glTranslatef(planetX, 0.0f, planetZ);

            // B. Orbit around planet
            float angle = globalTime * cubes[i].orbitSpeed + cubes[i].orbitAngleOffset;
            float cx = cubes[i].orbitRadius * cos(angle * 0.01745f);
            float cz = cubes[i].orbitRadius * sin(angle * 0.01745f);
            glTranslatef(cx, cubes[i].yOffset, cz);

            // C. Self Rotation
            glRotatef(globalTime * cubes[i].selfRotSpeed, cubes[i].selfRotAxisX, cubes[i].selfRotAxisY, 0.0f);

            drawCube(cubes[i].size);
        glPopMatrix();
    }

    glutSwapBuffers();
}

void reshape(int w, int h) {
    screenWidth = w; screenHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)w / h, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

void timer(int value) {
    if (!isPaused) globalTime += 0.05f; 
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27 || key == 'q' || key == 'Q') exit(0);
    if (key == 'p' || key == 'P' || key == ' ') isPaused = !isPaused;
}

void specialKeys(int key, int x, int y) {
    float amount = 2.0f;
    if (key == GLUT_KEY_LEFT) camRotateY -= amount;
    if (key == GLUT_KEY_RIGHT) camRotateY += amount;
    if (key == GLUT_KEY_UP) camRotateX += amount;
    if (key == GLUT_KEY_DOWN) camRotateX -= amount;
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screenWidth, screenHeight);
    glutCreateWindow("Planet Project");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    return 0;
}
