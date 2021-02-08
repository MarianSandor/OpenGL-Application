#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>
#include <irrKlang.h>

#include <iostream>

#pragma comment(lib, "irrKlang.lib") // link with irrKlang.dll

bool introAnimation = true;
int animation_step = 0;

// sound
irrklang::ISoundEngine* soundEngine = irrklang::createIrrKlangDevice();
float gunDelay = 5.0f;
float gunTime;
const float GUN_TIME_DEC = 1.0f;
float gunTimeDec;
float stepDelay = 20.0f;
float stepTime;
const float STEP_TIME_DEC = 1.0f;
float stepTimeDec;
int no_steps = 4;
int step = 0;
bool isMoving;

// window
int windowWidth = 1280;
int windowHeight = 960;
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// spotlight
const int NR_SPOT_LIGHTS = 17;
glm::vec3 streetLampColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 flashlightColor = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 carLightsColor = glm::vec3(0.93f, 0.63f, 0.43f);
bool lightsOn = false;

// pointlight
const int NR_POINT_LIGHTS = 16;

// shadow parameters
const unsigned int SHADOW_WIDTH = 8192;
const unsigned int SHADOW_HEIGHT = 8192;
GLuint shadowMapFBO;
GLuint depthMapTexture;
bool showDepthMap;

// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;
// -0.178662 4.57324 -20.6872
// camera
gps::Camera myCamera(
    glm::vec3(-28.0f, 17.0f, -41.0f),
    glm::vec3(-27.5f, 16.5f, -40.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

bool isFPP = false;
GLfloat cameraY;
GLfloat CAMERA_SPEED = 0.1f;
GLfloat cameraSpeed; 
GLfloat cameraSensitivity = 0.1f;
bool firstMouse{ true };

GLfloat lastX{ (GLfloat)windowWidth / 2 };
GLfloat lastY{ (GLfloat)windowHeight / 2 };

GLfloat yaw{ 44.5 };
GLfloat lastYaw;
GLfloat pitch{ -20 };
GLfloat lastPitch;

// character movement
GLfloat jumpDist1 = 0.5;
GLfloat jumpDist2 = 1;
bool isCrouch = false;
bool isJump = false;
bool flashlightOn = false;
float jumpTime = 0;
const float JUMP_INC = 1.0f;
float jumpInc;
GLfloat yawAngle = 180;
GLfloat pitchAngle = 0;
glm::vec3 pos;
glm::vec3 lastPos = glm::vec3(0.6f, 0.46f, -27.2f);

GLboolean pressedKeys[1024];
GLboolean mouseLeft;

// models
gps::Model3D lightCube;
gps::Model3D ground;
gps::Model3D characterStanding;
gps::Model3D characterCrouch;
gps::Model3D characterJump1;
gps::Model3D characterJump2;
gps::Model3D house;
gps::Model3D build1;
gps::Model3D build2;
gps::Model3D building;
gps::Model3D europeanHouse;
gps::Model3D fireDepartment;
gps::Model3D GasStation;
gps::Model3D powerPlant;
gps::Model3D warehouse;
gps::Model3D apartment;
gps::Model3D roadMedium;
gps::Model3D roadCorner;
gps::Model3D wall;
gps::Model3D firetruck;
gps::Model3D lexus;
gps::Model3D jeep;
gps::Model3D taxi;
gps::Model3D streetLampSingle;
gps::Model3D streetLampDouble;
gps::Model3D screenQuad;
GLfloat angle = 0;
GLfloat dx = 0;
GLfloat dy = 0;
GLfloat dz = -25;
GLfloat scaleFactor = 1;
GLfloat scaleFactorInc = scaleFactor / 100;

// shaders
gps::Shader myBasicShader;
gps::Shader skyboxShader;
gps::Shader depthMapShader;
gps::Shader screenQuadShader;
gps::Shader lightShader;
gps::Shader crosshairShader;

//skybox
gps::SkyBox mySkyBoxDay;
gps::SkyBox mySkyBoxNight;
bool isDay = true;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

//crosshair
GLuint crosshairVAO, crosshairVBO, crosshairEBO;
GLfloat crosshairData[] =
{
    0.0025f, 0.02f, 0.0f, 
    0.0025f, -0.02f, 0.0f,
    -0.0025f, 0.02f, 0.0f,
    -0.0025f, -0.02f, 0.0f,
    0.02f, 0.0025f, 0.0f,
    -0.02f, 0.0025f, 0.0f,
    0.02f, -0.0025f, 0.0f,
    -0.02f, -0.0025f, 0.0f
};
GLuint crosshairIndices[] = {
    2, 1, 0,
    1, 2, 3,
    4, 5, 6,
    6, 5, 7,
};

bool wireframe = false;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
    windowWidth = width;
    windowHeight = height;

    glViewport(0, 0, width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        showDepthMap = !showDepthMap;

    if (key == GLFW_KEY_N && action == GLFW_PRESS) {
        isDay = !isDay;
        lightsOn = !isDay;
    }

    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        lightsOn = !lightsOn;
    }

    if (key == GLFW_KEY_O && action == GLFW_PRESS) {
        wireframe = !wireframe;
    }

    if (key == GLFW_KEY_B && action == GLFW_PRESS && !introAnimation) {
        if (!isFPP) {
            myCamera.setCameraPosition(lastPos);
            yaw = lastYaw;
            pitch = 0.0f;
            myCamera.rotate(pitch, yaw);
        }

        isFPP = !isFPP;
    }

    if (key == GLFW_KEY_W || key == GLFW_KEY_S || key == GLFW_KEY_A || key == GLFW_KEY_D && action == GLFW_PRESS) {
        if (!isMoving) {
            isMoving = true;
            stepTime = 0.0f;
        }
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        flashlightOn = !flashlightOn;
        soundEngine->play2D("sounds/gun/flashlight1.wav");
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        if (!isJump && !isCrouch) {
            soundEngine->play2D("sounds/jumps/pl_jump1.wav");
            isJump = true;
            jumpTime = 15;
        }
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS) {
        std::cout << myCamera.getCameraTarget().x << " " << myCamera.getCameraTarget().y << " " << myCamera.getCameraTarget().z << " <==> ";
        std::cout << myCamera.getCameraPosition().x << " " << myCamera.getCameraPosition().y << " " << myCamera.getCameraPosition().z << " <==> ";
        std::cout << yaw << "  " << pitch << std::endl;
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }

    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        soundEngine->play2D("sounds/ultimate_sounds/unstoppable_ultimate.wav");
    }

    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        soundEngine->play2D("sounds/ultimate_sounds/ultrakill_ultimate.wav");
    }

    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        soundEngine->play2D("sounds/ultimate_sounds/triplekill_ultimate.wav");
    }

    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        soundEngine->play2D("sounds/ultimate_sounds/rampage_ultimate.wav");
    }

    if (key == GLFW_KEY_5 && action == GLFW_PRESS) {
        soundEngine->play2D("sounds/ultimate_sounds/multikill_ultimate.wav");
    }

    if (key == GLFW_KEY_6 && action == GLFW_PRESS) {
        soundEngine->play2D("sounds/ultimate_sounds/monsterkill_ultimate.wav");
    }

    if (key == GLFW_KEY_7 && action == GLFW_PRESS) {
        soundEngine->play2D("sounds/ultimate_sounds/megakill_ultimate.wav");
    }

    if (key == GLFW_KEY_8 && action == GLFW_PRESS) {
        soundEngine->play2D("sounds/ultimate_sounds/ludicrouskill_ultimate.wav");
    }

    if (key == GLFW_KEY_9 && action == GLFW_PRESS) {
        soundEngine->play2D("sounds/ultimate_sounds/holyshit_ultimate.wav");
    }

    if (key == GLFW_KEY_0 && action == GLFW_PRESS) {
        soundEngine->play2D("sounds/ultimate_sounds/nm_goodbadugly.wav");
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!introAnimation) {
        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;
        lastX = xpos;
        lastY = ypos;

        xoffset *= cameraSensitivity;
        yoffset *= cameraSensitivity;

        yaw += xoffset;
        pitch += yoffset;

        yawAngle -= xoffset;
        pitchAngle -= yoffset;

        if (pitchAngle > 20.0f)
            pitchAngle = 20.0f;
        if (pitchAngle < -20.0f)
            pitchAngle = -20.0f;

        if (isFPP) {
            if (pitch > 20.0f)
                pitch = 20.0f;
            if (pitch < -20.0f)
                pitch = -20.0f;
        }
        else {
            if (pitch > 89.0f)
                pitch = 89.0f;
            if (pitch < -89.0f)
                pitch = -89.0f;

            lastYaw = yaw;
            lastPitch = pitch;
        }

        myCamera.rotate(pitch, yaw);

        view = myCamera.getViewMatrix();
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mouseLeft = true;
            gunTime = 0.0f;
        }
        else if (action == GLFW_RELEASE) {
            mouseLeft = false;
        }
    }
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W] && !introAnimation) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_S] && !introAnimation) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_A] && !introAnimation) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

	if (pressedKeys[GLFW_KEY_D] && !introAnimation) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix
        normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	}

    if (pressedKeys[GLFW_KEY_E] && !isFPP && !introAnimation) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_Q] && !isFPP && !introAnimation) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_LEFT_CONTROL] && !isJump) {
        isCrouch = true;
    }
    else {
        isCrouch = false;
    }

    if (!pressedKeys[GLFW_KEY_W] && !pressedKeys[GLFW_KEY_S] && !pressedKeys[GLFW_KEY_A] && !pressedKeys[GLFW_KEY_D]) {
        isMoving = false;
    }

    if (isFPP && isMoving && !isCrouch && !isJump) {
        if (stepTime <= 0.0f) {
            switch (step) {
                case 0: 
                    soundEngine->play2D("sounds/steps/pl_step1.wav");
                    break;
                case 1:
                    soundEngine->play2D("sounds/steps/pl_step2.wav");
                    break;
                case 2:
                    soundEngine->play2D("sounds/steps/pl_step3.wav");
                    break;
                case 3:
                    soundEngine->play2D("sounds/steps/pl_step4.wav");
                    break;
            }
            stepTime = stepDelay;
            step = (step + 1) % no_steps;
        }
        else {
            stepTime -= stepTimeDec;
        }
    }

    if (mouseLeft && isFPP) {
        if (gunTime <= 0.0f) {
            soundEngine->play2D("sounds/gun/m4a1_unsil-1.wav");
            gunTime = gunDelay;
        }
        else {
            gunTime -= gunTimeDec;
        }
    }

    if (pressedKeys[GLFW_KEY_KP_7]) {
        angle -= 0.5f;
    }

    if (pressedKeys[GLFW_KEY_KP_9]) {
        angle += 0.5f;
    }

    if (pressedKeys[GLFW_KEY_KP_8]) {
        dz -= 0.01f;
    }

    if (pressedKeys[GLFW_KEY_KP_2]) {
        dz += 0.01f;
    }

    if (pressedKeys[GLFW_KEY_KP_6]) {
        dx += 0.01f;
    }

    if (pressedKeys[GLFW_KEY_KP_4]) {
        dx -= 0.01f;
    }

    if (pressedKeys[GLFW_KEY_KP_3]) {
        dy += 0.01f;
    }

    if (pressedKeys[GLFW_KEY_KP_1]) {
        dy -= 0.01f;
    }

    if (pressedKeys[GLFW_KEY_KP_ADD]) {
        scaleFactor += scaleFactorInc;
    }

    if (pressedKeys[GLFW_KEY_KP_SUBTRACT]) {
        scaleFactor -= scaleFactorInc;
    }

    if (pressedKeys[GLFW_KEY_KP_0]) {
        std::cout << "dx: " << dx << " dy: " << dy << " dz: " << dz << " angle: " << angle << " scaleFactor: " << scaleFactor << std::endl;
    }
}

void initOpenGLWindow() {
    myWindow.Create(windowWidth, windowHeight, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    //added by me
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetMouseButtonCallback(myWindow.getWindow(), mouseButtonCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initCrosshair() 
{
    glGenVertexArrays(1, &crosshairVAO);
    glBindVertexArray(crosshairVAO);

    glGenBuffers(1, &crosshairVBO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairData), crosshairData, GL_STATIC_DRAW);

    glGenBuffers(1, &crosshairEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, crosshairEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(crosshairIndices), crosshairIndices, GL_STATIC_DRAW);

    //vertex position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void initFBO() 
{
    glGenFramebuffers(1, &shadowMapFBO);

    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initSkyBox()
{
    std::vector<const GLchar*> faces;

    faces.push_back("skybox/day/clouds1_east.bmp");
    faces.push_back("skybox/day/clouds1_west.bmp");
    faces.push_back("skybox/day/clouds1_up.bmp");
    faces.push_back("skybox/day/clouds1_down.bmp");
    faces.push_back("skybox/day/clouds1_north.bmp");
    faces.push_back("skybox/day/clouds1_south.bmp");

    mySkyBoxDay.Load(faces);

    faces.clear();
    
    faces.push_back("skybox/night/512/right.png");
    faces.push_back("skybox/night/512/left.png");
    faces.push_back("skybox/night/512/up.png");
    faces.push_back("skybox/night/512/down.png");
    faces.push_back("skybox/night/512/back.png");
    faces.push_back("skybox/night/512/front.png");
    
    mySkyBoxNight.Load(faces);
}

void initModels() {
    characterStanding.LoadModel("models/character/Cs/cs_modelStanding.obj");
    characterCrouch.LoadModel("models/character/Cs/cs_modelCrouch.obj");
    characterJump1.LoadModel("models/character/Cs/cs_modelJump1.obj");
    characterJump2.LoadModel("models/character/Cs/cs_modelJump2.obj");
    
    house.LoadModel("models/buildings/House/House_01.obj");
    build1.LoadModel("models/buildings/build1/simple building.obj");
    build2.LoadModel("models/buildings/build2/simple building.obj");
    building.LoadModel("models/buildings/building/building.obj");
    apartment.LoadModel("models/buildings/apartment/apartment.obj");
    GasStation.LoadModel("models/buildings/GasStation/GasStation.obj");
    powerPlant.LoadModel("models/buildings/PowerPlant/Power_Plant_Mid.obj");
    fireDepartment.LoadModel("models/buildings/Fire_Department/fd.obj");
    europeanHouse.LoadModel("models/buildings/EuropeanHouse/house_01.obj");
    warehouse.LoadModel("models/buildings/Warehouse_obj/warehouse.obj");

    roadMedium.LoadModel("models/street/road/RoadMedium.obj");
    roadCorner.LoadModel("models/street/road/RoadCorner.obj");

    wall.LoadModel("models/wall/brickWall.obj");
    
    firetruck.LoadModel("models/cars/firetruck/benz.obj");
    jeep.LoadModel("models/cars/jeep/Jeep_Renegade_2016.obj");
    taxi.LoadModel("models/cars/taxi/13914_Taxi_v2_L1.obj");
    lexus.LoadModel("models/cars/lexus/lexus_hs.obj");

    streetLampSingle.LoadModel("models/lights/steetLampSingle/Lamp.obj");
    streetLampDouble.LoadModel("models/lights/streetLampDouble/Lamp05.obj");
    
    ground.LoadModel("models/ground/ground.obj");

    screenQuad.LoadModel("models/quad/quad.obj");

    lightCube.LoadModel("models/cube/cube.obj");
}

void initShaders() {
	myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
    myBasicShader.useShaderProgram();

    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();

    depthMapShader.loadShader("shaders/shadow.vert", "shaders/shadow.frag");
    depthMapShader.useShaderProgram();

    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    screenQuadShader.useShaderProgram();

    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    lightShader.useShaderProgram();

    crosshairShader.loadShader("shaders/crosshair.vert", "shaders/crosshair.frag");
    crosshairShader.useShaderProgram();
}

void updateSpotlights()
{
    myBasicShader.useShaderProgram();

    for (int i = 0; i < NR_SPOT_LIGHTS; i++) {
        char buffer[64];
        
        if (i < 10) {
            sprintf(buffer, "spotlight[%i].constant", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1.0f);

            sprintf(buffer, "spotlight[%i].linear", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 0.09f);

            sprintf(buffer, "spotlight[%i].quadratic", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 0.032);

            sprintf(buffer, "spotlight[%i].cutOff", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), glm::cos(glm::radians(15.0f)));

            sprintf(buffer, "spotlight[%i].outerCutOff", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), glm::cos(glm::radians(28.0f)));

            sprintf(buffer, "spotlight[%i].color", i);
            if (lightsOn) 
                glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1, glm::value_ptr(streetLampColor));
            else
                glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1, glm::value_ptr(glm::vec3(0.0f)));
        }
        else if (i < 16) {
            sprintf(buffer, "spotlight[%i].constant", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1.0f);

            sprintf(buffer, "spotlight[%i].linear", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 0.14f);

            sprintf(buffer, "spotlight[%i].quadratic", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 0.07f);

            sprintf(buffer, "spotlight[%i].cutOff", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), glm::cos(glm::radians(15.0f)));

            sprintf(buffer, "spotlight[%i].outerCutOff", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), glm::cos(glm::radians(17.50f)));

            sprintf(buffer, "spotlight[%i].color", i);
            if (lightsOn) 
                glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1, glm::value_ptr(carLightsColor));
            else 
                glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1, glm::value_ptr(glm::vec3(0.0f)));
        }
        else {
            sprintf(buffer, "spotlight[%i].constant", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1.0f);

            sprintf(buffer, "spotlight[%i].linear", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 0.14f);

            sprintf(buffer, "spotlight[%i].quadratic", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 0.07f);

            sprintf(buffer, "spotlight[%i].cutOff", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), glm::cos(glm::radians(10.0f)));

            sprintf(buffer, "spotlight[%i].outerCutOff", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), glm::cos(glm::radians(12.0f)));

            sprintf(buffer, "spotlight[%i].color", i);
            if (flashlightOn && isFPP)
                glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1, glm::value_ptr(flashlightColor));
            else
                glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1, glm::value_ptr(glm::vec3(0.0f)));
        }
    }
 
    glm::vec3 target = glm::vec3(13.4902+2.75f, -5.0f, -10.820);
    glm::vec3 position = glm::vec3(13.4902, 4.09, -10.8202);
    glm::mat4 lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat3 lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[0].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[0].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[0].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[0].direction"), 1, glm::value_ptr(target));

    target = glm::vec3(12.8702 + 2.75f, -5.0f, 12.1 - 2.75f);
    position = glm::vec3(12.8702, 4.09, 12.1001);
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[1].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[1].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[1].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[1].direction"), 1, glm::value_ptr(target));

    target = glm::vec3(11.5102 - 2.75f, -5.0f, 13.4803 + 2.75f);
    position = glm::vec3(11.5102, 4.09, 13.4803);
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[2].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[2].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[2].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[2].direction"), 1, glm::value_ptr(target));

    target = glm::vec3(12.8402 + 2.75f, -5.0f, -33.9502 + 2.75f);
    position = glm::vec3(12.8402, 4.09, -33.9502);
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[3].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[3].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[3].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[3].direction"), 1, glm::value_ptr(target));

    target = glm::vec3(11.4502 - 2.75f, -5.0f, -35.34 - 2.75f);
    position = glm::vec3(11.4502, 4.09, -35.34);
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[4].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[4].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[4].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[4].direction"), 1, glm::value_ptr(target));

    target = glm::vec3(-12.0202 - 2.75f, -5.0f, -33.9302 + 2.75f);
    position = glm::vec3(-12.0202, 4.09, -33.9302);
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[5].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[5].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[5].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[5].direction"), 1, glm::value_ptr(target));

    target = glm::vec3(-10.6701 + 2.75f, -5.0f, -35.28 - 2.75f);
    position = glm::vec3(-10.6701, 4.09, -35.28);
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[6].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[6].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[6].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[6].direction"), 1, glm::value_ptr(target));

    target = glm::vec3(-12.5802 - 2.75f, -5.0f, -10.8202);
    position = glm::vec3(-12.5802, 4.09, -10.8202);
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[7].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[7].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[7].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[7].direction"), 1, glm::value_ptr(target));

    target = glm::vec3(-12.0702 - 2.75f, -5.0f, 12.0602 - 2.75f);
    position = glm::vec3(-12.0702, 4.09, 12.0602);
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[8].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[8].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[8].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[8].direction"), 1, glm::value_ptr(target));

    target = glm::vec3(-10.6902 + 2.75f, -5.0f, 13.4402 + 2.75f);
    position = glm::vec3(-10.6902, 4.09, 13.4402);
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[9].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[9].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[9].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[9].direction"), 1, glm::value_ptr(target));

    target = glm::vec3(-14.4802, -1, 5.0f);
    position = glm::vec3(-14.4802, -0.269002, 2.96);
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[10].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[10].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[10].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[10].direction"), 1, glm::value_ptr(target));

    target = glm::vec3(-15.7202, -1, 5.0f);
    position = glm::vec3(-15.7202, -0.269002, 2.96);
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[11].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[11].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[11].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[11].direction"), 1, glm::value_ptr(target));

    target = glm::vec3(14.7301 + 10.0f, -1, 10.0f);
    position = glm::vec3(14.7301, -0.449002, 14.7402);
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[12].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[12].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[12].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[12].direction"), 1, glm::value_ptr(target));

    target = glm::vec3(15.0401 + 10.0f, -1, 13.0f);
    position = glm::vec3(15.0401, -0.449002, 15.8402);
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[13].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[13].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[13].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[13].direction"), 1, glm::value_ptr(target));

    target = glm::vec3(10, -1, -35.6094);
    position = glm::vec3(1.73, -0.129002, -35.6094);
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[14].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[14].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[14].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[14].direction"), 1, glm::value_ptr(target));

    target = glm::vec3(10, -1, -36.6493);
    position = glm::vec3(1.77, -0.129002, -36.6493);
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[15].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[15].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[15].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[15].direction"), 1, glm::value_ptr(target));

    myCamera.move(gps::MOVE_FORWARD, 0.4f);
    myCamera.move(gps::MOVE_DOWN, 0.15f);
    myCamera.move(gps::MOVE_RIGHT, 0.15f);

    target = myCamera.getCameraTarget();
    position = myCamera.getCameraPosition();
    lightView = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));
    lightNormalMatrix = glm::mat3(glm::inverseTranspose(lightView * model));
    
    myCamera.move(gps::MOVE_LEFT, 0.15f);
    myCamera.move(gps::MOVE_UP, 0.15f);
    myCamera.move(gps::MOVE_BACKWARD, 0.4f);

    glUniformMatrix3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[16].normalM"), 1, GL_FALSE, glm::value_ptr(lightNormalMatrix));
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[16].view"), 1, GL_FALSE, glm::value_ptr(lightView));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[16].position"), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotlight[16].direction"), 1, glm::value_ptr(target));
}

void updatePointlights() {
    myBasicShader.useShaderProgram();

    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        char buffer[64];

        if (i < 10) {
            sprintf(buffer, "pointlight[%i].constant", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1.0f);

            sprintf(buffer, "pointlight[%i].linear", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 0.7f);

            sprintf(buffer, "pointlight[%i].quadratic", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1.8f);

            sprintf(buffer, "pointlight[%i].color", i);
            if (lightsOn)
                glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1, glm::value_ptr(streetLampColor));
            else
                glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1, glm::value_ptr(glm::vec3(0.0f)));
        }
        else {
            sprintf(buffer, "pointlight[%i].constant", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1.0f);

            sprintf(buffer, "pointlight[%i].linear", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 0.99f);

            sprintf(buffer, "pointlight[%i].quadratic", i);
            glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 100.0f);

            sprintf(buffer, "pointlight[%i].color", i);
            if (lightsOn)
                glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1, glm::value_ptr(carLightsColor));
            else
                glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, buffer), 1, glm::value_ptr(glm::vec3(0.0f)));
        }
    }

    glm::vec3 position = glm::vec3(13.4902, 3.8, -10.8202);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[0].position"), 1, glm::value_ptr(position));

    position = glm::vec3(12.8702, 3.8, 12.1001);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[1].position"), 1, glm::value_ptr(position));

    position = glm::vec3(11.5102, 3.8, 13.4803);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[2].position"), 1, glm::value_ptr(position));

    position = glm::vec3(12.8402, 3.8, -33.9502);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[3].position"), 1, glm::value_ptr(position));

    position = glm::vec3(11.4502, 3.8, -35.34);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[4].position"), 1, glm::value_ptr(position));

    position = glm::vec3(-12.0202, 3.8, -33.9302);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[5].position"), 1, glm::value_ptr(position));

    position = glm::vec3(-10.6701, 3.8, -35.28);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[6].position"), 1, glm::value_ptr(position));

    position = glm::vec3(-12.5802, 3.8, -10.8202);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[7].position"), 1, glm::value_ptr(position));

    position = glm::vec3(-12.0702, 3.8, 12.0602);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[8].position"), 1, glm::value_ptr(position));

    position = glm::vec3(-10.6902, 3.8, 13.4402);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[9].position"), 1, glm::value_ptr(position));

    position = glm::vec3(-14.4802, -0.269002, 3.10);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[10].position"), 1, glm::value_ptr(position));

    position = glm::vec3(-15.7202, -0.269002, 3.10);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[11].position"), 1, glm::value_ptr(position));

    position = glm::vec3(14.7801, -0.449002, 14.7902);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[12].position"), 1, glm::value_ptr(position));

    position = glm::vec3(15.0901, -0.449002, 15.8902);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[13].position"), 1, glm::value_ptr(position));

    position = glm::vec3(1.83, -0.129002, -35.6094);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[14].position"), 1, glm::value_ptr(position));
     
    position = glm::vec3(1.87, -0.129002, -36.6493);
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "pointlight[15].position"), 1, glm::value_ptr(position));
}

void initUniforms() {
	myBasicShader.useShaderProgram();

    // create model matrix
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix
    normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
                               (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
                               0.1f, 80.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));	

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(-27.0f, 10.0f, 20.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    updateSpotlights();
    updatePointlights();
}

glm::mat4 computeLightSpaceTrMatrixDay() {
    glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const GLfloat near_plane = 0.1f, far_plane = 130.0f;
    glm::mat4 lightProjection = glm::ortho(-60.0f, 40.0f, -20.0f, 50.0f, near_plane, far_plane);

    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

    return lightSpaceTrMatrix;
}

glm::mat4 computeLightSpaceTrMatrixNight() {
    glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const GLfloat near_plane = 0.1f, far_plane = 150.0f;
    glm::mat4 lightProjection = glm::ortho(-40.0f, 62.5f, -30.0f, 50.0f, near_plane, far_plane);

    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;

    return lightSpaceTrMatrix;
}

void renderCharacter(gps::Shader shader, bool depthPass) {
    //dy_jump2 = -9.43
    //dy_jump1 = -9.11
    //dy_crouch = -10
    //dy_standing = -8.94

    static GLfloat lastYawAngle = 180.0f;

    // select active shader program
    shader.useShaderProgram();

    if (isFPP) {
        pos = myCamera.getCameraPosition();
        lastPos = pos;
        lastYawAngle = yawAngle;

        model = glm::translate(glm::mat4(1.0f), glm::vec3(pos));
        model = glm::rotate(model, glm::radians(yawAngle), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(pitchAngle), glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(-pos));
    }
    else {
        pitchAngle = 0.0f;
        model = glm::translate(glm::mat4(1.0f), glm::vec3(lastPos));
        model = glm::rotate(model, glm::radians(lastYawAngle), glm::vec3(0, 1, 0));
        model = glm::translate(model, glm::vec3(-lastPos));
        
        pos = lastPos;
    }

    pos.x += 0.15;
    pos.z += -0.05;

    if (isCrouch) {
        pos.y = -10;
        model = glm::translate(model, glm::vec3(pos));

        glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        if (!depthPass) {
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            updateSpotlights();
            updatePointlights();
        }

        characterCrouch.Draw(shader);
    }
    else if (isJump) {
        if (jumpTime > 11 || jumpTime < 4) {
            pos.y = -9.11;
            pos.y += jumpDist1;
            model = glm::translate(model, glm::vec3(pos));

            glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

            if (!depthPass) {
                normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
                glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
                updateSpotlights();
                updatePointlights();
            }

            characterJump1.Draw(shader);
        }
        else {
            pos.y = -9.43;
            pos.y += jumpDist2;
            model = glm::translate(model, glm::vec3(pos));

            glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

            if (!depthPass) {
                normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
                glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
                updateSpotlights();
                updatePointlights();
            }

            characterJump2.Draw(shader);
        }
    }
    else {
        pos.y = -8.94;
        model = glm::translate(model, glm::vec3(pos));

        glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

        if (!depthPass) {
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
            updateSpotlights();
            updatePointlights();
        }

        characterStanding.Draw(shader);
    }
}

void renderGround(gps::Shader shader, bool depthPass)
{
    // select active shader program
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    // draw character
    ground.Draw(shader);
}

void renderHouse_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 6.66f, -50.0f));
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.002));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    house.Draw(shader);
}

void renderHouse_2(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 6.66f, -13.5f));
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.002));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    house.Draw(shader);
}

void renderBuild1_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-17.5f, 3.35f, -50.33f));
    model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(4.35f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    build1.Draw(shader);
}

void renderBuild1_2(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(29.0f, 3.35f, -21.0f));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(4.35f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }
    build1.Draw(shader);
}

void renderBuild2_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(17.5, 3.35f, -50.33f));
    model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(4.35f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    build2.Draw(shader);
}

void renderBuild2_2(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(29.0f, 3.35f, -5.0f));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(4.35f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    build2.Draw(shader);
}

void renderApartment_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(28.7f, -1.01f, -36.0f));
    model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.50));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    apartment.Draw(shader);
}

void renderWarehouse_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-26.5f, 1.18f, -32.75f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.05f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    warehouse.Draw(shader);
}

void renderBuilding_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.3f, 3.60f, 25.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.086));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    building.Draw(shader);
}

void renderGasStation_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-27.5f, -1.01f, 0.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.011f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    GasStation.Draw(shader);
}

void renderPowerPlant_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(29.3, -1.01f, 12.5f));
    model = glm::scale(model, glm::vec3(0.3));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    powerPlant.Draw(shader);
}

void renderFireDepartment_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-29.5f, -1.08f, -16.0f));
    model = glm::rotate(model, glm::radians(90.f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.01f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    fireDepartment.Draw(shader);
}

void renderEuropeanHouse_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-26.5f, -1.0f, 13.63f));
    model = glm::rotate(model, glm::radians(270.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.05f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    europeanHouse.Draw(shader);
}

void renderBuildings(gps::Shader shader, bool depthPass)
{
    renderBuild1_1(shader, depthPass);
    renderBuild1_2(shader, depthPass);
    renderBuild2_1(shader, depthPass);
    renderBuild2_2(shader, depthPass);
    renderHouse_1(shader, depthPass);
    renderHouse_2(shader, depthPass);
    renderWarehouse_1(shader, depthPass);
    renderApartment_1(shader, depthPass);
    renderFireDepartment_1(shader, depthPass);
    renderGasStation_1(shader, depthPass);
    renderEuropeanHouse_1(shader, depthPass);
    renderBuilding_1(shader, depthPass);
    renderPowerPlant_1(shader, depthPass);
}

void renderRoad_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-14.0f, -0.95f, -8.15005));
    model = glm::scale(model, glm::vec3(2.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    roadMedium.Draw(shader);
}

void renderRoad_2(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-14.0f, -0.95f, -27.92f));
    model = glm::scale(model, glm::vec3(2.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    roadMedium.Draw(shader);
}

void renderRoad_3(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-8.26991, -0.95f, -37.2493));
    model = glm::rotate(model, glm::radians(-270.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(2.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    roadMedium.Draw(shader);
}

void renderRoad_4(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-9.08011, -0.95f, 15.425f));
    model = glm::rotate(model, glm::radians(-270.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(2.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    roadMedium.Draw(shader);
}

void renderRoad_5(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(14.8404, -0.95f, -8.15005));
    model = glm::scale(model, glm::vec3(2.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    roadMedium.Draw(shader);
}

void renderRoad_6(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(14.8404, -0.95f, -27.92f));
    model = glm::scale(model, glm::vec3(2.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    roadMedium.Draw(shader);
}

void renderRoadCorner_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-11.92f, -0.95f, 13.5502));
    model = glm::scale(model, glm::vec3(2.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    roadCorner.Draw(shader);
}

void renderRoadCorner_2(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-12.13, -0.95f, -35.1696f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(2.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    roadCorner.Draw(shader);
}

void renderRoadCorner_3(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(12.9802, -0.95f, 13.325f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(2.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    roadCorner.Draw(shader);
}

void renderRoadCorner_4(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(12.7604, -0.95f, -35.3896));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(2.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    roadCorner.Draw(shader);
}

void renderStreets(gps::Shader shader, bool depthPass)
{
    renderRoad_1(shader, depthPass);
    renderRoad_2(shader, depthPass);
    renderRoad_3(shader, depthPass);
    renderRoad_4(shader, depthPass);
    renderRoad_5(shader, depthPass);
    renderRoad_6(shader, depthPass);
    renderRoadCorner_1(shader, depthPass);
    renderRoadCorner_2(shader, depthPass);
    renderRoadCorner_3(shader, depthPass);
    renderRoadCorner_4(shader, depthPass);
}

void renderWall_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(32.9205, -0.869999, 0.930362));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.015f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    wall.Draw(shader);
}

void renderWall_2(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(32.9205, -0.869999, -15.4199));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.015f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    wall.Draw(shader);
}

void renderWall_3(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(32.9205, -0.869999, -26.4701));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.015f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    wall.Draw(shader);
}

void renderWall_4(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(33.6604, -0.869999, -45.8889));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.015f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    wall.Draw(shader);
}

void renderWall_5(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(25.1705, -0.869999, -54.0076));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.015f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    wall.Draw(shader);
}

void renderWall_6(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(7.74008, -0.869999, -53.781));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.015f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    wall.Draw(shader);
}

void renderWall_7(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-7.17007, -0.869999, -53.7011));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.015f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    wall.Draw(shader);
}

void renderWall_8(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-23.4304, -0.869999, -50.6416));
    model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.015f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    wall.Draw(shader);
}

void renderWall_9(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-31.6903, -0.869999, -42.133));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.015f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    wall.Draw(shader);
}

void renderWall_10(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-31.6603, -0.869999, -25.5212));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.015f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    wall.Draw(shader);
}

void renderWall_11(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-30.5002, -0.869999, -1.54987));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.015f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    wall.Draw(shader);
}

void renderWall_12(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-30.4702, -0.869999, 15.0302));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.015f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    wall.Draw(shader);
}

void renderWall_13(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-21.91, -0.869999, 23.3202));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.015f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    wall.Draw(shader);
}

void renderWall_14(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(23.7501, -0.869999, 23.3202));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.015f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    wall.Draw(shader);
}

void renderWall_15(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(32.0307, -0.869999, 15.63));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.015f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    wall.Draw(shader);
}

void renderWalls(gps::Shader shader, bool depthPass)
{
    renderWall_1(shader, depthPass);
    renderWall_2(shader, depthPass);
    renderWall_3(shader, depthPass);
    renderWall_4(shader, depthPass);
    renderWall_5(shader, depthPass);
    renderWall_6(shader, depthPass);
    renderWall_7(shader, depthPass);
    renderWall_8(shader, depthPass);
    renderWall_9(shader, depthPass);
    renderWall_10(shader, depthPass);
    renderWall_11(shader, depthPass);
    renderWall_12(shader, depthPass);
    renderWall_13(shader, depthPass);
    renderWall_14(shader, depthPass);
    renderWall_15(shader, depthPass);
}

void renderJeep(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.05, -0.949999, -36.1306));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.94));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    jeep.Draw(shader);
}

void renderTaxi_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(12.8602, -0.95, 16.0602));
    model = glm::rotate(model, glm::radians(-160.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.0086));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    taxi.Draw(shader);
}

void renderTaxi_2(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(12.7398, -0.87, -15));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-85.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.0086));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    taxi.Draw(shader);
}

void renderLexus(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-15.1102, -0.77, 1.04997));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.0539999));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    lexus.Draw(shader);
}

void renderFiretruck(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-15.2303, -1.01, -7.23007));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));
    model = glm::scale(model, glm::vec3(0.012f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    firetruck.Draw(shader);
}

void renderCars(gps::Shader shader, bool depthPass)
{
    renderJeep(shader, depthPass);
    renderTaxi_1(shader, depthPass);
    renderTaxi_2(shader, depthPass);
    renderLexus(shader, depthPass);
    renderFiretruck(shader, depthPass);
}

void renderStreetLampSingle_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-11.5902, -2.50f, -10.8302));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.1f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    streetLampSingle.Draw(shader);
}

void renderStreetLampSingle_2(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(12.4902, -2.50f, -10.8302));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.1f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    streetLampSingle.Draw(shader);
}

void renderStreetLampDouble_1(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(12.1902, -2.50f, 12.7702));
    model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.1f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    streetLampDouble.Draw(shader);
}

void renderStreetLampDouble_2(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(12.1502, -2.50f, -34.6402));
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.1f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    streetLampDouble.Draw(shader);
}

void renderStreetLampDouble_3(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-11.3802, -2.50f, 12.7401));
    model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.1f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    streetLampDouble.Draw(shader);
}

void renderStreetLampDouble_4(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-11.3502, -2.50f, -34.6026));
    model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0, 1, 0));
    model = glm::scale(model, glm::vec3(0.1f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        updateSpotlights();
        updatePointlights();
    }

    streetLampDouble.Draw(shader);
}

void renderStreetLamps(gps::Shader shader, bool depthPass)
{
    renderStreetLampSingle_1(shader, depthPass);
    renderStreetLampSingle_2(shader, depthPass);

    renderStreetLampDouble_1(shader, depthPass);
    renderStreetLampDouble_2(shader, depthPass);
    renderStreetLampDouble_3(shader, depthPass);
    renderStreetLampDouble_4(shader, depthPass);
}

void drawObjects(gps::Shader shader, bool depthPass)
{
    shader.useShaderProgram();

    // render the character
    renderCharacter(shader, depthPass);

    // render the buildings
    renderBuildings(shader, depthPass);

    // render the streets
    renderStreets(shader, depthPass);

    // render the street lamps
    renderStreetLamps(shader, depthPass);

    // render the cars
    renderCars(shader, depthPass);

    // render the walls
    renderWalls(shader, depthPass);

    // render ground
    renderGround(myBasicShader, depthPass);
}

void drawCrosshair() {
    crosshairShader.useShaderProgram();
    
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    glBindVertexArray(crosshairVAO);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
}

void drawLightCube()
{
    lightShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    model = glm::translate(glm::mat4(1.0f), glm::vec3(dx, dy, dz));
    model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    lightCube.Draw(lightShader);
}

void renderScene() {
    // set the light and update the uniforms
    glm::mat4 lightSpaceTrMatrix;

    if (isDay) {
        lightDir = glm::vec3(-47.0f, 30.0f, 40.0f);
        lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
        lightSpaceTrMatrix = computeLightSpaceTrMatrixDay();
    }
    else {
        lightDir = glm::vec3(43.3398f, 65.1561f, 45.5218f);
        lightColor = glm::vec3(0.1f, 0.1f, 0.1f); //dark light
        lightSpaceTrMatrix = computeLightSpaceTrMatrixNight();
    }

    myBasicShader.useShaderProgram();
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    // render the scene in the depth map
    depthMapShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(lightSpaceTrMatrix));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    drawObjects(depthMapShader, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//render the scene (with shadows)
    if (showDepthMap) {
        glViewport(0, 0, windowWidth, windowHeight);

        glClear(GL_COLOR_BUFFER_BIT);

        screenQuadShader.useShaderProgram();

        //bind the depth map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

        glDisable(GL_DEPTH_TEST);
        screenQuad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);
    }
    else {
        if (wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glViewport(0, 0, windowWidth, windowHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        myBasicShader.useShaderProgram();

        view = myCamera.getViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

        glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
            1,
            GL_FALSE,
            glm::value_ptr(lightSpaceTrMatrix));

        drawObjects(myBasicShader, false);

        if (!introAnimation) {
            drawCrosshair();
        }

        // draw light cube at light position
        //drawLightCube();
    }
   
    //render skybox
    if (isDay)
        mySkyBoxDay.Draw(skyboxShader, view, projection);
    else
        mySkyBoxNight.Draw(skyboxShader, view, projection);
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
    soundEngine->drop();
}

void updateDeltaTime() {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    cameraSpeed = CAMERA_SPEED + CAMERA_SPEED * deltaTime;
    jumpInc = JUMP_INC + JUMP_INC * deltaTime;
    gunTimeDec = GUN_TIME_DEC + GUN_TIME_DEC * deltaTime;
    stepTimeDec = STEP_TIME_DEC + STEP_TIME_DEC * deltaTime;
}

void processCharacterMovement() {
    if (jumpTime <= 0) {
        isJump = false;
    }
    else {
        jumpTime -= jumpInc;
    }

    if (isCrouch) {
        cameraY = -0.05f;
    }
    else if (isJump) {
        if (jumpTime > 11 || jumpTime < 4) {
            cameraY = 0.84f;
        }
        else {
            cameraY = 0.99f;
        }
    }
    else {
        cameraY = 0.46f;
    }
}

void processIntroAnimation() {
    glm::vec3 cameraPos;

    switch (animation_step)
    {
    case 0:
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        yaw -= 4 * cameraSensitivity;
        myCamera.rotate(pitch, yaw);

        cameraPos = myCamera.getCameraPosition();
        if (cameraPos.x > -15.0f) {
            cameraPos.x = -15.0f;
            myCamera.setCameraPosition(cameraPos);
            animation_step++;
        }
        break;

    case 1: 
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        myCamera.move(gps::MOVE_DOWN, cameraSpeed / 4.0f);
        yaw -= 5 * cameraSensitivity;
  
        if (yaw < -90.0f) {
            yaw = -90.0f;
            animation_step++;
        }
        myCamera.rotate(pitch, yaw);
        break;

    case 2:
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        myCamera.move(gps::MOVE_DOWN, cameraSpeed / 4.0f);
        pitch += 5 * cameraSensitivity;

        if (pitch > 0.0f) {
            pitch = 0.0f;
            animation_step++;
        }
        myCamera.rotate(pitch, yaw);
        break;
    
    case 3: 
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);

        cameraPos = myCamera.getCameraPosition();
        if (cameraPos.y < 0.46) {
            cameraPos.y = 0.46f;
            myCamera.setCameraPosition(cameraPos);
            animation_step++;
        }
        break;

    default:
        introAnimation = false;
        isFPP = true;
        lastYaw = yaw;
        lastPos = myCamera.getCameraPosition();
        break;
    } 

    //update view matrix
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    // compute normal matrix
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initCrosshair();
    initFBO();
    initSkyBox();
	initModels();
	initShaders();
	initUniforms();
    setWindowCallbacks();

	glCheckError();

    if (!soundEngine)
        return 0; // error starting up the engine
    
    // for developers only
    //soundEngine->play2D("sounds/ultimate_sounds/nm_goodbadugly.wav");

	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        //per-frame time logic
        updateDeltaTime();

        processCharacterMovement();

        processMovement();

        if (isFPP) {
            glm::vec3 cameraPos = myCamera.getCameraPosition();
            cameraPos.y = cameraY;
            myCamera.setCameraPosition(cameraPos);
        }

        if (introAnimation) {
            processIntroAnimation();
        }

	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
