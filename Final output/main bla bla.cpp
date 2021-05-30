#include <GL/glew.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include "ShaderProgramAttachment.h"
#include "imageloader.h"
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace std;
using namespace glm;

#define WIDTH 800
#define HEIGHT 600
#define numVAOs 20

typedef struct {
	GLfloat f1;
	GLfloat f2;
	GLfloat f3;
} colorValues;

GLuint renderingProgram;
GLuint vao[numVAOs];


GLuint uniformModel, uniformView, uniformProjection;

vector <GLuint> VBOs, CBOs, EBOs, TBOs;
vector <GLuint> texture_ids;

GLuint renderingSkybox;
GLuint VAO_skybox, VBO_skybox, texture_skybox;


//Camera
glm::vec3 eye(0.0f, 0.0f, 3.0f);
glm::vec3 center(0.0f, 0.0f, -1.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

//Time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

double mouse_x, mouse_y; //these will store the mouse position
bool obj_rotate = false, obj_translate = false;
int num_rotate = -1;

// camera
glm::vec3 dir;
glm::vec3 rot = { 0, 0, 0 };

string basepath = "assets/";
vector <string> objFiles = {"door_frame", "wooden_chairleg","metal_chairleg", "chair_cusion",
							"table_wood","kitchen_cabinet1", "flooring",
							"indoor_floor", "interior_wall", "darksink",
							"lightsink","curtain" , "kitchen_cabinet2" ,
							 "kitchen_cabinet3","cabinet","clock"
							};
vector <string> texFiles = {"dark wood", "wood5", "metal", "fabric 2",
							"Table", "kitchen", "dark wood",
							"floor1", "interior wall 1","dark_metal",
							"metal","fabric 2" , "kitchen",
							"dark wood","wood5", "dark wood"
							};
vector <string> skyFiles = { "Right", "Left", "Top", "Bottom", "Front", "Back" };

vector <vector <tinyobj::shape_t>> shapes;
vector <vector <tinyobj::material_t>> materials;
vector <vector<colorValues>> colors;

void initObjects() {
	string err; //error text
	for (int i = 0; i < objFiles.size(); i++) {
		objFiles[i] = basepath + objFiles[i] + ".obj";

		colorValues* cValues = new colorValues{ 1.0f, 1.0f, 1.0f };

		vector <tinyobj::shape_t>* tempShape = new vector <tinyobj::shape_t>();
		shapes.push_back(*tempShape);

		vector <tinyobj::material_t>* tempMaterial = new vector <tinyobj::material_t>();
		materials.push_back(*tempMaterial);

		tinyobj::LoadObj(shapes[i], materials[i], err, objFiles[i].c_str(), basepath.c_str());

		vector <colorValues>* tempColor = new vector <colorValues>(shapes[i][0].mesh.positions.size(), *cValues); //= new vector<colorValues>();
		colors.push_back(*tempColor);
		

		if (!err.empty()) {//did the OBJ load correctly?
			std::cerr << err << std::endl; // `err` may contain warning message.
		}
		else
			std::cout << "Loaded " << objFiles[i] << " with shapes: " << shapes[i].size() << std::endl; //print out number of meshes described in file

		GLuint* VBO = new GLuint();
		GLuint* CBO = new GLuint();
		GLuint* EBO = new GLuint();

		glBindVertexArray(vao[i]);

		glGenBuffers(1, VBO);
		glBindBuffer(GL_ARRAY_BUFFER, *VBO);
		glBufferData(GL_ARRAY_BUFFER, shapes[i][0].mesh.positions.size() * sizeof(float), &(shapes[i][0].mesh.positions[0]), GL_STATIC_DRAW);
		glEnableVertexAttribArray(glGetAttribLocation(renderingProgram, "v_vertex"));
		glVertexAttribPointer(glGetAttribLocation(renderingProgram, "v_vertex"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind buffers

		glGenBuffers(1, CBO);
		glBindBuffer(GL_ARRAY_BUFFER, *CBO);
		glBufferData(GL_ARRAY_BUFFER, colors[i].size() * sizeof(float), &(colors[i][0]), GL_STATIC_DRAW);
		glEnableVertexAttribArray(glGetAttribLocation(renderingProgram, "v_color"));
		glVertexAttribPointer(glGetAttribLocation(renderingProgram, "v_color"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind buffers

		glGenBuffers(1, EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, shapes[i][0].mesh.indices.size() * sizeof(unsigned int), &(shapes[i][0].mesh.indices[0]), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind buffers

		if (shapes[i][0].mesh.texcoords.size() != 0) {
			GLuint* TBO = new GLuint();
			glGenBuffers(1, TBO);
			glBindBuffer(GL_ARRAY_BUFFER, *TBO);
			glBufferData(GL_ARRAY_BUFFER, shapes[i][0].mesh.texcoords.size() * sizeof(float), &(shapes[i][0].mesh.texcoords[0]), GL_STATIC_DRAW);
			glEnableVertexAttribArray(glGetAttribLocation(renderingProgram, "v_uv"));
			glVertexAttribPointer(glGetAttribLocation(renderingProgram, "v_uv"), 2, GL_FLOAT, GL_FALSE, 0, 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind buffers

			string file = basepath + texFiles[i] + ".bmp";
			Image* image = loadBMP(file.c_str());

			GLuint* texture_id = new GLuint();
			glGenTextures(1, texture_id);
			glBindTexture(GL_TEXTURE_2D, *texture_id);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixels);
			texture_ids.push_back(*texture_id);

			TBOs.push_back(*TBO);
		}

		glBindVertexArray(0);

		VBOs.push_back(*VBO);
		CBOs.push_back(*CBO);
		EBOs.push_back(*EBO);

		
	}
}

float vertices[] = {     
		-140.0f,  140.0f, -140.0f,
		-140.0f, -140.0f, -140.0f,
		 140.0f, -140.0f, -140.0f,
		 140.0f, -140.0f, -140.0f,
		 140.0f,  140.0f, -140.0f,
		-140.0f,  140.0f, -140.0f,

		-140.0f, -140.0f,  140.0f,
		-140.0f, -140.0f, -140.0f,
		-140.0f,  140.0f, -140.0f,
		-140.0f,  140.0f, -140.0f,
		-140.0f,  140.0f,  140.0f,
		-140.0f, -140.0f,  140.0f,

		 140.0f, -140.0f, -140.0f,
		 140.0f, -140.0f,  140.0f,
		 140.0f,  140.0f,  140.0f,
		 140.0f,  140.0f,  140.0f,
		 140.0f,  140.0f, -140.0f,
		 140.0f, -140.0f, -140.0f,

		-140.0f, -140.0f,  140.0f,
		-140.0f,  140.0f,  140.0f,
		 140.0f,  140.0f,  140.0f,
		 140.0f,  140.0f,  140.0f,
		 140.0f, -140.0f,  140.0f,
		-140.0f, -140.0f,  140.0f,

		-140.0f,  140.0f, -140.0f,
		 140.0f,  140.0f, -140.0f,
		 140.0f,  140.0f,  140.0f,
		 140.0f,  140.0f,  140.0f,
		-140.0f,  140.0f,  140.0f,
		-140.0f,  140.0f, -140.0f,

		-140.0f, -140.0f, -140.0f,
		-140.0f, -140.0f,  140.0f,
		 140.0f, -140.0f, -140.0f,
		 140.0f, -140.0f, -140.0f,
		-140.0f, -140.0f,  140.0f,
		 140.0f, -140.0f,  140.0f
};

void initSkybox() {
	

	glGenVertexArrays(1, &VAO_skybox);
	glGenBuffers(1, &VBO_skybox);
	glBindVertexArray(VAO_skybox);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_skybox);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(glGetAttribLocation(renderingSkybox, "skybox_vertex"));
	//glVertexAttribPointer(glGetAttribLocation(renderingSkybox, "skybox_vertex"), 2, GL_FLOAT, GL_FALSE, 0, 0);
	//glBindBuffer(GL_ARRAY_BUFFER, 0); //unbind buffers

	glGenTextures(1, &texture_skybox);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_skybox);

	int width, height, nrChannels;
	for (int i = 0; i < skyFiles.size(); i++) {
		string file = basepath + "skybox_" + skyFiles[i] + ".png";
		unsigned char* data = stbi_load(file.c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
			std::cout << "Cubemap texture loaded: " << file << std::endl;
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << skyFiles[i] << std::endl;
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


}

void init(GLFWwindow* window) {
	renderingProgram = createShaderProgram();
	renderingSkybox = createSkyboxShader();
	glGenVertexArrays(numVAOs, vao);
	initObjects();
	initSkybox();
};

void display(GLFWwindow* window, double currentTime) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(renderingProgram);

	glm::mat4 projectionMatrix(1.0);
	projectionMatrix = glm::perspective(glm::radians(50.0f), (GLfloat)800 / (GLfloat)600, 0.1f, 100.0f);
	uniformProjection = glGetUniformLocation(renderingProgram, "u_projection");
	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	glm::mat4 viewMatrix(1.0);
	viewMatrix = glm::lookAt(eye, eye + center, up);
	uniformView = glGetUniformLocation(renderingProgram, "u_view");
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	
	glm::mat4 model(1.0f);
	uniformModel = glGetUniformLocation(renderingProgram, "u_model");
	glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

	for (int i = 0; i < objFiles.size(); i++) {
		glBindVertexArray(vao[i]);

		GLuint u_texture = glGetUniformLocation(renderingProgram, "u_texture");
		glUniform1i(u_texture, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_ids[i]);
		
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
		glVertexAttribPointer(glGetAttribLocation(renderingProgram, "v_vertex"), 3, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, CBOs[i]);
		glVertexAttribPointer(glGetAttribLocation(renderingProgram, "v_color"), 3, GL_FLOAT, GL_FALSE, 0, 0); 
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i]);

		glDrawElements(GL_TRIANGLES, shapes[i][0].mesh.indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	//Begin Skybox
	glUseProgram(renderingSkybox);
	glDepthFunc(GL_LEQUAL);
	viewMatrix = glm::mat4(glm::mat3(viewMatrix));

	glUniformMatrix4fv(glGetUniformLocation(renderingSkybox, "u_view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(glGetUniformLocation(renderingSkybox, "u_projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	glBindVertexArray(VAO_skybox);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture_skybox);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // Set depth function back to default
}

void processInput(GLFWwindow* window)
{
	float cameraSpeed = 2.5 * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		eye += cameraSpeed * center;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		eye -= cameraSpeed * center;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		eye -= glm::normalize(glm::cross(center, up)) * cameraSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		eye += glm::normalize(glm::cross(center, up)) * cameraSpeed;
	}
}

int main(void) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OBJ Loader", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }

	init(window);

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//read keyboard input in main loop
		processInput(window);
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);

}

