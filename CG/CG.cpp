// CG.cpp : Este archivo contiene la función "main". La ejecución del programa comienza y termina ahí.
//

#include <iostream>
#include <cmath>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Librería para carga de texturas
#include "stb_image.h"

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "SOIL2/SOIL2.h"

#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Texture.h"

// Prototipos de funciones
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement(void);
void LoadTextures(void);
//void animacion();
void showDebugVars();

// Carga y generación de texturas 
unsigned int generateTextures(char*, bool);

// Dimensiones de la ventana
const GLuint WIDTH = 1026, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Cámara
// Posición inicial de la cámara
Camera  camera(glm::vec3(0.0f, 0.0f, 2.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool keys[1024];
bool firstMouse = true;

float range = 0.0f;
float spotAngle = 0.0f;


// Light attributes
glm::vec3 lightPos(0.0f, -5.0f, 0.0f);
bool active;

// Posición de las 6 luces puntuales
glm::vec3 pointLightPositions[] = {
	glm::vec3(0.0f,  1.0f,  2.0f),
	//glm::vec3(0.0f, 0.0f, 0.0f),
	//glm::vec3(0.0f,  0.0f, 0.0f),
	//glm::vec3(0.0f,  0.0f, 0.0f),
	//glm::vec3(0.0f,  0.0f, 0.0f),
	//glm::vec3(0.0f,  0.0f, 0.0f)
};

// Posición de la spotlight
glm::vec3 LightP1;

// Variables para realizar transformaciones interactivas a los modelos
float movX, movY, movZ;
float scaleX, scaleY, scaleZ;
float rot = 0.0f;

// Deltatime
GLfloat deltaTime = 0.0f;	// Tiempo entre el cuadro actual y el cuadro anterior
GLfloat lastFrame = 0.0f;  	// Tiempo del cuadro anterior

// Mapa especular y texturas
GLuint specular_map, texture0;

unsigned int generateTextures(const char* filename, bool alfa) {
	unsigned int textureID;

	// Establece los parámetros de ajuste de textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Establece los parámetros de filtrado de textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);

	// Carga la imagen, crea la textura y genera los mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // Dile a stb_image.h que voltee las texturas cargadas en el eje y

	unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (data)
	{
		if (alfa)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		return textureID;
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
		return 100;
	}

	stbi_image_free(data);
}

void LoadTextures() {
	// 0 -> Imagen sin transparencia
	// 1 -> Imagen con transparencia
	specular_map = generateTextures("Images/mapa_blanco.jpg", 0);
	texture0 = generateTextures("Images/coeur_de_pirate.jpg", 0);
}

int main()
{
	// Inicializa GLFW
	glfwInit();
	
	// Establece todas las opciones requeridas para GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); //OpenGL 4.0
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	// Cree un objeto GLFWwindow para utilizar las funciones de GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Título de la ventana", nullptr, nullptr);

	if (nullptr == window)
	{
		std::cout << "Error al crear la ventana GLFW" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Establece las funciones de devolución de llamada requeridas
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// Opción de GFLW: Captura el cursor dentro del entorno y deshabilita el puntero
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	/* Establezca esto en verdadero para que GLEW sepa utilizar un 
	enfoque moderno para recuperar punteros y extensiones de funciones*/
	glewExperimental = GL_TRUE;

	// Inicialice GLEW para configurar los punteros de la función OpenGL
	if (GLEW_OK != glewInit())
	{
		std::cout << "Error al inicializar GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Define las dimensiones de la vista
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);


	// Opciones de OpenGL
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	/*
	Shaders de OpenGL
	
	lightingShader
	Colore los objetos según su textura y contiene los tres modelos de iluminación; 
	con sus componentes ambientales, difusas y especulares.

	modelLoadingShader
	Colorea los objetos según su textura e ignora los efectos de iluminación.

	lampShader
	Colorea los objetos de blanco. Útil para posicionar lamparas (luces).

	skyboxShader
	Programa que realiza un cubemapping para generar un skybox.
	*/
	Shader lightingShader("Shaders/lighting.vs", "Shaders/lighting.frag");
	Shader modelLoadingShader("Shaders/modelLoading.vs", "Shaders/modelLoading.frag");
	Shader lampShader("Shaders/lamp.vs", "Shaders/lamp.frag");
	Shader skyboxShader("Shaders/SkyBox.vs", "Shaders/SkyBox.frag");

	// Carga de modelos OBJ
	Model cubo((char*)"Models/Prueba/cube.obj");
	
	// Texturas del skybox
	vector<const GLchar*> faces;
	faces.push_back("SkyBox/beach/posx.jpg"); 
	faces.push_back("SkyBox/beach/negx.jpg");

	faces.push_back("SkyBox/beach/posy.jpg");
	faces.push_back("SkyBox/beach/negy.jpg");

	faces.push_back("SkyBox/beach/posz.jpg");
	faces.push_back("SkyBox/beach/negz.jpg");

	// Texturizando el cubo con las seis caras cargadas
	GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);

	//Carga de texturas
	LoadTextures();

	// Configurar datos de vértices (y búfer(es)) y punteros de atributos
	GLfloat vertices[] =
	{
		// Posición            // Normales              // Coordenadas de textura
		+0.0f, +0.0f, 0.0f,    0.0f, 0.0f, 1.0f,		0.0f,0.0f,
		+1.0f, +0.0f, 0.0f,	   0.0f, 0.0f, 1.0f,		1.0f,0.0f,
		+1.0f, +1.0f, 0.0f,    0.0f, 0.0f, 1.0f,		1.0f,1.0f,
		+0.0f, +1.0f, 0.0f,    0.0f, 0.0f, 1.0f,		0.0f,1.0f,
		// Esto dibuja un plano unitario en XY
		
		//Lamparas puntuales
		//Triangulo 1
		+0.0f, +0.0f, 0.0f,    0.0f, 0.0f, 1.0f,		0.0f,0.0f,
		+1.0f, +0.0f, 0.0f,	   0.0f, 0.0f, 1.0f,		1.0f,0.0f,
		+1.0f, +1.0f, 0.0f,    0.0f, 0.0f, 1.0f,		1.0f,1.0f,
		//Triangulo 2
		+0.0f, +0.0f, 0.0f,    0.0f, 0.0f, 1.0f,		0.0f,0.0f,
		+0.0f, +1.0f, 0.0f,	   0.0f, 0.0f, 1.0f,		1.0f,0.0f,
		+1.0f, +1.0f, 0.0f,    0.0f, 0.0f, 1.0f,		1.0f,1.0f,

	};

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

	GLuint indices[] = {
		// Indices para dibujar un plano en XY
		0,1,3,
		1,2,3
	};


	// Primero, configure el VAO del contenedor (y VBO)
	GLuint VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Atributo de posición
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	
	// Atributo de normales
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	
	// Atributo de coordenadas de texturizado
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);

	// Luego, establecemos el VAO de la luz (VBO permanece igual. Después de todo, los vértices son los mismos para el objeto de luz (también un cubo 3D))
	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	// Solo necesitamos vincularnos al VBO (para vincularlo con glVertexAttribPointer), no es necesario llenarlo; Los datos de VBO ya contienen todo lo que necesitamos.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Establecer los atributos de vértice (solo datos de posición para la lámpara))
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0); // Note that we skip over the other data in our buffer object (we don't need the normals/textures, only positions).
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	//Configuración del skybox
	GLuint skyboxVBO, skyboxVAO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	// Establecer unidades de textura
	lightingShader.Use();
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0);
	glUniform1i(glGetUniformLocation(lightingShader.Program, "material.specular"), 1);

	// Tipo de proyección
	glm::mat4 projection = glm::perspective(camera.GetZoom(), (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);
	//glm::mat4 projection = glm::ortho(-5.0f, 5.0f, -3.0f, 3.0f, 0.1f, 10.0f);

	while (!glfwWindowShouldClose(window))
	{
		// Variables para debuguear
		//showDebugVars();

		// Calcular deltatime del cuadro actual
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Compruebe si se ha activado algún evento (tecla presionada, mouse movido, etc.) y llame a las funciones de respuesta correspondientes
		glfwPollEvents();
		DoMovement();


		// Color de fondo RGBA
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Use el sombreador correspondiente al configurar uniformes y dibujar los objetos.
		lightingShader.Use();
		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
		
		// Establecer propiedades del material
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 4.0f);
		// == ==========================
		// Aquí configuramos todos los uniformes para los 5/6 tipos de luces que tenemos. Tenemos que configurarlos manualmente e indexarlos
		// la estructura PointLight adecuada en la matriz para establecer cada variable uniforme. Esto se puede hacer más amigable con el código
		// definiendo los tipos de luz como clases y estableciendo sus valores allí, o usando un enfoque uniforme más eficiente
		// mediante el uso de 'Objetos de búfer uniformes', pero eso es algo que discutimos en el tutorial 'GLSL avanzado'.
		// == ==========================

		// Configuración de la luz direccional
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.0f, 0.0f, 0.0f);

		// Configuración de las luces puntuales. Hasta 8 configurando #define NUMBER_OF_POINT_LIGHTS en lightning.frag
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), 0.0f, 0.5f, 0.5f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), 0.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 0.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.14);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 0.07);

		// Configuración de la luz tipo "Spotlight"
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), LightP1.x * 0.0f, LightP1.y * 0.0f, LightP1.z * 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), LightP1.x * 0.0f, LightP1.y * 0.0f, LightP1.z * 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 0.6f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.05f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.032f);

		// Configuración de los conos interior y exterior de esta luz
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(15.0f)));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(23.0f))); 

		// Establecer las propiedades del material ¿Qué tan reflejantes serán?
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

		// Crear transformaciones de cámara
		glm::mat4 view;
		view = camera.GetViewMatrix();

		// Obtén las ubicaciones de uniformes
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// Pasa matrices al shader
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// Activa el mapa especular y carga la textura a utilizar en los modelos de iluminación
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specular_map);

		//Enlaza al arreglo de vértices al VAO
		glBindVertexArray(VAO);

		// ============================================
		// Comienza el renderizado de toda la geometría
		// ============================================

		// Matriz de modelo
		glm::mat4 model(1);

		// Activa el mapa difuso y carga la textura a utilizar en la geometría
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture0);

		model = glm::translate(model, glm::vec3(-0.5f, 0.5f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		//Dibuja un plano en XY
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f));
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		// Dibuja un modelo OBJ o FBX
		cubo.Draw(lightingShader);

		// Destruye el arreglo de vértices del VAO
		glBindVertexArray(0);

		// Cambio y configuración de shader
		lampShader.Use();
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		// Establecer matrices de vista y proyección
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		//model = glm::mat4(1);
		//model = glm::translate(model, lightPos);
		//model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		
		// Dibuja el objeto de luz(usando los atributos de vértice de la luz)
		glBindVertexArray(lightVAO);
	
		//Dibuja tantos planos como luces puntuales se tengan
		for (GLuint i = 0; i < 1/*# luces*/; i++)
		{
			model = glm::mat4(1);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.3f)); // Make it a smaller cube
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			
			//Dibuja la luz como un plano en XY
			glDrawArrays(GL_TRIANGLES, 4, 6);

			printf("Light [%i]: %0.2f %0.2f %0.2f\n", i, pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z);
		}

		//Destruye el arreglo de vértices del LightVAO
		glBindVertexArray(0); 

		// Cambio y configuración de shader
		modelLoadingShader.Use();
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		// Establecer matrices de vista y proyección
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		model = glm::mat4(1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		
		//Enlaza al arreglo de vértices al VAO
		glBindVertexArray(VAO);

		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		//model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
		//model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));

		// Dibuja un modelo OBJ o FBX con el nuevo shader
		cubo.Draw(modelLoadingShader);

		//Destruye el arreglo de vértices del VAO
		glBindVertexArray(0);

		// Cambia la función de profundidad para que la prueba de profundidad pase cuando los valores sean iguales al contenido del búfer de profundidad
		glDepthFunc(GL_LEQUAL);  
		
		// Cambio y configuración de shader
		skyboxShader.Use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// Enlaza al arreglo de vértices del skybox al VAO
		glBindVertexArray(skyboxVAO);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36); //Dibujando todos los vértices de skyboxVertices[]

		//Destruye el arreglo de vértices del skyboxVAO
		glBindVertexArray(0);

		// Establece la función de profundidad a la predeterminada
		glDepthFunc(GL_LESS); 
			   
		// Cambia los búferes de pantalla
		glfwSwapBuffers(window);

		//Hasta aquí se ha dibujado un cuadro del escenario y se mantendrá dentro de este ciclo hasta que se presione ESC
	}
	
	//Destruye todos los arreglos de vértices y limpia los búferes
	glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(1, &lightVAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);

	//Termine GLFW, borrando todos los recursos asignados por GLFW.
	glfwTerminate();
	return 0;

}

void showDebugVars() {
	printf("X= %0.2f Y= %0.2f, Z= %0.2f\n", movX, movY, movZ);
	printf("scX= %0.2f scY= %0.2f, scZ= %0.2f\n", scaleX, scaleY, scaleZ);
	printf("camX= %0.2f camY= %0.2f, camZ= %0.2f\n", camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
}

//void animacion()
//{
//	//Inserta aquí animaciones por estado
//}

// Mueve / altera las posiciones de la cámara según la entrada del usuario
void DoMovement()
{

	// Controles de cámara
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
		camera.ProcessKeyboard(RIGHT, deltaTime);

	// Controles de las luces puntuales
	if (keys[GLFW_KEY_F])
	{
		pointLightPositions[0].x -= 0.1f;
	}
	if (keys[GLFW_KEY_H])
	{
		pointLightPositions[0].x += 0.1f;
	}
	if (keys[GLFW_KEY_T])
	{
		pointLightPositions[0].y += 0.1f;
	}
	if (keys[GLFW_KEY_G])
	{
		pointLightPositions[0].y -= 0.1f;
	}
	if (keys[GLFW_KEY_R])
	{
		pointLightPositions[0].z -= 0.1f;
	}
	if (keys[GLFW_KEY_Y])
	{
		pointLightPositions[0].z += 0.1f;
	}

	// Controles para mover en los tres ejes
	if (keys[GLFW_KEY_J])
		movX -= 0.1f;
	if (keys[GLFW_KEY_L])
		movX += 0.1f;
	if (keys[GLFW_KEY_I])
		movY += 0.1f;
	if (keys[GLFW_KEY_K])
		movY -= 0.1f;
	if (keys[GLFW_KEY_U])
		movZ -= 0.1;
	if (keys[GLFW_KEY_O])
		movZ += 0.1;

	// Controles para escalar en los tres ejes
	if (keys[GLFW_KEY_1])
		scaleX += 0.04f;
	if (keys[GLFW_KEY_2])
		scaleX -= 0.04f;
	if (keys[GLFW_KEY_3])
		scaleY += 0.04f;
	if (keys[GLFW_KEY_4])
		scaleY -= 0.04f;
	if (keys[GLFW_KEY_5])
		scaleZ += 0.04f;
	if (keys[GLFW_KEY_6])
		scaleZ -= 0.04f;
}

// Se llama cada vez que se presiona / suelta una tecla a través de GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// Cerrar ventana si presiona tecla ESC
	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}

	// Interruptor de luz
	// LightP1 es un vector que se multiplicará por otro correspondiente a algún atributo de luz como dirección, posición o sus componentes.
	if (keys[GLFW_KEY_SPACE])
	{
		active = !active;
		if (active)
			LightP1 = glm::vec3(1.0, 1.0, 1.0);
		else
			LightP1 = glm::vec3(0.0f, 0.0f, 0.0f);
	}
}

// Procesamiento del movimiento de la cámara mediante el mouse
void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;	// Invertido ya que las coordenadas y van de abajo a izquierda

	lastX = xPos;
	lastY = yPos;

	camera.ProcessMouseMovement(xOffset, yOffset);
}

