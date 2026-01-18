// !! complex: sistem de particule (comete, explozii pe soare)
// post processing (bloom, motion blur)

// tried to implement but did not work (does not detect the normal map texture for earth); normal/bump mapping pentru planete
//Sursa tutorial skybox: https://learnopengl.com/Advanced-OpenGL/Cubemaps



#include <windows.h>        //	Utilizarea functiilor de sistem Windows (crearea de ferite, manipularea fisierelor si directoarelor);
#include <stdlib.h>         //  Biblioteci necesare pentru citirea shaderelor;
#include <stdio.h>
#include <math.h>           //  Adaugat pentru functii trigonometrice (sin, cos)
#include <GL/glew.h>        //  Definește prototipurile functiilor OpenGL si constantele necesare pentru programarea OpenGL moderna; 
#include <GL/freeglut.h>    //	Include functii pentru gestionarea ferestrelor si evenimentelor;
#include "loadShaders.h"	//	Fisierul care face legatura intre program si shadere;
#include "glm/glm.hpp"		//	Bibloteci utilizate pentru transformari grafice;
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <stack>
#include "SOIL.h"
#include "glm/gtc/quaternion.hpp" 
#include "glm/gtx/quaternion.hpp"
#include <vector>
#include <string>
#include <iostream>
#include "objloader.hpp"


GLuint
	VaoId,
	VboId,
	EboId,
	ProgramId,
	viewModelLocation,
	projLocation,
	codColLocation,
	objectTypeLocation, 
	textureLocation, 
	textureEarthId,
	textureMoonId,
	textureSunId,
	textureMercuryId,
	textureVenusId,
	textureMarsId,
	textureJupiterId,
	textureSaturnId,
	textureUranusId,
	textureNeptuneId,
	lightPosLocation,
	OrbitVaoId, OrbitVboId,
	RingVaoId, RingVboId
;

// incarcam asteroidul prin objloader
GLuint AstVaoId, AstVboId, AstUvVboId, AstNormalVboId, AstColorVboId;
std::vector<glm::vec3> ast_vertices;
std::vector<glm::vec2> ast_uvs;
std::vector<glm::vec3> ast_normals;

struct Asteroid 
{
    glm::vec3 position;
    float scale;
    float rotationSpeed;
    float angleOffset;
    glm::vec3 rotationAxis;
};

std::vector<Asteroid> asteroidBelt;

float PI = 3.141592;
float const U_MIN = -PI / 2, U_MAX = PI / 2, V_MIN = 0, V_MAX = 2 * PI;
int const NR_PARR = 20, NR_MERID = 40;
int const NR_VF = (NR_PARR + 1) * (NR_MERID + 1);
float step_u = (U_MAX - U_MIN) / NR_PARR, step_v = (V_MAX - V_MIN) / NR_MERID;
float radius = 50.0f;

//	Dimensiunile ferestrei de afisare;
GLfloat winWidth = 1400, winHeight = 600;

int codCol;

float timeElapsed;
bool showOrbits = true;

//	Elemente pentru matricea de vizualizare;
float dist = 3000.0f;
float alpha = 0.0f;
float beta = 0.0f;
float incrAlpha1 = 0.01f;
float incrAlpha2 = 0.01f;

// Punctul de referinta
float refX = 0.0f, refY = 0.0f, refZ = 0.0f;

//	Elemente pentru matricea de proiectie;
float xMin = -700.f, xMax = 700.f, yMin = -300.f, yMax = 300.f,
zNear = 100.f, zFar = 5000.f, 
width = 1400.f, height = 600.f, fov = 90.f * PI / 180;

//	Vectori pentru matricea de vizualizare;
glm::vec3 obs, pctRef, vert;

//	variabile catre matricele de transformare pentru fiecare corp ceresc
glm::mat4
	view, projection,
	translateSystem,
	rotateSun, scaleSun,
	scaleEarth, rotateEarthAxis, rotateEarth, translateEarth,
	scaleMoon, rotateMoon, translateMoon,
	scaleMercury, rotateMercury, translateMercury,
	scaleVenus, rotateVenus, translateVenus,
	scaleMars, rotateMars, translateMars,
	scaleJupiter, rotateJupiter, translateJupiter,
	scaleSaturn, rotateSaturn, translateSaturn,
	scaleUranus, rotateUranus, translateUranus,
	scaleNeptune, rotateNeptune, translateNeptune;

// stiva de matrice - esentiala pentru mostenirea transformarilor intr-un sistem ierarhic: soare, planete, pamant + luna

std::stack<glm::mat4> mvStack;

// pentru camera, folosim cuaternioni, nu unghiurile lui Euler pentru a evita suprapunerea axelor de rotatie

// Conform cursului, cuaternionul va stoca orientarea camerei.
glm::quat viewQuaternion = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

// Skybox
GLuint skyboxVAO, skyboxVBO, skyboxTexture;
GLuint skyboxProgram;

float skyboxVertices[] = {     
	-1.0f,  1.0f, -1.0f, // desenam efectiv stele pe interiorul unui cub, nu va fi afectat de modificarea observatorului
	-1.0f, -1.0f, -1.0f, // va da impresia de spatiu infinit
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};


void InitSkybox()
{
	skyboxProgram = LoadShaders("skybox.vert", "skybox.frag");
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void InitAsteroids()
{
	bool res = loadOBJ("Asteroid_1.obj", ast_vertices, ast_uvs, ast_normals);
	
	glGenVertexArrays(1, &AstVaoId);
	glBindVertexArray(AstVaoId);

	glGenBuffers(1, &AstVboId);
	glBindBuffer(GL_ARRAY_BUFFER, AstVboId);
	glBufferData(GL_ARRAY_BUFFER, ast_vertices.size() * sizeof(glm::vec3), &ast_vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glGenBuffers(1, &AstUvVboId);
	glBindBuffer(GL_ARRAY_BUFFER, AstUvVboId);
	glBufferData(GL_ARRAY_BUFFER, ast_uvs.size() * sizeof(glm::vec2), &ast_uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glGenBuffers(1, &AstNormalVboId);
	glBindBuffer(GL_ARRAY_BUFFER, AstNormalVboId);
	glBufferData(GL_ARRAY_BUFFER, ast_normals.size() * sizeof(glm::vec3), &ast_normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	std::vector<glm::vec3> ast_colors;
	for (size_t i = 0; i < ast_vertices.size(); i++) 
	{
		// asteroizii variaza in culoare (nuante de roca)
		float r = 0.4f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.2f;
		ast_colors.push_back(glm::vec3(r, r * 0.9f, r * 0.8f));
	}
	
	glGenBuffers(1, &AstColorVboId);
	glBindBuffer(GL_ARRAY_BUFFER, AstColorVboId);
	glBufferData(GL_ARRAY_BUFFER, ast_colors.size() * sizeof(glm::vec3), &ast_colors[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// pozitionare intre Marte si Jupiter pentru a forma centura de asteroizi
	int numAsteroids = 150;
	float innerRadius = 850.0f; // raza interioara a inelului
	float outerRadius = 1000.0f; // raza exterioara a inelului

	for (int i = 0; i < numAsteroids; i++) 
	{
		// Generam pozitie polara aleatorie si convertim in cartezian
		float angle = (float)(rand() % 360) * PI / 180.0f;
		float r = innerRadius + (float)(rand() % 100) / 100.0f * (outerRadius - innerRadius);
		float y = (float)(rand() % 100 - 50) / 2.0f; // variatie mica pe Oy

		float x = r * cos(angle);
		float z = r * sin(angle);

		Asteroid ast;

		ast.position = glm::vec3(x, y, z);
		ast.scale = 2.0f + (float)(rand() % 100) / 100.0f * 2.0f; // variatie dimensiune
		ast.rotationSpeed = (float)(rand() % 100) / 1000.0f; // pentru o miscare naturala, asteroizii se rotesc
		ast.angleOffset = angle; 
		ast.rotationAxis = glm::normalize(glm::vec3((float)(rand() % 100), (float)(rand() % 100), (float)(rand() % 100)));

		asteroidBelt.push_back(ast);
	}
}


void CreateOrbitVBO(void)
{
	glm::vec4 OrbitVertices[360];
	for (int i = 0; i < 360; i++)
	{
		float angle = (float)i * PI * 2.0f / 360.0f;
		OrbitVertices[i] = glm::vec4(cos(angle), 0.0f, sin(angle), 1.0f);
	}

	glGenVertexArrays(1, &OrbitVaoId);
	glBindVertexArray(OrbitVaoId);

	glGenBuffers(1, &OrbitVboId);
	glBindBuffer(GL_ARRAY_BUFFER, OrbitVboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(OrbitVertices), OrbitVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
}

void ProcessNormalKeys(unsigned char key, int x, int y)
{
	switch (key) {
	case '+':
		dist -= 5.0;	//	Apasarea tastelor `+` si `-` schimba pozitia observatorului (se apropie / departeaza);
		break;
	case '-':
		dist += 5.0;
		break;
	case ' ':
		showOrbits = !showOrbits;
		break;
	}
	if (key == 27)
		exit(0);
}

// implementare observator cu cuaternioni pentru a evita gimbal lock

void ProcessSpecialKeys(int key, int xx, int yy)
{
	float theta = 0.05f;

	// elemente pentru constructia cuaternionului de rotatie q = (s, v)
	// conform slide 49:
	float s = cos(theta / 2.0f);
	float sin_val = sin(theta / 2.0f);

	glm::quat qRotate;

	switch (key)
	{
		case GLUT_KEY_LEFT: // rotire pe Oy
			qRotate = glm::quat(s, 0.0f * sin_val, 1.0f * sin_val, 0.0f * sin_val);
			viewQuaternion = qRotate * viewQuaternion;
			break;

		case GLUT_KEY_RIGHT: // rotire inversa pe Oy
			s = cos(-theta / 2.0f);
			sin_val = sin(-theta / 2.0f);
			qRotate = glm::quat(s, 0.0f * sin_val, 1.0f * sin_val, 0.0f * sin_val);
			viewQuaternion = qRotate * viewQuaternion;
			break;

		case GLUT_KEY_UP: // rotire pe Ox
			qRotate = glm::quat(s, 1.0f * sin_val, 0.0f * sin_val, 0.0f * sin_val);
			viewQuaternion = viewQuaternion * qRotate;
			break;

		case GLUT_KEY_DOWN: // rotire inversa pe Ox
			s = cos(-theta / 2.0f);
			sin_val = sin(-theta / 2.0f);
			qRotate = glm::quat(s, 1.0f * sin_val, 0.0f * sin_val, 0.0f * sin_val);
			viewQuaternion = viewQuaternion * qRotate;
			break;
	}
}

GLuint LoadTexture(const char* imagePath)
{
	GLuint textureId;

	glGenTextures(1, &textureId);

	glBindTexture(GL_TEXTURE_2D, textureId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, channels;
	unsigned char* image = SOIL_load_image(imagePath, &width, &height, &channels, SOIL_LOAD_RGB);

	if (image)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		SOIL_free_image_data(image);
	}
	else
		printf("Eroare la incarcarea texturii: %s\n", imagePath);

	glBindTexture(GL_TEXTURE_2D, 0);

	return textureId;
}

//  Crearea si compilarea obiectelor de tip shader;
void CreateShaders(void)
{
	ProgramId = LoadShaders("example.vert", "example.frag");
	glUseProgram(ProgramId);
}

//  Se initializeaza un vertex Buffer Object (VBO) pentru tranferul datelor spre memoria placii grafice (spre shadere);
void CreateVBO(void)
{
	// generare procedurala pentru sfera
	glm::vec4 Vertices[NR_VF];
	glm::vec3 Colors[NR_VF];
	glm::vec3 Normals[NR_VF];
	glm::vec2 TexCoords[NR_VF]; //adaugat pentru texturi
	GLushort Indices[6 * NR_VF]; // Buffer suficient pentru indici

	int index;
	for (int merid = 0; merid < NR_MERID + 1; merid++)
	{
		for (int parr = 0; parr < NR_PARR + 1; parr++)
		{
			float u = U_MIN + parr * step_u;
			float v = V_MIN + merid * step_v;

			float x_vf = radius * cosf(u) * cosf(v);
			float y_vf = radius * cosf(u) * sinf(v);
			float z_vf = radius * sinf(u);

			index = merid * (NR_PARR + 1) + parr;
			Vertices[index] = glm::vec4(x_vf, y_vf, z_vf, 1.0);
			Colors[index] = glm::vec3(0.5f + 0.5 * sinf(u), 0.5f + 0.5 * cosf(v), 0.5f + 0.5 * sinf(u + v));


			Normals[index] = glm::normalize(glm::vec3(x_vf, y_vf, z_vf));
			float tex_s = (float)merid / NR_MERID;
			float tex_t = (float)parr / NR_PARR;
			TexCoords[index] = glm::vec2(tex_s, tex_t);
		}
	}

	// Generare indici pentru fete (GL_QUADS)
	int k = 0;
	for (int merid = 0; merid < NR_MERID; merid++)
	{
		for (int parr = 0; parr < NR_PARR; parr++)
		{
			int index_curent = merid * (NR_PARR + 1) + parr;
			int index_dr = (merid + 1) * (NR_PARR + 1) + parr;

			// Indici pentru GL_QUADS
			Indices[k++] = index_curent;
			Indices[k++] = index_dr;
			Indices[k++] = index_dr + 1;
			Indices[k++] = index_curent + 1;
		}
	}

	//  Transmiterea datelor prin buffere;

	glGenVertexArrays(1, &VaoId);
	glBindVertexArray(VaoId);

	glGenBuffers(1, &VboId);
	glBindBuffer(GL_ARRAY_BUFFER, VboId);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(Colors) + sizeof(TexCoords) + sizeof(Normals), NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices), sizeof(Colors), Colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(Colors), sizeof(TexCoords), TexCoords);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(Colors) + sizeof(TexCoords), sizeof(Normals), Normals);

	glGenBuffers(1, &EboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	// Pozitie
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Culoare
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(Vertices));

	// Coordonate texturare
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(Vertices) + sizeof(Colors)));

	// Normale
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(Vertices) + sizeof(Colors) + sizeof(TexCoords)));
}

// inel Saturn
void CreateRingVBO(void) 
{
	struct VertexFormat 
	{
		glm::vec4 position;
		glm::vec3 color;
		glm::vec2 texCoord;
		glm::vec3 normal;
	};

	std::vector<VertexFormat> vertices;
	int segments = 72; 
	float innerRadius = 1.3f; 
	float outerRadius = 2.2f;

	for (int i = 0; i <= segments; i++) 
	{
		float angle = (float)i * 2.0f * PI / segments;
		float c = cos(angle);
		float s = sin(angle);

		VertexFormat v1;
		v1.position = glm::vec4(innerRadius * c, 0.0f, innerRadius * s, 1.0f);
		v1.color = glm::vec3(0.7f, 0.6f, 0.5f); // Culoare bej pentru inel
		v1.texCoord = glm::vec2(0.0f, (float)i / segments);
		v1.normal = glm::vec3(0.0f, 1.0f, 0.0f); // Normala in sus
		vertices.push_back(v1);

		VertexFormat v2;
		v2.position = glm::vec4(outerRadius * c, 0.0f, outerRadius * s, 1.0f);
		v2.color = glm::vec3(0.7f, 0.6f, 0.5f);
		v2.texCoord = glm::vec2(1.0f, (float)i / segments);
		v2.normal = glm::vec3(0.0f, 1.0f, 0.0f);
		vertices.push_back(v2);
	}

	glGenVertexArrays(1, &RingVaoId);
	glBindVertexArray(RingVaoId);

	glGenBuffers(1, &RingVboId);
	glBindBuffer(GL_ARRAY_BUFFER, RingVboId);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexFormat), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec4)));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec4) + sizeof(glm::vec3)));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec4) + sizeof(glm::vec3) + sizeof(glm::vec2)));
}

void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}

void DestroyVBO(void)
{
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VboId);
	glDeleteBuffers(1, &EboId);
	glDeleteBuffers(1, &OrbitVboId); 

	glDeleteVertexArrays(1, &AstVaoId);
	glDeleteBuffers(1, &AstVboId);
	glDeleteBuffers(1, &AstUvVboId);
	glDeleteBuffers(1, &AstNormalVboId);
	glDeleteBuffers(1, &AstColorVboId);
}

void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

void Initialize(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	CreateVBO();					
	CreateShaders();

	viewModelLocation = glGetUniformLocation(ProgramId, "viewModel");
	projLocation = glGetUniformLocation(ProgramId, "projection");
	codColLocation = glGetUniformLocation(ProgramId, "codCol");
	objectTypeLocation = glGetUniformLocation(ProgramId, "objectType");

	textureLocation = glGetUniformLocation(ProgramId, "myTexture");
	lightPosLocation = glGetUniformLocation(ProgramId, "lightPos");

	textureEarthId = LoadTexture("earth_map.jpg");
	textureMoonId = LoadTexture("moon_map.jpg");
	textureSunId = LoadTexture("sun.jpg");
	textureMercuryId = LoadTexture("mercury.jpg");
	textureVenusId = LoadTexture("venus.jpg");
	textureMarsId = LoadTexture("mars.jpg");
	textureJupiterId = LoadTexture("jupiter.jpg");
	textureSaturnId = LoadTexture("saturn.jpg");
	textureUranusId = LoadTexture("uranus.jpg");
	textureNeptuneId = LoadTexture("neptune.jpg");

	InitSkybox();
	CreateOrbitVBO();
	CreateRingVBO();
	InitAsteroids();

	projection = glm::perspective(fov, GLfloat(width) / GLfloat(height), zNear, zFar);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);
}

void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	timeElapsed = glutGet(GLUT_ELAPSED_TIME);

	// extragem componentele cuaternionului q = s + ai + bj + ck
	// a, b, c reprezinta partea imaginara si formeaza componenta vectoriala, s este scalarul
	// in GLM: w=s, x=a, y=b, z=c
	float s = viewQuaternion.w;
	float a = viewQuaternion.x;
	float b = viewQuaternion.y;
	float c = viewQuaternion.z;

	//construim matricea de rotatie (slide 51) - convertim cuaternionul in matrice

	glm::mat4 rotationMat(1.0f); // matricea identitate

	rotationMat[0][0] = s * s + a * a - b * b - c * c;
	rotationMat[0][1] = 2 * a * b + 2 * s * c;
	rotationMat[0][2] = 2 * a * c - 2 * s * b;
	rotationMat[1][0] = 2 * a * b - 2 * s * c;
	rotationMat[1][1] = s * s - a * a + b * b - c * c;
	rotationMat[1][2] = 2 * b * c + 2 * s * a;
	rotationMat[2][0] = 2 * a * c + 2 * s * b;
	rotationMat[2][1] = 2 * b * c - 2 * s * a;
	rotationMat[2][2] = s * s - a * a - b * b + c * c;

	// vectorul initial al observatorului este pe Oz la distanta 'dist'
	glm::vec4 initialCamPos = glm::vec4(0.0f, 0.0f, dist, 1.0f);
	glm::vec4 rotatedCamPos = rotationMat * initialCamPos;

	obs.x = refX + rotatedCamPos.x;
	obs.y = refY + rotatedCamPos.y;
	obs.z = refZ + rotatedCamPos.z;

	pctRef = glm::vec3(refX, refY, refZ);

	// verticala trebuie si ea rotita pentru ca observatorul sa nu se rastoarne
	// vectorul vertical initial este (0, 1, 0)
	glm::vec4 initialUp = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	glm::vec4 rotatedUp = rotationMat * initialUp;
	vert = glm::vec3(rotatedUp);

	view = glm::lookAt(obs, pctRef, vert);

	// skybox

	glDepthFunc(GL_LEQUAL); // depth test
	glUseProgram(skyboxProgram);

	glm::mat4 viewSkybox = glm::mat4(glm::mat3(view)); // eliminam translatia din matricea de vizualizare pentru skybox, pastrand rotatia, pentru ca skybox-ul sa stea pe loc

	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "view"), 1, GL_FALSE, glm::value_ptr(viewSkybox));
	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(skyboxVAO);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
	glUseProgram(ProgramId);
	glBindVertexArray(VaoId);

	glm::vec4 lightPosView = view * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glUniform3f(lightPosLocation, lightPosView.x, lightPosView.y, lightPosView.z);

	//miscare

	translateSystem = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));
	rotateSun = glm::mat4(1.0f); //nu se misca
	scaleSun = glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 4.0f, 4.0f)); 

	// Pamantul se obtine scaland sfera initiala
	scaleEarth = glm::scale(glm::mat4(1.0f), glm::vec3(0.5, 0.5, 0.5));
	// se roteste in jurul propriei axe
	rotateEarthAxis = glm::rotate(glm::mat4(1.0f), (float)0.001 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	// se roteste in jurul astrului central
	rotateEarth = glm::rotate(glm::mat4(1.0f), (float)0.0005 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	// este translatat in raport cu Soarele
	translateEarth = glm::translate(glm::mat4(1.0f), glm::vec3(600.0, 0.0, 0.0));

	// Mercur
	scaleMercury = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
	rotateMercury = glm::rotate(glm::mat4(1.0f), (float)0.002 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	translateMercury = glm::translate(glm::mat4(1.0f), glm::vec3(300.0, 0.0, 0.0));

	// Venus
	scaleVenus = glm::scale(glm::mat4(1.0f), glm::vec3(0.45f, 0.45f, 0.45f));
	rotateVenus = glm::rotate(glm::mat4(1.0f), (float)0.0008 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	translateVenus = glm::translate(glm::mat4(1.0f), glm::vec3(450.0, 0.0, 0.0));

	// Marte
	scaleMars = glm::scale(glm::mat4(1.0f), glm::vec3(0.35f, 0.35f, 0.35f));
	rotateMars = glm::rotate(glm::mat4(1.0f), (float)0.0004 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	translateMars = glm::translate(glm::mat4(1.0f), glm::vec3(750.0, 0.0, 0.0));

	// Jupiter
	scaleJupiter = glm::scale(glm::mat4(1.0f), glm::vec3(1.2f, 1.2f, 1.2f));
	rotateJupiter = glm::rotate(glm::mat4(1.0f), (float)0.0002 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	translateJupiter = glm::translate(glm::mat4(1.0f), glm::vec3(1100.0, 0.0, 0.0));

	// Saturn
	scaleSaturn = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	rotateSaturn = glm::rotate(glm::mat4(1.0f), (float)0.00015 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	translateSaturn = glm::translate(glm::mat4(1.0f), glm::vec3(1400.0, 0.0, 0.0));

	// Uranus
	scaleUranus = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));
	rotateUranus = glm::rotate(glm::mat4(1.0f), (float)0.0001 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	translateUranus = glm::translate(glm::mat4(1.0f), glm::vec3(1700.0, 0.0, 0.0));

	// Neptun
	scaleNeptune = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));
	rotateNeptune = glm::rotate(glm::mat4(1.0f), (float)0.00008 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	translateNeptune = glm::translate(glm::mat4(1.0f), glm::vec3(2000.0, 0.0, 0.0));

	// Luna: rotatie in jurul Pamantului
	rotateMoon = glm::rotate(glm::mat4(1.0f), (float)0.003 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	translateMoon = glm::translate(glm::mat4(1.0f), glm::vec3(80.0, 0.0, 0.0));
	scaleMoon = glm::scale(glm::mat4(1.0f), glm::vec3(0.2, 0.2, 0.2));

	mvStack.push(view);                 
	mvStack.top() *= translateSystem;	
	mvStack.push(mvStack.top());        
	
	mvStack.top() *= rotateSun;
	// rotatie pentru a corecta orientarea texturii
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleSun;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureSunId);
	glUniform1i(textureLocation, 0);
	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	//orbite
	if (showOrbits)
	{
		glUniform1i(objectTypeLocation, 12);
		glBindVertexArray(OrbitVaoId);
		glLineWidth(1.0f);

		std::vector<float> orbits = 
		{
			300.0f,  // Mercur
			450.0f,  // Venus
			600.0f,  // Pamant
			750.0f,  // Marte
			1100.0f, // Jupiter
			1400.0f, // Saturn
			1700.0f, // Uranus
			2000.0f  // Neptun
		};

		for (float r : orbits) 
		{
			mvStack.push(mvStack.top());
			mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(r, 1.0f, r));
			glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
			glDrawArrays(GL_LINE_LOOP, 0, 360);
			mvStack.pop();
		}

		glBindVertexArray(VaoId);
	}

	// Mercur
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateMercury;
	mvStack.top() *= translateMercury;
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleMercury;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 4); 
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureMercuryId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Venus
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateVenus;
	mvStack.top() *= translateVenus;
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleVenus;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 5);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureVenusId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Marte
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateMars;
	mvStack.top() *= translateMars;
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleMars;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 6);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureMarsId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Jupiter
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateJupiter;
	mvStack.top() *= translateJupiter;
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleJupiter;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 7);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureJupiterId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Saturn
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateSaturn;
	mvStack.top() *= translateSaturn;

	// salvam matricea curenta (centrul lui Saturn) pentru a desena inelul tot aici
	glm::mat4 saturnCenterMatrix = mvStack.top();

	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleSaturn;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 8);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureSaturnId);
	glUniform1i(textureLocation, 0);

	glBindVertexArray(VaoId);
	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));

    // inel 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE); // dezactivam depth mask pt blending corect cu skybox
	glDisable(GL_CULL_FACE);

	mvStack.top() = saturnCenterMatrix; // revenim la centrul lui Saturn

	// inclinam inelul
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 50.0f)); 

	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 13);

	glBindVertexArray(RingVaoId);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 146); // 72 segmente * 2 vertecsi + 2 de inchidere

	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glBindVertexArray(VaoId);

	mvStack.pop();

	// Uranus
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateUranus;
	mvStack.top() *= translateUranus;
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleUranus;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 9);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureUranusId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Neptun
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateNeptune;
	mvStack.top() *= translateNeptune;
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleNeptune;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 10);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureNeptuneId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// centura de asteroizi
	if (ast_vertices.size() > 0)
	{
		glUniform1i(objectTypeLocation, 14);
		glBindVertexArray(AstVaoId);

		for (const auto& ast : asteroidBelt) {
			mvStack.push(mvStack.top()); // fiecare asteroid are matrice proprie
			mvStack.top() *= glm::translate(glm::mat4(1.0f), ast.position);
			mvStack.top() *= glm::rotate(glm::mat4(1.0f), ast.rotationSpeed * timeElapsed, ast.rotationAxis);
			mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(ast.scale, ast.scale, ast.scale));
			glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
			glDrawArrays(GL_TRIANGLES, 0, ast_vertices.size());
			
			mvStack.pop();
		}
		glBindVertexArray(VaoId);
	}

	// SISTEMUL PAMANT-LUNA
	// Pamant
	mvStack.top() *= rotateEarth; // Rotatie in jurul Soarelui
	mvStack.top() *= translateEarth; // Pozitionare pe orbita

	mvStack.push(mvStack.top()); // SALVAM POZITIA PAMANTULUI PENTRU LUNA 
	
	mvStack.top() *= rotateEarthAxis; // Rotatie in jurul axei proprii
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleEarth;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureEarthId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Luna
	mvStack.top() *= rotateMoon; // rotatie in jurul Pamantului
	mvStack.top() *= translateMoon; // distanta fata de Pamant
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleMoon;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 3);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureMoonId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	glutSwapBuffers();
	glFlush();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);	
	glutInitWindowSize(winWidth, winHeight);						
	glutInitWindowPosition(100, 100);								
	glutCreateWindow("Proiect 2");		
	glewInit();
	Initialize();							
	glutDisplayFunc(RenderFunction);		
	glutIdleFunc(RenderFunction);			
	glutKeyboardFunc(ProcessNormalKeys);	
	glutSpecialFunc(ProcessSpecialKeys);
	glutCloseFunc(Cleanup);					
	glutMainLoop();
	return 0;
}