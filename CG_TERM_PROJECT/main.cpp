#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "fmod.hpp"
#include "fmod_errors.h"
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <time.h>
#include <stdlib.h>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <fstream>
#include <sstream>

#define WINDOWX 800
#define WINDOWY 800

#define GLUT_KEY_SPACE 0x20


using namespace std;
using namespace FMOD;

random_device rd;
default_random_engine dre(rd());
uniform_real_distribution<float>uid(0.0f, 1.0f);

void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid InitBuffer();
void InitShader();
GLchar* filetobuf(const char* file);

GLuint shaderID;

GLuint vertexShader;
GLuint fragmentShader;

GLuint VAO, VBO[3];

////////////////////////사운드
System* ssystem;
Sound* bgm;
Sound* click_sound;
Channel* channel = 0;
FMOD_RESULT result;
void* extradriverdata = 0;


glm::vec3 rect_vertex[] = {
    {0.5, 0.5, 0.0},
    {-0.5, -0.5, 0.0},
    {0.5, -0.5, 0.0},
    {0.5, 0.5, 0.0},
    {-0.5, 0.5, 0.0},
    {-0.5, -0.5, 0.0}
};

glm::vec3 rect_normal[] = {
    {0.0, 0.0, 1.0},
    {0.0, 0.0, 1.0},
    {0.0, 0.0, 1.0},
    {0.0, 0.0, 1.0},
    {0.0, 0.0, 1.0},
    {0.0, 0.0, 1.0}
};

glm::vec2 rect_uv[] = {
    {1, 1},
    {0, 0},
    {1, 0},
    {1, 1},
    {0, 1},
    {0, 0}
};

class UIMesh {
public:
    string textureFile;
    GLuint texture;
    unsigned char* data;

    void Texturing() {
        int width, height, nrChannels;
        data = stbi_load(textureFile.c_str(), &width, &height, &nrChannels, 0);

        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0 + texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
        glUniform1i(glGetUniformLocation(shaderID, "outTexture"), texture);
    }
    void Bind() {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), rect_vertex, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec3), rect_normal, GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(glm::vec2), rect_uv, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
    }
    void Draw() {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};
UIMesh title_logo;
UIMesh press_space;
UIMesh speech_bubble;
UIMesh story_background;

class Mesh {
public:
    vector<glm::vec3> vertex;
    vector<glm::vec3> normals;
    vector<glm::vec2> uvs;

    string textureFile;
    GLuint texture;
    unsigned char* data;

    void Texturing() {
        int width{}, height{}, nrChannels{};
        data = stbi_load(textureFile.c_str(), &width, &height, &nrChannels, 0);

        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0 + texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
        glUniform1i(glGetUniformLocation(shaderID, "outTexture"), texture);
    }

    void Bind() {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(glm::vec3), vertex.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
    }
    void Draw() {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertex.size());
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    void FanDraw() {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertex.size());
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
};

class Plane {
public:
    vector<Mesh>mesh;
    int texture_cnt;

    float rotate_x = 0.0f;
    float rotate_y = 0.0f;
    float rotate_z = 0.0f;
    float size = 1.0f;
    glm::vec3 move = glm::vec3(0.0f);

    glm::mat4 my_TR() {
        glm::mat4 TR = glm::mat4(1.0f);
        TR = glm::translate(TR, move);
        TR = glm::rotate(TR, (float)glm::radians(rotate_x), glm::vec3(1.0f, 0.0f, 0.0f));
        TR = glm::rotate(TR, (float)glm::radians(rotate_y), glm::vec3(0.0f, 1.0f, 0.0f));
        TR = glm::rotate(TR, (float)glm::radians(rotate_z), glm::vec3(0.0f, 0.0f, 1.0f));
        TR = glm::scale(TR, glm::vec3(size));
        return TR;
    }
};
Plane Cube;
Plane SpongeBob;
Plane Krabs;
Plane BikiniMap;
Plane bread[2];
Plane fryfan;
Plane CuttingBoard;
Plane Potato;
Plane PotatoChips;
Plane FryerBasket;

float BackGround[] = { 0.0, 0.0, 0.0 };

glm::mat4 TR = glm::mat4(1.0f);

void keyboard(unsigned char, int, int);
void SpecialKeyboard(int key, int x, int y);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void TimerFunction(int value);
GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
void LoadOBJ(const char* filename, vector<Mesh>& out_mesh);
void LoadOBJ_single(const char* filename, vector<Mesh>& out_mesh);
void LoadMTL(const char* FileName, const char* mtlFileName, vector<Mesh>& out_mesh, int& tex_cnt);

void make_vertexShaders()
{

    GLchar* vertexShaderSource;

    vertexShaderSource = filetobuf("vertex.glsl");

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
        cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << endl;
        exit(-1);
    }
}
void make_fragmentShaders()
{
    const GLchar* fragmentShaderSource = filetobuf("fragment.glsl");

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLint result;
    GLchar errorLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
        cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << endl;
        exit(-1);
    }

}
GLuint make_shaderProgram()
{
    GLint result;
    GLchar errorLog[512];
    GLuint ShaderProgramID;
    ShaderProgramID = glCreateProgram();
    glAttachShader(ShaderProgramID, vertexShader);
    glAttachShader(ShaderProgramID, fragmentShader);
    glLinkProgram(ShaderProgramID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &result);
    if (!result) {
        glGetProgramInfoLog(ShaderProgramID, 512, NULL, errorLog);
        cerr << "ERROR: shader program 연결 실패\n" << errorLog << endl;
        exit(-1);
    }
    glUseProgram(ShaderProgramID);
    return ShaderProgramID;
}
void InitShader()
{
    make_vertexShaders();
    make_fragmentShaders();
    shaderID = make_shaderProgram();
}
GLvoid InitBuffer() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(3, VBO);
    glBindVertexArray(VAO);
}

bool start = true;
int SCENE = 0;

glm::vec3 CameraPos = { 5.0f, 5.0f, 5.0f };
glm::vec3 CameraAt = { -5.0f,-1.0f, -2.0f };
float rotate_y = 0.0f;

float light_x = 5.0f, light_y = 5.0f, light_z = 5.0f;
float light_angle = 0.0;

// 시작화면
glm::vec3 START_CameraPos[4] = {
    {5.0f, 5.0f, 5.0f},
    {2.0f, 2.6f, 2.5f},
    {0.6f, 2.6f, 2.5f},
    {-1.4f, 1.5f, 1.0f}
};

glm::vec3 START_CameraAt[4] = {
    { -5.0f,-1.0f, -2.0f},
    {-3.5f, 1.0f, -2.0f},
    {-2.0f, 1.0f, -2.0f},
    {-0.6f, 2.0f, -2.0f}
};

bool start_timer = false;
int start_index = 0, end_index = 1;
float line_t = 0.0f;
float title_logo_ty = 0.4f;

// 스토리 화면
bool Story_Show = true;
bool breath_direction = true;
bool spongebob_talk = true;
string Text = "resource/Text_";
int Text_cnt = 0;
int timer_cnt = 0;
int krabs_talk = 0;
float breath_ty = 0.5f;

//게임화면
UIMesh game_ui;
UIMesh flip_bar;
bool potato_cooked_finish = false;
bool oil_timer = false;
bool potato_fry_timer = false;
float score[4][2] = { 0 };
float bar_move = 0.0f;
float time_angle = 0.0f;
int bar_dir = 1;
int jcnt = 0;
bool flip_bar_dir = true;
float flip_bar_tx = -0.8f, oil_scale_y = 0.0f;
int potato_gauge = 0;
float potato_chips_trans[7][2] = {
    {0.5f, 0.1f}, {0.3f, -0.1f}, {0.2f, 0.0f}, {0.0f, -0.1f},
    {-0.2f, 0.1f}, {-0.3f, -0.1f}, {-0.5f, 0.1f}
};
bool potato_show = true;
float potato_scale_x = 0.05f, potato_tx = 0.0f;


GLvoid drawScene() {
    if (start) {
        start = false;
        LoadOBJ("cube.obj", Cube.mesh);
        Cube.mesh[0].textureFile = "white.png";
        LoadOBJ("spongebob/spongebob.obj", SpongeBob.mesh);
        LoadMTL("spongebob", "spongebob/spongebob.mtl", SpongeBob.mesh, SpongeBob.texture_cnt);
        SpongeBob.mesh[1].textureFile = "spongebob/z3spon3.png";
        LoadOBJ("Mr Krabs/mrkrabs.obj", Krabs.mesh);
        LoadMTL("Mr Krabs", "Mr Krabs/mrkrabs.mtl", Krabs.mesh, Krabs.texture_cnt);
        LoadOBJ("BikiniMap/map.obj", BikiniMap.mesh);
        LoadMTL("BikiniMap", "BikiniMap/map.mtl", BikiniMap.mesh, BikiniMap.texture_cnt);
        LoadOBJ_single("food/MUFF_T_CR.obj", bread[0].mesh);
        bread[0].mesh[0].textureFile = "food/MUFF_T_CR_01.png";
        bread[0].size = 0.5; bread[1].size = 0.5;
        bread[0].move = glm::vec3(1.3 * cos(0), 0, 1.3 * sin(0) - 1.3);
        bread[1].move = glm::vec3(1.3 * cos(glm::radians(90.0f)), 0, 1.3 * sin(glm::radians(90.0f)) - 1.3);
        LoadOBJ_single("food/MUFF_T_HE.obj", bread[1].mesh);
        bread[1].mesh[0].textureFile = "food/MUFF_T_HE_01.png";
        fryfan.size = 1.0f;
        fryfan.rotate_x = -100.0f;
        fryfan.move = glm::vec3(0.000000, 1.500000, 4.899998);
        LoadOBJ_single("tool/pan.obj", fryfan.mesh);
        fryfan.mesh[0].textureFile = "tool/TOBJ_0.png";
        LoadOBJ("Cutting Board/cuttingboard.obj", CuttingBoard.mesh);
        CuttingBoard.mesh[0].textureFile = "Cutting Board/cuttingboard_d.png";
        LoadOBJ("Potato/potato.obj", Potato.mesh);
        Potato.mesh[0].textureFile = "Potato/potato.png";
        LoadOBJ("PotatoChips/Chips.obj", PotatoChips.mesh);
        PotatoChips.mesh[0].textureFile = "PotatoChips/sang_Chips.png";
        LoadOBJ("FryerBasket/FryerBasket.obj", FryerBasket.mesh);
        FryerBasket.mesh[0].textureFile = "FryerBasket/FryerBasket.png";
        {
            title_logo.textureFile = "resource/title_logo.png";
            press_space.textureFile = "resource/press_space_bar.png";
            flip_bar.textureFile = "resource/flip_bar.png";
        }

        // 사운드
        result = System_Create(&ssystem);
        if (result != FMOD_OK)
            exit(0);
        ssystem->init(32, FMOD_INIT_NORMAL, extradriverdata);
        ssystem->createSound("sound/title_bgm.mp3", FMOD_LOOP_NORMAL, 0, &bgm);
        ssystem->createSound("sound/button_click_sound.wav", FMOD_DEFAULT, 0, &click_sound);
        ssystem->playSound(bgm, 0, false, &channel);
    }
    stbi_set_flip_vertically_on_load(true);
    glViewport(0, 0, WINDOWX, WINDOWY);
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);    //배경

    glUseProgram(shaderID);
    glBindVertexArray(VAO);// 쉐이더 , 버퍼 배열 사용

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    unsigned int modelLocation = glGetUniformLocation(shaderID, "model");
    unsigned int viewLocation = glGetUniformLocation(shaderID, "view");
    unsigned int projLocation = glGetUniformLocation(shaderID, "projection");
    unsigned int lightPosLocation = glGetUniformLocation(shaderID, "lightPos");
    unsigned int lightColorLocation = glGetUniformLocation(shaderID, "lightColor");
    unsigned int objColorLocation = glGetUniformLocation(shaderID, "objectColor");

    glm::mat4 Vw;
    glm::mat4 Pj;
    glm::mat4 Cp;
    glm::vec3 cameraPos;
    glm::vec3 cameraDirection;
    glm::vec3 cameraUp;

    glUniform3f(lightPosLocation, light_x, light_y, light_z);
    glUniform4f(lightColorLocation, 1.0, 1.0, 1.0, 1.0f);
    glUniform4f(objColorLocation, 1.0f, 1.0f, 1.0f, 1.0f);

    // 그리기 코드    

    //////////////////////////////////////////////////////////////////////////////////////////////
    cameraPos = glm::vec4(CameraPos, 1.0f); //--- 카메라 위치
    cameraDirection = glm::vec3(CameraAt); //--- 카메라 바라보는 방향
    cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 위쪽 방향
    Vw = glm::mat4(1.0f);
    Vw = glm::lookAt(cameraPos, cameraDirection, cameraUp);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &Vw[0][0]);

    Pj = glm::mat4(1.0f);
    Pj = glm::perspective(glm::radians(45.0f), (float)WINDOWX / (float)WINDOWY, 0.1f, 100.0f);
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &Pj[0][0]);
    //////////////////////////////////////////////////////////////////////////////////////////////
    switch (SCENE) {
    case 0:
    {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, 0.0f));

        TR = glm::scale(TR, glm::vec3(0.1f, 0.1f, 0.1f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

        for (int i = 0; i < BikiniMap.mesh.size(); ++i) {
            BikiniMap.mesh[i].Texturing();
            BikiniMap.mesh[i].Bind();
            BikiniMap.mesh[i].Draw();
        }
    }
    break;
    case 1:
    {
        if (Story_Show) {
            TR = glm::mat4(1.0f);
            TR = glm::translate(TR, glm::vec3(-0.5f, breath_ty, 0.0f));
            TR = glm::rotate(TR, (float)glm::radians(40.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            TR = glm::scale(TR, glm::vec3(0.4f, 0.4f, 0.4f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

            for (int i = 0; i < SpongeBob.mesh.size(); ++i) {
                if (spongebob_talk) SpongeBob.mesh[0].Texturing();
                else SpongeBob.mesh[1].Texturing();
                SpongeBob.mesh[i].Bind();
                SpongeBob.mesh[i].Draw();
            }
        }
        else {
            TR = glm::mat4(1.0f);
            TR = glm::translate(TR, glm::vec3(0.5f, breath_ty, 0.0f));
            TR = glm::rotate(TR, (float)glm::radians(-40.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            TR = glm::scale(TR, glm::vec3(0.4f, 0.4f, 0.4f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

            for (int i = 0; i < Krabs.mesh.size(); ++i) {
                if (krabs_talk == 0)Krabs.mesh[0].textureFile = "Mr Krabs/mrkrab2.png";
                else if (krabs_talk == 8)Krabs.mesh[0].textureFile = "Mr Krabs/mrkrab2.png";
                else if (krabs_talk == 9)Krabs.mesh[0].textureFile = "Mr Krabs/mrkrab3.png";
                else {
                    if (krabs_talk % 2 == 0)Krabs.mesh[0].textureFile = "Mr Krabs/mrkrab1.png";
                    else Krabs.mesh[0].textureFile = "Mr Krabs/mrkrab4.png";
                }
                Krabs.mesh[0].Texturing();
                Krabs.mesh[i].Bind();
                Krabs.mesh[i].Draw();
            }
        }
    }
    break;
    case 2:
    {

    }
    break;
    case 3:
    {

        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(glm::lookAt(glm::vec3(0, 6, 2), glm::vec3(0, 0, 0), cameraUp)));

        for (int i = 0; i < 2; i++) {
            for (Mesh m : bread[i].mesh) {
                bread[i].mesh[0].Texturing();
                m.Bind();
                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(bread[i].my_TR()));
                m.Draw();
            }
            for (int j = 0; j < fryfan.mesh.size(); j++) {
                fryfan.mesh[0].Texturing();
                fryfan.mesh[j].Bind();
                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(fryfan.my_TR()));
                fryfan.mesh[j].Draw();
            }
        }
    }
    break;
    case 6:
    {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, 0.0f));
        TR = glm::rotate(TR, (float)glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        TR = glm::scale(TR, glm::vec3(150.0f, 150.0f, 150.0f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        CuttingBoard.mesh[0].Texturing();
        CuttingBoard.mesh[0].Bind();
        CuttingBoard.mesh[0].Draw();

        if (potato_show) {
            TR = glm::mat4(1.0f);
            TR = glm::translate(TR, glm::vec3(potato_tx, 0.2f, 0.0f));
            TR = glm::scale(TR, glm::vec3(potato_scale_x, 0.05f, 0.05f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

            Potato.mesh[0].Texturing();
            Potato.mesh[0].Bind();
            Potato.mesh[0].FanDraw();
        }

        for (int i = 0; i < (potato_gauge / 3); ++i) {
            TR = glm::mat4(1.0f);
            TR = glm::translate(TR, glm::vec3(potato_chips_trans[i][0], 0.1f, potato_chips_trans[i][1]));
            TR = glm::rotate(TR, (float)glm::radians(30.0f * i), glm::vec3(0.0f, 1.0f, 0.0f));
            TR = glm::scale(TR, glm::vec3(0.5f, 0.5f, 0.5f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

            PotatoChips.mesh[0].Texturing();
            PotatoChips.mesh[0].Bind();
            PotatoChips.mesh[0].Draw();
        }
    }
    break;
    case 7:
    {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(0.0f, -0.8f, -1.0f));
        TR = glm::scale(TR, glm::vec3(1.5f, 1.5f, 1.5f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        
        if (flip_bar_tx >= -0.8f && flip_bar_tx <= -0.1f)PotatoChips.mesh[0].textureFile = "PotatoChips/sang_Chips.png";
        else if (flip_bar_tx >= -0.1f && flip_bar_tx <= 0.1f)PotatoChips.mesh[0].textureFile = "PotatoChips/Chips.png";
        else if (flip_bar_tx >= 0.6f)PotatoChips.mesh[0].textureFile = "PotatoChips/tan_Chips.png";
        PotatoChips.mesh[0].Texturing();
        PotatoChips.mesh[0].Bind();
        PotatoChips.mesh[0].Draw();
        

        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(0.0f, -0.2f, -2.0f));
        TR = glm::rotate(TR, (float)glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        TR = glm::scale(TR, glm::vec3(1.0f, 0.2f, 0.2f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

        for (int i = 0; i < FryerBasket.mesh.size(); ++i) {
            FryerBasket.mesh[0].Texturing();
            FryerBasket.mesh[i].Bind();
            FryerBasket.mesh[i].Draw();
        }

        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(0.0f, -1.0f, -1.0f));
        TR = glm::scale(TR, glm::vec3(0.8f, oil_scale_y, 0.5f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        glUniform4f(objColorLocation, 1.0f, 1.0f, 0.0f, 0.5f);
        for (int i = 0; i < Cube.mesh.size(); ++i) {
            Cube.mesh[0].Texturing();
            Cube.mesh[i].Bind();
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            Cube.mesh[i].Draw();
            glDisable(GL_BLEND);
        }
        glUniform4f(objColorLocation, 1.0f, 1.0f, 1.0f, 1.0f);
    }
    break;
    }

    // UI
    //////////////////////////////////////////////////////////////////////////////////////////////
    Vw = glm::mat4(1.0f);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &Vw[0][0]);
    Pj = glm::mat4(1.0f);
    Pj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -100.0f, 100.0f);
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &Pj[0][0]);
    //////////////////////////////////////////////////////////////////////////////////////////////
    switch (SCENE) {
    case 0:
    {
        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(0.0f, title_logo_ty, 0.0f));
        TR = glm::scale(TR, glm::vec3(1.6f, 1.0f, 1.0f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

        title_logo.Texturing();
        title_logo.Bind();
        title_logo.Draw();

        if (!start_timer) {
            TR = glm::mat4(1.0f);
            TR = glm::translate(TR, glm::vec3(0.0f, -0.8f, 0.0f));
            TR = glm::scale(TR, glm::vec3(0.9f, 0.08f, 1.0f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

            press_space.Texturing();
            press_space.Bind();
            press_space.Draw();
        }
    }
    break;
    case 1:
    {
        if (Story_Show) {
            TR = glm::mat4(1.0f);
            TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, -99.0f));
            TR = glm::scale(TR, glm::vec3(2.0f, 2.0f, 1.0f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));

            story_background.textureFile = "resource/story_bg_0.png";
            story_background.Texturing();
            story_background.Bind();
            story_background.Draw();
        }
        else {
            TR = glm::mat4(1.0f);
            TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, -99.0f));
            TR = glm::scale(TR, glm::vec3(2.0f, 2.0f, 1.0f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
            story_background.textureFile = "resource/story_bg_1.png";
            story_background.Texturing();
            story_background.Bind();
            story_background.Draw();
        }

        TR = glm::mat4(1.0f);
        TR = glm::translate(TR, glm::vec3(0.0f, -0.25f, 0.0f));
        TR = glm::scale(TR, glm::vec3(2.0f, 1.5f, 1.0f));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
        string textfile = Text + to_string(Text_cnt) + ".png";

        speech_bubble.textureFile = textfile;
        speech_bubble.Texturing();
        speech_bubble.Bind();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        speech_bubble.Draw();
        glDisable(GL_BLEND);
    }
    break;
    case 3:
    {
        { //배경
            TR = glm::mat4(1.0f);
            TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, -99.0f));
            TR = glm::scale(TR, glm::vec3(2.0f, 2.0f, 1.0f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
            story_background.textureFile = "resource/story_bg_0.png";
            story_background.Texturing();
            story_background.Bind();
            story_background.Draw();
        }
        { //시계
            game_ui.textureFile = "resource/clock.png";
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(0.33f, 0.38f, 2.0f))));
            game_ui.Texturing();
            game_ui.Bind();
            game_ui.Draw();
            game_ui.textureFile = "resource/clock_pointer.png";
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f))* glm::rotate(glm::mat4(1.0f), glm::radians(time_angle), glm::vec3(0, 0, 1))* glm::translate(glm::mat4(1.0f), glm::vec3(0.33f, 0.38f, 2.2f))));
            game_ui.Texturing();
            game_ui.Bind();
            game_ui.Draw();
        }
        { //기본 바 
            game_ui.textureFile = "resource/fry_ui.png";
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 1.0f))));
            game_ui.Texturing();
            game_ui.Bind();
            game_ui.Draw();
        }
        { //볶기 바 
            game_ui.textureFile = "resource/rotate_bar.png";

            game_ui.Texturing();
            game_ui.Bind();
            for (int c = 0; c < (int)score[3][0] / 10.0 && c < 20; c++) {
                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::mat4(1.0f), glm::vec3(2.0, 2.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f + c * 0.011, 0.0f, 3.0f))));
                game_ui.Draw();
            }


        }
        { //뒤집기 바 
            game_ui.textureFile = "resource/fry_bar.png";
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 3.0f))));
            game_ui.Texturing();
            game_ui.Bind();
            for (int c = 0; c < (int)score[3][1] && c < 20; c++) {
                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::mat4(1.0f), glm::vec3(2.0, 2.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f + c * 0.011, 0.0f, 2.1f))));
                game_ui.Draw();
            }
        }
        {
            game_ui.textureFile = "resource/flip_bar.png";
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(bar_move, 0.0f, 2.2f))));
            game_ui.Texturing();
            game_ui.Bind();
            game_ui.Draw();
        }
        
    }
    break;
    case 6:
    { 
        {
            game_ui.textureFile = "resource/rotate_bar.png";
            game_ui.Texturing();
            game_ui.Bind();
            for (int i = 0; i < potato_gauge; i++) {
                glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(glm::scale(glm::mat4(1.0f), glm::vec3(2.0, 2.0f, 1.0f)) * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f + i * 0.011, 0.0f, 0.0f))));
                game_ui.Draw();
            }
        }
        { // 움직이는 바
            TR = glm::mat4(1.0f);
            TR = glm::translate(TR, glm::vec3(flip_bar_tx, 0.0f, 0.0f));
            TR = glm::scale(TR, glm::vec3(2.0f, 2.0f, 1.0f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
            flip_bar.Texturing();
            flip_bar.Bind();
            flip_bar.Draw();
        }
        { //기본 바 
            TR = glm::mat4(1.0f);
            TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, 0.0f));
            TR = glm::scale(TR, glm::vec3(2.0f, 2.0f, 1.0f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
            game_ui.textureFile = "resource/potato_ui.png";
            game_ui.Texturing();
            game_ui.Bind();
            game_ui.Draw();
        }
    }
        break;
    case 7:
    {        
        { // 움직이는 바
            TR = glm::mat4(1.0f);
            TR = glm::translate(TR, glm::vec3(flip_bar_tx, 0.0f, 0.0f));
            TR = glm::scale(TR, glm::vec3(2.0f, 2.0f, 1.0f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
            flip_bar.Texturing();
            flip_bar.Bind();
            flip_bar.Draw();
        }
        { //기본 바 
            TR = glm::mat4(1.0f);
            TR = glm::translate(TR, glm::vec3(0.0f, 0.0f, 0.0f));
            TR = glm::scale(TR, glm::vec3(2.0f, 2.0f, 1.0f));
            glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(TR));
            game_ui.textureFile = "resource/fry_potato_ui.png";
            game_ui.Texturing();
            game_ui.Bind();
            game_ui.Draw();
        }

    }
    break;
    }

    glutSwapBuffers();
    glutPostRedisplay();
}

GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
    glViewport(0, 0, h, h);
}

void keyboard(unsigned char key, int x, int y) {
    if (key >= '0' && key <= '9')SCENE = (int)key - 48;
    switch (SCENE) {
    case 0: // 타이틀 화면
        switch (key) {
        case GLUT_KEY_SPACE:
            if (!start_timer)start_timer = !start_timer;
            break;
        }
        break;
    case 1:
        switch (key) {
        case GLUT_KEY_SPACE:
            Story_Show = !Story_Show;
            if (Text_cnt < 7) {
                ssystem->playSound(click_sound, 0, false, &channel);
                Text_cnt++;
            }
            break;
        }
        break;
    case 3:
        switch (key) {
        case 'z':
            fryfan.rotate_z -= 10.0f;
            break;
        case 'Z':
            fryfan.rotate_z += 10.0f;
            break;
        case 'x': fryfan.rotate_x -= 10.0f; break;
        case 'X': fryfan.rotate_x += 10.0f; break;
        case 'y': fryfan.rotate_y -= 10.0f; break;
        case 'Y': fryfan.rotate_y += 10.0f; break;
        case GLUT_KEY_SPACE:
        {   if (bar_move >= -0.070000 && bar_move <= 0.020000 && jcnt == 0)jcnt = 1, score[3][1] += 4.0f;
        break; }
        }
        break;
    case 6:
        switch (key) {
        case GLUT_KEY_SPACE:
            if (flip_bar_tx >= -0.2f && flip_bar_tx <= 0.2f) {
                if (potato_gauge <= 21) {
                    potato_gauge += 3;
                    potato_scale_x -= 0.007; potato_tx -= 0.1f;
                    if (potato_gauge >= 21) {
                        potato_gauge = 20;
                        potato_show = false;
                    }
                }
            }
            break;
        }
        break;
    case 7:
        switch (key) {
        case GLUT_KEY_SPACE:
            if (!oil_timer && oil_scale_y <= 0.0f) oil_timer = !oil_timer;
            if (potato_fry_timer) {
                potato_fry_timer = !potato_fry_timer;
                potato_cooked_finish = true;
            }
            break;
        }
        break;
    }
    glutPostRedisplay();
}

void SpecialKeyboard(int key, int x, int y)
{
    switch (key) {
    case GLUT_KEY_RIGHT:
        if (SCENE == 3) bread[0].move.z += 0.1;
        break;
    case GLUT_KEY_LEFT:
        if (SCENE == 3) bread[0].move.z -= 0.1;
        break;
    case GLUT_KEY_UP:
        if (SCENE == 3) bread[0].move.y += 0.1;
        break;
    case GLUT_KEY_DOWN:
        if (SCENE == 3) bread[0].move.y -= 0.1;
        break;
    }
    glutPostRedisplay();
}

void TimerFunction(int value)
{
    switch (SCENE) {
    case 0:
    {
        if (start_timer) {
            title_logo_ty += 0.1f;
            if (line_t >= 1.0f && start_index < 2) {
                line_t = 0.0f;
                start_index++; end_index++;
            }
            else if (line_t >= 1.0f && start_index == 2) {
                line_t += 0.1f;
                if (line_t >= 2.0f) {
                    SCENE = 1;
                    channel->stop();
                    bgm->release();
                    ssystem->createSound("sound/story_bgm.mp3", 0, 0, &bgm);
                    ssystem->playSound(bgm, 0, false, &channel);
                }
            }
            else {
                CameraPos = (float)(1.0 - line_t) * START_CameraPos[start_index] + (float)(line_t)*START_CameraPos[end_index];
                CameraAt = (float)(1.0 - line_t) * START_CameraAt[start_index] + (float)(line_t)*START_CameraAt[end_index];
                line_t += 0.1f;
            }
        }
    }
    break;
    case 1:
    {
        CameraPos = { 0.0f, 2.0f, 3.0f };
        CameraAt = { 0.0f, 0.0f, -10.0f };
        spongebob_talk = !spongebob_talk;
        krabs_talk = (krabs_talk + 1) % 10;
        if (breath_direction) {
            breath_ty += 0.005f;
            if (breath_ty >= 0.52f)breath_direction = !breath_direction;
        }
        else {
            breath_ty -= 0.005f;
            if (breath_ty <= 0.5f)breath_direction = !breath_direction;
        }
        if (Text_cnt == 7) {
            timer_cnt++;
            if (timer_cnt == 60)SCENE = 2;
        }
    }
    break;
    case 3: {
        time_angle += 1.0f;
        bar_move += (bar_dir) * 0.1;
        if (bar_move > 0.45) bar_dir = -1, bar_move = 0.45;
        if (bar_move < -0.45)bar_dir = 1, bar_move = -0.45;
        if (jcnt > 0 && jcnt < 10) {
            fryfan.rotate_x += 2.0f;
            for (int c = 0; c < 2; c++) bread[c].move.y += 0.4, bread[c].rotate_z += 10.0f;
            jcnt++;
        }
        if (jcnt >= 9) {
            fryfan.rotate_x -= 2.0f;
            for (int c = 0; c < 2; c++) bread[c].move.y -= 0.4, bread[c].rotate_z += 10.0f;
            jcnt++;
            if (jcnt == 19)printf("%f", bread[0].rotate_z), jcnt = 0, bread[0].move.y = 0, bread[1].move.y = 0, fryfan.rotate_x = -100.0f;
        }
    }
          break;
    case 6:
    {
        CameraPos = { 0.0f, 2.0f, 2.0f };
        CameraAt = { 0.0f, 0.0f, 0.0f };
        if (flip_bar_dir) {
            flip_bar_tx += 0.1f;
            if (flip_bar_tx >= 0.8f)flip_bar_dir = !flip_bar_dir;
        }
        else {
            flip_bar_tx -= 0.1f;
            if (flip_bar_tx <= -0.8f)flip_bar_dir = !flip_bar_dir;
        }
    }
    break;
    case 7:
    {
        CameraPos = { 0.0f, 2.0f, 2.0f };
        CameraAt = { 0.0f, 0.0f, 0.0f };
        if (oil_timer) {
            oil_scale_y += 0.01f;
            if (oil_scale_y >= 0.5f) {
                oil_timer = !oil_timer;
                potato_fry_timer = true;
            }
        }
        if (potato_fry_timer) {
            flip_bar_tx += 0.05f;
            if (flip_bar_tx >= 0.8f) {
                potato_fry_timer = !potato_fry_timer;
                potato_cooked_finish = true;
            }
        }
    }
    break;
    }

    glutPostRedisplay();
    glutTimerFunc(60, TimerFunction, 1);
}

double mx, my;
double sx, sy;
double ex, ey;
bool left_down = 0;
float bread_angle = 0;
void Mouse(int button, int state, int x, int y)
{
    mx = ((double)x - WINDOWX / 2.0) / (WINDOWX / 2.0);
    my = -(((double)y - WINDOWY / 2.0) / (WINDOWY / 2.0));
    if (SCENE != 3)
    {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            printf("POS : (%f, %f, %f )\n", CameraPos.x, CameraPos.y, CameraPos.z);
            left_down = 1;
            sx = mx;
            sy = my;
        }
        if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
            printf("AT : (%f, %f, %f )\n", CameraAt.x, CameraAt.y, CameraAt.z);
            left_down = 0;
        }
    }
    else {
        if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
            printf("fryfan_rotate : (%f, %f, %f )\n", fryfan.rotate_x, fryfan.rotate_y, fryfan.rotate_z);
            printf("score =%f\n", score[3][0]);
            printf("bread_angle =%f\n", bread_angle);
        }
        if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {

        }

    }

}

void Motion(int x, int y)
{
    mx = ((double)x - WINDOWX / 2.0) / (WINDOWX / 2.0);
    my = -(((double)y - WINDOWY / 2.0) / (WINDOWY / 2.0));
    if (SCENE != 3) {
        if (left_down) {
            ex = mx;
            ey = my;
            CameraAt.x += (ex - sx);
            CameraAt.y += (ey - sy);
        }
    }
    else {
        if (jcnt == 0) {
            fryfan.rotate_x = -100.0f + fabs(atan2(mx, my));
            fryfan.rotate_z = fabs(atan2(mx, my));
            score[3][0] += fabs(bread_angle - atan2(mx, my));
            bread_angle = atan2(mx, my);
            bread[0].move = glm::vec3(1.3 * cos(bread_angle), 0, 1.3 * sin(bread_angle) - 1.3);
            bread[1].move = glm::vec3(1.3 * cos(bread_angle + glm::radians(90.0f)), 0, 1.3 * sin(bread_angle + glm::radians(90.0f)) - 1.3);
        }
    }

    glutPostRedisplay();
}

GLchar* filetobuf(const char* file) {
    FILE* fptr;
    long length;
    char* buf;
    fptr = fopen(file, "rb");
    if (!fptr)
        return NULL;
    fseek(fptr, 0, SEEK_END);
    length = ftell(fptr);
    buf = (char*)malloc(length + 1);
    fseek(fptr, 0, SEEK_SET);
    fread(buf, length, 1, fptr);
    fclose(fptr);
    buf[length] = 0;
    return buf;
}
void LoadOBJ_single(const char* filename, vector <Mesh>& out_mesh) {
    out_mesh.push_back({});
    vector<int> vertexindices, uvindices, normalindices;
    vector<GLfloat> temp_vertex;
    vector<GLfloat> temp_uvs;
    vector<GLfloat> temp_normals;
    ifstream in(filename, ios::in);
    if (in.fail()) {
        cout << "Impossible to open file" << endl;
        return;
    }
    while (!in.eof()) {
        string lineHeader;
        in >> lineHeader;
        if (lineHeader == "v") {
            glm::vec3 vertex;
            in >> vertex.x >> vertex.y >> vertex.z;
            temp_vertex.push_back(vertex.x);
            temp_vertex.push_back(vertex.y);
            temp_vertex.push_back(vertex.z);
        }
        else if (lineHeader == "vt") {
            glm::vec2 uv;
            in >> uv.x >> uv.y;
            temp_uvs.push_back(uv.x);
            temp_uvs.push_back(uv.y);
        }
        else if (lineHeader == "vn") {
            glm::vec3 normal;
            in >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal.x);
            temp_normals.push_back(normal.y);
            temp_normals.push_back(normal.z);
        }
        else if (lineHeader == "f") {
            string vertex1, vertex2, vertex3;
            unsigned int vertexindex[3], uvindex[3], normalindex[3];
            for (int k = 0; k < 3; ++k) {
                string temp, temp2;
                int cnt{ 0 }, cnt2{ 0 };
                in >> temp;
                while (1) {
                    while ((int)temp[cnt] != 47 && cnt < temp.size()) {
                        temp2 += (int)temp[cnt];
                        cnt++;
                    }
                    if ((int)temp[cnt] == 47 && cnt2 == 0) {
                        vertexindex[k] = atoi(temp2.c_str());
                        vertexindices.push_back(vertexindex[k]);
                        cnt++; cnt2++;
                        temp2.clear();
                    }
                    else if ((int)temp[cnt] == 47 && cnt2 == 1) {
                        uvindex[k] = atoi(temp2.c_str());
                        uvindices.push_back(uvindex[k]);
                        cnt++; cnt2++;
                        temp2.clear();
                    }
                    else if (temp[cnt] = '\n' && cnt2 == 2) {
                        normalindex[k] = atoi(temp2.c_str());
                        normalindices.push_back(normalindex[k]);
                        break;
                    }
                }
            }
        }
        else {
            continue;
        }
    }
    for (int i = 0; i < vertexindices.size(); ++i) {
        unsigned int vertexIndex = vertexindices[i];
        vertexIndex = (vertexIndex - 1) * 3;
        glm::vec3 vertex = { temp_vertex[vertexIndex], temp_vertex[vertexIndex + 1], temp_vertex[vertexIndex + 2] };
        out_mesh.back().vertex.push_back(vertex);
    }
    for (unsigned int i = 0; i < uvindices.size(); ++i) {
        unsigned int uvIndex = uvindices[i];
        uvIndex = (uvIndex - 1) * 2;
        glm::vec2 uv = { temp_uvs[uvIndex], temp_uvs[uvIndex + 1] };
        out_mesh.back().uvs.push_back(uv);
    }
    for (unsigned int i = 0; i < normalindices.size(); ++i) {
        unsigned int normalIndex = normalindices[i];
        normalIndex = (normalIndex - 1) * 3;
        glm::vec3 normal = { temp_normals[normalIndex], temp_normals[normalIndex + 1], temp_normals[normalIndex + 2] };
        out_mesh.back().normals.push_back(normal);
    }
}
void LoadOBJ(const char* filename, vector<Mesh>& out_mesh) {
    vector<int> vertexindices, uvindices, normalindices;
    vector<GLfloat> temp_vertex;
    vector<GLfloat> temp_uvs;
    vector<GLfloat> temp_normals;
    ifstream in(filename, ios::in);
    if (in.fail()) {
        cout << "Impossible to open file" << endl;
        return;
    }
    while (!in.eof()) {
        string lineHeader;
        in >> lineHeader;
        if (lineHeader == "v") {
            glm::vec3 vertex;
            in >> vertex.x >> vertex.y >> vertex.z;
            temp_vertex.push_back(vertex.x);
            temp_vertex.push_back(vertex.y);
            temp_vertex.push_back(vertex.z);
        }
        else if (lineHeader == "vt") {
            glm::vec2 uv;
            in >> uv.x >> uv.y;
            temp_uvs.push_back(uv.x);
            temp_uvs.push_back(uv.y);
        }
        else if (lineHeader == "vn") {
            glm::vec3 normal;
            in >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal.x);
            temp_normals.push_back(normal.y);
            temp_normals.push_back(normal.z);
        }
        else if (lineHeader == "usemtl") {
            while (!in.eof()) {
                in >> lineHeader;
                if (lineHeader == "f") {
                    string vertex1, vertex2, vertex3;
                    unsigned int vertexindex[3], uvindex[3], normalindex[3];
                    for (int k = 0; k < 3; ++k) {
                        string temp, temp2;
                        int cnt{ 0 }, cnt2{ 0 };
                        in >> temp;
                        while (1) {
                            while ((int)temp[cnt] != 47 && cnt < temp.size()) {
                                temp2 += (int)temp[cnt];
                                cnt++;
                            }
                            if ((int)temp[cnt] == 47 && cnt2 == 0) {
                                vertexindex[k] = atoi(temp2.c_str());
                                vertexindices.push_back(vertexindex[k]);
                                cnt++; cnt2++;
                                temp2.clear();
                            }
                            else if ((int)temp[cnt] == 47 && cnt2 == 1) {
                                uvindex[k] = atoi(temp2.c_str());
                                uvindices.push_back(uvindex[k]);
                                cnt++; cnt2++;
                                temp2.clear();
                            }
                            else if (temp[cnt] = '\n' && cnt2 == 2) {
                                normalindex[k] = atoi(temp2.c_str());
                                normalindices.push_back(normalindex[k]);
                                break;
                            }
                        }
                    }
                }
                else if (lineHeader == "#")break;
                else continue;
            }
            Mesh temp_mesh;
            for (int i = 0; i < vertexindices.size(); ++i) {
                unsigned int vertexIndex = vertexindices[i];
                vertexIndex = (vertexIndex - 1) * 3;
                glm::vec3 vertex = { temp_vertex[vertexIndex], temp_vertex[vertexIndex + 1], temp_vertex[vertexIndex + 2] };
                temp_mesh.vertex.push_back(vertex);
            }
            for (unsigned int i = 0; i < uvindices.size(); ++i) {
                unsigned int uvIndex = uvindices[i];
                uvIndex = (uvIndex - 1) * 2;
                glm::vec2 uv = { temp_uvs[uvIndex], temp_uvs[uvIndex + 1] };
                temp_mesh.uvs.push_back(uv);
            }
            for (unsigned int i = 0; i < normalindices.size(); ++i) {
                unsigned int normalIndex = normalindices[i];
                normalIndex = (normalIndex - 1) * 3;
                glm::vec3 normal = { temp_normals[normalIndex], temp_normals[normalIndex + 1], temp_normals[normalIndex + 2] };
                temp_mesh.normals.push_back(normal);
            }
            out_mesh.push_back(temp_mesh);
        }
    }

    cout << "obj load" << endl;
}

void LoadMTL(const char* FileName, const char* mtlFileName, vector<Mesh>& out_mesh, int& tex_cnt) {
    int curr_mesh = -1;
    ifstream in(mtlFileName, ios::in);

    if (in.fail()) {
        cout << "Impossible to open MTL file: " << mtlFileName << endl;
        return;
    }

    string line;
    string currentMaterial;

    while (getline(in, line)) {
        istringstream iss(line);
        string token;
        iss >> token;

        if (token == "newmtl") {
            iss >> currentMaterial;

            curr_mesh++;
            tex_cnt = curr_mesh;
        }/*
        else if (token == "Ka") {
            glm::vec3 ambientColor;
            iss >> ambientColor.x >> ambientColor.y >> ambientColor.z;
            out_mesh[curr_mesh].Ka = ambientColor;
        }
        else if (token == "Kd") {
            glm::vec3 diffuseColor;
            iss >> diffuseColor.x >> diffuseColor.y >> diffuseColor.z;
            out_mesh[curr_mesh].Kd = diffuseColor;
        }
        else if (token == "Ks") {
            glm::vec3 specularColor;
            iss >> specularColor.x >> specularColor.y >> specularColor.z;
            out_mesh[curr_mesh].Ks = specularColor;
        }
        else if (token == "Ns") {
            float shininess;
            iss >> shininess;
            out_mesh[curr_mesh].Ns = shininess;
        }
        else if (token == "d") {
            float dissolve;
            iss >> dissolve;
            out_mesh[curr_mesh].d = dissolve;
        }
        else if (token == "Ni") {
            float opticalDensity;
            iss >> opticalDensity;
            out_mesh[curr_mesh].Ni = opticalDensity;
        }
        else if (token == "illum") {
            int illuminationModel;
            iss >> illuminationModel;
            out_mesh[curr_mesh].illum = illuminationModel;
        }*/
        // Add more cases for other MTL properties if needed
        else if (token == "map_Kd") {
            string texs;
            iss >> texs;
            string texfile;
            texfile += FileName;
            texfile += '/';
            texfile += texs;
            out_mesh[curr_mesh].textureFile = texfile;
        }
    }

    cout << "mtl load" << endl;
}

void main(int argc, char** argv)
{
    srand((unsigned int)time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WINDOWX, WINDOWY);
    glutCreateWindow("종만이와 밍디");
    glewExperimental = GL_TRUE;
    glewInit();

    InitShader();
    InitBuffer();

    glutKeyboardFunc(keyboard);
    glutSpecialFunc(SpecialKeyboard);
    glutTimerFunc(60, TimerFunction, 1);
    glutDisplayFunc(drawScene);
    glutReshapeFunc(Reshape);
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);
    glutMainLoop();
}