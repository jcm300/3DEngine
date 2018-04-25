/**
 * engine.c
 * Purpose: Reads an XML file specifying which models to render and the files in which this models have been specified. 
 *
 * @author João Vieira 
 * @author José Martins
 * @author Miguel Quaresma
 * @author Simão Barbosa
 */

#ifdef __linux__
#include <unistd.h>
#include <GL/glew.h>
#include <GL/glut.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
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

/* case t=u:
 *      start of group -> glPushMatrix();
 * case t=o:
 *      end of group -> glPopMatrix();
 * case t=t:
 *      translate with 3 arguments in args(X;Y;Z);
 * case t=r:
 *      rotate with 4 arguments in args(angle;axisX;axisY;axisZ);
 * case t=s:
 *      scale with 3 arguments in args(X;Y;Z);
 * case t=c:
 *		catmull translate with variable args(time;number_of_points;points...);
 * case t=i:
 *		rotate 360º in certain time with args(time;axisX;axisY;axisZ); 
 * case t=m:
 *      models with 1 argument in args(number of models);
 */
typedef struct transforms{
    char t;
    float *args;
    struct transforms *next;
}*Transforms;

float radius = 10.0f;
float alfa = M_PI/4;
float beta = M_PI/4;
float change = 0.025;
float changeR = 0.2;
float xLocation=0.f; 
float yLocation=1.5f; 
float zLocation=0.f; 
float alfafpc=90;
float xLookAt=1.f; 
float yLookAt=1.5f; 
float zLookAt=0.f;
int fpc=0;
GLuint buffers[1];
GLenum mode = GL_FILL;
Points *models; 
Transforms *transforms;

long readln (int fildes, void * buf, size_t nbyte){
	int i,x;
	char* s = (char *) buf;
	for (i=0; i<nbyte && (x=read(fildes,&s[i],1))>0 && s[i]!='\n';i++);
	if (s[i]=='\n') i++;
	return i;
}

int parseModel(xmlChar * file, Points *m) {

	int fd,x,i=0,j=0,ret=0;
	char buffer[100];
	float coord[3];
	char* aux;
        
    fd = open(file,O_RDONLY);

    if(fd>0){
        x=readln(fd,buffer,100);
        if(x>0){
            Points auxM=(Points)malloc(sizeof(struct modelPoints));
            buffer[x-1]=0;
            auxM->size=atoi(buffer)*3;
            auxM->points = (float*)malloc(sizeof(float)*auxM->size);
            auxM->next = NULL;
            while(j++<auxM->size && (x=readln(fd,buffer,100))>0){
                aux = strtok(buffer," ");
                coord[0] = atof(aux);
                aux = strtok (NULL, " ");
                coord[1] = atof(aux);
                aux = strtok (NULL, " ");
                coord[2] = atof(aux);
                auxM->points[i++]=coord[0];
                auxM->points[i++]=coord[1];
                auxM->points[i++]=coord[2];
            }
            *m=auxM;
            ret = 1;
        }
    }
    else printf("Couldn't open file %s\n", file);
    return ret;
}

Points *parseModels(xmlNodePtr cur, Points *m, Transforms *t){
    int models=0;
    while(cur){
        if(!xmlStrcmp(cur->name,(const xmlChar*)"model")){
            if(parseModel(xmlGetProp(cur,(const xmlChar*)"file"),m)){
                m = &((*m)->next);
                models++;
            }
            else exit(1);
        }
        cur = cur -> next;
    }
    Transforms auxT=(Transforms)malloc(sizeof(struct transforms));
    auxT->t = 'm';
    auxT->args =(float*)malloc(sizeof(float)*1);
    auxT->args[0]=models;
    auxT->next=NULL;
    *t=auxT;
    return m;
}

void push(Transforms *t){
    Transforms auxT=(Transforms)malloc(sizeof(struct transforms));
    auxT->t = 'u';
    auxT->args=NULL;
    auxT->next=NULL;
    *t=auxT;
}

void translate(xmlNodePtr cur, Transforms *t){
    Transforms auxT=(Transforms)malloc(sizeof(struct transforms));
    auxT->t = 't';
    auxT->args =(float*)malloc(sizeof(float)*3);
    auxT->args[0]= (xmlGetProp(cur,(const xmlChar*)"X")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"X")) : 0;
    auxT->args[1]= (xmlGetProp(cur,(const xmlChar*)"Y")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"Y")) : 0;
    auxT->args[2]= (xmlGetProp(cur,(const xmlChar*)"Z")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"Z")) : 0;
    auxT->next=NULL;
    *t=auxT;
}

void catmullTranslate(xmlNodePtr cur, Transforms *t){
    Transforms auxT=(Transforms)malloc(sizeof(struct transforms));
    auxT->t = 'c';
    int nChildren = xmlChildElementCount(cur);
    printf(" ");
    auxT->args =(float*)malloc(sizeof(float)*(3*nChildren+2));
    auxT->args[0] = atof(xmlGetProp(cur,(const xmlChar*)"time"));
    auxT->args[1] = nChildren;
    int i = 2;
    cur = cur -> xmlChildrenNode;
    while(cur){
        if(!xmlStrcmp(cur->name,(const xmlChar*)"point")){
            auxT->args[i++]= (xmlGetProp(cur,(const xmlChar*)"X")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"X")) : 0;
            auxT->args[i++]= (xmlGetProp(cur,(const xmlChar*)"Y")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"Y")) : 0;
            auxT->args[i++]= (xmlGetProp(cur,(const xmlChar*)"Z")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"Z")) : 0;
        }
        cur = cur -> next;
    }
    auxT->next=NULL;
    *t=auxT;
}

void rotate(xmlNodePtr cur, Transforms *t){
    Transforms auxT=(Transforms)malloc(sizeof(struct transforms));
    auxT->t = 'r';
    auxT->args =(float*)malloc(sizeof(float)*4);
    auxT->args[0]= (xmlGetProp(cur,(const xmlChar*)"angle")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"angle")) : 0;
    auxT->args[1]= (xmlGetProp(cur,(const xmlChar*)"axisX")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"axisX")) : 0;
    auxT->args[2]= (xmlGetProp(cur,(const xmlChar*)"axisY")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"axisY")) : 0;
    auxT->args[3]= (xmlGetProp(cur,(const xmlChar*)"axisZ")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"axisZ")) : 0;
    auxT->next=NULL;
    *t=auxT;
}

void timeRotate(xmlNodePtr cur, Transforms *t){
	Transforms auxT=(Transforms)malloc(sizeof(struct transforms));
	auxT->t = 'i';
    auxT->args =(float*)malloc(sizeof(float)*4);
    auxT->args[0]= atof(xmlGetProp(cur,(const xmlChar*)"time"));
    auxT->args[1]= (xmlGetProp(cur,(const xmlChar*)"axisX")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"axisX")) : 0;
    auxT->args[2]= (xmlGetProp(cur,(const xmlChar*)"axisY")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"axisY")) : 0;
    auxT->args[3]= (xmlGetProp(cur,(const xmlChar*)"axisZ")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"axisZ")) : 0;
    auxT->next=NULL;
    *t=auxT;
}

void scale(xmlNodePtr cur, Transforms *t){
    Transforms auxT=(Transforms)malloc(sizeof(struct transforms));
    auxT->t = 's';
    auxT->args =(float*)malloc(sizeof(float)*3);
    auxT->args[0]= (xmlGetProp(cur,(const xmlChar*)"X")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"X")) : 0;
    auxT->args[1]= (xmlGetProp(cur,(const xmlChar*)"Y")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"Y")) : 0;
    auxT->args[2]= (xmlGetProp(cur,(const xmlChar*)"Z")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"Z")) : 0;
    auxT->next=NULL;
    *t=auxT;
}

void pop(Transforms *t){
    Transforms auxT=(Transforms)malloc(sizeof(struct transforms));
    auxT->t = 'o';
    auxT->args=NULL;
    auxT->next=NULL;
    *t=auxT;
}

Transforms *parseGroup(xmlNodePtr cur, Points *m, Transforms *t){
    push(t);
    t = &((*t)->next);
    while(cur){
        if(!xmlStrcmp(cur->name,(const xmlChar*)"translate")){
            if (xmlGetProp(cur,(const xmlChar*)"time")!=NULL) catmullTranslate(cur,t);
            else translate(cur,t);
            t = &((*t)->next);
        }
        if(!xmlStrcmp(cur->name,(const xmlChar*)"rotate")){
            if (xmlGetProp(cur,(const xmlChar*)"time")!=NULL) timeRotate(cur,t);
            else rotate(cur,t);
            t = &((*t)->next);
        }
        if(!xmlStrcmp(cur->name,(const xmlChar*)"scale")){
            scale(cur,t);
            t = &((*t)->next);
        }
        if(!xmlStrcmp(cur->name,(const xmlChar*)"models")){
            while(*m) m = &((*m)->next);
            m = parseModels(cur -> xmlChildrenNode, m, t);
            t = &((*t)->next);
        }
        if(!xmlStrcmp(cur->name,(const xmlChar*)"group")){
            t = parseGroup(cur -> xmlChildrenNode, m, t);
            t = &((*t)->next);
        }
        cur = cur -> next;
    }
    pop(t);
    return t;
} 

void parseNodes(xmlNodePtr cur, Points *ml, Transforms *tl){
    Points * m=ml;
    Transforms *t=tl; 
    
    while(cur){
        if(!xmlStrcmp(cur->name,(const xmlChar*)"group")){
            t = parseGroup(cur -> xmlChildrenNode, m, t);
            t = &((*t)->next);
        }
        cur = cur -> next;
    }
}

void xmlParser(char * file){
    xmlDocPtr doc;
    xmlNodePtr cur;

    doc = xmlParseFile(file);
    if(!doc){
        fprintf(stderr, "Couldn't parse xml file\n");
        exit(1);
    }else{
        cur = xmlDocGetRootElement(doc);
        if(!cur){
            fprintf(stderr, "Empty xml file\n");
            exit(1);
        }else{
            if(!xmlStrcmp(cur->name,(const xmlChar*)"scene")){
                cur = cur -> xmlChildrenNode;
                models=(Points *)malloc(sizeof(void*));
                *models=NULL;
                transforms=(Transforms *)malloc(sizeof(void*));
                *transforms=NULL;
                parseNodes(cur,models,transforms);
            }else{
                fprintf(stderr, "Don't recognize sintax\n");
                exit(1);
            }   
        }
    }
    //xmlFreeDoc(doc);
    //xmlFreeNode(cur);
}

/**
 * Updates first person camera current position
 * fwd: indicates whether the movement is forward or backwards
 * lat: indicates whether the movement is lateral
 * rot: indicates whether the movement is rotational
**/
void updateFstPrsn(int fwd, int lat,int rot){
    float dX,dZ,dMod,rX,rZ;
    dX=xLookAt-xLocation; 
    dZ=zLookAt-zLocation; 

    if(fwd){
        xLocation += 0.1*dX*fwd;
        zLocation += 0.1*dZ*fwd;
        xLookAt += 0.1*dX*fwd;
        zLookAt += 0.1*dZ*fwd;
    }else if(lat){
        rX=-dZ; 
        rZ=dX; 
        xLocation += 0.1*rX*lat;
        zLocation += 0.1*rZ*lat;
        xLookAt += 0.1*rX*lat;
        zLookAt += 0.1*rZ*lat;
    }else if(rot){
        alfafpc = alfafpc + rot;
        xLookAt = xLocation + sin(alfafpc * 3.14 /180);
        zLookAt = zLocation + cos(alfafpc * 3.14 /180);
    }
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

void drawModels(int begin, int end){
    Points auxM=*models;
    int i=0;

    glColor3f(1.0,1.0,1.0); //white color

    glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);
    glVertexPointer(3,GL_FLOAT,0,0);
    
    while(auxM && i<begin) {
        auxM = auxM -> next;
        i++;
    }

    while(auxM && i<end){
        glBufferData(GL_ARRAY_BUFFER,(auxM->size)*sizeof(float),auxM->points,GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLES,0,auxM->size);
        auxM = auxM->next;
        i++;
    }
}

void buildRotMatrix(float *x, float *y, float *z, float *m) {

    m[0] = x[0]; m[1] = x[1]; m[2] = x[2]; m[3] = 0;
    m[4] = y[0]; m[5] = y[1]; m[6] = y[2]; m[7] = 0;
    m[8] = z[0]; m[9] = z[1]; m[10] = z[2]; m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}

void cross(float *a, float *b, float *res) {

    res[0] = a[1]*b[2] - a[2]*b[1];
    res[1] = a[2]*b[0] - a[0]*b[2];
    res[2] = a[0]*b[1] - a[1]*b[0];
}

void normalize(float *a) {

    float l = sqrt(a[0]*a[0] + a[1] * a[1] + a[2] * a[2]);
    a[0] = a[0]/l;
    a[1] = a[1]/l;
    a[2] = a[2]/l;
}

void getCatmullRomPoint(float t, float *p0, float *p1, float *p2, float *p3, float *pos, float *deriv) {

    // catmull-rom matrix
    float m[4][4] = {   {-0.5f,  1.5f, -1.5f,  0.5f},
                        { 1.0f, -2.5f,  2.0f, -0.5f},
                        {-0.5f,  0.0f,  0.5f,  0.0f},
                        { 0.0f,  1.0f,  0.0f,  0.0f}};
    // Compute A = M * P
    float a[4][3];

    for(int i=0;i<4;i++){
        for(int j=0;j<3;j++)
            a[i][j]=m[i][0]*p0[j]+m[i][1]*p1[j]+m[i][2]*p2[j]+m[i][3]*p3[j];  //p0j+p1j+p2j+p3j
    }
    
    // Compute pos = T * A
    for(int i=0;i<3;i++)
        pos[i]=powf(t,3)*a[0][i]+powf(t,2)*a[1][i]+t*a[2][i]+a[3][i];
        
    // compute deriv = T' * A
    for(int i=0;i<3;i++)
        deriv[i]=3*powf(t,2)*a[0][i]+2*t*a[1][i]+a[2][i];
}


// given  global t, returns the point in the curve
void getGlobalCatmullRomPoint(float gt, float *pos, float *deriv, long pointCount, float *points) {
    float t = gt * pointCount; // this is the real global t
    int index = floor(t);  // which segment
    t = t - index; // where within  the segment

    // indices store the points
    int indices[4]; 
    indices[0] = (index + pointCount-1)%pointCount;   
    indices[1] = (indices[0]+1)%pointCount;
    indices[2] = (indices[1]+1)%pointCount; 
    indices[3] = (indices[2]+1)%pointCount;

    getCatmullRomPoint(t, points+indices[0]*3, points+indices[1]*3, points+indices[2]*3, points+indices[3]*3, pos, deriv);
}

void renderCatmullRomCurve(long pointCount, float *points) {
    float pos[3],deriv[3];

    // desenhar a curva usando segmentos de reta - GL_LINE_LOOP
    glBegin(GL_LINE_LOOP);
        for(float gt=0.f;gt<1.f;gt+=0.0001f){
            getGlobalCatmullRomPoint(gt,pos,deriv,pointCount,points);
            glVertex3f(pos[0],pos[1],pos[2]);
        }
    glEnd();
}


void draw(){
    float deriv[3],pos[3],z[3],m[16];
    static float lY[3]={0.f,1.f,0.f};
    Transforms auxT=*transforms;
    int init = 0;
    float gt,time;

    while(auxT){
        switch(auxT->t){
            case 'u':
                glPushMatrix();
                break;
            case 't':
                glTranslatef(auxT->args[0],auxT->args[1],auxT->args[2]);
                break;
            case 'r':
                glRotatef(auxT->args[0],auxT->args[1],auxT->args[2],auxT->args[3]);
                break;
            case 's':
                glScalef(auxT->args[0],auxT->args[1],auxT->args[2]);
                break;
            case 'c':
                time = glutGet(GLUT_ELAPSED_TIME) / 1000;
                gt =  fmod(time,auxT->args[0]) / auxT->args[0];
                renderCatmullRomCurve((long)auxT->args[1],&(auxT->args[2]));
                getGlobalCatmullRomPoint(gt,pos,deriv,(long)auxT->args[1],&(auxT->args[2]));
                normalize(deriv);
                normalize(lY);
                normalize(z);
                cross(deriv,lY,z);
                cross(z,deriv,lY);
                buildRotMatrix(deriv,lY,z,m);
                glTranslatef(pos[0],pos[1],pos[2]);
                glPushMatrix();
                glMultMatrixf(m);
                break;
            case 'i':
            	time = glutGet(GLUT_ELAPSED_TIME) / 1000;
                gt =  fmod(time,auxT->args[0]) / auxT->args[0];
                glRotatef(360 * gt,auxT->args[1] * gt,auxT->args[2] * gt,auxT->args[3] * gt);
                break;
            case 'm':
                drawModels(init,init+(int)auxT->args[0]);
                init= init + (int)auxT->args[0];
                break;
            case 'o':
                glPopMatrix();
                break;
            default:
                break;
        }
        auxT= auxT -> next;
    }
}

void renderScene(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Camera
	glLoadIdentity();
    if(fpc){
        gluLookAt(xLocation, yLocation, zLocation, 
            xLookAt,yLookAt,zLookAt,
            0.0f,1.0f,0.0f);
    }else{
        gluLookAt(radius * cos(beta) * sin(alfa), radius * sin(beta), radius * cos(beta) * cos(alfa), 
            0.0,0.0,0.0,
            0.0f,1.0f,0.0f);
    }
	    
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glPolygonMode(GL_FRONT_AND_BACK, mode); //change mode

	// drawing instructions
	drawXYZ();

    draw();

	// End of Frame
	glutSwapBuffers();
}


void processKeys(unsigned char c, int xx, int yy) {

	switch(c)
	{
		case 'w': //forward
            if(fpc) updateFstPrsn(1,0,0);
            else radius -= changeR;
			break;
		case 's': //backward
            if(fpc) updateFstPrsn(-1,0,0);
            else radius += changeR;
			break;
        case 'd': //rightward
            if(fpc) updateFstPrsn(0,1,0);
            
            break;
        case 'a': //leftward
            if(fpc) updateFstPrsn(0,-1,0);
            break;
        case 'm':
            if(mode==GL_FILL) mode = GL_LINE;
            else if(mode==GL_LINE) mode = GL_POINT;
            else if(mode==GL_POINT) mode = GL_FILL;
	        break;
        case 't':   //toggle first person camera
            fpc=!fpc;
            break;
    }
	glutPostRedisplay();
}


void processSpecialKeys(int key, int xx, int yy) {
	switch(key)
	{
		case GLUT_KEY_LEFT:
            if(fpc) updateFstPrsn(0,0,1);
            else alfa -= change;
			break;
		case GLUT_KEY_RIGHT:
            if(fpc) updateFstPrsn(0,0,-1);
            else alfa += change;
			break;
		case GLUT_KEY_UP:
			if (!fpc && beta < 1.5f) beta += change;
			break;
		case GLUT_KEY_DOWN:
			if (!fpc && beta > -1.5f) beta -= change;
			break;
	}
	glutPostRedisplay();
}


int main(int argc, char **argv) {
    if(argc==2){
        xmlParser(argv[1]);
	    
        glutInit(&argc, argv);
	    glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	    glutInitWindowPosition(100,100);
	    glutInitWindowSize(800,800);
	    glutCreateWindow("3DEngine");
		
	    glutDisplayFunc(renderScene);
        glutIdleFunc(renderScene);
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

	    glutMainLoop();
    }else fprintf(stderr, "Wrong number of arguments.\n");
	
    return 1;
}
