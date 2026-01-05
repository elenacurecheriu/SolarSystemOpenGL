//   - transformarile de modelare si cea de vizualizare sunt inglobate intr-o singura matrice;
//	 - folosirea stivelor de matrice;
//   - generare procedurala sfera;

// TODO !! instanciated rendering pentru asteroizi (centura de asteroizi intre Marte si Jupiter?)
// adaugare in shader texturare pentru toate planetele
// !!complex: inelele lui Saturn, folosire blending pentru a le face transparente
// cuaternioni pentru rotatii
// sa se vada orbita planetelor (cercuri in planul xy)
// fundal cu stele pentru a da impresia de spatiu infinit + LUMINA DE LA STELE; instantiated rendering pentru stele
// !! complex: gravitatie, miscarea planetelor nu este "hardcoded", ci rezultatul fortei de gravitatie
// atmosfera pamantului (efect de glow) (atmospheric scattering)
// efect de lens flare pentru soare
// normal/bump mapping pentru planete
// !! complex: sistem de particule (comete, explozii pe soare)
// post processing (bloom, motion blur)



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
	textureMoonId,
	lightPosLocation
	;

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
float obsX = 0.0, obsY = 0.0, obsZ = 3000.f, // distanta marita pentru a vedea tot sistemul
refX = 0.0f, refY = 0.0f, refZ = -100.f,
vX = 0.0;
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
scaleNeptune, rotateNeptune, translateNeptune,
scalePluto, rotatePluto, translatePluto;

// Stiva de matrice - inglobeaza matricea de modelare si cea de vizualizare
std::stack<glm::mat4> mvStack;

// TODO de adaugat functii suplimentare pentru controlul camerei
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
		printf("Eroare la incarcarea texturii: %s\n", imagePath); //?
	
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
	// Locatia pozitiei sursei de lumina
	lightPosLocation = glGetUniformLocation(ProgramId, "lightPos");

	textureEarthId = LoadTexture("earth_map.jpg"); // Placeholder
	textureMoonId = LoadTexture("moon_map.jpg");   // Placeholder
	

	//	Realizarea proiectiei - pot fi utilizate si alte variante;
	/* projection = glm::ortho(xMin, xMax, yMin, yMax, zNear, zFar);*/
		/*projection = glm::frustum(xMin, xMax, yMin, yMax, zNear, zFar);*/ //foarte departe
	/*projection = glm::perspective(fov, GLfloat(width) / GLfloat(height), zNear, zFar);*/ //default
	 projection = glm::infinitePerspective(fov, GLfloat(width) / GLfloat(height), zNear); //cute
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

	// Trimitem pozitia Soarelui catre shader
	// Soarele este in (0,0,0) in World Space. Transformam in View Space.

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

	// Pluto
	scalePluto = glm::scale(glm::mat4(1.0f), glm::vec3(0.15f, 0.15f, 0.15f));
	rotatePluto = glm::rotate(glm::mat4(1.0f), (float)0.00005 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	translatePluto = glm::translate(glm::mat4(1.0f), glm::vec3(2200.0, 0.0, 0.0));

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
	mvStack.top() *= scaleSun;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 1);

	// Dezlegam orice textura pentru siguranta (sau folosim textura 0)
	glBindTexture(GL_TEXTURE_2D, 0);

	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Mercur
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateMercury;
	mvStack.top() *= translateMercury;
	mvStack.top() *= scaleMercury;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 4); // 4 pentru Mercur
	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Venus
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateVenus;
	mvStack.top() *= translateVenus;
	mvStack.top() *= scaleVenus;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 5); // 5 pentru Venus
	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Marte
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateMars;
	mvStack.top() *= translateMars;
	mvStack.top() *= scaleMars;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 6); // 6 pentru Marte
	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Jupiter
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateJupiter;
	mvStack.top() *= translateJupiter;
	mvStack.top() *= scaleJupiter;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 7); // 7 pentru Jupiter
	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Saturn
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateSaturn;
	mvStack.top() *= translateSaturn;
	mvStack.top() *= scaleSaturn;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 8); // 8 pentru Saturn
	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Uranus
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateUranus;
	mvStack.top() *= translateUranus;
	mvStack.top() *= scaleUranus;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 9); // 9 pentru Uranus
	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Neptun
	mvStack.push(mvStack.top());
	mvStack.top() *= rotateNeptune;
	mvStack.top() *= translateNeptune;
	mvStack.top() *= scaleNeptune;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 10); // 10 pentru Neptun
	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

	// Pluto
	mvStack.push(mvStack.top());
	mvStack.top() *= rotatePluto;
	mvStack.top() *= translatePluto;
	mvStack.top() *= scalePluto;
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	glUniform1i(objectTypeLocation, 11); // 11 pentru Pluto
	glDrawElements(GL_QUADS, NR_MERID * NR_PARR * 4, GL_UNSIGNED_SHORT, (void*)(0));
	mvStack.pop();

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