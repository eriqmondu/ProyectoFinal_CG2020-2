#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"



#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"
#include "texture.h"

// Special headers for the FBX animated models
#include "shader_m.h"
#include "model_m.h"

#include <iostream>

// Internal Windows sound libraries
#include <windows.h>
#include <mmsystem.h>
#include <mciapi.h>
#pragma comment(lib, "Winmm.lib")

// Function definition
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void gameMode(GLFWwindow* window);
unsigned int loadTexture(const char* path, bool gammaCorrection);
void renderQuad();
void renderCube();
void inputKeyframes(GLFWwindow* window);
void initialKeyFrames();

// Music definition
void mainMusic();
void macarenaMusic();

//Keep a 16:9 resolution, please.
const unsigned int SCR_WIDTH = 640;
const unsigned int SCR_HEIGHT = 360;

// Bloom effect switch
bool bloom = true;
bool bloomKeyPressed = false;
float exposure = 1.0f;

bool keys[1024];

//Initial Scene camera
Camera cameraScene(2.8636f, 5.2606f, -1.9636f, 0.0f, 1.0f, 0.0f, 93.7998, -26.50);
bool camScen = false;
int cam = 0;

// Initial camera location
glm::vec3 posCam(3.3f, 5.5f, 1.8f);
Camera camera(posCam);

//Animation flags
float playAnimation, enableAnimation;

float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

int animationCount = 0;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

float elapsedTime = 0.0f;

// Other variables
float movX, movY, movZ;
float movX2, movY2, movZ2;

bool animacion = false;
bool game = true;
///////////////////////////////KEYFRAMES/////////////////////


//NEW// Keyframes
float posXcam = posCam.x, posYcam = posCam.y, posZcam = posCam.z;
float movCam_x = 0.0f, movCam_y = 0.0f, movCam_z = 0.0f;

#define MAX_FRAMES 60       //Max number of keyframes
int i_max_steps = 90;
int i_curr_steps = 23;      //Frames between keyframes
typedef struct _frame
{
    //Variables to save Key Frames
    float movCam_x;     //Variable to PositionX
    float movCam_y;     //Variable to PositionY
    float movCam_z;     //Variable to PositionZ
    float movCam_xInc;      //Variable to IncrementX
    float movCam_yInc;      //Variable to IncrementY
    float movCam_zInc;      //Variable to IncrementZ
}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 200;            //Set number of Keyframes (originally 60)
bool play_key = false;
int playIndex = 0;

void resetElements(void)
{
    movCam_x = KeyFrame[0].movCam_x;
    movCam_y = KeyFrame[0].movCam_y;
    movCam_z = KeyFrame[0].movCam_z;
    //animationCount = 0;
    cameraScene.changeScene(2.8636f, 5.2608f, -1.9636f, 93.7998f, -26.50);
}

void interpolation(void)
{
    KeyFrame[playIndex].movCam_xInc = (KeyFrame[playIndex + 1].movCam_x - KeyFrame[playIndex].movCam_x) / i_max_steps;
    KeyFrame[playIndex].movCam_yInc = (KeyFrame[playIndex + 1].movCam_y - KeyFrame[playIndex].movCam_y) / i_max_steps;
    KeyFrame[playIndex].movCam_zInc = (KeyFrame[playIndex + 1].movCam_z - KeyFrame[playIndex].movCam_z) / i_max_steps;
}


void animate(void)
{
    //Object movement
    if (play_key)
    {
        if (i_curr_steps >= i_max_steps) //end of animation between frames?
        {
            playIndex++;
            printf("playindex : %d\n", playIndex);
            if (playIndex > FrameIndex - 2) //end of total animation?
            {
                printf("Frame index= %d\n", FrameIndex);
                printf("End animation\n");
                playIndex = 0;
                play_key = false;
            }
            else //Next frame interpolations
            {
                i_curr_steps = 0; //Reset counter
                //Interpolation
                interpolation();
            }
        }
        else
        {
            //printf("Last Position\n");
            //printf("max steps: %f", i_max_steps);
            //Draw animation
            movCam_x += KeyFrame[playIndex].movCam_xInc;
            movCam_y += KeyFrame[playIndex].movCam_yInc;
            movCam_z += KeyFrame[playIndex].movCam_zInc;
            i_curr_steps++;
        }
    }
}
/*End Keyframes code*/

// Music functiones

void mainMusic() {
    //www.youtube.com/watch?v=SFxj6CZM2jE
    PlaySound(NULL, NULL, SND_ASYNC);
    PlaySound(TEXT("epic_music.wav"), NULL, SND_FILENAME | SND_LOOP | SND_ASYNC);
}

void macarenaMusic() {
    PlaySound(NULL, NULL, SND_ASYNC);
    //www.youtube.com/watch?v=obUHDyWFMi8
    PlaySound(TEXT("macarena.wav"), NULL, SND_FILENAME | SND_LOOP | SND_ASYNC);
}

int main()
{
    //mainMusic();
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Creating a OpenGL context with GFLW
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Proyecto Final CG 2020-2", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Enabling and testing OpenGL
    // ---------------------------
    glEnable(GL_DEPTH_TEST);


    // Build and compile the shaders for the Blur and Bloom effect
    // -----------------------------------------------------------
    Shader shader("Shaders/7.bloom.vs", "Shaders/7.bloom.fs");
    Shader shaderLight("Shaders/7.bloom.vs", "Shaders/7.light_box.fs");
    Shader shaderBlur("Shaders/7.blur.vs", "Shaders/7.blur.fs");
    Shader shaderBloomFinal("Shaders/7.bloom_final.vs", "Shaders/7.bloom_final.fs");

    // Skybox shader
    // -----------------------------
    Shader skyboxShader("Shaders/SkyBox.vs", "Shaders/SkyBox.frag");

    // Animated skinning FBX shader (thanks Sergio Vite!)
    // --------------------------------------------------
    ShaderAnim ourShader("Shaders/vertex_skinning.vs", "Shaders/fragment_skinning.fs");
    #define MAX_RIGGING_BONES 100
    ourShader.setBonesIDs(MAX_RIGGING_BONES);


    // OBJ model loading
    // -----------------
    std::cout << "Loading OBJ Models..." << std::endl;
    Model edificio((char*)"Models/Ciudad_final.obj");

    // Floating text objects
    Model start((char*)"Models/Text/start_running.obj");
    Model jump1((char*)"Models/Text/1.obj");
    Model jump_run1((char*)"Models/Text/2.obj");
    Model jump_slide((char*)"Models/Text/3.obj");
    Model cool((char*)"Models/Text/4.obj");
    Model jump_again((char*)"Models/Text/5.obj");
    Model dive((char*)"Models/Text/6.obj");
    Model fly((char*)"Models/Text/7.obj");
    Model macarena((char*)"Models/Text/8.obj");

    // FBX model loading
    // -----------------
    std::cout << "Loading FBX Models..." << std::endl;
    ModelAnim batman("Models/Batman_test.fbx");



    // Initial bones transformation
    // -----------------------------
    glm::mat4 gBones[MAX_RIGGING_BONES];

    batman.SetPose(0.0f, gBones);

    float fps = (float)batman.getFramerate();
    int keys = (int)batman.getNumFrames();

    //int animationCount = 0;


    // Skybox texture face loading
    // ---------------------------
    std::cout << "Loading Skybox faces..." << std::endl;
    vector<const GLchar*> faces;
    faces.push_back("SkyBox/sky/right.jpg");
    faces.push_back("SkyBox/sky/left.jpg");

    faces.push_back("SkyBox/sky/up.jpg");
    faces.push_back("SkyBox/sky/bottom.jpg");

    faces.push_back("SkyBox/sky/front.jpg");
    faces.push_back("SkyBox/sky/back.jpg");

    // Loading all the faces to the cube
    // -----------------------------
    GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);

    // Load textures
    // -------------
    //unsigned int woodTexture = loadTexture("resources/textures/wood.png", true); // note that we're loading the texture as an SRGB texture
    //unsigned int containerTexture = loadTexture("resources/textures/container2.png", true); // note that we're loading the texture as an SRGB texture

    GLfloat skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    +1.0f, -1.0f, -1.0f,
    +1.0f, -1.0f, -1.0f,
    +1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    +1.0f, -1.0f, -1.0f,
    +1.0f, -1.0f,  1.0f,
    +1.0f,  1.0f,  1.0f,
    +1.0f,  1.0f,  1.0f,
    +1.0f,  1.0f, -1.0f,
    +1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    +1.0f,  1.0f,  1.0f,
    +1.0f,  1.0f,  1.0f,
    +1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
    +1.0f,  1.0f, -1.0f,
    +1.0f,  1.0f,  1.0f,
    +1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    +1.0f, -1.0f, -1.0f,
    +1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    +1.0f, -1.0f,  1.0f
    };

    // Configure Skybox
    // -----------------------------
    GLuint skyboxVBO, skyboxVAO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

    // Configure (floating point) framebuffers
    // ---------------------------------------
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness treshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }

    // Create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    // Finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring effect
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // We clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // Also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    // Lighting info
    // -------------

    // Positions (only if static, otherwise insert lightPositions inside the render loop)
    //std::vector<glm::vec3> lightPositions;
    //lightPositions.push_back(glm::vec3(0.0f, 2.0f, 0.0f));
    //lightPositions.push_back(glm::vec3(0.0f, 5.0f, 0.0f));
    //lightPositions.push_back(glm::vec3(0.0f, 5.0f, 3.0f));

    // Colors (only if static, otherwise insert lightColors inside the render loop)
    std::vector<glm::vec3> lightColors;
    lightColors.push_back(glm::vec3(10.0, 10.0, 10.0));
    lightColors.push_back(glm::vec3(54.9f, 31.2f, 20.8f));


    // Bloom and Blur shaders configuration
    // ------------------------------------
    shader.use();
    shader.setInt("diffuseTexture", 0);
    shaderBlur.use();
    shaderBlur.setInt("image", 0);
    shaderBloomFinal.use();
    shaderBloomFinal.setInt("scene", 0);
    shaderBloomFinal.setInt("bloomBlur", 1);

    //Function to call initial Keyframes
    initialKeyFrames();

    // Render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // Just a noob debug var here 
        //printf("X= %0.2f Y= %0.2f, Z= %0.2f\n", movX2, movY2, movZ2);

        // Interactive light positions
        std::vector<glm::vec3> lightPositions;
        lightPositions.push_back(glm::vec3(0.40f, 5.00f, -0.40f));
        //lightPositions.push_back(glm::vec3(0.80f, 4.8f, 0.8f));
        lightPositions.push_back(glm::vec3(sin(glfwGetTime() / 2) * 3, -1.0f, -sin(glfwGetTime() / 2) * 3));
        //lightPositions.push_back(glm::vec3(movX2, movY2, movZ2));


        // Per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        elapsedTime += deltaTime;
        if (elapsedTime > 1.0f / fps) {
            if (animacion) {
                animationCount++;
                if (animationCount > keys - 1) {
                    animationCount = 0;
                    animacion = false;
                }
                cout << "Second: " << animationCount << endl;
                batman.SetPose((float)animationCount, gBones);
                elapsedTime = 0.0f;
            }
        }

        // input
        // -----
        processInput(window);
        
        if (game) {
            gameMode(window);
        }


        /*Scence changes*/

        switch (animationCount)
        {
        case 0:
            cameraScene.changeScene(2.8636f, 5.2608f, -1.9636f, 93.7998f, -26.50);
        break;
        case 135:
            cameraScene.changeScene(3.0616f, 5.1799f, -1.5149f, -180.0f, -26.49f);
            break;
        case 231:
            cameraScene.changeScene(2.4075f, 5.4863f, -1.7135f, 88.99f, -69.60f);
            break;
        case 250: //New eriq camera
            cameraScene.changeScene(2.029508f, 3.613027f, -1.581665f, -329.300049f, 37.300014f);
            break;
        case 320: // Go back to camera at the same position as frame 231
            cameraScene.changeScene(2.4075f, 5.4863f, -1.7135f, 88.99f, -69.60f);
            break;
        case 335:
            cameraScene.changeScene(1.9850f, 3.1212f, -2.8479f, 128.8997f, 7.10f);
            break;
        case 552:
            cameraScene.changeScene(-0.5924f, 3.1663f, -0.7010f, -23.7999f, -9.7999);
            break;
        //case 670:
        //    cameraScene.changeScene(-1.3569f, 3.1977f, 0.1210f, -41.80f, -13.1999f);
        //    break;
        case 660: // New
            cameraScene.changeScene(0.057982f, 2.509783f, -0.995944, -617.000549, 53.099998);
            break;
        case 700:
            cameraScene.changeScene(-1.3569f, 3.1977f, 0.1210f, -41.80f, -13.1999f);
            break;
        case 733: //New
            cameraScene.changeScene(-0.647285, 2.735793, 0.155906, -486.000427, 43.100075);
            break;
        case 768:
            cameraScene.changeScene(-0.8170f, 3.3595f, -2.0336f, 128.6998f, -15.2999f);
            break;
        case 927:
            cameraScene.changeScene(-0.2752f, 3.4310f, -2.9159f, -232.1002f, -16.60f);
            break;
        case 994:
            cameraScene.changeScene(-0.2082f, 2.6050f, -1.0338f, -127.0997f, 20.0999f);
            break;
        case 1098:
            cameraScene.changeScene(1.1197f, 3.2867f, 0.0763f, -139.4997f, -30.30f);
            break;
        case 1178: //New
            cameraScene.changeScene(0.464260f, 2.627294f, -0.193444f, -488.500641f, -6.300014);
            break;
        //case 1220:
        //    cameraScene.changeScene(1.1197f, 3.2867f, 0.0763f, -139.4997f, -30.30f);
        //    break;
        case 1228:
            cameraScene.changeScene(-0.4493f, 2.0741f, 1.3832f, -31.8999f, -2.7999);
            break;
        case 1280: //New camera
            cameraScene.changeScene(1.698401f, 2.070444, 0.413728, -527.800537f, 21.000065);
            break;
        //case 1355: // Go back camera as frame 1228
        //    cameraScene.changeScene(-0.4493f, 2.0741f, 1.3832f, -31.8999f, -2.7999);
        //    break;
        case 1355: //New, after the big jump
            cameraScene.changeScene(1.144406f, 2.028810f, 0.315741f, 23.700129, -25.999989);
            break;
        case 1450: // New camera
            cameraScene.changeScene(0.349872f, 1.736649f, 0.905867, -404.100494f, 10.200009f);
            break;
        //case 1489: //Quita esta, es igual que la 1620
        //    cameraScene.changeScene(-2.1590f, 1.9138f, 1.6268f, -18.1998f, -8.20f);
        //    break;
        case 1545: // Last jump camera
            cameraScene.changeScene(-0.566445f, 1.637479f, 0.748491f, -300.400757, 20.100063);
            break;
        case 1620: // Final macarena camera
            cameraScene.changeScene(-2.1590f, 1.9138f, 1.6268f, -18.1998f, -8.20f);
            break;
        default:
            break;
        }

        //glm::vec3 p(camera.GetPosition());
        //printf("Camara x=%f\t y=%f\t z=%f\n", p.x, p.y, p.z);
        //printf("Yaw = %f\t", camera.GetYaw());
        //printf("Pitch = %f\n", camera.GetPitch());

        // Me interesa la posición de la cámaras en tomas
        glm::vec3 p(cameraScene.GetPosition());
        printf("Camara x=%f\t y=%f\t z=%f\n", p.x, p.y, p.z);
        printf("Yaw = %f\t", cameraScene.GetYaw());
        printf("Pitch = %f\n", cameraScene.GetPitch());

        //For keyframes
        inputKeyframes(window);
        animate();
        camera.animateCam(posXcam + movCam_x, posYcam + movCam_y, posZcam + movCam_z);

        // Render scene
        // ------
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Removed clear color for keep untouched the Skybox color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. Render scene into floating point framebuffer
        // -----------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 projection;
        glm::mat4 view;

        if (camScen) {
            projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            view = camera.GetViewMatrix();
        }
        else
        {
            projection = glm::perspective(glm::radians(cameraScene.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            view = cameraScene.GetViewMatrix();
        }
        glm::mat4 model = glm::mat4(1.0f);

        // Enable skinning shader for the animated FBX
        // -------------------------------------------
        ourShader.use();
        //if (camScen) {
        //    projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        //    view = camera.GetViewMatrix();
        //}
        //else
        //{
        //    projection = glm::perspective(glm::radians(cameraScene.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        //    view = cameraScene.GetViewMatrix();
        //}
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.001f, 0.001f, 0.001f));   // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);

        ourShader.setMat4("gBones", MAX_RIGGING_BONES, gBones);
        glUniformMatrix4fv(glGetUniformLocation(ourShader.ID, "gBones"), 100, GL_FALSE, &gBones[0][0][0]);

        for (unsigned int b = 0; b < MAX_RIGGING_BONES; b++) {
            ourShader.SetBoneTransform(b, gBones[b]);
        }

        batman.Draw(ourShader);

        // Enable Bloom shader for the models
        // -------------------------------------------
        shader.use();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);


        // Set lighting uniforms

        for (unsigned int i = 0; i < lightPositions.size(); i++)
        {
            shader.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
            shader.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
        }
        if (camScen) {
            shader.setVec3("viewPos", camera.Position);
        }
        else
        {
            shader.setVec3("viewPos", cameraScene.Position);
        }

        model = glm::mat4(1);
        //model = glm::translate(model, glm::vec3(movX, movY, movZ));
        model = glm::scale(model, glm::vec3(0.1f));
        //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        shader.setMat4("model", model);

        // Draw an OBJ model
        edificio.Draw(shader);

        model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(movX, movY, movZ));
        model = glm::scale(model, glm::vec3(0.1f));
        //model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        shader.setMat4("model", model);

        // Floating text with key actions
        if (game) {
            if (animationCount >= 0 && animationCount < 50)
                start.Draw(shader); //S key
            if (animationCount >= 135 && animationCount < 231)
                jump1.Draw(shader); //J key
            if (animationCount >= 335 && animationCount < 552)
                jump_run1.Draw(shader); //R key
            if (animationCount >= 552 && animationCount < 670)
                jump_slide.Draw(shader); //S key
            if (animationCount >= 670 && animationCount < 768)
                cool.Draw(shader); //Cool message
            if (animationCount >= 768 && animationCount < 927)
                jump_again.Draw(shader); //J Key
            if (animationCount >= 994 && animationCount < 1098)
                dive.Draw(shader); //D key
            if (animationCount >= 1228 && animationCount < 1440)
                fly.Draw(shader); //F key

        }
        if (animationCount >= 1680)
            macarena.Draw(shader); //M key



        // Finally show all the light sources as bright cubes
        shaderLight.use();
        shaderLight.setMat4("projection", projection);
        shaderLight.setMat4("view", view);

        for (unsigned int i = 0; i < lightPositions.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(lightPositions[i]));
            model = glm::scale(model, glm::vec3(0.25f));
            shaderLight.setMat4("model", model);
            shaderLight.setVec3("lightColor", lightColors[i]);
            //renderCube();
        }

        // Draw the Skybox
        // ---------------
        glDepthFunc(GL_LEQUAL);  // Change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        if (camScen) {
            view = glm::mat4(glm::mat3(camera.GetViewMatrix()));    // Remove any translation component of the view matrix
        }
        else
        {
            view = glm::mat4(glm::mat3(cameraScene.GetViewMatrix()));   // Remove any translation component of the view matrix
        }
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36); // Drawing from skyboxVertices
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // Set depth function back to default

        glActiveTexture(GL_TEXTURE0); // Disable Skybox texture unit

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. blur bright fragments with two-pass Gaussian Blur 
        // --------------------------------------------------
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        shaderBlur.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            shaderBlur.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. Now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
        // --------------------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderBloomFinal.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        shaderBloomFinal.setInt("bloom", bloom);
        shaderBloomFinal.setFloat("exposure", exposure);
        renderQuad();

        std::cout << "bloom: " << (bloom ? "on" : "off") << "| exposure: " << exposure << std::endl;

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void gameMode(GLFWwindow* window) {
    if (animationCount == 219) {
        animacion = false; //Deshabilita la animación
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) //Continua solicitando presionar J
            animacion = true;
    }
    if (animationCount == 361) {
        animacion = false; 
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) 
            animacion = true;
    }
    if (animationCount == 618) {
        animacion = false;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            animacion = true;
    }
    if (animationCount == 838) {
        animacion = false;
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
            animacion = true;
    }
    if (animationCount == 1004) {
        animacion = false;
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
            animacion = true;
    }
    if (animationCount == 1259) {
        animacion = false;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
            animacion = true;
    }
    if (animationCount == 1695) {
        animacion = false;
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            animacion = true;
            macarenaMusic();
        }

    }

}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        animationCount = 0;
        animacion = true;
        game = true;
        mainMusic();
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) //disable game mode, demo mode
        game = false;
    //Repara bug en donde la animación chafea

    //if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    //    camera.ProcessKeyboard(FORWARD, deltaTime);
    //if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    //    camera.ProcessKeyboard(BACKWARD, deltaTime);
    //if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    //    camera.ProcessKeyboard(LEFT, deltaTime);
    //if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    //    camera.ProcessKeyboard(RIGHT, deltaTime);
    //
    //if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    //    movX2 -= 0.1f;
    //if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
    //    movX2 += 0.1f;
    //if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
    //    movY2 += 0.1f;
    //if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    //    movY2 -= 0.1f;
    //if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
    //    movZ2 -= 0.1;
    //if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
    //    movZ2 += 0.1;
    //Switch cameras
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (cam == 0) {
            camScen = !camScen;
            cam++;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
        cam = 0;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !bloomKeyPressed)
    {
        bloom = !bloom;
        bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        bloomKeyPressed = false;
    }

    //if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    //{
    //    if (exposure > 0.0f)
    //        exposure -= 0.01f;
    //    else
    //        exposure = 0.0f;
    //}
    //else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    //{
    //    exposure += 0.01f;
    //}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;


    //camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path, bool gammaCorrection)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

//////Function to enable keyframes animation
void inputKeyframes(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        if (playAnimation < 1)
        {
            if (play_key == false && (FrameIndex > 1))
            {
                resetElements();
                //First Interpolation               
                interpolation();
                play_key = true;
                playIndex = 0;
                i_curr_steps = 0;
                playAnimation++;
                printf("Press 0 to enable animation'\n");
                enableAnimation = 0;
            }
            else
            {
                play_key = false;
            }
        }
    }
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
    {
        if (enableAnimation < 1)
        {
            playAnimation = 0;
        }
    }
}

void initialKeyFrames() {
    //Initial Keyframes to camera
    KeyFrame[0].movCam_x = 0.0f;            //First building
    KeyFrame[0].movCam_y = 0.0f;
    KeyFrame[0].movCam_z = 0.0f;

    KeyFrame[1].movCam_x = -0.15;
    KeyFrame[1].movCam_y = 0.0;
    KeyFrame[1].movCam_z = -1.0;

    KeyFrame[2].movCam_x = -0.15;
    KeyFrame[2].movCam_y = 0.0f;
    KeyFrame[2].movCam_z = -1.6f;

    KeyFrame[3].movCam_x = -0.15f;
    KeyFrame[3].movCam_y = 0.0f;
    KeyFrame[3].movCam_z = -2.0f;

    KeyFrame[4].movCam_x = -0.35f;
    KeyFrame[4].movCam_y = 0.0f;
    KeyFrame[4].movCam_z = -2.6f;

    KeyFrame[5].movCam_x = -0.35f;
    KeyFrame[5].movCam_y = 0.0f;           //Movements in the same position
    KeyFrame[5].movCam_z = -2.6f;

    KeyFrame[6].movCam_x = -0.4f;           //First jump
    KeyFrame[6].movCam_y = 0.0f;
    KeyFrame[6].movCam_z = -2.6f;

    KeyFrame[7].movCam_x = -0.9f;
    KeyFrame[7].movCam_y = -0.7f;
    KeyFrame[7].movCam_z = -2.7f;

    KeyFrame[8].movCam_x = -1.0f;
    KeyFrame[8].movCam_y = -1.5f;
    KeyFrame[8].movCam_z = -2.9;

    KeyFrame[9].movCam_x = -1.2f;
    KeyFrame[9].movCam_y = -1.75f;
    KeyFrame[9].movCam_z = -3.0;

    KeyFrame[10].movCam_x = -1.7f;
    KeyFrame[10].movCam_y = -1.5f;
    KeyFrame[10].movCam_z = -3.0;

    KeyFrame[11].movCam_x = -1.7f;
    KeyFrame[11].movCam_y = -1.5f;
    KeyFrame[11].movCam_z = -3.4;

    KeyFrame[12].movCam_x = -1.8f;
    KeyFrame[12].movCam_y = -1.5f;
    KeyFrame[12].movCam_z = -3.5;

    KeyFrame[13].movCam_x = -2.0f;
    KeyFrame[13].movCam_y = -1.6f;
    KeyFrame[13].movCam_z = -3.5;

    KeyFrame[14].movCam_x = -2.2f;
    KeyFrame[14].movCam_y = -1.6f;
    KeyFrame[14].movCam_z = -3.5;

    KeyFrame[15].movCam_x = -2.4f;
    KeyFrame[15].movCam_y = -1.85f;
    KeyFrame[15].movCam_z = -3.0;

    KeyFrame[16].movCam_x = -2.4f;
    KeyFrame[16].movCam_y = -1.85f;
    KeyFrame[16].movCam_z = -2.5;

    KeyFrame[17].movCam_x = -2.7f;
    KeyFrame[17].movCam_y = -1.7f;
    KeyFrame[17].movCam_z = -2.0;

    KeyFrame[18].movCam_x = -2.9f;
    KeyFrame[18].movCam_y = -1.6f;
    KeyFrame[18].movCam_z = -1.8;

    KeyFrame[19].movCam_x = -3.2f;
    KeyFrame[19].movCam_y = -1.6f;
    KeyFrame[19].movCam_z = -1.7;

    KeyFrame[20].movCam_x = -3.5f;
    KeyFrame[20].movCam_y = -1.6f;
    KeyFrame[20].movCam_z = -1.6;

    KeyFrame[21].movCam_x = -3.9f;
    KeyFrame[21].movCam_y = -1.6f;
    KeyFrame[21].movCam_z = -1.5;

    KeyFrame[22].movCam_x = -4.4f;
    KeyFrame[22].movCam_y = -1.6f;
    KeyFrame[22].movCam_z = -1.5;

    KeyFrame[23].movCam_x = -4.7f;
    KeyFrame[23].movCam_y = -1.6f;
    KeyFrame[23].movCam_z = -1.7;

    KeyFrame[24].movCam_x = -4.7f;
    KeyFrame[24].movCam_y = -1.6f;
    KeyFrame[24].movCam_z = -1.9;

    KeyFrame[25].movCam_x = -4.7f;
    KeyFrame[25].movCam_y = -1.4f;
    KeyFrame[25].movCam_z = -2.3;

    KeyFrame[26].movCam_x = -4.7f;
    KeyFrame[26].movCam_y = -1.6f;
    KeyFrame[26].movCam_z = -2.9;

    KeyFrame[27].movCam_x = -4.5f;
    KeyFrame[27].movCam_y = -1.6f;
    KeyFrame[27].movCam_z = -3.4;

    KeyFrame[28].movCam_x = -4.3f;
    KeyFrame[28].movCam_y = -1.6f;
    KeyFrame[28].movCam_z = -3.2;

    KeyFrame[29].movCam_x = -4.2f;
    KeyFrame[29].movCam_y = -1.7f;
    KeyFrame[29].movCam_z = -2.9;

    KeyFrame[30].movCam_x = -4.0f;
    KeyFrame[30].movCam_y = -2.1f;
    KeyFrame[30].movCam_z = -2.7;

    KeyFrame[31].movCam_x = -4.0f;
    KeyFrame[31].movCam_y = -2.5f;
    KeyFrame[31].movCam_z = -2.4;

    KeyFrame[32].movCam_x = -3.7f;
    KeyFrame[32].movCam_y = -2.3f;
    KeyFrame[32].movCam_z = -2.0;

    KeyFrame[33].movCam_x = -3.5f;
    KeyFrame[33].movCam_y = -2.3f;
    KeyFrame[33].movCam_z = -1.8;

    KeyFrame[34].movCam_x = -3.2f;
    KeyFrame[34].movCam_y = -2.3f;
    KeyFrame[34].movCam_z = -1.6;

    KeyFrame[35].movCam_x = -3.2f;
    KeyFrame[35].movCam_y = -2.5f;
    KeyFrame[35].movCam_z = -1.3;

    KeyFrame[36].movCam_x = -3.0f;
    KeyFrame[36].movCam_y = -2.5f;
    KeyFrame[36].movCam_z = -1.1;

    KeyFrame[37].movCam_x = -2.7f;
    KeyFrame[37].movCam_y = -2.5f;
    KeyFrame[37].movCam_z = -1.0;

    KeyFrame[38].movCam_x = -2.3f;
    KeyFrame[38].movCam_y = -2.6f;
    KeyFrame[38].movCam_z = -1.0;

    KeyFrame[39].movCam_x = -2.0f;
    KeyFrame[39].movCam_y = -2.8f;
    KeyFrame[39].movCam_z = -0.8;

    KeyFrame[40].movCam_x = -2.0f;
    KeyFrame[40].movCam_y = -3.0f;
    KeyFrame[40].movCam_z = -0.5;

    KeyFrame[41].movCam_x = -2.5f;
    KeyFrame[41].movCam_y = -3.2f;
    KeyFrame[41].movCam_z = -0.6;

    KeyFrame[42].movCam_x = -2.9f;
    KeyFrame[42].movCam_y = -3.1f;
    KeyFrame[42].movCam_z = -0.6;

    KeyFrame[43].movCam_x = -3.3f;
    KeyFrame[43].movCam_y = -3.1f;
    KeyFrame[43].movCam_z = -0.7;

    KeyFrame[44].movCam_x = -3.5f;
    KeyFrame[44].movCam_y = -3.2f;
    KeyFrame[44].movCam_z = -0.4;

    KeyFrame[45].movCam_x = -3.8f;
    KeyFrame[45].movCam_y = -3.0f;
    KeyFrame[45].movCam_z = 0.0;

    KeyFrame[46].movCam_x = -4.1f;
    KeyFrame[46].movCam_y = -3.0f;
    KeyFrame[46].movCam_z = 0.2;

    KeyFrame[47].movCam_x = -4.7f;
    KeyFrame[47].movCam_y = -3.1f;
    KeyFrame[47].movCam_z = 0.2;

    KeyFrame[48].movCam_x = -5.0f;
    KeyFrame[48].movCam_y = -3.1f;
    KeyFrame[48].movCam_z = 0.2;

    KeyFrame[49].movCam_x = -5.1f;
    KeyFrame[49].movCam_y = -3.3f;
    KeyFrame[49].movCam_z = 0.1;

    KeyFrame[50].movCam_x = -5.2f;
    KeyFrame[50].movCam_y = -3.3f;
    KeyFrame[50].movCam_z = 0.1;

    KeyFrame[51].movCam_x = -5.3f;
    KeyFrame[51].movCam_y = -3.4f;
    KeyFrame[51].movCam_z = 0.1;

    KeyFrame[52].movCam_x = -5.2f;
    KeyFrame[52].movCam_y = -3.4f;
    KeyFrame[52].movCam_z = 0.1;

    KeyFrame[53].movCam_x = -5.3f;
    KeyFrame[53].movCam_y = -3.3f;
    KeyFrame[53].movCam_z = 0.1;

    KeyFrame[54].movCam_x = -5.2f;
    KeyFrame[54].movCam_y = -3.3f;
    KeyFrame[54].movCam_z = 0.1;

    KeyFrame[55].movCam_x = -5.3f;
    KeyFrame[55].movCam_y = -3.4f;
    KeyFrame[55].movCam_z = 0.1;

    KeyFrame[56].movCam_x = -5.2f;
    KeyFrame[56].movCam_y = -3.4f;
    KeyFrame[56].movCam_z = 0.1;
}