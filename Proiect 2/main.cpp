
//   - transformarile de modelare si cea de vizualizare sunt inglobate intr-o singura matrice;
//	 - folosirea stivelor de matrice;
//   - generare procedurala sfera;

// instanciated rendering pentru asteroizi

#include <windows.h>        //	Utilizarea functiilor de sistem Windows (crearea de ferestre, manipularea fisierelor si directoarelor);
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
#include<stack>


GLuint
VaoId,
VboId,
EboId,
ProgramId,
viewModelLocation,
projLocation,
codColLocation,
objectTypeLocation; // Locatie uniforma noua

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

	//  Se creeaza / se leaga un VAO (Vertex Array Object) - util cand se utilizeaza mai multe VBO;
	glGenVertexArrays(1, &VaoId);                                                   //  Generarea VAO si indexarea acestuia catre variabila VaoId;
	glBindVertexArray(VaoId);

	//  Se creeaza un buffer pentru VARFURI - COORDONATE si CULORI;
	glGenBuffers(1, &VboId);														//  Generarea bufferului si indexarea acestuia catre variabila VboId;
	glBindBuffer(GL_ARRAY_BUFFER, VboId);											//  Setarea tipului de buffer - atributele varfurilor;
	//  Alocam spatiu pentru varfuri si culori
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(Colors), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices), sizeof(Colors), Colors);

	//	Se creeaza un buffer pentru INDICI;
	glGenBuffers(1, &EboId);														//  Generarea bufferului si indexarea acestuia catre variabila EboId;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);									//  Setarea tipului de buffer - atributele varfurilor;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	//	Se activeaza lucrul cu atribute;
	//  Se asociaza atributul (0 = coordonate) pentru shader;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
	//  Se asociaza atributul (1 =  culoare) pentru shader;
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(Vertices));
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

	// Soarele se roteste in jurul propriei axe
	rotateSun = glm::rotate(glm::mat4(1.0f), -(float)0.0005 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));

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

	// 1) Pentru Soare (astrul central)

	// Actualizare a matricei din varful stivei
	// Rotatie in jurul axei proprii
	mvStack.top() *= rotateSun;         // In varful stivei:  view * translateSystem * rotateSun          

	// Transmitere matrice catre shader
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	// Setam tipul obiectului = 1 (Soare)
	glUniform1i(objectTypeLocation, 1);

	//	Desenarea Soarelui (Sfera procedurala)
	codCol = 0;
	glUniform1i(codColLocation, codCol);
	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));

	// Eliminare Soare din varf stiva
	mvStack.pop();		                 // In varful stivei:   view * translateSystem 


	// 2) Pentru planeta

	// Actualizare a matricei din varful stivei
	// Rotatie in jurul Soarelui
	mvStack.top() *= rotatePlanet;		// In varful stivei:  view * translateSystem * rPl
	// Deplasare fata de centrul Soarelui
	mvStack.top() *= translatePlanet;   // In varful stivei:  view * translateSystem * rPl * tPl

	// -- Aici suntem la pozitia Planetei. Salvam matricea pentru Satelit --
	mvStack.push(mvStack.top());        // Stiva: ... PlanetCenter, PlanetCenter

	// Rotatie in jurul axei proprii a Planetei
	mvStack.top() *= rotatePlanetAxis;
	// Scalare (redimensionare obiect 3D)
	mvStack.top() *= scalePlanet;

	// Transmitere matrice
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 2); // 2 = Planeta

	//	Desenarea Planetei
	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));

	// Scoatem matricea de desenare a Planetei, revenim la Centrul Planetei
	mvStack.pop();                      // Stiva: ... PlanetCenter

	// 3) Pentru Satelit

	// Rotatie in jurul Planetei
	mvStack.top() *= rotateSat;
	// Deplasare fata de Planeta
	mvStack.top() *= translateSat;
	// Scalare Satelit
	mvStack.top() *= scaleSat;

	// Transmitere matrice
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 3); // 3 = Satelit

	// Desenarea Satelitului
	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));

	// Scoatem matricea Satelitului
	// Aceasta este ultima matrice ramasa in stiva pentru acest frame (cea derivata din PlanetCenter), 
	// deci dupa acest pop stiva este goala.
	mvStack.pop(); // Stiva: goala.

	// Aici s-a oprit secventa de pops. 
	// Nu mai este nevoie de alt pop pentru ca am consumat matricea sistemului 
	// prin transformarile planetei (modificand top() direct).

	glutSwapBuffers();	//	Inlocuieste imaginea deseneata in fereastra cu cea randata; 
	glFlush();			//  Asigura rularea tuturor comenzilor OpenGL apelate anterior;
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