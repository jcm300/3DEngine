/**
 * engine.c
 * Purpose: Reads an XML file specifying which models to render and the files in which this models have been specified. 
 *
 * @author João Vieira 
 * @author José Martins
 * @author Miguel Quaresma
 * @author Simão Barbosa
 */
#include <stdlib.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <unistd.h>
#elif __linux__
#include <unistd.h>
#include <GL/glew.h>
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
#include <libxml/parser.h>
#include <libxml/tree.h>

//linked array, each element store all points of one model
typedef struct modelPoints {
     float *points;
     int size;
     struct modelPoints *next;
}*Points;

float radius = 10.0f;
float alfa = M_PI/4;
float beta = M_PI/4;
float change = 0.025;
float changeR = 0.2;
GLuint buffers[1];
Points *models; 

long readln (int fildes, void * buf, size_t nbyte){
	int i,x;
	char* s = (char *) buf;
	for (i=0; i<nbyte && (x=read(fildes,&s[i],1))>0 && s[i]!='\n';i++);
	if (s[i]=='\n') i++;
	return i;
}

void parseModel(xmlChar * file, Points *models) {

	int fd,x,i=0;
	char buffer[100];
	float coord[3];
	char* aux;
    
    *models = (Points)malloc(sizeof(struct modelPoints));
    (*models)->size = ;
    (*models)->points = realloc((*models)->points,(*models)->size);
    (*models)->next = NULL;
    fd = open(file,O_RDONLY);

    if(fd>0){
        while ((x=readln(fd,buffer,100))>0){

    	    aux = strtok(buffer," ");
	        coord[0] = atof(aux);
		    aux = strtok (NULL, " ");
		    coord[1] = atof(aux);
		    aux = strtok (NULL, " ");
		    coord[2] = atof(aux);

		    (*models)->points[i++]=coord[0];
            (*models)->points[i++]=coord[1];
            (*models)->points[i++]=coord[2];
        }
    }
}

void parseNodes(xmlNodePtr cur, Points *models){ 
    while(cur){
        if(!xmlStrcmp(cur->name,(const xmlChar*)"model")){
            parseModel(xmlGetProp(cur,(const xmlChar*)"file"),models);
            models = &((*models) -> next);
        }
        cur = cur -> next;
    }
}

void xmlParser(char * file){
    xmlDocPtr doc;
    xmlNodePtr cur;

    doc = xmlParseFile(file);
    if(!doc) fprintf(stderr, "Couldn't parse xml file\n");
    else{
        cur = xmlDocGetRootElement(doc);
        if(!cur) fprintf(stderr, "Empty xml file\n");
        else{
            if(!xmlStrcmp(cur->name,(const xmlChar*)"scene")){
                cur = cur -> xmlChildrenNode;
                models = (Points *)malloc(sizeof(Points *));
                parseNodes(cur,models);
            }else fprintf(stderr, "Don't recognize sintax\n");   
        }
    }
    //xmlFreeDoc(doc);
    //xmlFreeNode(cur);
}

void changeSize(int w, int h) {

	if(h == 0) h = 1;

	float ratio = w * 1.0 / h;

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity();
		
	glViewport(0, 0, w, h);

	gluPerspective(45.0f ,ratio, 1.0f ,1000.0f);

	glMatrixMode(GL_MODELVIEW);

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

void drawModels(){
    while(models){
        glBufferData(GL_ARRAY_BUFFER,((*models)->size)*sizeof(float),(*models)->points,GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLES,0,(*models)->size);
        models = &((*models)->next);
    }
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

    drawModels();

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
    if(argc>1){
        xmlParser(argv[1]);
	    
        glutInit(&argc, argv);
	    glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	    glutInitWindowPosition(100,100);
	    glutInitWindowSize(800,800);
	    glutCreateWindow("3DEngine");
		
	    glutDisplayFunc(renderScene);
	    glutReshapeFunc(changeSize);
	
	    glutKeyboardFunc(processKeys);
	    glutSpecialFunc(processSpecialKeys);
        
        #ifndef __APPLE__
            glewInit();
        #endif

	    glEnable(GL_DEPTH_TEST);
	    glEnable(GL_CULL_FACE);
        glEnableClientState(GL_VERTEX_ARRAY);

        glGenBuffers(1,buffers);
        glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);
        glVertexPointer(3,GL_FLOAT,0,0);

	    glutMainLoop();
    }else fprintf(stderr, "Wrong number of arguments.\n");
	
    return 1;
}
