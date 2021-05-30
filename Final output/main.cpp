#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

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

#define WIDTH 1920//800
#define HEIGHT 1080//600
#define numVAOs 19

typedef struct {
	GLfloat f1;
	GLfloat f2;
	GLfloat f3;
} colorValues;

GLuint renderingProgram;
GLuint vao[numVAOs];


GLuint uniformModel, uniformView, uniformProjection, uniformLight, uniformCamera;

vector <GLuint> VBOs, CBOs, EBOs, TBOs, LBOs;
vector <GLuint> texture_ids;

GLuint renderingSkybox;
GLuint VAO_skybox, VBO_skybox, texture_skybox;


//Camera
glm::vec3 eye(0.0f, 0.0f, 3.0f);
glm::vec3 center(0.0f, 0.0f, -1.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

//Yaw, pitch, field of view
vector <float> camera = { -90.0f, 0.0f, 45.0f };

//Mouse
bool mouse;
double mouse_x = WIDTH / 2.0f;
double mouse_y = HEIGHT / 2.0f;
float sensitivity = 0.5f;

//Time
float deltaTime = 0.0f;
float lastFrame = 0.0f;


string basepath = "assets/";
vector <string> objFiles = { "door_frame" }; /*, "wooden_chairleg", "metal_chairleg", "chair_cusion",
							"table_wood","kitchen_cabinet1", "flooring",
							"indoor_floor", "interior_wall", "darksink",
							"lightsink","curtain" , "kitchen_cabinet2" ,
							"kitchen_cabinet3","cabinet","clock", 
	"wall",
							"interiorceiling","roof"
							};*/
vector <string> texFiles = { "dark wood" }; /* , "wood5", "metal", "fabric 2",
							"Table", "kitchen", "dark wood",
							"floor1", "interior wall 1","dark_metal",
							"metal","fabric 2" , "kitchen",
							"dark wood","wood5", "dark wood",
	"Table",
							"dark wood", "roof 1"
							};*/
vector <string> skyFiles = { "Right", "Left", "Top", "Bottom", "Front", "Back" };

vector <vector <tinyobj::shape_t>> shapes;
vector <vector <tinyobj::material_t>> materials;
vector <vector<colorValues>> colors;
vector <vector<colorValues>> norms;
float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
};

//unsigned int fbo, texture, rbo, framebuffer, texColorBuffer, quadVAO;

void initObjects() {
	string err; //error text
	for (int i = 0; i < objFiles.size(); i++) {
		objFiles[i] = basepath + objFiles[i] + ".obj";

		colorValues* cValues = new colorValues{ 1.0f, 0.0f, 0.0f };
		colorValues* nValues = new colorValues{ 0.0f, 0.0f, -1.0f};

		vector <tinyobj::shape_t>* tempShape = new vector <tinyobj::shape_t>();
		shapes.push_back(*tempShape);

		vector <tinyobj::material_t>* tempMaterial = new vector <tinyobj::material_t>();
		materials.push_back(*tempMaterial);

		tinyobj::LoadObj(shapes[i], materials[i], err, objFiles[i].c_str(), basepath.c_str());

		vector <colorValues>* tempColor = new vector <colorValues>(shapes[i][0].mesh.positions.size(), *cValues);
		colors.push_back(*tempColor);

		vector <colorValues>* tempNorm = new vector <colorValues>(shapes[i][0].mesh.positions.size(), *nValues);
		norms.push_back(*tempNorm);
		

		if (!err.empty()) {//did the OBJ load correctly?
			std::cerr << err << std::endl; // `err` may contain warning message.
		}
		else
			std::cout << i + 1 << " Loaded object: " << objFiles[i] << " with shapes: " << shapes[i].size() << std::endl; //print out number of meshes described in file

		GLuint* VBO = new GLuint();
		GLuint* CBO = new GLuint();
		GLuint* EBO = new GLuint();
		GLuint* LBO = new GLuint();

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

		glGenBuffers(1, LBO);
		glBindBuffer(GL_ARRAY_BUFFER, *LBO);
		glBufferData(GL_ARRAY_BUFFER, norms[i].size() * sizeof(float), &(norms[i][0]), GL_STATIC_DRAW);
		glEnableVertexAttribArray(glGetAttribLocation(renderingProgram, "v_normal"));
		glVertexAttribPointer(glGetAttribLocation(renderingProgram, "v_normal"), 3, GL_FLOAT, GL_FALSE, 0, 0);
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

			std::cout << i + 1 << " Loaded texture: " << file << std::endl;
		}
		else {
			std::cout << i + 1 << " Loaded texture: " << "no texcoords" << std::endl;
		}

		glBindVertexArray(0);

		VBOs.push_back(*VBO);
		CBOs.push_back(*CBO);
		EBOs.push_back(*EBO);
		LBOs.push_back(*LBO);
		
	}

	/*
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo);
	}

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenTextures(1, &texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	unsigned int quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	*/
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

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glUseProgram(renderingProgram);

	glm::vec3 lightPos(0.0f, 0.0f, 2.0f);
	uniformLight = glGetUniformLocation(renderingProgram, "light_pos");
	glUniform3fv(uniformLight, 1, glm::value_ptr(lightPos));

	uniformCamera = glGetUniformLocation(renderingProgram, "camera_pos");
	glUniform3fv(uniformCamera, 1, glm::value_ptr(eye));

	glm::mat4 projectionMatrix(1.0);
	projectionMatrix = glm::perspective(glm::radians(camera[2]), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
	uniformProjection = glGetUniformLocation(renderingProgram, "u_projection");
	glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

	glm::mat4 viewMatrix(1.0);
	viewMatrix = glm::lookAt(eye, eye + center, up);
	uniformView = glGetUniformLocation(renderingProgram, "u_view");
	glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	

	for (int i = 0; i < objFiles.size(); i++) {
		glBindVertexArray(vao[i]);

		glm::mat4 model(1.0f);
		uniformModel = glGetUniformLocation(renderingProgram, "u_model");
		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

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
	

	/*
	// first pass
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
	glEnable(GL_DEPTH_TEST);
	//DrawScene();

	// second pass
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(renderingProgram);
	glBindVertexArray(quadVAO);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);
*/

	ImGui::Begin("Try Window");
	ImGui::Button("Hello!");
	ImGui::End();

	ImGui::ShowDemoWindow();
	
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void processInput(GLFWwindow* window)
{
	float cameraSpeed = 2.5 * deltaTime;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

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

void mouseInput(GLFWwindow* window, double x, double y) {
	if (mouse) {
		mouse_x = x;
		mouse_y = y;
	}

	float distance_x = x - mouse_x;
	float distance_y = y - mouse_y;
	
	mouse_x = x;
	mouse_y = y;

	distance_x *= sensitivity;
	distance_y != sensitivity;

	camera[0] += distance_x;
	camera[1] -= distance_y;

	if (camera[1] > 89.0f) {
		camera[1] = 89.0f;
	}
	if (camera[1] < -89.0f) {
		camera[1] = -89.0f;
	}

	glm::vec3 front;
	front.x = cos(glm::radians(camera[0])) * cos(glm::radians(camera[1]));
	front.y = sin(glm::radians(camera[1]));
	front.z = sin(glm::radians(camera[0])) * cos(glm::radians(camera[1]));

	center = glm::normalize(front);
}

void scrollInput(GLFWwindow* window, double x, double y) {
	if (camera[2] >= 1.0f && camera[2] <= 45.0f) {
		camera[2] -= y;
	}
	
	if (camera[2] <= 1.0f) {
		camera[2] = 1.0f;
	}

	if (camera[2] >= 45.0f) {
		camera[2] = 45.0f;
	}
}

int main(void) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OBJ Loader", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }

	
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	ImGui::StyleColorsDark();
	
	init(window);

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//read keyboard input in main loop
		//glfwSetCursorPosCallback(window, mouseInput);
		//glfwSetScrollCallback(window, scrollInput);
		processInput(window);
		display(window, glfwGetTime());
		glfwSwapBuffers(window);
		glfwPollEvents();

		//int display_w, display_h;
		//glfwGetFramebufferSize(window, &display_w, &display_h);
		//glViewport(0, 0, display_w, display_h);
	}

	
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);

}

