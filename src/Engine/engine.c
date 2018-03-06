/**
 * engine.c
 * Purpose: Reads an XML file specifying which models to render and the files in which this models have been specified. 
 *
 * @author João Vieira 
 * @author José Martins
 * @author Miguel Quaresma
 * @author Simão Barbosa
 */

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <unistd.h>
#elif __linux__
#include <unistd.h>
#include <GL/glut.h>
#else
#include<io.h>
#include <GL/glut.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

float radius = 10.0f;
float alfa = M_PI/4;
float beta = M_PI/4;
float change = 0.025;
float changeR = 0.2;

void changeSize(int w, int h) {

	if(h == 0) h = 1;

	float ratio = w * 1.0 / h;

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
		
	glViewport(0, 0, w, h);

	gluPerspective(45.0f ,ratio, 1.0f ,1000.0f);

	glMatrixMode(GL_MODELVIEW);

}

ssize_t readln (int fildes, void * buf, size_t nbyte){
	int i,x;
	char* s = buf;
	for (i=0; i<nbyte && (x=read(fildes,&s[i],1))>0 && s[i]!='\n';i++);
	if (s[i]=='\n') i++;
	return i;
}

void drawObject() {

	int fd,x;
	char buffer[20];
	int coord[3];
	char* aux;

	glColor3f(1,0,0);

	glBegin(GL_TRIANGLES);

		fd = open("box.txt",O_RDONLY);

		while ((x=readln(fd,buffer,20))>0){

			aux = strtok(buffer," ");
			coord[0] = atoi(aux);
			aux = strtok (NULL, " ");
			coord[1] = atoi(aux);
			aux = strtok (NULL, " ");
			coord[2] = atoi(aux);

			glVertex3f(coord[0], coord[1], coord[2]);

		}

	glEnd();

}

void drawXYZ(){

	glColor3f(1.0,0.0,0.0); // red x
    glBegin(GL_LINES);
    glVertex3f(-3.0, 0.0f, 0.0f);
    glVertex3f(3.0, 0.0f, 0.0f);
    glEnd();
    glFlush();
  
    glColor3f(0.0,1.0,0.0); // green y
    glBegin(GL_LINES);
    glVertex3f(0.0, -3.0f, 0.0f);
    glVertex3f(0.0, 3.0f, 0.0f);
    glEnd();
    glFlush();
 
    glColor3f(0.0,0.0,1.0); // blue z
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0f ,-3.0f );
    glVertex3f(0.0, 0.0f ,3.0f );
    glEnd();
    glFlush();

}


void renderScene(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Camera
	glLoadIdentity();
	gluLookAt(radius * cos(beta) * sin(alfa), radius * sin(beta), radius * cos(beta) * cos(alfa), 
		      0.0,0.0,0.0,
			  0.0f,1.0f,0.0f);

	// drawing instructions
	drawXYZ();

	drawObject();

	// End of Frame
	glutSwapBuffers();
}


void processKeys(unsigned char c, int xx, int yy) {

	switch(c)
	{
		case 'w':
			radius -= changeR;
			break;
		case 's':
			radius += changeR;
			break;
	}
	glutPostRedisplay();
}


void processSpecialKeys(int key, int xx, int yy) {
	switch(key)
	{
		case GLUT_KEY_LEFT:
			alfa -= change;
			break;
		case GLUT_KEY_RIGHT:
			alfa += change;
			break;
		case GLUT_KEY_UP:
			if (beta < 1.5f) beta += change;
			break;
		case GLUT_KEY_DOWN:
			if (beta > -1.5f) beta -= change;
			break;
	}
	glutPostRedisplay();
}


int main(int argc, char **argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800,800);
	glutCreateWindow("3DEngine");
		
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	
	glutKeyboardFunc(processKeys);
	glutSpecialFunc(processSpecialKeys);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
	glutMainLoop();
	
	return 1;
}
