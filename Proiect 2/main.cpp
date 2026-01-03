//   - transformarile de modelare si cea de vizualizare sunt inglobate intr-o singura matrice;
//	 - folosirea stivelor de matrice;
//   - generare procedurala sfera;

// instanciated rendering pentru asteroizi

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
	textureMoonId;

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

//	Elemente pentru matricea de vizualizare;
float obsX = 0.0, obsY = 0.0, obsZ = 500.f, // Distanta marita pentru a vedea tot sistemul
refX = 0.0f, refY = 0.0f, refZ = -100.f,
vX = 0.0;
//	Elemente pentru matricea de proiectie;
float xMin = -700.f, xMax = 700.f, yMin = -300.f, yMax = 300.f,
zNear = 100.f, zFar = 1000.f, // ZFar marit
width = 1400.f, height = 600.f, fov = 90.f * PI / 180;

//	Vectori pentru matricea de vizualizare;
glm::vec3
obs, pctRef, vert;

//	Variabile catre matricile de transformare;
glm::mat4
view, projection,
translateSystem,
rotateSun,
scalePlanet, rotatePlanetAxis, rotatePlanet, translatePlanet,
scaleSat, rotateSat, translateSat;

// Stiva de matrice - inglobeaza matricea de modelare si cea de vizualizare
std::stack<glm::mat4> mvStack;

void ProcessNormalKeys(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'l':			//	Apasarea tastelor `l` si `r` modifica pozitia verticalei in planul de vizualizare;
		vX += 0.1;
		break;
	case 'r':
		vX -= 0.1;
		break;
	case '+':			//	Apasarea tastelor `+` si `-` schimba pozitia observatorului (se departeaza / aproprie);
		obsZ += 10;
		break;
	case '-':
		obsZ -= 10;
		break;
	}
	if (key == 27)
		exit(0);
}

void ProcessSpecialKeys(int key, int xx, int yy)
{
	switch (key)				//	Procesarea tastelor 'LEFT', 'RIGHT', 'UP', 'DOWN';
	{							//	duce la deplasarea observatorului pe axele Ox si Oy;
	case GLUT_KEY_LEFT:
		obsX -= 20;
		break;
	case GLUT_KEY_RIGHT:
		obsX += 20;
		break;
	case GLUT_KEY_UP:
		obsY += 20;
		break;
	case GLUT_KEY_DOWN:
		obsY -= 20;
		break;
	}
}

//Texturare Luna + Pamant
GLuint LoadTexture(const char* imagePath)
{
	GLuint textureId;

	// 1. Generarea identificatorului [cite: 84]
	glGenTextures(1, &textureId);

	// 2. Legarea texturii [cite: 94]
	glBindTexture(GL_TEXTURE_2D, textureId);

	// 3. Setarea parametrilor de wrapping si filtrare [cite: 150, 203]
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 4. Incarcarea imaginii folosind SOIL 
	int width, height, channels;
	unsigned char* image = SOIL_load_image(imagePath, &width, &height, &channels, SOIL_LOAD_RGB);

	if (image)
	{
		// 5. Generarea texturii efective 
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		SOIL_free_image_data(image); // Eliberare memorie RAM
	}
	else
	{
		printf("Eroare la incarcarea texturii: %s\n", imagePath);
	}

	// Dezlegare textura
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
		
			// ADAGUARE: Calcul coordonate texturare (s, t) intre 0 si 1 [cite: 239]
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

	// ADAGUARE: Alocam memorie si pentru TexCoords
	// Ordine in buffer: Vertices | Colors | TexCoords
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(Colors) + sizeof(TexCoords), NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices), sizeof(Colors), Colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(Colors), sizeof(TexCoords), TexCoords); // Incarcam datele de textura

	glGenBuffers(1, &EboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	// Atribut 0: Pozitie
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Atribut 1: Culoare
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(Vertices));

	// ADAGUARE: Atribut 2: Coordonate Texturare 
	glEnableVertexAttribArray(2);
	// Offset-ul este dimensiunea Vertices + dimensiunea Colors
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(Vertices) + sizeof(Colors)));
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

	//  Eliberaea obiectelor de tip VAO;
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
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

	// ADAGUARE: Incarcarea texturilor
	textureEarthId = LoadTexture("earth_map.jpg"); // Placeholder
	textureMoonId = LoadTexture("moon_map.jpg");   // Placeholder

	//	Realizarea proiectiei - pot fi utilizate si alte variante;
	// projection = glm::ortho(xMin, xMax, yMin, yMax, zNear, zFar);
	//	projection = glm::frustum(xMin, xMax, yMin, yMax, zNear, zFar);
	projection = glm::perspective(fov, GLfloat(width) / GLfloat(height), zNear, zFar);
	// projection = glm::infinitePerspective(fov, GLfloat(width) / GLfloat(height), zNear); 
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);
}

//	Functia de desenare a graficii pe ecran;
void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//  Se curata ecranul OpenGL pentru a fi desenat noul continut (bufferul de culoare & adancime);
	glEnable(GL_DEPTH_TEST);                                //  Activarea testului de adancime

	// Variabila care indica timpul scurs de la initializare
	timeElapsed = glutGet(GLUT_ELAPSED_TIME);

	//	Matricea de vizualizare - actualizare
	//	Pozitia observatorului;
	obs = glm::vec3(obsX, obsY, obsZ);
	//	Pozitia punctului de referinta;
	refX = obsX; refY = obsY;
	pctRef = glm::vec3(refX, refY, refZ);
	//	Verticala din planul de vizualizare; 
	vert = glm::vec3(vX, 1.0f, 0.0f);
	view = glm::lookAt(obs, pctRef, vert);

	// Matrice pentru miscarea obiectelor din sistem
	// 
	// Intregul sistem se deplaseaza prin translatie
	translateSystem = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));

	// Soarele nu se roteste
	rotateSun = glm::mat4(1.0f);

	// Planeta se obtine scaland sfera initiala
	scalePlanet = glm::scale(glm::mat4(1.0f), glm::vec3(0.5, 0.5, 0.5));
	// Planeta se roteste in jurul propriei axe
	rotatePlanetAxis = glm::rotate(glm::mat4(1.0f), (float)0.001 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	// Planeta se roteste in jurul astrului central
	rotatePlanet = glm::rotate(glm::mat4(1.0f), (float)0.0005 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	// Planeta este translatata in raport cu astrul central
	translatePlanet = glm::translate(glm::mat4(1.0f), glm::vec3(250.0, 0.0, 0.0));

	// Satelit: rotatie in jurul planetei, translatie mica, scalare mare
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

	// --- 1. SOARELE (Fara textura, doar culoare) ---
	mvStack.top() *= rotateSun;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 1);

	// Dezlegam orice textura pentru siguranta (sau folosim textura 0)
	glBindTexture(GL_TEXTURE_2D, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// --- 2. PLANETA (Pamant - Cu Textura) ---
	mvStack.top() *= rotatePlanet;
	mvStack.top() *= translatePlanet;
	mvStack.push(mvStack.top());

	mvStack.top() *= rotatePlanetAxis;
	mvStack.top() *= scalePlanet;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 2);

	// ADAGUARE: Activare si Legare Textura Pamant [cite: 209]
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureEarthId);
	glUniform1i(textureLocation, 0); // Trimitem indexul unitatii de textura (0)

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// --- 3. SATELIT (Luna - Cu Textura) ---
	mvStack.top() *= rotateSat;
	mvStack.top() *= translateSat;
	mvStack.top() *= scaleSat;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 3);

	// ADAGUARE: Activare si Legare Textura Luna
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
	//  Se initializeaza GLUT si contextul OpenGL si se configureaza fereastra si modul de afisare;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);		//	Se folosesc 2 buffere pentru desen (unul pentru afisare si unul pentru randare => animatii cursive) si culori RGB + 1 buffer pentru adancime;
	glutInitWindowSize(winWidth, winHeight);						//  Dimensiunea ferestrei;
	glutInitWindowPosition(100, 100);								//  Pozitia initiala a ferestrei;
	glutCreateWindow("Sistem Solar - Stive Matrice & Sfera Procedurala");		//	Creeaza fereastra de vizualizare, indicand numele acesteia;

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