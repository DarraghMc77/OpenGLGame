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
#include <SOIL.h>
#include "text.h"
#include <irrKlang.h>

using namespace irrklang;

ISoundEngine *SoundEngine = createIrrKlangDevice();

/*----------------------------------------------------------------------------
                   MESH TO LOAD
  ----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
#define TEST_CUBE "../textcube.obj"
#define STADIUM_NAME "../texturedstadium.obj"
#define PLAYER_NAME "../luiginoarm.obj"
#define BALL_NAME "../texturedfootball.obj"
#define ENEMY_NAME "../mariowithhelmet.obj"
#define PLAYER_ARM "../luigiarmonaxis.obj"
#define FIELD "../texturedfield.obj";
#define PODIUM "../podium.obj";
#define SKYBOX "../skybox.obj";
#define WIN_PLAYER "../luigitexturedwithhelmet.obj"
/*----------------------------------------------------------------------------
  ----------------------------------------------------------------------------*/

#define PI 3.14159265359


//change if more meshes loaded
int numberOfMeshes = 9;

int g_point_count[9];
std::vector<float> g_vp[9], g_vn[9], g_vt[9];
unsigned int vaos[9];


int twidth, theight;
GLuint textures[6];

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

GLfloat yaw = -90.0f;	
GLfloat pitch = 0.0f;
GLfloat lastX = (float)width / 2.0;
GLfloat lastY = (float)height / 2.0;
GLfloat fov = 45.0f;

//player movement
GLfloat xplayer = 0.0f;
GLfloat yplayer = 0.0f;
GLfloat zplayer = 4.0f;
GLfloat playerRotate = 0.0f;
GLfloat playerSpeed;
GLfloat armRotate = 0;

//3rd person camera
GLfloat distanceFromPlayer = 50.0f;
GLfloat angleAroundPlayer = 30.0f;

//ball movement
GLfloat xBall = 0.0f;
GLfloat yBall = 0.0f;
GLfloat zBall = 0.0f;

//enemy movement
int numberOfEnemies = 5;
bool increasing = true;
bool deleteEnemy = false;

//score
int scoreText;
int score;
int gameOverText;

//time
DWORD firstTime;
int timeText;

//screens
bool inGame = false;
bool gameOver = false;
bool winner = false;
bool startScreen = true;

int winnerText;
int startScreenText;

bool thirdPerson = true;

bool throwBall = false;

bool throwing = false;

class Ball {
	float xposition;
	float yposition;
	float zposition;
	float ballRotation;
	float visible;
public:
	void setPos(float x, float y, float z, float playerRotation) {
		xposition = x;
		yposition = y;
		zposition = z;
		ballRotation = playerRotation;
		visible = true;
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

	float getRotation() {
		return ballRotation;
	}

	bool isVisible() {
		return visible;
	}

	void deleteBall() {
		visible = false;
	}

	void movement() {
		zposition -= 0.1f;
	}
};

class Enemy {
	float xposition;
	float yposition;
	float zposition;
	bool dead;
	bool increasing;
	bool increasingJump;

	public:
		void setPos(float x, float y, float z) {
			xposition = x;
			yposition = y;
			zposition = z;
			dead = false;
			increasing = true;
			increasingJump = true;
		}

		void setStatus() {
			dead = true;
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

		void incrementX() {
			xposition += 0.02f;
		}

		void decrementX() {
			xposition -= 0.02f;
		}

		void incrementY() {
			yposition += 0.01f;
		}

		void decrementY() {
			yposition -= 0.01f;
		}

		void incrementZ() {
			zposition += 0.5f;
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

		void movement() {
			if (increasing) {
				incrementX();
				if (xposition > 2.2f) {
					incrementZ();
					increasing = false;
				}
			}
			else {
				decrementX();
				if (xposition < -2.2f) {
					incrementZ();
					increasing = true;
				}
			}
		}

		void celebrate() {
			if (increasingJump) {
				incrementY();
				if (yposition > 1.2f) {
					increasingJump = false;
				}
			}
			else {
				decrementY();
				if (yposition < 0.8f) {
					increasingJump = true;
				}
			}
		}

		bool hit(float ballX, float ballY, float ballZ) {
			return(xposition + 0.3 > ballX - 0.1f &&
				xposition - 0.3 < ballX + 0.1f &&
				yposition + 1 > ballY - 0.1f &&
				yposition - 1 < ballY + 0.1f &&
				zposition + 0.3 > ballZ - 0.4 &&
				zposition - 0.3 < ballZ + 0.4);
		}

		bool isDead() {
			return dead;
		}

};

class Player {
	float xposition;
	float yposition;
	float zposition;
	float rotation;
public:
	void setPos(float x, float y, float z) {
		xposition = x;
		yposition = y;
		zposition = z;
		//rotatation = r;
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

	float getRotation() {
		return rotation;
	}

	bool dead(Enemy mario) {
		return(xposition+0.2 > mario.getXPos()-0.3 &&
			xposition-0.2 < mario.getXPos() + 0.3 &&
			yposition+1 > mario.getYPos() - 1 &&
			yposition-1 < mario.getYPos() + 1 &&
			zposition+0.2 > mario.getZPos() - 0.3 &&
			zposition-0.2 < mario.getZPos() + 0.3);
	}
};


Enemy enemies[5];
Ball ball;
std::vector<Ball> balls;
Player player;

void collideWithWall() {
	if (player.getXPos() <= -2.999999) {
		player.setPos(-2.999999, player.getYPos(), player.getZPos());
	}
	if (player.getXPos() >= 2.899999) {
		player.setPos(2.899999, player.getYPos(), player.getZPos());
	}
	if (player.getZPos() >= 5.0) {
		player.setPos(player.getXPos(), player.getYPos(), 5.0);
	}
}

char* updateScore(int newScore) {
	score += newScore;
	char updatedScore[50];
	sprintf(updatedScore, "Score: %d\n", score);
	return updatedScore;
}

char* updateTime(int minutes, int seconds) {
	char updatedTime[50];
	sprintf(updatedTime, "Time:  %d:%d\n", minutes, seconds);
	return updatedTime;
}

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
	cameraPos = glm::vec3(player.getXPos(), (player.getYPos() + 0.5f), (player.getZPos() + 1.0));
}

bool load_mesh(const char* file_name[]) {
	for (int i = 0; i < numberOfMeshes; i++) {
		const aiScene* scene = aiImportFile(file_name[i], aiProcess_Triangulate); // TRIANGLES!
		//fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		if (!scene) {
			fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
			return false;
		}
		//printf("  %i animations\n", scene->mNumAnimations);
		//printf("  %i cameras\n", scene->mNumCameras);
		//printf("  %i lights\n", scene->mNumLights);
		//printf("  %i materials\n", scene->mNumMaterials);
		//printf("  %i meshes\n", scene->mNumMeshes);
		//printf("  %i textures\n", scene->mNumTextures);
		g_point_count[i] = 0;

		for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
			const aiMesh* mesh = scene->mMeshes[m_i];
			//printf("    %i vertices in mesh\n", mesh->mNumVertices);
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

	const char* names[8];
	names[0] = STADIUM_NAME;
	names[1] = PLAYER_NAME;
	names[2] = BALL_NAME;
	names[3] = ENEMY_NAME;
	names[4] = FIELD;
	names[5] = PLAYER_ARM;
	names[6] = PODIUM;
	names[7] = SKYBOX;
	names[8] = WIN_PLAYER;
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

		unsigned int vt_vbo = 0;
		glGenBuffers (1, &vt_vbo);
		glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
		glBufferData (GL_ARRAY_BUFFER, g_point_count[i] * 2 * sizeof (float), &g_vt[i][0], GL_STATIC_DRAW);

		glGenVertexArrays(1, &vaos[i]);
		glBindVertexArray(vaos[i]);

		glEnableVertexAttribArray(loc1);
		glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
		glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(loc2);
		glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
		glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(loc3);
		glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
		glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);

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
	
	//start screen
	if (startScreen) {

		glBindTexture(GL_TEXTURE_2D, textures[1]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		// Root of the Hierarchy (stadium)
		glm::mat4 view = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, -40.0));
		mat4 persp_proj = perspective(45.0, (float)width / (float)height, 0.1, 100.0);
		mat4 model = identity_mat4();
		view = glm::lookAt(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f) + cameraFront, cameraUp);

		glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
		glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);

		glBindVertexArray(vaos[0]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[0]);

		//draw mesh 4 (field)
		glBindTexture(GL_TEXTURE_2D, textures[2]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);
		glBindVertexArray(vaos[4]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[4]);

		//draw mesh 6 skybox
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		mat4 model9 = identity_mat4();
		model9 = scale(model9, vec3(2.0f, 2.0f, 2.0f));
		model9 = translate(model9, vec3(0.0f, -0.5f, 0.0f));
		mat4 model10 = model9 * model;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model10.m);

		glBindVertexArray(vaos[7]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[7]);

		//player
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		mat4 model2 = identity_mat4();
		model2 = scale(model2, vec3(0.6f, 0.6f, 0.6f));
		model2 = rotate_y_deg(model2, 180.0f);
		model2 = translate(model2, vec3(-0.5f, 0.0f, -2.0f));
		mat4 globalmodel = model * model2;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel.m);

		glBindVertexArray(vaos[8]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[8]);

		glBindTexture(GL_TEXTURE_2D, textures[3]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		model2 = identity_mat4();
		model2 = scale(model2, vec3(0.6f, 0.6f, 0.6f));
		model2 = rotate_y_deg(model2, 0.0f);
		model2 = translate(model2, vec3(0.5f, 0.0f, -2.0f));
		globalmodel = model * model2;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel.m);

		glBindVertexArray(vaos[3]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[3]);

		draw_texts();
	}

	//game screen
	else if(inGame){

		//if enemy touches player (game over)
		if (player.dead(enemies[0]) && !enemies[0].isDead()) {
			inGame = false;
			gameOver = true;
		}

		//if player in endzone (wins)
		if (player.getZPos() < -5.0f) {
			inGame = false;
			gameOver = false;
			winner = true;
		}

		//third person on player
		if (thirdPerson) {
			calculateCameraPosition();
		}
		//third person on thrown ball
		//if (throwBall) {
		//	cameraPos = glm::vec3(xBall, yBall, (zBall+1));
		//}

		glBindTexture(GL_TEXTURE_2D, textures[1]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);


		// Root of the Hierarchy (stadium)
		glm::mat4 view = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, -40.0));
		mat4 persp_proj = perspective(45.0, (float)width / (float)height, 0.1, 100.0);
		mat4 model = identity_mat4();
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
		glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);

		glBindVertexArray(vaos[0]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[0]);


		//draw mesh 4 (field)
		glBindTexture(GL_TEXTURE_2D, textures[2]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);
		glBindVertexArray(vaos[4]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[4]);

		//draw mesh 6 skybox
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		mat4 model9 = identity_mat4();
		model9 = scale(model9, vec3(2.0f, 2.0f, 2.0f));
		model9 = translate(model9, vec3(0.0f, -0.5f, 0.0f));
		mat4 model10 = model9 * model;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model10.m);

		glBindVertexArray(vaos[7]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[7]);
		

		if (player.dead(enemies[0]) && !enemies[0].isDead()) {
			SoundEngine->removeAllSoundSources();
			SoundEngine->play2D("../smb_gameover.wav", GL_FALSE);
			inGame = false;
			gameOver = true;
		}

		if (player.getZPos() < -5.0f) {
			inGame = false;
			gameOver = false;
			winner = true;
			SoundEngine->removeAllSoundSources();
			SoundEngine->play2D("../smb_stage_clear.wav", GL_FALSE);
		}

		// draw mesh 1 (player)
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		mat4 model2 = identity_mat4();
		model2 = scale(model2, vec3(0.6f, 0.6f, 0.6f));
		model2 = rotate_y_deg(model2, playerRotate);
		model2 = translate(model2, vec3(player.getXPos(), player.getYPos(), player.getZPos()));
		mat4 globalmodel = model * model2;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel.m);

		glBindVertexArray(vaos[1]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[1]);

		//throwing animation
		if (throwing) {
			mat4 model4 = identity_mat4();
			model4 = rotate_x_deg(model4, 180.0f);
			model4 = translate(model4, vec3(0.095f, 0.42f, 0.0f));
			mat4 globalmodel3 = globalmodel * model4;
			glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel3.m);

			glBindVertexArray(vaos[5]);
			glDrawArrays(GL_TRIANGLES, 0, g_point_count[5]);
		}
		else {
			mat4 model4 = identity_mat4();
			model4 = rotate_x_deg(model4, 0.0f);
			model4 = translate(model4, vec3(0.095f, 0.42f, 0.0f));
			mat4 globalmodel3 = globalmodel * model4;
			glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel3.m);

			glBindVertexArray(vaos[5]);
			glDrawArrays(GL_TRIANGLES, 0, g_point_count[5]);
		}


		// draw mesh 2 (footballs)
		int current = 0;
		for (std::vector<Ball>::iterator it = balls.begin(); it < balls.end(); ++it) {
			if (balls[current].isVisible()) {
				mat4 model3 = identity_mat4();
				model3 = scale(model3, vec3(0.5f, 0.5f, 0.5f));
				model3 = rotate_y_deg(model3, balls[current].getRotation());
				model3 = translate(model3, vec3(balls[current].getXPos(), balls[current].getYPos(), balls[current].getZPos()));
				mat4 globalmodel2 = model * model3;
				glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel2.m);

				glBindVertexArray(vaos[2]);
				glDrawArrays(GL_TRIANGLES, 0, g_point_count[2]);
				for (int i = 0; i < numberOfEnemies; i++) {
					if (enemies[i].hit(balls[current].getXPos(), balls[current].getYPos(), balls[current].getZPos()) && !enemies[i].isDead()) {
						//balls[current] = balls.back(); balls.pop_back(); current--; //needs to be changed
						balls[current].deleteBall();
						update_text(scoreText, updateScore(50));
						enemies[i].setStatus();
					}
				}
				if (balls[current].getZPos() < -5.0f) {
					//balls[current] = balls.back(); balls.pop_back(); current--;
					balls[current].deleteBall();
				}
				balls[current].movement();
			}
			current++;
		}

		//draw mesh 3 (Enemies)
		glBindTexture(GL_TEXTURE_2D, textures[3]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		for (int i = 0; i < numberOfEnemies; i++) {
			if (!enemies[i].isDead()) {
				if (player.dead(enemies[i])) {
					inGame = false;
					gameOver = true;
				}
				mat4 model4 = identity_mat4();
				model4 = translate(model4, vec3(enemies[i].getXPos(), enemies[i].getYPos(), enemies[i].getZPos()));
				mat4 globalmodel3 = model * model4;
				glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel3.m);
				enemies[i].movement();

				glBindVertexArray(vaos[3]);
				glDrawArrays(GL_TRIANGLES, 0, g_point_count[3]);
			}
		}

		draw_texts();
	}

	//Game Over Screen
	else if (gameOver) {

		glBindTexture(GL_TEXTURE_2D, textures[1]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		// Draw Stadium
		glm::mat4 view = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, -40.0));
		mat4 persp_proj = perspective(45.0, (float)width / (float)height, 0.1, 100.0);
		mat4 model = identity_mat4();
		view = glm::lookAt(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f) + cameraFront, cameraUp);

		// update uniforms & draw
		glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
		glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);

		glBindVertexArray(vaos[0]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[0]);

		//draw mesh 4 (field)
		glBindTexture(GL_TEXTURE_2D, textures[2]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);
		glBindVertexArray(vaos[4]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[4]);

		//draw mesh 6 skybox
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		mat4 model9 = identity_mat4();
		model9 = scale(model9, vec3(2.0f, 2.0f, 2.0f));
		model9 = translate(model9, vec3(0.0f, -0.5f, 0.0f));
		mat4 model10 = model9 * model;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model10.m);

		glBindVertexArray(vaos[7]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[7]);

		//player
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		mat4 model2 = identity_mat4();
		model2 = scale(model2, vec3(0.6f, 0.6f, 0.6f));
		model2 = rotate_y_deg(model2, 180.0f);
		model2 = translate(model2, vec3(-0.8f, 0.4f, -4.0f));
		mat4 globalmodel = model * model2;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel.m);

		glBindVertexArray(vaos[8]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[8]);

		//enemy
		enemies[0].celebrate();
		glBindTexture(GL_TEXTURE_2D, textures[3]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		model2 = identity_mat4();
		model2 = scale(model2, vec3(0.6f, 0.6f, 0.6f));
		model2 = rotate_y_deg(model2, 0.0f);
		model2 = translate(model2, vec3(0.0f, enemies[0].getYPos(), -4.0f));
		globalmodel = model * model2;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel.m);

		glBindVertexArray(vaos[3]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[3]);

		//podium
		glBindTexture(GL_TEXTURE_2D, textures[4]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		mat4 model8 = identity_mat4();
		model8 = scale(model8, vec3(0.4f, 0.4f, 0.4f));
		model8 = translate(model8, vec3(0.0f, -0.09f, -4.0f));
		globalmodel = model * model8;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel.m);

		glBindVertexArray(vaos[6]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[6]);

		update_text(gameOverText, "Game Over");
		draw_texts();
	}

	//Win Screen
	else if (winner) {

		glBindTexture(GL_TEXTURE_2D, textures[1]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		// Draw Stadium
		glm::mat4 view = glm::translate(glm::mat4(1.0), glm::vec3(0.0, 0.0, -40.0));
		mat4 persp_proj = perspective(45.0, (float)width / (float)height, 0.1, 100.0);
		mat4 model = identity_mat4();
		view = glm::lookAt(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f) + cameraFront, cameraUp);

		glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
		glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model.m);

		glBindVertexArray(vaos[0]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[0]);

		//draw mesh 4 (field)
		glBindTexture(GL_TEXTURE_2D, textures[2]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);
		glBindVertexArray(vaos[4]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[4]);

		//draw mesh 6 skybox
		glBindTexture(GL_TEXTURE_2D, textures[5]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		mat4 model9 = identity_mat4();
		model9 = scale(model9, vec3(2.0f, 2.0f, 2.0f));
		model9 = translate(model9, vec3(0.0f, -0.5f, 0.0f));
		mat4 model10 = model9 * model;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, model10.m);

		glBindVertexArray(vaos[7]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[7]);

		//player
		glBindTexture(GL_TEXTURE_2D, textures[3]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		mat4 model2 = identity_mat4();
		model2 = scale(model2, vec3(0.6f, 0.6f, 0.6f));
		model2 = rotate_y_deg(model2, 0.0f);
		model2 = translate(model2, vec3(-0.8f, 0.4f, -4.0f));
		mat4 globalmodel = model * model2;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel.m);

		glBindVertexArray(vaos[3]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[3]);

		//enemy
		enemies[0].celebrate();
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		model2 = identity_mat4();
		model2 = scale(model2, vec3(0.6f, 0.6f, 0.6f));
		model2 = rotate_y_deg(model2, 180.0f);
		model2 = translate(model2, vec3(0.0f, enemies[0].getYPos(), -4.0f));
		globalmodel = model * model2;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel.m);

		glBindVertexArray(vaos[8]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[8]);

		//podium
		glBindTexture(GL_TEXTURE_2D, textures[4]);
		glUniform1i(glGetUniformLocation(shaderProgramID, "theTexture"), 0);

		mat4 model8 = identity_mat4();
		model8 = scale(model8, vec3(0.4f, 0.4f, 0.4f));
		model8 = translate(model8, vec3(0.0f, -0.09f, -4.0f));
		globalmodel = model * model8;
		glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalmodel.m);

		glBindVertexArray(vaos[6]);
		glDrawArrays(GL_TRIANGLES, 0, g_point_count[6]);
		update_text(winnerText, "Touchdown!\nYou Win!");
		draw_texts();
	}
	
    glutSwapBuffers();
}


void updateScene() {	


	// Placeholder code, if you want to work with framerate
	// Wait until at least 16ms passed since start of last frame (Effectively caps framerate at ~60fps)
	static DWORD  last_time = 0;
	DWORD curr_time = timeGetTime();
	float  delta = (curr_time - last_time) * 0.001f;
	if (delta > 0.03f)
		delta = 0.03f;
	last_time = curr_time;

	// rotate the model slowly around the y axis
	rotate_y+=0.2f;

	//forward motion of ball
	//yBall += 0.04f;
	zBall -= 0.1f;

	DWORD time = curr_time - firstTime;
	int seconds = (int)((time / 1000) % 60);
    int minutes = (int)((time / 1000) / 60);
	if (inGame) {
		update_text(timeText, updateTime(minutes, seconds));
	}

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{

	SoundEngine->play2D("../smb.mp3", GL_FALSE);

	firstTime = timeGetTime();
	init_text_rendering("../freemono.png", "../freemono.meta", width, height);	
	double t = timeGetTime();

	//create text boxes
	scoreText = add_text(updateScore(0), 0.45f, 0.9f, 35.0f, 1.0, 1.0, 1.0, 1.0);
	timeText = add_text(updateTime(0, 0), 0.45f, 0.99f, 35.0f, 1.0, 1.0, 1.0, 1.0);
	gameOverText = add_text("", -0.2f, 0.0f, 35.0f, 1.0, 1.0, 1.0, 1.0);
	winnerText = add_text("", -0.2f, 0.0f, 35.0f, 1.0, 1.0, 1.0, 1.0);
	startScreenText = add_text("Madden 2017: Mario Vs Luigi Edition\n          Press q to start!"
		, -0.8f, 0.5f, 35.0f, 1.0, 1.0, 1.0, 1.0);
	
	//load in textures
	const char* textureNames[6];
	textureNames[0] = "../luigiD.jpg";
	textureNames[1] = "../textures.png";
	textureNames[2] = "../GiantsField.jpg.";
	textureNames[3] = "../marioD.jpg";
	textureNames[4] = "../numbers.jpg";
	textureNames[5] = "../skybox.png";

	for (int i = 0; i < 6; i++) {
		glGenTextures(1, &textures[i]);
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		unsigned char* image = SOIL_load_image(textureNames[i], &twidth, &theight, 0, SOIL_LOAD_RGBA);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, twidth, theight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		SOIL_free_image_data(image);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	//create player
	player.setPos(xplayer, -0.09f, zplayer);

	//create enemies
	for (int i = 0; i < 5; i++) {
		enemies[i].setPos(0.0f, 0.0f, -3.0f);
	}

	float randomX = rand() % 5;
	float randomZ = rand() % 5;
	float r2 = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 5));
	printf("randoms %f, %f", randomX, randomZ, r2);

	float offset = 1.0f;
	for (int i = 0; i < numberOfEnemies; i++) {
		enemies[i].setPos(offset, 0.0f, -1.0f);
		offset -= 0.5f;
	}

	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	// load mesh into a vertex buffer array
	generateObjectBufferMesh();
	
}

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
		//xplayer += 0.5f * sin((PI / 180)*playerRotate);
		//zplayer += 0.5f * cos((PI / 180)*playerRotate);
		player.setPos(player.getXPos(), player.getYPos(), player.getZPos()+0.5f);
		collideWithWall();
	}
	if (key == 't') {
		//xplayer += -0.5f * sin((PI / 180)*playerRotate);
		//zplayer += -0.5f * cos((PI / 180)*playerRotate);
		player.setPos(player.getXPos(), player.getYPos(), player.getZPos() - 0.5f);
		collideWithWall();
	}
	if (key == 'f') {
		//xplayer -= 0.1f;
		player.setPos(player.getXPos() - 0.1f, player.getYPos(), player.getZPos());
		collideWithWall();
		//playerRotate += 4.0f;
	}
	if (key == 'h') {
		//xplayer += 0.1f;
		player.setPos(player.getXPos() + 0.1f, player.getYPos(), player.getZPos());
		collideWithWall();
		//playerRotate -= 4.0f;
	}

	//camera selection
	if (key == 'n') {
		thirdPerson = true;
	}
	if (key == 'm') {
		thirdPerson = false;
	}

	//move arm
	if (key == 'o') {
		throwing = true;
	}
	if (key == 'p') {
		throwing = false;
	}

	if (key == 'q') {
		update_text(startScreenText, "");
		startScreen = false;
		inGame = true;
		gameOver = false;
		winner = false;
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

void mouseClick(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		SoundEngine->play2D("../smb_fireball.wav", GL_FALSE);
		//create ball object
		throwing = false;
		Ball b;
		b.setPos(player.getXPos() + 0.2f, player.getYPos() + 0.2f, player.getZPos(), playerRotate);
		balls.push_back(b);
		yBall = 0.0f;
		zBall = 0.0f;
		ball.setPos(0.0f, yBall, zBall, playerRotate);
		throwBall = true;
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		throwing = true;
	}
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
	glutMouseFunc(mouseClick);
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











