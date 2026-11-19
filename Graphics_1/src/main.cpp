/*  Base code by Alun Evans 2016 LaSalle (aevanss@salleurl.edu) modified by: Conrado Ruiz, Ferran Ruiz 2024*/

// student name: Teodor Arsovski, Mateja Dzunic

//include some standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>

//include OpenGL libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//include some custom code files
#include "glfunctions.h"	//include all OpenGL stuff
#include "Shader.h"			// class to compile shaders

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtx/rotate_vector.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// header file for the structure of an object
#include "Object3D.h"

// Declare the global vector for objects
std::vector<Object3D> objects;
using namespace std;
using namespace glm;
// camera position varaibles
vec3 cameraPos = vec3(8.0f, 8.0f, 8.0f);
vec3 cameraCenter = vec3(0.0f, 0.0f, -2.0f);
vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);
// more camera variables
float yaw1 = -90.0f;
float pitch1 = 0.0;

bool firstMouse = true;
float lastX = 400;
float  lastY = 300;

//global variables to help us do things
int g_ViewportWidth = 812; int g_ViewportHeight = 812; // Default window size, in pixels
double mouse_x, mouse_y;	//variables storing mouse position
const vec3 g_backgroundColor(0.2f, 0.2f, 0.2f); // background colour - a GLM 3-component vector
// rotation and movement variables 
float rotationAngle = 0.0f;
float rotationSpeed = 35.0f; // Adjust speed for better rotation visibility
double lastFrameTime = 0.0; // Time of the last frame
double deltaTime = 0.0;

// light direction
vec3 g_light_dir(10.0, 10.0, 10.0);

GLuint g_simpleShader = 0;	//shader identifier
GLuint g_simpleShader_sky = 0;// skybox shader identifier

float xpos = -13.85; // initial x position on the board of the first pawn (used as reference for the other peices)
float diff = 2.23; // difference of the xposition on the chessboard

// variables used for the animation
bool simulationRunning = false;
int phase = 0;
std::vector<vec3> locationsPawns(8);
std::vector<vec3> locationsPawns1(8);
std::vector<vec3> locationsBishops(2);
vec3 positionQueen;
vec3 positionKing;

GLuint texture_id_earth_normal; // global variable for the earth bump map
GLuint texture_id_earth_spec; // global varable for the earth spec map

Object3D firePlane;         // To store the loaded plane
GLuint fireTextureID;       // To store the fire sprite sheet texture
int totalFrames = 72;       // 6 rows x 12 columns
float frameDuration = 0.05f; // used to determine the speed at which the t3extures cycle
int currentFrame = 0;       // Current animation frame
float frameTimeAccumulator = 0.0f; // For frame timing
unsigned char* firePixels;

// function to reset the locations of the peices after the animation ends
void resetLocations() {
	for (int j = 0; j < 8; j++) {
		locationsPawns.at(j) = glm::vec3(0.17f, -1.0f, xpos + diff * j);
		locationsPawns1.at(j) = glm::vec3(11.25f, -1.0f, xpos + diff * j);
	}

	locationsBishops.at(0) = vec3(-2.2f, -1.0f, 6.55 + xpos + diff * 2);
	locationsBishops.at(1) = vec3(-2.2f, -1.0f, 6.55 + xpos + diff * 5);
	positionQueen = vec3(-2.2f, -1.0f, 2.75 + xpos + diff * 4);
	positionKing = vec3(13.35f, -1.0f, 1.0 + xpos + diff * 5);

	rotationAngle = 0.0;
	rotationSpeed = 35.0f;
}
// function to load a object and texture
void loadObject(Object3D obj, string filename, string path) {
	int width, height, numChannels;

	obj.filename = filename;
	obj.path = path;
	
	bool ret = tinyobj::LoadObj(obj.shapes, obj.filename.c_str());
	
	obj.vao.resize(obj.shapes.size());
	obj.numTriangles.resize(obj.shapes.size());

	if (ret) {
		cout << "OBJ file: " << obj.filename << " sucessfully loaded\n";
	}
	else {
		cout << "OBJ file: " << obj.filename << " file cannot be found\n";
	}

	for (int i = 0; i < obj.shapes.size(); i++) {
		obj.vao[i] = gl_createAndBindVAO();
		std::cout << "vao: " << obj.vao[i];
		gl_createAndBindAttribute(&(obj.shapes[i].mesh.positions[0]), obj.shapes[i].mesh.positions.size() * sizeof(float), g_simpleShader, "a_vertex", 3);
		gl_createIndexBuffer(&(obj.shapes[i].mesh.indices[0]), obj.shapes[i].mesh.indices.size() * sizeof(unsigned int));
		gl_createAndBindAttribute(&(obj.shapes[i].mesh.texcoords[0]), obj.shapes[i].mesh.texcoords.size() * sizeof(GLfloat), g_simpleShader, "a_uv", 2);
		gl_createAndBindAttribute(&(obj.shapes[i].mesh.normals[0]), obj.shapes[i].mesh.normals.size() * sizeof(float), g_simpleShader, "a_normal", 3);
		gl_unbindVAO();
		obj.numTriangles[i] = obj.shapes[i].mesh.indices.size() / 3;
	}

	stbi_set_flip_vertically_on_load(true); // Remove if texture is flipped.
	obj.pixels = stbi_load(obj.path.c_str(), &width, &height, &numChannels, 0);

	glGenTextures(1, &obj.textureID);
	glBindTexture(GL_TEXTURE_2D, obj.textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, // target
		0, // level = 0 base, no mipmap
		GL_RGB, // how the data will be stored(Grayscale, RGB, RGBA)
		width, // width of the image
		height, // height of the image
		0, //border
		GL_RGB, // format of original data
		GL_UNSIGNED_BYTE,// type of data
		obj.pixels);

	objects.push_back(obj);
}
// function that loads all the objects and textures
void load()
{
	//**********************
	// CODE TO SET GEOMETRY
	//**********************
	resetLocations();

	Object3D shuttel, sky, rope, board, pawn, pawn1, knight, knight1, 
		bishop, bishop1, king, king1, queen, queen1, rook, rook1, earth, rings, fire;

 	//**********************
	// CODE TO LOAD EVERYTHING INTO MEMORY
	//**********************
	int width, height, numChannels;
	//load the shader
	Shader simpleShader("src/shader.vert", "src/shader.frag");
	g_simpleShader = simpleShader.program;

	Shader simpleShader_sky("src/shader_sky.vert","src/shader_sky.frag");
	g_simpleShader_sky = simpleShader_sky.program;

	sky.filename = "assets/sphere.obj";
	sky.path = "textures/milkyway.bmp";
	bool ret_sky = tinyobj::LoadObj(sky.shapes, sky.filename.c_str());

	sky.vao.resize(sky.shapes.size());
	sky.numTriangles.resize(sky.shapes.size());

	for (int i = 0; i < sky.shapes.size(); i++) {
		sky.vao[0] = gl_createAndBindVAO();
		std::cout << "vao: " << sky.vao[i];
		gl_createAndBindAttribute(&(sky.shapes[i].mesh.positions[0]), sky.shapes[i].mesh.positions.size() * sizeof(float), g_simpleShader_sky, "a_vertex", 3);
		gl_createIndexBuffer(&(sky.shapes[i].mesh.indices[0]), sky.shapes[i].mesh.indices.size() * sizeof(unsigned int));
		gl_createAndBindAttribute(&(sky.shapes[i].mesh.texcoords[0]), sky.shapes[i].mesh.texcoords.size() * sizeof(GLfloat), g_simpleShader_sky, "a_uv", 2);
		gl_unbindVAO();
		sky.numTriangles[i] = sky.shapes[i].mesh.indices.size() / 3;
	}
	// texture 
	stbi_set_flip_vertically_on_load(true); // Remove if texture is flipped.
	sky.pixels = stbi_load(sky.path.c_str(), &width, &height, &numChannels, 0);

	glGenTextures(1, &sky.textureID);
	glBindTexture(GL_TEXTURE_2D, sky.textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,sky.pixels);
	
	objects.push_back(sky);

	loadObject(shuttel, "assets/shuttel.obj", "textures/shuttel.jpg");
	loadObject(rope, "assets/rope.obj", "textures/rope1.png");
	loadObject(board, "assets/chessboard.obj", "textures/proba.jpg");
	loadObject(pawn, "assets/chess_pawn.obj", "textures/pawn_light.jpg");
	loadObject(pawn1, "assets/chess_pawn.obj", "textures/pawn_dark.jpg");
	loadObject(knight, "assets/knight.obj", "textures/knight_light.jpg");
	loadObject(knight1, "assets/knight.obj", "textures/knight_dark.jpg");
	loadObject(bishop, "assets/bishop.obj", "textures/bishop_light.jpg");
	loadObject(bishop1, "assets/bishop.obj", "textures/bishop_dark.jpg");
	loadObject(king, "assets/king.obj", "textures/king_light.jpg");
	loadObject(king1, "assets/king.obj", "textures/king_dark.jpg");
	loadObject(queen, "assets/queen.obj", "textures/queen_light.jpg");
	loadObject(queen1, "assets/queen.obj", "textures/queen_dark.jpg");
	loadObject(rook, "assets/rook.obj", "textures/rook_light.jpg");
	loadObject(rook, "assets/rook.obj", "textures/rook_dark.jpg");
	loadObject(earth, "assets/sphere.obj", "textures/earth.bmp");

	char* pathN = "textures/earthnormal.bmp"; // Path to normal image
	int widthNormal, heightNormal, numChannelsNormal;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* pixelsNormal = stbi_load(pathN, &widthNormal, &heightNormal, &numChannelsNormal, 0); // Load image data
	// Depending on the number of channels of the image, we need to tell OpenGL if it is grayscale, RGB or RGBA (RGB +transparent channel)
	GLenum channelsNormal = GL_RGB;
	switch (numChannelsNormal) {
	case 1:
		channelsNormal = GL_RED;
		break;
	case 3:
		channelsNormal = GL_RGB;
		break;
	case 4:
		channelsNormal = GL_RGBA;
		break;
	}
	glGenTextures(1, &texture_id_earth_normal);
	glBindTexture(GL_TEXTURE_2D, texture_id_earth_normal);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, channelsNormal, widthNormal, heightNormal, 0, channelsNormal, GL_UNSIGNED_BYTE,
		pixelsNormal);

	char* pathSpec = "textures/earthspec.bmp"; // Path to specular image
	int widthSpec, heightSpec, numChannelsSpec;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* pixelsSpec = stbi_load(pathSpec, &widthSpec, &heightSpec, &numChannelsSpec, 0); // Load image data
	// Depending on the number of channels of the image, we need to tell OpenGL if it is grayscale, RGB or RGBA (RGB + transparent channel)
	GLenum channelsSpec = GL_RGB; // Default
	switch (numChannelsSpec) {
	case 1:
		channelsSpec = GL_RED;
		break;
	case 3:
		channelsSpec = GL_RGB;
		break;
	case 4:
		channelsSpec = GL_RGBA;
		break;
	}
	glGenTextures(1, &texture_id_earth_spec);
	glBindTexture(GL_TEXTURE_2D, texture_id_earth_spec);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, channelsSpec, widthSpec, heightSpec, 0, channelsSpec, GL_UNSIGNED_BYTE, pixelsSpec); 
	
	rings.filename = "assets/plane.obj"; 
	bool ret_rings = tinyobj::LoadObj(rings.shapes, rings.filename.c_str());

	if (!ret_rings || rings.shapes.empty()) {
		std::cerr << "Failed to load plane.obj for rings!" << std::endl;
	}

	rings.vao.resize(rings.shapes.size());
	rings.numTriangles.resize(rings.shapes.size());

	for (int i = 0; i < rings.shapes.size(); i++) {
		rings.vao[i] = gl_createAndBindVAO();
		gl_createAndBindAttribute(&(rings.shapes[i].mesh.positions[0]), rings.shapes[i].mesh.positions.size() * sizeof(float), g_simpleShader_sky, "a_vertex", 3);
		gl_createIndexBuffer(&(rings.shapes[i].mesh.indices[0]), rings.shapes[i].mesh.indices.size() * sizeof(unsigned int));
		gl_createAndBindAttribute(&(rings.shapes[i].mesh.texcoords[0]), rings.shapes[i].mesh.texcoords.size() * sizeof(GLfloat), g_simpleShader_sky, "a_uv", 2);
		gl_createAndBindAttribute(&(rings.shapes[i].mesh.normals[0]), rings.shapes[i].mesh.normals.size() * sizeof(float), g_simpleShader_sky, "a_normal", 3);
		gl_unbindVAO();

		rings.numTriangles[i] = rings.shapes[i].mesh.indices.size() / 3;
	}

	rings.path = "textures/rings.png"; 
	stbi_set_flip_vertically_on_load(true);
	rings.pixels = stbi_load(rings.path.c_str(), &width, &height, &numChannels, 0);

	glGenTextures(1, &rings.textureID);
	glBindTexture(GL_TEXTURE_2D, rings.textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rings.pixels);

	objects.push_back(rings);

	firePlane.filename = "assets/plane.obj"; // Use your actual path
	bool ret_firePlane = tinyobj::LoadObj(firePlane.shapes, firePlane.filename.c_str());

	if (!ret_firePlane || firePlane.shapes.empty()) {
		std::cerr << "Failed to load plane.obj for firePlane!" << std::endl;
	}

	firePlane.vao.resize(firePlane.shapes.size());
	firePlane.numTriangles.resize(firePlane.shapes.size());

	for (int i = 0; i < firePlane.shapes.size(); i++) {
		firePlane.vao[i] = gl_createAndBindVAO();
		gl_createAndBindAttribute(&(firePlane.shapes[i].mesh.positions[0]), firePlane.shapes[i].mesh.positions.size() * sizeof(float), g_simpleShader_sky, "a_vertex", 3);
		gl_createIndexBuffer(&(firePlane.shapes[i].mesh.indices[0]), firePlane.shapes[i].mesh.indices.size() * sizeof(unsigned int));
		gl_createAndBindAttribute(&(firePlane.shapes[i].mesh.texcoords[0]), firePlane.shapes[i].mesh.texcoords.size() * sizeof(GLfloat), g_simpleShader_sky, "a_uv", 2);
		gl_createAndBindAttribute(&(firePlane.shapes[i].mesh.normals[0]), firePlane.shapes[i].mesh.normals.size() * sizeof(float), g_simpleShader_sky, "a_normal", 3);
		gl_unbindVAO();

		firePlane.numTriangles[i] = firePlane.shapes[i].mesh.indices.size() / 3;
	}

	firePlane.path = "textures/flame.png"; 
	stbi_set_flip_vertically_on_load(true);
	firePlane.pixels = stbi_load(firePlane.path.c_str(), &width, &height, &numChannels, 0);

	glGenTextures(1, &firePlane.textureID);
	glBindTexture(GL_TEXTURE_2D, firePlane.textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, firePlane.pixels);

	objects.push_back(firePlane);
}
// ------------------------------------------------------------------------------------------
// This function actually draws to screen and called non-stop, in a loop
// ------------------------------------------------------------------------------------------
void draw()
{
	//clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	mat4 projection_matrix = perspective(
		90.0f, // Field of view
		1.0f, // Aspect ratio
		0.1f, // near plane (distance from camera)
		65.0f // Far plane (distance from camera)
	);
	// camera setup
	mat4 view_matrix = glm::lookAt(cameraPos, cameraCenter + cameraPos, cameraUp);
	GLuint view_loc = glGetUniformLocation(g_simpleShader, "u_view");
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	glUseProgram(g_simpleShader_sky);

	Object3D sky = objects.at(0);

	sky.modelLoc = glGetUniformLocation(g_simpleShader_sky,
		"u_model");
	GLuint view_loc_sky = glGetUniformLocation(g_simpleShader_sky,
		"u_view");
	GLuint projection_loc_sky = glGetUniformLocation(g_simpleShader_sky,
		"u_projection");
	//set MVP
	sky.model = translate(mat4(1.0f), cameraPos) * scale(mat4(1.0f), vec3(10.0f, 10.0f, 10.0f)); // folows the camera
	//send all values to shader
	glUniformMatrix4fv(sky.modelLoc, 1, GL_FALSE, glm::value_ptr(sky.model));
	glUniformMatrix4fv(view_loc_sky, 1, GL_FALSE, glm::value_ptr(view_matrix));
	glUniformMatrix4fv(projection_loc_sky, 1, GL_FALSE, glm::value_ptr(projection_matrix));

	GLuint u_texture_sky = glGetUniformLocation(g_simpleShader_sky,"u_texture");
	glUniform1i(u_texture_sky, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sky.textureID);
	gl_bindVAO(sky.vao[0]);
	glDrawElements(GL_TRIANGLES, 3 * sky.numTriangles[0], GL_UNSIGNED_INT, 0);

	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);

	glUseProgram(g_simpleShader);
	// light setup
	GLuint light_loc = glGetUniformLocation(g_simpleShader, "u_light_dir");
	glUniform3f(light_loc, g_light_dir.x, g_light_dir.y, g_light_dir.z);

	GLuint cam_pos_loc = glGetUniformLocation(g_simpleShader, "u_cam_pos");
	// this is the camera position, eye/cameraPos
	glUniform3f(cam_pos_loc, cameraPos.x, cameraPos.y, cameraPos.z);
	GLuint ambient_loc = glGetUniformLocation(g_simpleShader, "u_ambient");
	glUniform3f(ambient_loc, 0.1, 0.1, 0.1); // grey shadows
	GLuint diffuse_loc = glGetUniformLocation(g_simpleShader, "u_diffuse");
	glUniform3f(diffuse_loc, 1.0, 1.0, 1.0); // white light
	GLuint specular_loc = glGetUniformLocation(g_simpleShader, "u_specular");
	glUniform3f(specular_loc, 1.0, 1.0, 1.0);
	GLuint shininess_loc = glGetUniformLocation(g_simpleShader, "u_shininess");
	glUniform1f(shininess_loc, 20.0);

	GLuint projection_loc = glGetUniformLocation(g_simpleShader, "u_projection");
	projection_matrix = perspective(
		90.0f, // Field of view
		1.0f, // Aspect ratio
		0.1f, // near plane (distance from camera)
		100.0f // Far plane (distance from camera)
	);
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

	// camera setup
	view_matrix = glm::lookAt(cameraPos, cameraCenter + cameraPos, cameraUp);
	view_loc = glGetUniformLocation(g_simpleShader, "u_view");
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));

	// drawing of the objects
	Object3D shuttel = objects.at(1);
	for (int i = 0; i < shuttel.vao.size(); i++) {
		gl_bindVAO(shuttel.vao[i]);
		shuttel.model = translate(mat4(1.0f), vec3(0.0f, 15.0f, -2.0f)) * glm::rotate(mat4(1.0f), 90.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(2.5f, 2.5f, 2.5f));
		shuttel.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
		glUniformMatrix4fv(shuttel.modelLoc, 1, GL_FALSE, glm::value_ptr(shuttel.model));

		shuttel.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
		glUniform1i(shuttel.u_texture, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, shuttel.textureID);

		shuttel.normalMatrix = mat3(transpose(inverse(shuttel.model)));
		shuttel.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
		glUniformMatrix3fv(shuttel.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(shuttel.normalMatrix));

		glDrawElements(GL_TRIANGLES, 3 * shuttel.numTriangles[0], GL_UNSIGNED_INT, 0);
	}
	Object3D rope = objects.at(2);
	for (int i = 0; i < rope.vao.size(); i++) {
		gl_bindVAO(rope.vao[i]);
		rope.model = translate(mat4(1.0f), vec3(0.0f, -2.0f, -2.1f)) * scale(mat4(1.0f), vec3(0.5f, 2.15f, 0.5f));
		rope.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
		glUniformMatrix4fv(rope.modelLoc, 1, GL_FALSE, glm::value_ptr(rope.model));

		rope.normalMatrix = mat3(transpose(inverse(rope.model)));
		rope.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
		glUniformMatrix3fv(rope.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(rope.normalMatrix));

		rope.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
		glUniform1i(rope.u_texture, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rope.textureID);

		glDrawElements(GL_TRIANGLES, 3 * rope.numTriangles[0], GL_UNSIGNED_INT, 0);
	}
	Object3D board = objects.at(3);
	for (int i = 0; i < board.vao.size(); i++) {
		gl_bindVAO(board.vao[i]);
		board.model = translate(mat4(1.0f), vec3(0.0f, -2.0f, -2.0f)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f)) * glm::rotate(mat4(1.0f), 90.0f, vec3(0.0, 0.0, 1.0));
		board.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
		glUniformMatrix4fv(board.modelLoc, 1, GL_FALSE, glm::value_ptr(board.model));

		board.normalMatrix = mat3(transpose(inverse(board.model)));
		board.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
		glUniformMatrix3fv(board.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(board.normalMatrix));

		board.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
		glUniform1i(board.u_texture, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, board.textureID);

		glDrawElements(GL_TRIANGLES, 3 * board.numTriangles[0], GL_UNSIGNED_INT, 0);
	}
	Object3D pawn = objects.at(4);
	Object3D pawn1 = objects.at(5);
	Object3D knight = objects.at(6);
	Object3D knight1 = objects.at(7);
	Object3D bishop = objects.at(8);
	Object3D bishop1 = objects.at(9);
	Object3D king = objects.at(10);
	Object3D king1 = objects.at(11);
	Object3D queen = objects.at(12);
	Object3D queen1 = objects.at(13);
	Object3D rook = objects.at(14);
	Object3D rook1 = objects.at(15);
	Object3D earth = objects.at(16);
	Object3D rings = objects.at(17);
	Object3D fire = objects.at(18);

	for (int i = 0; i < earth.vao.size(); i++) {
		gl_bindVAO(earth.vao[i]);
		earth.model = translate(mat4(1.0f), vec3(-20.0f, -25.0f, -30.0f)) * scale(mat4(1.0f), vec3(27.0f, 27.0f, 27.0f));
		earth.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
		glUniformMatrix4fv(earth.modelLoc, 1, GL_FALSE, glm::value_ptr(earth.model));

		earth.normalMatrix = mat3(transpose(inverse(earth.model)));
		earth.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
		glUniformMatrix3fv(earth.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(earth.normalMatrix));

		earth.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
		glUniform1i(earth.u_texture, 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, earth.textureID);

		GLuint u_texture_normal = glGetUniformLocation(g_simpleShader, "u_texture_normal");
		glUniform1i(u_texture_normal, 1);
		GLuint flag = glGetUniformLocation(g_simpleShader, "u_flag");
		glUniform1i(flag, 1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture_id_earth_normal);

		GLuint u_texture_spec = glGetUniformLocation(g_simpleShader, "u_texture_spec");
		glUniform1i(u_texture_spec, 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, texture_id_earth_spec);

		glDrawElements(GL_TRIANGLES, 3 * earth.numTriangles[0], GL_UNSIGNED_INT, 0);
		glUniform1i(flag, 0);


	}

	for (int j = 0; j < 8; j++) {
		for (int i = 0; i < pawn.vao.size(); i++) {
			gl_bindVAO(pawn.vao[i]);
			pawn.model = translate(mat4(1.0f), locationsPawns.at(j)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f));
			pawn.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
			glUniformMatrix4fv(pawn.modelLoc, 1, GL_FALSE, glm::value_ptr(pawn.model));

			pawn.normalMatrix = mat3(transpose(inverse(pawn.model)));
			pawn.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
			glUniformMatrix3fv(pawn.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(pawn.normalMatrix));

			pawn.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
			glUniform1i(pawn.u_texture, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, pawn.textureID);

			glDrawElements(GL_TRIANGLES, 3 * pawn.numTriangles[0], GL_UNSIGNED_INT, 0);
			gl_bindVAO(pawn1.vao[i]);
			pawn1.model = translate(mat4(1.0f), locationsPawns1.at(j)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f));
			pawn1.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
			glUniformMatrix4fv(pawn1.modelLoc, 1, GL_FALSE, glm::value_ptr(pawn1.model));

			pawn1.normalMatrix = mat3(transpose(inverse(pawn1.model)));
			pawn1.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
			glUniformMatrix3fv(pawn1.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(pawn1.normalMatrix));

			pawn1.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
			glUniform1i(pawn1.u_texture, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, pawn1.textureID);

			glDrawElements(GL_TRIANGLES, 3 * pawn1.numTriangles[0], GL_UNSIGNED_INT, 0);

		}
		if (j == 1) {
			for (int i = 0; i < knight.shapes.size(); i++) {
				gl_bindVAO(knight.vao[i]);
				knight.model = translate(mat4(1.0f), vec3(-2.2f, -1.0f, 8.2 + xpos + diff * j)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f)) * glm::rotate(mat4(1.0f), -90.0f, vec3(0.0, 0.0, 1.0));
				knight.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
				glUniformMatrix4fv(knight.modelLoc, 1, GL_FALSE, glm::value_ptr(knight.model));

				knight.normalMatrix = mat3(transpose(inverse(knight.model)));
				knight.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
				glUniformMatrix3fv(knight.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(knight.normalMatrix));

				knight.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
				glUniform1i(knight.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, knight.textureID);

				glDrawElements(GL_TRIANGLES, 3 * knight.numTriangles[0], GL_UNSIGNED_INT, 0);
				
				gl_bindVAO(knight1.vao[i]);
				knight1.model = translate(mat4(1.0f), vec3(2.05f, -1.0f, xpos + diff * j)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f)) * glm::rotate(mat4(1.0f), 90.0f, vec3(0.0, 0.0, 1.0));
				knight1.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
				glUniformMatrix4fv(knight1.modelLoc, 1, GL_FALSE, glm::value_ptr(knight1.model));

				knight1.normalMatrix = mat3(transpose(inverse(knight1.model)));
				knight1.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
				glUniformMatrix3fv(knight1.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(knight1.normalMatrix));

				knight1.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
				glUniform1i(knight1.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, knight1.textureID);

				glDrawElements(GL_TRIANGLES, 3 * knight1.numTriangles[0], GL_UNSIGNED_INT, 0);
			}
		}
		if (j == 6) {
			for (int i = 0; i < knight.shapes.size(); i++) {
				gl_bindVAO(knight.vao[i]);
				knight.model = translate(mat4(1.0f), vec3(-2.2f, -1.0f, 8.2 + xpos + diff * 6)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f)) * glm::rotate(mat4(1.0f), -90.0f, vec3(0.0, 0.0, 1.0));
				knight.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
				glUniformMatrix4fv(knight.modelLoc, 1, GL_FALSE, glm::value_ptr(knight.model));

				knight.normalMatrix = mat3(transpose(inverse(knight.model)));
				knight.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
				glUniformMatrix3fv(knight.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(knight.normalMatrix));

				knight.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
				glUniform1i(knight.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, knight.textureID);

				glDrawElements(GL_TRIANGLES, 3 * knight.numTriangles[0], GL_UNSIGNED_INT, 0);

				gl_bindVAO(knight1.vao[i]);
				knight1.model = translate(mat4(1.0f), vec3(2.05f, -1.0f, xpos + diff * j)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f)) * glm::rotate(mat4(1.0f), 90.0f, vec3(0.0, 0.0, 1.0));
				knight1.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
				glUniformMatrix4fv(knight1.modelLoc, 1, GL_FALSE, glm::value_ptr(knight1.model));

				knight1.normalMatrix = mat3(transpose(inverse(knight1.model)));
				knight1.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
				glUniformMatrix3fv(knight1.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(knight1.normalMatrix));

				knight1.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
				glUniform1i(knight1.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, knight1.textureID);

				glDrawElements(GL_TRIANGLES, 3 * knight1.numTriangles[0], GL_UNSIGNED_INT, 0);
			}
		}
		if (j == 2 || j == 5) {
			for (int i = 0; i < bishop.shapes.size(); i++) {
				gl_bindVAO(bishop.vao[i]);
				if (j == 2) {
					bishop.model = translate(mat4(1.0f), locationsBishops.at(0)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f)) * glm::rotate(mat4(1.0f), -90.0f, vec3(0.0, 0.0, 1.0));
				}
				else {
					bishop.model = translate(mat4(1.0f), locationsBishops.at(1)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f)) * glm::rotate(mat4(1.0f), -90.0f, vec3(0.0, 0.0, 1.0));
				}
				bishop.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
				glUniformMatrix4fv(bishop.modelLoc, 1, GL_FALSE, glm::value_ptr(bishop.model));

				bishop.normalMatrix = mat3(transpose(inverse(bishop.model)));
				bishop.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
				glUniformMatrix3fv(bishop.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(bishop.normalMatrix));

				bishop.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
				glUniform1i(bishop.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, bishop.textureID);

				glDrawElements(GL_TRIANGLES, 3 * bishop.numTriangles[0], GL_UNSIGNED_INT, 0);

				gl_bindVAO(bishop1.vao[i]);
				bishop1.model = translate(mat4(1.0f), vec3(2.05f, -1.0f, 1.55 + xpos + diff * j)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f)) * glm::rotate(mat4(1.0f), 90.0f, vec3(0.0, 0.0, 1.0));
				bishop1.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
				glUniformMatrix4fv(bishop1.modelLoc, 1, GL_FALSE, glm::value_ptr(bishop1.model));

				bishop1.normalMatrix = mat3(transpose(inverse(bishop1.model)));
				bishop1.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
				glUniformMatrix3fv(bishop1.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(bishop1.normalMatrix));

				bishop1.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
				glUniform1i(bishop1.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, bishop1.textureID);

				glDrawElements(GL_TRIANGLES, 3 * bishop1.numTriangles[0], GL_UNSIGNED_INT, 0);
			}
		}
		if (j == 4) {
			for (int i = 0; i < queen.shapes.size(); i++) {
				gl_bindVAO(queen.vao[i]);
				queen.model = translate(mat4(1.0f), positionQueen) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f)) * glm::rotate(mat4(1.0f), -90.0f, vec3(0.0, 0.0, 1.0));
				queen.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
				glUniformMatrix4fv(queen.modelLoc, 1, GL_FALSE, glm::value_ptr(queen.model));

				queen.normalMatrix = mat3(transpose(inverse(queen.model)));
				queen.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
				glUniformMatrix3fv(queen.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(queen.normalMatrix));

				queen.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
				glUniform1i(queen.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, queen.textureID);

				glDrawElements(GL_TRIANGLES, 3 * queen.numTriangles[0], GL_UNSIGNED_INT, 0);
			}
			for (int i = 0; i < queen1.shapes.size(); i++) {
				gl_bindVAO(queen1.vao[i]);
				queen1.model = translate(mat4(1.0f), vec3(13.35f, -1.0f, 2.7 + xpos + diff * j)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f)) * glm::rotate(mat4(1.0f), -90.0f, vec3(0.0, 0.0, 1.0));
				queen1.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
				glUniformMatrix4fv(queen1.modelLoc, 1, GL_FALSE, glm::value_ptr(queen1.model));

				queen1.normalMatrix = mat3(transpose(inverse(queen1.model)));
				queen1.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
				glUniformMatrix3fv(queen1.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(queen1.normalMatrix));

				queen1.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
				glUniform1i(queen1.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, queen1.textureID);

				glDrawElements(GL_TRIANGLES, 3 * queen1.numTriangles[0], GL_UNSIGNED_INT, 0);
			}
		}
		if (j == 5) {
			for (int i = 0; i < king1.shapes.size(); i++) {
				gl_bindVAO(king1.vao[i]);
				king1.model = translate(mat4(1.0f), positionKing) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f)) * glm::rotate(mat4(1.0f), -90.0f, vec3(0.0, 0.0, 1.0)) * glm::rotate(mat4(1.0f), -rotationAngle, vec3(1.0, 0.0, 0.0));
				king1.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
				glUniformMatrix4fv(king1.modelLoc, 1, GL_FALSE, glm::value_ptr(king1.model));

				king1.normalMatrix = mat3(transpose(inverse(king1.model)));
				king1.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
				glUniformMatrix3fv(king1.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(king1.normalMatrix));

				king1.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
				glUniform1i(king1.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, king1.textureID);

				glDrawElements(GL_TRIANGLES, 3 * king1.numTriangles[0], GL_UNSIGNED_INT, 0);
			}
			for (int i = 0; i < king.shapes.size(); i++) {
				gl_bindVAO(king.vao[i]);
				king.model = translate(mat4(1.0f), vec3(-2.2f, -1.0f, 1.0 + xpos + diff * j)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f)) * glm::rotate(mat4(1.0f), -90.0f, vec3(0.0, 0.0, 1.0));
				king.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
				glUniformMatrix4fv(king.modelLoc, 1, GL_FALSE, glm::value_ptr(king.model));

				king.normalMatrix = mat3(transpose(inverse(king.model)));
				king.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
				glUniformMatrix3fv(king.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(king.normalMatrix));

				king.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
				glUniform1i(king.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, king.textureID);

				glDrawElements(GL_TRIANGLES, 3 * king.numTriangles[0], GL_UNSIGNED_INT, 0);
			}

		}
		if (j == 0) {
			for (int i = 0; i < rook.shapes.size(); i++) {
				gl_bindVAO(rook.vao[i]);
				rook.model = translate(mat4(1.0f), vec3(-13.6f, -1.0f, -1.7 + xpos + diff * j)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f)) * glm::rotate(mat4(1.0f), -90.0f, vec3(0.0, 0.0, 1.0));
				rook.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
				glUniformMatrix4fv(rook.modelLoc, 1, GL_FALSE, glm::value_ptr(rook.model));

				rook.normalMatrix = mat3(transpose(inverse(rook.model)));
				rook.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
				glUniformMatrix3fv(rook.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(rook.normalMatrix));

				rook.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
				glUniform1i(rook.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, rook.textureID);

				glDrawElements(GL_TRIANGLES, 3 * rook.numTriangles[0], GL_UNSIGNED_INT, 0);

				gl_bindVAO(rook1.vao[i]);
				rook1.model = translate(mat4(1.0f), vec3(13.35f, -1.0f, 9.8 + xpos + diff * j)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f)) * glm::rotate(mat4(1.0f), 90.0f, vec3(0.0, 0.0, 1.0));
				rook1.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
				glUniformMatrix4fv(rook1.modelLoc, 1, GL_FALSE, glm::value_ptr(rook1.model));

				rook1.normalMatrix = mat3(transpose(inverse(rook1.model)));
				rook1.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
				glUniformMatrix3fv(rook1.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(rook1.normalMatrix));

				rook1.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
				glUniform1i(rook1.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, rook1.textureID);

				glDrawElements(GL_TRIANGLES, 3 * rook1.numTriangles[0], GL_UNSIGNED_INT, 0);
			}
		}
		if (j == 7) {
			for (int i = 0; i < rook.shapes.size(); i++) {
				gl_bindVAO(rook.vao[i]);
				rook.model = translate(mat4(1.0f), vec3(-13.6f, -1.0f, -1.7 + xpos + diff * j)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f)) * glm::rotate(mat4(1.0f), -90.0f, vec3(0.0, 0.0, 1.0));
				rook.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
				glUniformMatrix4fv(rook.modelLoc, 1, GL_FALSE, glm::value_ptr(rook.model));

				rook.normalMatrix = mat3(transpose(inverse(rook.model)));
				rook.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
				glUniformMatrix3fv(rook.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(rook.normalMatrix));

				rook.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
				glUniform1i(rook.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, rook.textureID);

				glDrawElements(GL_TRIANGLES, 3 * rook.numTriangles[0], GL_UNSIGNED_INT, 0);

				gl_bindVAO(rook1.vao[i]);
				rook1.model = translate(mat4(1.0f), vec3(13.35f, -1.0f, 9.8 + xpos + diff * j)) * glm::rotate(mat4(1.0f), 270.0f, vec3(1.0, 0.0, 0.0)) * scale(mat4(1.0f), vec3(0.35f, 0.35f, 0.35f)) * glm::rotate(mat4(1.0f), 90.0f, vec3(0.0, 0.0, 1.0));
				rook1.modelLoc = glGetUniformLocation(g_simpleShader, "u_model");
				glUniformMatrix4fv(rook1.modelLoc, 1, GL_FALSE, glm::value_ptr(rook1.model));

				rook1.normalMatrix = mat3(transpose(inverse(rook1.model)));
				rook1.normalMatrixLoc = glGetUniformLocation(g_simpleShader, "u_normalMatrix");
				glUniformMatrix3fv(rook1.normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(rook1.normalMatrix));

				rook1.u_texture = glGetUniformLocation(g_simpleShader, "u_texture");
				glUniform1i(rook1.u_texture, 0);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, rook1.textureID);

				glDrawElements(GL_TRIANGLES, 3 * rook1.numTriangles[0], GL_UNSIGNED_INT, 0);
			}
		}
	}

	glUseProgram(g_simpleShader_sky);
	// enable blending for the last 2 objects
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);

	rings.u_texture = glGetUniformLocation(g_simpleShader_sky, "u_texture");

	glUniform1i(rings.u_texture, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rings.textureID);
	rings.modelLoc = glGetUniformLocation(g_simpleShader_sky, "u_model");
	rings.model = translate(mat4(1.0f), vec3(-17.5f, -10.0f, -22.0f)) * rotate(mat4(1.0f), 105.0f, vec3(1.0, 0.0, 0.0f)) * scale(mat4(1.0f), vec3(80.0, 80.0, 80.0));
	glUniformMatrix4fv(rings.modelLoc, 1, GL_FALSE, glm::value_ptr(rings.model));
	view_loc = glGetUniformLocation(g_simpleShader_sky, "u_view");
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	projection_loc = glGetUniformLocation(g_simpleShader_sky, "u_projection");
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
	gl_bindVAO(rings.vao[0]);
	glDrawElements(GL_TRIANGLES, 3 * rings.numTriangles[0], GL_UNSIGNED_INT, 0);

	GLuint isAnimatedLoc = glGetUniformLocation(g_simpleShader_sky, "u_isAnimated");
	GLuint textureGridLoc = glGetUniformLocation(g_simpleShader_sky, "u_textureGrid");
	GLuint currentFrameLoc = glGetUniformLocation(g_simpleShader_sky, "u_currentFrame");
	
	glUniform1i(isAnimatedLoc, true);               // Enable animation
	glUniform2f(textureGridLoc, 12.0f, 6.0f);       // 12 columns, 6 rows
	glUniform1i(currentFrameLoc, currentFrame);     // Set current frame index
	
	vec3 firePosition = vec3(0.0f, 10.25f, -1.35f);
	vec3 upVector = vec3(0.0f, 1.0f, 0.0f);    

	vec3 lookDir = normalize(vec3(cameraPos.x, 0.0f, cameraPos.z) - vec3(firePosition.x, 0.0f, firePosition.z));
	vec3 right = normalize(cross(upVector, lookDir));
	mat4 billboardMatrix = mat4(vec4(right, 0.0f), vec4(upVector, 0.0f), vec4(-lookDir, 0.0f), vec4(0.0f, 0.0f, 0.0f, 1.0f));

	fire.u_texture = glGetUniformLocation(g_simpleShader_sky, "u_texture");

	glUniform1i(fire.u_texture, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fire.textureID);
	
	fire.model = translate(mat4(1.0f), vec3(0.0f, 10.25f, -1.35f)) * billboardMatrix * scale(mat4(1.0f), vec3(5.0f, 5.0f, 5.0f)) * rotate(mat4(1.0f), 180.0f, vec3(1.0, 0.0, 0.0f));
	fire.modelLoc = glGetUniformLocation(g_simpleShader_sky, "u_model");
	glUniformMatrix4fv(fire.modelLoc, 1, GL_FALSE, glm::value_ptr(fire.model));

	view_loc = glGetUniformLocation(g_simpleShader_sky, "u_view");
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	projection_loc = glGetUniformLocation(g_simpleShader_sky, "u_projection");
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
	gl_bindVAO(fire.vao[0]);
	glDrawElements(GL_TRIANGLES, 3 * fire.numTriangles[0], GL_UNSIGNED_INT, 0);

	glUniform1i(isAnimatedLoc, false); // so that the skybox is drawn good
	glDisable(GL_BLEND);

}

// ------------------------------------------------------------------------------------------
// This function is called every time you press a screen
// ------------------------------------------------------------------------------------------
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, 1);
	//reload
	float const cameraSpeed = 0.25;
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		load();
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cameraPos += glm::normalize(glm::cross(cameraCenter, cameraUp)) * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cameraPos -= glm::normalize(glm::cross(cameraCenter, cameraUp)) * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cameraPos += cameraSpeed * cameraCenter;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cameraPos -= cameraSpeed * cameraCenter;
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		simulationRunning = true;
	}
	if (key == GLFW_KEY_N && action == GLFW_PRESS) {
		simulationRunning = false;
		phase = 0;
		resetLocations();
	}
}
// ------------------------------------------------------------------------------------------
// This function is called every time you click the mouse
// ------------------------------------------------------------------------------------------
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        cout << "Left mouse down at" << mouse_x << ", " << mouse_y << endl;
    }
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

	if (firstMouse) // initially set to true
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
	lastX = xpos;
	lastY = ypos;

	const float sensitivity = 0.2f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw1 += xoffset;
	pitch1 += yoffset;

	if (pitch1 > 89.0f)
		pitch1 = 89.0f;
	if (pitch1 < -89.0f)
		pitch1 = -89.0f;

	glm::vec3 direction;
	direction.x = cos(glm::radians(yaw1)) * cos(glm::radians(pitch1));
	direction.y = sin(glm::radians(pitch1));
	direction.z = sin(glm::radians(yaw1)) * cos(glm::radians(pitch1));
	cameraCenter = glm::normalize(direction);
}
// function that runs the aimation
void startAnimation() {
	if (locationsPawns.at(4).x < 4.55) {
		locationsPawns.at(4).x = locationsPawns.at(4).x + 3.5f * deltaTime;
	}
	else {
		phase = 1;
	}
	if (phase == 1 && locationsPawns1.at(5).x > 6.9) {
		locationsPawns1.at(5).x = locationsPawns1.at(5).x - 3.5f * deltaTime;
	}
	else if (locationsPawns1.at(5).x <= 6.9) {
		phase = 2;
	}
	if (phase == 2 && locationsBishops.at(1).x < 2.215) {
		locationsBishops.at(1).x = locationsBishops.at(1).x + 3.5f * deltaTime;
		locationsBishops.at(1).z = locationsBishops.at(1).z - 3.5f * deltaTime;
	}
	else if (locationsBishops.at(1).x >= 2.215) {
		phase = 3;
	}
	if (phase == 3 && locationsPawns1.at(6).x > 6.9) {
		locationsPawns1.at(6).x = locationsPawns1.at(6).x - 3.5f * deltaTime;
	}
	else if (locationsPawns1.at(6).x <= 6.9) {
		phase = 4;
	}
	if (phase == 4 && positionQueen.z < 6.63) {
		positionQueen.z = positionQueen.z + 3.5f * deltaTime;
		positionQueen.x = positionQueen.x + 3.525f * deltaTime;
	}
	else if (positionQueen.z >= 6.63) {
		phase = 5;
	}
	if (phase == 5 && rotationAngle < 55) {
		rotationAngle += rotationSpeed * deltaTime;
	}
	else if (rotationAngle >= 55) {
		phase = 6;
		rotationSpeed = 10.0;
	}
	if (phase == 6 && positionKing.y > -120) {
		positionKing.y = positionKing.y - 6 * deltaTime;
		rotationAngle += rotationSpeed * deltaTime;
	}
}
int main(void)
{
	//setup window and boring stuff, defined in glfunctions.cpp
	GLFWwindow* window;
	if (!glfwInit())return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(g_ViewportWidth, g_ViewportHeight, "Hello OpenGL!", NULL, NULL);
	if (!window) {glfwTerminate();	return -1;}
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	glewInit();

	//input callbacks
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);

	glClearColor(g_backgroundColor.x, g_backgroundColor.y, g_backgroundColor.z, 1.0f);
	glfwSetCursorPosCallback(window, mouse_callback);

	//load all the resources
	load();

    // Loop until the user closes the window
    while (!glfwWindowShouldClose(window))
    {
		double currentFrameTime = glfwGetTime(); // current time
		deltaTime = currentFrameTime - lastFrameTime; // Time difference between frames
		lastFrameTime = currentFrameTime; //update last frame time

		frameTimeAccumulator += deltaTime; // Accumulate time
		if (frameTimeAccumulator > frameDuration) {
			frameTimeAccumulator = 0.0f;
			currentFrame = (currentFrame + 1) % totalFrames; // Loop through frames
		}

		if (simulationRunning) {
			startAnimation();
		}

		draw();

        // Swap front and back buffers
        glfwSwapBuffers(window);
        
        // Poll for and process events
        glfwPollEvents();
        
        //mouse position must be tracked constantly (callbacks do not give accurate delta)
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
    }
    //terminate glfw and exit
    glfwTerminate();
    return 0;
}