#define _CRT_SECURE_NO_DEPRECATE

//Some Windows Headers (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include "maths_funcs.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Assimp includes

#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.



/*----------------------------------------------------------------------------
                   MESH TO LOAD
  ----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define STADIUM_NAME "../neweststadium.obj"
#define PLAYER_NAME "../luiginoarm.obj"
#define BALL_NAME "../footballsmall.obj"
#define ENEMY_NAME "../patrick.obj"
#define PLAYER_ARM "../luigiarmonaxis.obj"
/*----------------------------------------------------------------------------
  ----------------------------------------------------------------------------*/

#define PI 3.14159265359


//change if more meshes loaded
int numberOfMeshes = 5;

int g_point_count[5];
std::vector<float> g_vp[5], g_vn[5], g_vt[5];
unsigned int vaos[5];

// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;
GLuint shaderProgramID;


unsigned int mesh_vao = 0;
int width = 800;
int height = 600;

GLuint loc1, loc2, loc3;
GLfloat rotate_y = 0.0f;

//camera controls
glm::vec3 cameraPos = glm::vec3(0.0f, 2.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

GLfloat yaw = -90.0f;	// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
GLfloat pitch = 0.0f;
GLfloat lastX = (float)width / 2.0;
GLfloat lastY = (float)height / 2.0;
GLfloat fov = 45.0f;

//player movement
GLfloat xplayer = 0.0f;
GLfloat yplayer = 0.0f;
GLfloat zplayer = 0.0f;

//3rd person camera
GLfloat distanceFromPlayer = 50.0f;
GLfloat angleAroundPlayer = 30.0f;

//ball movement
GLfloat xBall = 0.0f;
GLfloat yBall = 0.0f;
GLfloat zBall = 0.0f;

bool thirdPerson = true;

bool throwBall = false;

bool throwing = false;

class Enemy {
	float xposition;
	float yposition;
	float zposition;
	public:
		void setPos(float x, float y, float z) {
			xposition = x;
			yposition = y;
			zposition = z;
		}

		float getXPos() {
			return xposition;
		}

		float getYPos() {
			return yposition;
		}

		float getZPos() {
			return zposition;
		}

		void moveToPlayer() {
			if (xposition > 0) {
				xposition -= 0.01;
			}
			if (yposition > 0) {
				yposition -= 0.01;
			}
			if (zposition < 0) {
				zposition += 0.01;
			}
		}
};

class Player {
	float xposition;
	float yposition;
	float zposition;
public:
	void setPos(float x, float y, float z) {
		xposition = x;
		yposition = y;
		zposition = z;
	}

	float getXPos() {
		return xposition;
	}

	float getYPos() {
		return yposition;
	}

	float getZPos() {
		return zposition;
	}
};

class Ball {
	float xposition;
	float yposition;
	float zposition;
public:
	void setPos(float x, float y, float z) {
		xposition = x;
		yposition = y;
		zposition = z;
	}

	float getXPos() {
		return xposition;
	}

	float getYPos() {
		return yposition;
	}

	float getZPos() {
		return zposition;
	}
};



Enemy enemies[5];

#pragma region MESH LOADING
/*----------------------------------------------------------------------------
                   MESH LOADING FUNCTION
  ----------------------------------------------------------------------------*/
float calculateHorizontalDistance() {
	return distanceFromPlayer * cos((pitch * (PI / 180)));
}

float calculateVerticalDistance() {
	return distanceFromPlayer * sin((pitch * (PI / 180)));
}

void calculateCameraPosition() {
	float horizontalDistance = distanceFromPlayer * cos((pitch * (PI / 180)));
	float verticalDistance = distanceFromPlayer * sin((pitch * (PI / 180)));
	float theta = angleAroundPlayer;
	float offsetX = horizontalDistance * sin((theta * (PI/180)));
	float offsetZ = horizontalDistance * cos((theta * (PI / 180)));
	cameraPos = glm::vec3((xplayer), (yplayer + 0.5f), (zplayer + 1.0));
}

bool load_mesh(const char* file_name[]) {
	for (int i = 0; i < numberOfMeshes; i++) {
		const aiScene* scene = aiImportFile(file_name[i], aiProcess_Triangulate); // TRIANGLES!
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		if (!scene) {
			fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
			return false;
		}
		printf("  %i animations\n", scene->mNumAnimations);
		printf("  %i cameras\n", scene->mNumCameras);
		printf("  %i lights\n", scene->mNumLights);
		printf("  %i materials\n", scene->mNumMaterials);
		printf("  %i meshes\n", scene->mNumMeshes);
		printf("  %i textures\n", scene->mNumTextures);
		g_point_count[i] = 0;

		for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
			const aiMesh* mesh = scene->mMeshes[m_i];
			printf("    %i vertices in mesh\n", mesh->mNumVertices);
			g_point_count[i] += mesh->mNumVertices;
			for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
				if (mesh->HasPositions()) {
					const aiVector3D* vp = &(mesh->mVertices[v_i]);
					//printf ("      vp %i (%f,%f,%f)\n", v_i, vp->x, vp->y, vp->z);
					g_vp[i].push_back(vp->x);
					g_vp[i].push_back(vp->y);
					g_vp[i].push_back(vp->z);
				}
				if (mesh->HasNormals()) {
					const aiVector3D* vn = &(mesh->mNormals[v_i]);
					//printf ("      vn %i (%f,%f,%f)\n", v_i, vn->x, vn->y, vn->z);
					g_vn[i].push_back(vn->x);
					g_vn[i].push_back(vn->y);
					g_vn[i].push_back(vn->z);
				}
				if (mesh->HasTextureCoords(0)) {
					const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
					//printf ("      vt %i (%f,%f)\n", v_i, vt->x, vt->y);
					g_vt[i].push_back(vt->x);
					g_vt[i].push_back(vt->y);
				}
				if (mesh->HasTangentsAndBitangents()) {
					// NB: could store/print tangents here
				}
			}
		}

		aiReleaseImport(scene);
	}
	return true;
}

#pragma endregion MESH LOADING

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS

// Create a NULL-terminated string by reading the provided file
char* readShaderSource(const char* shaderFile) {   
    FILE* fp = fopen(shaderFile, "rb"); //!->Why does binary flag "RB" work and not "R"... wierd msvc thing?

    if ( fp == NULL ) { return NULL; }

    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);

    fseek(fp, 0L, SEEK_SET);
    char* buf = new char[size + 1];
    fread(buf, 1, size, fp);
    buf[size] = '\0';

    fclose(fp);

    return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }
	const char* pShaderSource = readShaderSource( pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
    glCompileShader(ShaderObj);
    GLint success;
	// check for shader related errors using glGetShaderiv
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }
	// Attach the compiled shader object to the program object
    glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
    shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

	// Create two shader objects, one for the vertex, and one for the fragment shader
    AddShader(shaderProgramID, "../Shaders/simpleVertexShader.txt", GL_VERTEX_SHADER);
    AddShader(shaderProgramID, "../Shaders/simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };
	// After compiling all shader objects and attaching them to the program, we can finally link it
    glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
    glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
    glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS

void generateObjectBufferMesh() {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/

	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.
	const char* names[5];
	names[0] = STADIUM_NAME;
	names[1] = PLAYER_NAME;
	names[2] = BALL_NAME;
	names[3] = ENEMY_NAME;
	names[4] = PLAYER_ARM;
	load_mesh(names);

	for (int i = 0; i < numberOfMeshes; i++) {
		unsigned int vp_vbo = 0;

		loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
		loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
		loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

		glGenBuffers(1, &vp_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
		glBufferData(GL_ARRAY_BUFFER, g_point_count[i] * 3 * sizeof(float), &g_vp[i][0], GL_STATIC_DRAW);
		unsigned int vn_vbo = 0;
		glGenBuffers(1, &vn_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
		glBufferData(GL_ARRAY_BUFFER, g_point_count[i] * 3 * sizeof(float), &g_vn[i][0], GL_STATIC_DRAW);

		//	This is for texture coordinates which you don't currently need, so I have commented it out
		//	unsigned int vt_vbo = 0;
		//	glGenBuffers (1, &vt_vbo);
		//	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
		//	glBufferData (GL_ARRAY_BUFFER, g_point_count * 2 * sizeof (float), &g_vt[0], GL_STATIC_DRAW);

		glGenVertexArrays(1, &vaos[i]);
		glBindVertexArray(vaos[i]);

		glEnableVertexAttribArray(loc1);
		glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
		glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(loc2);
		glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
		glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

		glBindVertexArray(0);
	}

}



#pragma endregion VBO_FUNCTIONS


void display(){

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable (GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor (0.5f, 0.5f, 0.5f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram (shaderProgramID);

	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation (shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation (shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation (shaderProgramID, "proj");
	
	//third person on player
	if(thirdPerson){
		calculateCameraPosition();
	}
	//third person on thrown ball
	if (throwBall = throwBall) {
		cameraPos = glm::vec3(xBall, yBall, (zBall+1));
	}
	// Root of the Hierarchy (stadium)
	glm::mat4 view = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, -40.0));
	mat4 persp_proj = perspective(45.0, (float)width/(float)height, 0.1, 100.0);
	mat4 model = identity_mat4 ();
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	//printf("(%f, %f, %f)", cameraPos, cameraPos + cameraFront, cameraUp);

	// update uniforms & draw
	glUniformMatrix4fv (proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv (view_mat_location, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv (matrix_location, 1, GL_FALSE, model.m);

	// draw mesh 1 (player)
	glBindVertexArray(vaos[0]);
	glDrawArrays (GL_TRIANGLES, 0, g_point_count[0]);

	mat4 model2 = identity_mat4();
	model2 = translate(model2, vec3(xplayer, -0.09f, zplayer));
	mat4 globalmodel = model * model2;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel.m);

	glBindVertexArray(vaos[1]);
	glDrawArrays(GL_TRIANGLES, 0, g_point_count[1]);

	// draw mesh 2 (ball)
	if (throwBall) {
		mat4 model3 = identity_mat4();
		model3 = translate(model3, vec3(0.0f, yBall, zBall));
		mat4 globalmodel2 = globalmodel * model3;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel2.m);

		glBindVertexArray(vaos[2]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[2]);
		//throwBall = false;
	}

	// draw enemy keep array of enemies (can increase it depending on which level)
	//printf("z position::::  %f", enemies[0].getZPos());

	mat4 model4 = identity_mat4();
	model4 = translate(model4, vec3(enemies[0].getXPos(), enemies[0].getYPos(), enemies[0].getZPos()));
	mat4 globalmodel3 = globalmodel * model4;
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel3.m);

	glBindVertexArray(vaos[3]);
	glDrawArrays(GL_TRIANGLES, 0, g_point_count[3]);


	//throwing animation
	if (throwing) {
		mat4 model4 = identity_mat4();
		model4 = rotate_x_deg(model4, 180.0f);
		model4 = translate(model4, vec3(0.095f, 0.44f, 0.0f));
		mat4 globalmodel3 = globalmodel * model4;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel3.m);

		glBindVertexArray(vaos[4]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[4]);
	}
	else {
		mat4 model4 = identity_mat4();
		//model4 = rotate_x_deg(model4, 180.0f);
		model4 = translate(model4, vec3(0.095f, 0.44f, 0.0f));
		mat4 globalmodel3 = globalmodel * model4;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel3.m);

		glBindVertexArray(vaos[4]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[4]);
	}
	//make timer, end of timer release ball, during timer move arm up then back down using throwing boolean function


    glutSwapBuffers();
}


void updateScene() {	

	// Placeholder code, if you want to work with framerate
	// Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
	static DWORD  last_time = 0;
	DWORD  curr_time = timeGetTime();
	float  delta = (curr_time - last_time) * 0.001f;
	if (delta > 0.03f)
		delta = 0.03f;
	last_time = curr_time;

	// rotate the model slowly around the y axis
	rotate_y+=0.2f;

	//forward motion of ball
	yBall += 0.04f;
	zBall -= 0.1f;

	//move enemy towards player
	//enemies[0].moveToPlayer();

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	for (int i = 0; i < 5; i++) {
		enemies[i].setPos(0.0f, 0.0f, -3.0f);
	}
	float randomX = rand() % 5;
	float randomZ = rand() % 5;
	float r2 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 5));
	printf("randoms %f, %f", randomX, randomZ, r2);
	enemies[0].setPos(randomX, 0.0f, -(randomZ));
	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	// load mesh into a vertex buffer array
	generateObjectBufferMesh();
	
}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
	//camera
	GLfloat cameraSpeed = 0.05f;
	if(key=='w'){
		cameraPos += cameraFront * cameraSpeed;
	}
	if (key == 's') {
		cameraPos -= cameraFront * cameraSpeed;
	}
	if (key == 'a') {
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (key == 'd') {
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}

	//player
	if (key == 'g') {
		zplayer += 0.1f;
	}
	if (key == 't') {
		zplayer -= 0.1f;
	}
	if (key == 'f') {
		xplayer -= 0.1f;
	}
	if (key == 'h') {
		xplayer += 0.1f;
	}

	//camera selection
	if (key == 'n') {
		thirdPerson = true;
	}
	if (key == 'm') {
		thirdPerson = false;
	}

	//create ball object
	if (key == 'l') {
		yBall = 0.0f;
		zBall = 0.0f;
		throwBall = true;
	}

	//move arm
	if (key == 'o') {
		throwing = true;
	}
	if (key == 'p') {
		throwing = false;
	}
}

bool firstMouse = true;
void mouse(int xpos, int ypos) {
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to left
	lastX = xpos;
	lastY = ypos;

	GLfloat sensitivity = 0.05;	// Change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// Make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

int main(int argc, char** argv){
	// Set up the window
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("Hello Triangle");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keypress);
	glutPassiveMotionFunc(mouse);

	 // A call to glewInit() must be done after glut is initialized!
    GLenum res = glewInit();
	// Check for any errors
    if (res != GLEW_OK) {
      fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
      return 1;
    }
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
    return 0;
}











