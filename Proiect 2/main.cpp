
// !! complex: sistem de particule (comete, explozii pe soare)
// post processing (bloom, motion blur)



//doesn't work yet:
// normal/bump mapping pentru planete
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
#include <direct.h> // Pentru _getcwd
#include "objloader.hpp"


GLuint
VaoId,
VboId,
EboId,
ProgramId,
viewModelLocation,
projLocation,
codColLocation,
objectTypeLocation, // Locatie uniforma noua
textureLocation, // Locatia uniformei "myTexture"
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

// Asteroid Elements
GLuint AstVaoId, AstVboId, AstUvVboId, AstNormalVboId, AstColorVboId;
std::vector<glm::vec3> ast_vertices;
std::vector<glm::vec2> ast_uvs;
std::vector<glm::vec3> ast_normals;

struct Asteroid {
    glm::vec3 position;
    float scale;
    float rotationSpeed;
    float angleOffset;
    glm::vec3 rotationAxis;
};
std::vector<Asteroid> asteroidBelt;

float PI = 3.141592;

// elementele sferei luate din 08_03
float const U_MIN = -PI / 2, U_MAX = PI / 2, V_MIN = 0, V_MAX = 2 * PI;
int const NR_PARR = 20, NR_MERID = 40;
int const NR_VF = (NR_PARR + 1) * (NR_MERID + 1);
float step_u = (U_MAX - U_MIN) / NR_PARR, step_v = (V_MAX - V_MIN) / NR_MERID;
float radius = 50.0f;

//	Dimensiunile ferestrei de afisare;
GLfloat
winWidth = 1400, winHeight = 600;

//	Variabila ce determina schimbarea culorii pixelilor in shader;
int codCol;

// Variabila pentru timpul scurs
float timeElapsed;

// Variabila pentru a controla vizibilitatea orbitelor
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
zNear = 100.f, zFar = 5000.f, // ZFar marit
width = 1400.f, height = 600.f, fov = 90.f * PI / 180;

//	Vectori pentru matricea de vizualizare;
glm::vec3
obs, pctRef, vert;

//	Variabile catre matricile de transformare;
glm::mat4
view, projection,
translateSystem,
rotateSun, scaleSun,
scalePlanet, rotatePlanetAxis, rotatePlanet, translatePlanet,
scaleSat, rotateSat, translateSat,
scaleMercury, rotateMercury, translateMercury,
scaleVenus, rotateVenus, translateVenus,
scaleMars, rotateMars, translateMars,
scaleJupiter, rotateJupiter, translateJupiter,
scaleSaturn, rotateSaturn, translateSaturn,
scaleUranus, rotateUranus, translateUranus,
scaleNeptune, rotateNeptune, translateNeptune;
// Removed Pluto variables

// Stiva de matrice - inglobeaza matricea de modelare si cea de vizualizare
std::stack<glm::mat4> mvStack;

// MODIFICARE: Variabile pentru vizualizare cu cuaternioni
// Conform cursului, cuaternionul va stoca orientarea camerei.
glm::quat viewQuaternion = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // Identitate: s=1, v=(0,0,0)


// Skybox
GLuint skyboxVAO, skyboxVBO, skyboxTexture;
GLuint skyboxProgram;

// Vertics pentru Skybox (cub de 1x1x1 centric)
float skyboxVertices[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
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
	// Shaders
	skyboxProgram = LoadShaders("skybox.vert", "skybox.frag");

	// VAO/VBO
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
	// 1. Load Model
	// Ensure Asteroid_1.obj is in the correct directory (project root usually)
	if (!loadOBJ("Asteroid_1.obj", ast_vertices, ast_uvs, ast_normals)) {
		printf("Failed to load Asteroid_1.obj. Generating fallback geometry.\n");
		
		// Generare cub simplu ca fallback
		float s = 0.5f;
		ast_vertices = {
			{-s,-s, s}, { s,-s, s}, { s, s, s}, {-s,-s, s}, { s, s, s}, {-s, s, s},
			{ s,-s,-s}, {-s,-s,-s}, {-s, s,-s}, { s,-s,-s}, {-s, s,-s}, { s, s,-s},
			{-s,-s,-s}, {-s,-s, s}, {-s, s, s}, {-s,-s,-s}, {-s, s, s}, {-s, s,-s},
			{ s,-s, s}, { s,-s,-s}, { s, s,-s}, { s,-s, s}, { s, s,-s}, { s, s, s},
			{-s, s, s}, { s, s, s}, { s, s,-s}, {-s, s, s}, { s, s,-s}, {-s, s,-s},
			{-s,-s,-s}, { s,-s,-s}, { s,-s, s}, {-s,-s,-s}, { s,-s, s}, {-s,-s, s}
		};
		// Umplem normalale (aproximativ sferice) si UV-uri dummy
		for (size_t i = 0; i < ast_vertices.size(); i++) {
			ast_normals.push_back(glm::normalize(ast_vertices[i]));
			ast_uvs.push_back(glm::vec2(0.0f, 0.0f));
		}
	}

	// 2. Setup VAO/VBO
	glGenVertexArrays(1, &AstVaoId);
	glBindVertexArray(AstVaoId);

	// Vertices
	glGenBuffers(1, &AstVboId);
	glBindBuffer(GL_ARRAY_BUFFER, AstVboId);
	glBufferData(GL_ARRAY_BUFFER, ast_vertices.size() * sizeof(glm::vec3), &ast_vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// UVs
	glGenBuffers(1, &AstUvVboId);
	glBindBuffer(GL_ARRAY_BUFFER, AstUvVboId);
	glBufferData(GL_ARRAY_BUFFER, ast_uvs.size() * sizeof(glm::vec2), &ast_uvs[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Normals
	glGenBuffers(1, &AstNormalVboId);
	glBindBuffer(GL_ARRAY_BUFFER, AstNormalVboId);
	glBufferData(GL_ARRAY_BUFFER, ast_normals.size() * sizeof(glm::vec3), &ast_normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Colors - Populate with gray/brown
	std::vector<glm::vec3> ast_colors;
	for (size_t i = 0; i < ast_vertices.size(); i++) {
		// Randomize slight color variation
		float r = 0.4f + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.2f;
		ast_colors.push_back(glm::vec3(r, r * 0.9f, r * 0.8f));
	}
	
	glGenBuffers(1, &AstColorVboId);
	glBindBuffer(GL_ARRAY_BUFFER, AstColorVboId);
	glBufferData(GL_ARRAY_BUFFER, ast_colors.size() * sizeof(glm::vec3), &ast_colors[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 3. Generate Belt Positions
	int numAsteroids = 150;
	float innerRadius = 850.0f; 
	float outerRadius = 1000.0f;

	for (int i = 0; i < numAsteroids; i++) {
		float angle = (float)(rand() % 360) * PI / 180.0f;
		float r = innerRadius + (float)(rand() % 100) / 100.0f * (outerRadius - innerRadius);
		float y = (float)(rand() % 100 - 50) / 2.0f;

		float x = r * cos(angle);
		float z = r * sin(angle);

		Asteroid ast;
		ast.position = glm::vec3(x, y, z);
		// Scale factor marit pentru a fi vizibil (modelul are raza ~0.5)
		ast.scale = 2.0f + (float)(rand() % 100) / 100.0f * 2.0f;

		ast.rotationSpeed = (float)(rand() % 100) / 1000.0f;
		ast.angleOffset = angle; // Initial orbit angle, if we want to move them along orbit
		ast.rotationAxis = glm::normalize(glm::vec3((float)(rand() % 100), (float)(rand() % 100), (float)(rand() % 100)));

		asteroidBelt.push_back(ast);
	}
}

// Functie pentru generarea gemetriei orbitei (cerc)
void CreateOrbitVBO(void)
{
	// Generam un cerc unitar in planul XZ
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

void ProcessSpecialKeys(int key, int xx, int yy)
{
	// Definim unghiul de rotatie incremental (theta)
	float theta = 0.05f;

	// Elemente pentru constructia cuaternionului de rotatie q = (s, v)
	// Conform slide 49: s = cos(theta/2), v = sin(theta/2) * u
	float s = cos(theta / 2.0f);
	float sin_val = sin(theta / 2.0f);

	glm::quat qRotate;

	switch (key)
	{
	case GLUT_KEY_LEFT:
		// Rotatie in jurul axei Y (0, 1, 0) - Stanga
		// u = (0, 1, 0)
		qRotate = glm::quat(s, 0.0f * sin_val, 1.0f * sin_val, 0.0f * sin_val);
		// Actualizam cuaternionul camerei prin inmultire
		viewQuaternion = qRotate * viewQuaternion;
		break;

	case GLUT_KEY_RIGHT:
		// Rotatie in jurul axei Y (0, 1, 0) - Dreapta (theta negativ)
		s = cos(-theta / 2.0f);
		sin_val = sin(-theta / 2.0f);
		qRotate = glm::quat(s, 0.0f * sin_val, 1.0f * sin_val, 0.0f * sin_val);
		viewQuaternion = qRotate * viewQuaternion;
		break;

	case GLUT_KEY_UP:
		// Rotatie in jurul axei X locale (1, 0, 0) - Sus
		// u = (1, 0, 0)
		qRotate = glm::quat(s, 1.0f * sin_val, 0.0f * sin_val, 0.0f * sin_val);
		// Inmultim la dreapta pentru rotatie locala sau stanga pentru globala.
		// Pentru "orbitare" in jurul axei locale X:
		viewQuaternion = viewQuaternion * qRotate;
		break;

	case GLUT_KEY_DOWN:
		// Rotatie in jurul axei X locale (1, 0, 0) - Jos (theta negativ)
		s = cos(-theta / 2.0f);
		sin_val = sin(-theta / 2.0f);
		qRotate = glm::quat(s, 1.0f * sin_val, 0.0f * sin_val, 0.0f * sin_val);
		viewQuaternion = viewQuaternion * qRotate;
		break;
	}
}
//texturare Luna + Pamant
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
		printf("Eroare la incarcarea texturii: %s\n", imagePath); //?/

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
			// Culoare variabila per varf (pentru cerinta ii)
			Colors[index] = glm::vec3(0.5f + 0.5 * sinf(u), 0.5f + 0.5 * cosf(v), 0.5f + 0.5 * sinf(u + v));

			// calcul normale

			Normals[index] = glm::normalize(glm::vec3(x_vf, y_vf, z_vf));
			// Calcul coordonate texturare (s, t) intre 0 si 1
			// Mapam paralele si meridianele pe patratul [0,1]x[0,1]
			float tex_s = (float)merid / NR_MERID; // sau parr, depinde de orientarea imaginii
			float tex_t = (float)parr / NR_PARR;
			TexCoords[index] = glm::vec2(tex_s, tex_t);
		}
	}

	// Generare Indici pentru fete (GL_QUADS)
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
	// Adaugam normalele la final
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(Colors) + sizeof(TexCoords), sizeof(Normals), Normals);

	glGenBuffers(1, &EboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	// Atribut 0: Pozitie
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Atribut 1: Culoare
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(Vertices));

	// Atribut 2: Coordonate Texturare
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(Vertices) + sizeof(Colors)));

	// Atribut 3: Normale
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(Vertices) + sizeof(Colors) + sizeof(TexCoords)));
}

void CreateRingVBO(void)
{
	// Structura vertexului trebuie sa coincida cu layout-ul din shader:
	// 0: Pos(4), 1: Color(3), 2: Tex(2), 3: Norm(3)
	struct VertexFormat {
		glm::vec4 position;
		glm::vec3 color;
		glm::vec2 texCoord;
		glm::vec3 normal;
	};

	std::vector<VertexFormat> vertices;
	int segments = 72; // Numar de segmente pentru cerc
	float innerRadius = 1.3f; // Relativ la raza planetei
	float outerRadius = 2.2f;

	for (int i = 0; i <= segments; i++) {
		float angle = (float)i * 2.0f * PI / segments;
		float c = cos(angle);
		float s = sin(angle);

		// Pentru fiecare pas, adaugam 2 vertecsi: unul interior, unul exterior
		// Vertex Interior
		VertexFormat v1;
		v1.position = glm::vec4(innerRadius * c, 0.0f, innerRadius * s, 1.0f);
		v1.color = glm::vec3(0.7f, 0.6f, 0.5f); // Culoare bej pentru inel
		v1.texCoord = glm::vec2(0.0f, (float)i / segments);
		v1.normal = glm::vec3(0.0f, 1.0f, 0.0f); // Normala in sus
		vertices.push_back(v1);

		// Vertex Exterior
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

	// Setam atributele (layout-ul este identic cu cel din example.vert)
	// Pos
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);
	// Color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec4)));
	// Tex
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec4) + sizeof(glm::vec3)));
	// Normal
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec4) + sizeof(glm::vec3) + sizeof(glm::vec2)));
}

//  Elimina obiectele de tip shader dupa rulare;
void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}

//  Eliminarea obiectelor de tip VBO dupa rulare;
void DestroyVBO(void)
{
	//  Eliberarea atributelor din shadere (pozitie, culoare, texturare etc.);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	//  Stergerea bufferelor pentru VARFURI (Coordonate, Culori), INDICI;
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VboId);
	glDeleteBuffers(1, &EboId);
	glDeleteBuffers(1, &OrbitVboId); // Stergem si bufferul orbitei

	glDeleteVertexArrays(1, &AstVaoId);
	glDeleteBuffers(1, &AstVboId);
	glDeleteBuffers(1, &AstUvVboId);
	glDeleteBuffers(1, &AstNormalVboId);
	glDeleteBuffers(1, &AstColorVboId);
}

//  Functia de eliberare a resurselor alocate de program;
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

//  Setarea parametrilor necesari pentru fereastra de vizualizare;
void Initialize(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);		//  Culoarea de fond a ecranului (Negru);
	CreateVBO();								//  Trecerea datelor de randare spre bufferul folosit de shadere;
	CreateShaders();							//  Initilizarea shaderelor;

	//	Instantierea variabilelor uniforme pentru a "comunica" cu shaderele;
	viewModelLocation = glGetUniformLocation(ProgramId, "viewModel");
	projLocation = glGetUniformLocation(ProgramId, "projection");
	codColLocation = glGetUniformLocation(ProgramId, "codCol");
	objectTypeLocation = glGetUniformLocation(ProgramId, "objectType");

	//Incarcare texturi
	// Locatia sampler-ului din fragment shader
	textureLocation = glGetUniformLocation(ProgramId, "myTexture");
	// Locatia pozitiei sursei de lumina
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

	// Initializeaza Skybox (VAO, Shaders, Texturi)
	InitSkybox();

	// Initializeaza Orbita
	CreateOrbitVBO();
	CreateRingVBO();
	InitAsteroids();

	//	Realizarea proiectiei - pot fi utilizate si alte variante;
	/* projection = glm::ortho(xMin, xMax, yMin, yMax, zNear, zFar);*/
		/*projection = glm::frustum(xMin, xMax, yMin, yMax, zNear, zFar);*/ //foarte departe
	projection = glm::perspective(fov, GLfloat(width) / GLfloat(height), zNear, zFar); //default
	/*projection = glm::infinitePerspective(fov, GLfloat(width) / GLfloat(height), zNear);*/
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);
}

//	Functia de desenare a graficii pe ecran;
void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	timeElapsed = glutGet(GLUT_ELAPSED_TIME);

	// --- IMPLEMENTARE VIZUALIZARE CU CUATERNIONI (CURS 12) ---

	// 1. Extragem componentele cuaternionului q = s + ai + bj + ck
	// In GLM: w=s, x=a, y=b, z=c
	float s = viewQuaternion.w;
	float a = viewQuaternion.x;
	float b = viewQuaternion.y;
	float c = viewQuaternion.z;

	// 2. Construim Matricea de Rotatie 3x3 conform Slide 51 
	// Atentie: GLM foloseste column-major order (col, row), dar formula matematica e standard.
	// Slide 51:
	// Row 1: s^2+a^2-b^2-c^2, 2ab-2sc,       2ac+2sb
	// Row 2: 2ab+2sc,       s^2-a^2+b^2-c^2, 2bc-2sa
	// Row 3: 2ac-2sb,       2bc+2sa,         s^2-a^2-b^2+c^2

	glm::mat4 rotationMat(1.0f); // Initializare identitate

	// Coloana 0 (vectorul Right transformat)
	rotationMat[0][0] = s * s + a * a - b * b - c * c;
	rotationMat[0][1] = 2 * a * b + 2 * s * c;
	rotationMat[0][2] = 2 * a * c - 2 * s * b;

	// Coloana 1 (vectorul Up transformat)
	rotationMat[1][0] = 2 * a * b - 2 * s * c;
	rotationMat[1][1] = s * s - a * a + b * b - c * c;
	rotationMat[1][2] = 2 * b * c + 2 * s * a;

	// Coloana 2 (vectorul Forward transformat)
	rotationMat[2][0] = 2 * a * c + 2 * s * b;
	rotationMat[2][1] = 2 * b * c - 2 * s * a;
	rotationMat[2][2] = s * s - a * a - b * b + c * c;

	// 3. Calculam pozitia observatorului
	// Vectorul initial al camerei (fara rotatie) este pe axa Z la distanta 'dist'
	glm::vec4 initialCamPos = glm::vec4(0.0f, 0.0f, dist, 1.0f);

	// Rotim pozitia folosind matricea derivata din cuaternion
	glm::vec4 rotatedCamPos = rotationMat * initialCamPos;

	// Setam pozitia observatorului
	obs.x = refX + rotatedCamPos.x;
	obs.y = refY + rotatedCamPos.y;
	obs.z = refZ + rotatedCamPos.z;

	// Punctul de referinta ramane neschimbat
	pctRef = glm::vec3(refX, refY, refZ);

	// Verticala trebuie si ea rotita pentru ca camera sa nu se rastoarne
	// Vectorul vertical initial este (0, 1, 0)
	glm::vec4 initialUp = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
	glm::vec4 rotatedUp = rotationMat * initialUp;
	vert = glm::vec3(rotatedUp);

	view = glm::lookAt(obs, pctRef, vert);

	// --- DESENARE SKYBOX ---
	// Se deseneaza PRIMUL sau ULTIMUL. Desenam ULTIMUL pentru optimizare (folosind adancimea maxima)
	// Dar schimbam functia de adancime la GL_LEQUAL
		// --- DESENARE SKYBOX ---
	// Se deseneaza PRIMUL sau ULTIMUL. Desenam ULTIMUL pentru optimizare 
	glDepthFunc(GL_LEQUAL);
	glUseProgram(skyboxProgram);

	// Eliminam translatia din matricea de vizualizare pentru Skybox
	glm::mat4 viewSkybox = glm::mat4(glm::mat3(view));

	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "view"), 1, GL_FALSE, glm::value_ptr(viewSkybox));
	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glBindVertexArray(skyboxVAO);

	// TEXTURE BINDING REMOVED
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
	// glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Masca de adancime oprita
	glDepthMask(GL_FALSE);

	// Dezactivam face culling pentru interior
	glDisable(GL_CULL_FACE);

	glDrawArrays(GL_TRIANGLES, 0, 36);

	// Reactivam setarile implicite
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
	// glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // Removed
	glBindVertexArray(0);

	// Revenim la setarile normale
	glDepthFunc(GL_LESS);
	glUseProgram(ProgramId);

	// Re-bind the VAO for the solar system objects (sphere geometry)
	// This restores the EBO binding required for glDrawElements
	glBindVertexArray(VaoId);

	glm::vec4 lightPosView = view * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	glUniform3f(lightPosLocation, lightPosView.x, lightPosView.y, lightPosView.z);

	// Matrice pentru miscarea obiectelor din sistem
	// Intregul sistem se deplaseaza prin translatie
	translateSystem = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));

	// Soarele nu se roteste
	rotateSun = glm::mat4(1.0f);
	scaleSun = glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 4.0f, 4.0f));


	// Pamantul se obtine scaland sfera initiala
	scalePlanet = glm::scale(glm::mat4(1.0f), glm::vec3(0.5, 0.5, 0.5));
	// se roteste in jurul propriei axe
	rotatePlanetAxis = glm::rotate(glm::mat4(1.0f), (float)0.001 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	// se roteste in jurul astrului central
	rotatePlanet = glm::rotate(glm::mat4(1.0f), (float)0.0005 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	// este translatat in raport cu Soarele
	translatePlanet = glm::translate(glm::mat4(1.0f), glm::vec3(600.0, 0.0, 0.0));

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

	// Link catre Pluto eliminat de aici

	// Luna: rotatie in jurul planetei, translatie mica, scalare mare
	rotateSat = glm::rotate(glm::mat4(1.0f), (float)0.003 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	translateSat = glm::translate(glm::mat4(1.0f), glm::vec3(80.0, 0.0, 0.0));
	scaleSat = glm::scale(glm::mat4(1.0f), glm::vec3(0.2, 0.2, 0.2));

	// Desenarea primitivelor + manevrarea stivei de matrice
	// 
	// Matricea de vizualizare este adaugata in varful stivei de matrice
	mvStack.push(view);                  // In varful stivei:   view 

	// 0) Pentru intregul sistem
	// Matrice de translatie pentru intregul sistem
	mvStack.top() *= translateSystem;	 // In varful stivei:  view * translateSystem 
	mvStack.push(mvStack.top());         // Pe poz 2 a stivei: view * translateSystem 

	// Soare
	mvStack.top() *= rotateSun;
	// Rotatie 90 grade pe X pentru orientare corecta textura
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleSun;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 1);

	// Textura Soare
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureSunId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// --- DESENARE ORBITE PLANETE ---
	if (showOrbits)
	{
		glUniform1i(objectTypeLocation, 12); // Tip pentru orbită (fara lumina)
		glBindVertexArray(OrbitVaoId);
		glLineWidth(1.0f);

		// Lista razelor orbitelor planetelor
		std::vector<float> orbitRadii = {
			300.0f,  // Mercur
			450.0f,  // Venus
			600.0f,  // Pamant
			750.0f,  // Marte
			1100.0f, // Jupiter
			1400.0f, // Saturn
			1700.0f, // Uranus
			2000.0f  // Neptun
			// Pluto removed
		};

		for (float r : orbitRadii) {
			mvStack.push(mvStack.top());
			mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(r, 1.0f, r));
			glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
			glDrawArrays(GL_LINE_LOOP, 0, 360);
			mvStack.pop();
		}

		// Revenim la VAO-ul sferei pentru desenarea planetelor
		glBindVertexArray(VaoId);
	}
	// --------------------------------

	// Mercur
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateMercury;
	mvStack.top() *= translateMercury;
	// Rotatie 90 grade pe X pentru orientare corecta textura
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleMercury;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 4); // 4 pentru Mercur

	// Textura Mercur
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureMercuryId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Venus
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateVenus;
	mvStack.top() *= translateVenus;
	// Rotatie 90 grade pe X pentru orientare corecta textura
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleVenus;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 5); // 5 pentru Venus

	// Textura Venus
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureVenusId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Marte
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateMars;
	mvStack.top() *= translateMars;
	// Rotatie 90 grade pe X pentru orientare corecta textura
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleMars;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 6); // 6 pentru Marte

	// Textura Marte
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureMarsId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Jupiter
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateJupiter;
	mvStack.top() *= translateJupiter;
	// Rotatie 90 grade pe X pentru orientare corecta textura
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleJupiter;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 7); // 7 pentru Jupiter

	// Textura Jupiter
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureJupiterId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Saturn
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateSaturn;
	mvStack.top() *= translateSaturn;

	// Salvam matricea curenta (centrul lui Saturn) pentru a desena inelul tot aici
	glm::mat4 saturnCenterMatrix = mvStack.top();

	// 1. Desenam Planeta Saturn (Opaca)
	// Rotatie 90 grade pe X pentru orientare corecta textura (aplicata doar la planeta, nu si la save-ul pentru inel)
	// Atentie: save-ul s-a facut inainte. Acum rotim MV-ul curent.
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleSaturn;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 8); // 8 pentru Saturn

	// Textura Saturn
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureSaturnId);
	glUniform1i(textureLocation, 0);

	glBindVertexArray(VaoId); // Asiguram ca e VAO-ul sferei
	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));

	// 2. Desenam Inelele (Transparente) - Implementare Curs Pag 35
	glEnable(GL_BLEND); // Activam blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Formula standard
	glDepthMask(GL_FALSE); // Oprim scrierea in Depth Buffer (recomandat pt transparente)
	glDisable(GL_CULL_FACE); // Dezactivam culling pentru a vedea inelele din ambele parti

	mvStack.top() = saturnCenterMatrix; // Revenim la centrul lui Saturn

	// Scalam si rotim inelul
	// Il facem plat pe Y (desi geometria e deja plata, o putem mari)
	// Rotim putin inelul sa se vada frumos
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 50.0f)); // Raza inelului ajustata la dimensiunea planetei (r=50)

	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 13); // COD NOU PENTRU INEL

	glBindVertexArray(RingVaoId);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 146); // 72 segmente * 2 vertecsi + 2 de inchidere

	// Restauram starile OpenGL
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE); // Reactivam culling
	glBindVertexArray(VaoId); // Revenim la sfera pentru urmatoarele planete

	mvStack.pop();

	// Uranus
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateUranus;
	mvStack.top() *= translateUranus;
	// Rotatie 90 grade pe X pentru orientare corecta textura
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleUranus;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 9); // 9 pentru Uranus

	// Textura Uranus
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureUranusId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Neptun
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateNeptune;
	mvStack.top() *= translateNeptune;
	// Rotatie 90 grade pe X pentru orientare corecta textura
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleNeptune;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 10); // 10 pentru Neptun

	// Textura Neptun
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureNeptuneId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();


	// Draw Asteroids
	if (ast_vertices.size() > 0)
	{
		glUniform1i(objectTypeLocation, 14); // Cod asteroid
		glBindVertexArray(AstVaoId);

		for (const auto& ast : asteroidBelt) {
			mvStack.push(mvStack.top());
			
			// Translate to position
			mvStack.top() *= glm::translate(glm::mat4(1.0f), ast.position);
			
			// Local rotation
			mvStack.top() *= glm::rotate(glm::mat4(1.0f), ast.rotationSpeed * timeElapsed, ast.rotationAxis);
			
			// Scale
			mvStack.top() *= glm::scale(glm::mat4(1.0f), glm::vec3(ast.scale, ast.scale, ast.scale));

			glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
			glDrawArrays(GL_TRIANGLES, 0, ast_vertices.size());
			
			mvStack.pop();
		}
		// Restore VAO
		glBindVertexArray(VaoId);
	}

	// Pamant
	mvStack.top() *= rotatePlanet;
	mvStack.top() *= translatePlanet;
	mvStack.push(mvStack.top());

	mvStack.top() *= rotatePlanetAxis;
	// Rotim textura Pamantului sa fie orientata corespunzator
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scalePlanet;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 2);

	// Textura Pamant
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureEarthId);
	glUniform1i(textureLocation, 0); // Trimitem indexul unitatii de textura (0)

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Luna
	mvStack.top() *= rotateSat;
	mvStack.top() *= translateSat;
	// Rotatie 90 grade pe X pentru orientare corecta textura
	mvStack.top() *= glm::rotate(glm::mat4(1.0f), PI / 2, glm::vec3(1.0f, 0.0f, 0.0f));
	mvStack.top() *= scaleSat;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 3);

	// textura Luna
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureMoonId);
	glUniform1i(textureLocation, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	glutSwapBuffers();
	glFlush();
}

//	Punctul de intrare in program, se ruleaza rutina OpenGL;
int main(int argc, char* argv[])
{
	// Afisare director curent de lucru pentru diagnosticare
	char cwd[1024];
	if (_getcwd(cwd, sizeof(cwd)) != NULL) {
		printf("Directorul curent de lucru este: %s\n", cwd);
	}
	else {
		perror("getcwd() error");
	}

	//  Se initializeaza GLUT si contextul OpenGL si se configureaza fereastra si modul de afisare;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);		//	Se folosesc 2 buffere pentru desen (unul pentru afisare si unul pentru randare => animatii cursive) si culori RGB + 1 buffer pentru adancime;
	glutInitWindowSize(winWidth, winHeight);						//  Dimensiunea ferestrei;
	glutInitWindowPosition(100, 100);								//  Pozitia initiala a ferestrei;
	glutCreateWindow("Proiect 2");		//	Creeaza fereastra de vizualizare, indicand numele acesteia;

	//	Se initializeaza GLEW si se verifica suportul de extensii OpenGL modern disponibile pe sistemul gazda;
	//  Trebuie initializat inainte de desenare;

	glewInit();

	Initialize();							//  Setarea parametrilor necesari pentru fereastra de vizualizare; 
	glutDisplayFunc(RenderFunction);		//  Desenarea scenei in fereastra;
	glutIdleFunc(RenderFunction);			//	Asigura rularea continua a randarii;
	glutKeyboardFunc(ProcessNormalKeys);	//	Functii ce proceseaza inputul de la tastatura utilizatorului;
	glutSpecialFunc(ProcessSpecialKeys);
	glutCloseFunc(Cleanup);					//  Eliberarea resurselor alocate de program;

	//  Bucla principala de procesare a evenimentelor GLUT (functiile care incep cu glut: glutInit etc.) este pornita;
	//  Prelucreaza evenimentele si deseneaza fereastra OpenGL pana cand utilizatorul o inchide;

	glutMainLoop();

	return 0;
}