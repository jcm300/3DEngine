/*
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
#endif

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
#include <IL/il.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

//linked array, each element stores all points of one model
typedef struct modelPoints {
     float *points;
     float *normals;
     float *textureC;
     GLuint textureId;
     float *colours;
     int size;
     struct modelPoints *next;
}*Points;

typedef struct light {
    char t;
    float *args;
    struct light *next;
}*Lights;

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
float betafpc=90;
float xLookAt=1.f; 
float yLookAt=1.5f; 
float zLookAt=0.f;
int fpc=0;
GLuint buffers[3];
GLenum mode = GL_FILL;
Points *models; 
Transforms *transforms;
Lights *lights;
int numLights=0;

long readln (int fildes, void * buf, size_t nbyte){
	int i,x;
	char* s = (char *) buf;
	for (i=0; i<nbyte && (x=read(fildes,&s[i],1))>0 && s[i]!='\n';i++);
	if (s[i]=='\n') i++;
	return i;
}

int loadTexture(xmlChar *texture){
    unsigned int t,tw,th;
	unsigned char *texData;
	unsigned int texID;

	ilInit();
	ilEnable(IL_ORIGIN_SET);
	ilGenImages(1,&t);
	ilBindImage(t);
	ilLoadImage((ILstring)texture);
	tw = ilGetInteger(IL_IMAGE_WIDTH);
	th = ilGetInteger(IL_IMAGE_HEIGHT);
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	texData = ilGetData();

	glGenTextures(1,&texID);
	
	glBindTexture(GL_TEXTURE_2D,texID);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
		
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, texData);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}

void getVals(xmlNodePtr cur, float *aux, int i, int *existColour, char *type){
    type[4]='R'; type[5]='\0';
    xmlChar *a1 = xmlGetProp(cur,(const xmlChar*)type);
    type[4]='G'; 
    xmlChar *a2 = xmlGetProp(cur,(const xmlChar*)type);
    type[4]='B';
    xmlChar *a3 = xmlGetProp(cur,(const xmlChar*)type);

    if(a1!=NULL || a2!=NULL || a3!=NULL){
        *existColour=1;
        aux[i]= (a1!=NULL) ? atof(a1) : 0;
        aux[i+1]= (a2!=NULL) ? atof(a2) : 0;
        aux[i+2]= (a3!=NULL) ? atof(a3) : 0;
        aux[i+3]= 1.f;
    }else{
        aux[i]=aux[i+1]=aux[i+2]=aux[i+3]=-1;
    }
}

void getColours(xmlNodePtr cur, float **colours){
    float *aux=(float*)malloc(sizeof(float)*17);
    int existColour = 0, i=0;
    char type[6];

    strcpy(type,"diff");
    getVals(cur,aux,i,&existColour,type);
    i=i+4;
    
    strcpy(type,"spec");
    getVals(cur,aux,i,&existColour,type);
    aux[8] = (xmlGetProp(cur,(const xmlChar*)"shininess")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"shininess")) : 0;
    i=i+5;

    strcpy(type,"emis");
    getVals(cur,aux,i,&existColour,type);
    i=i+4;   

    strcpy(type,"ambi");
    getVals(cur,aux,i,&existColour,type);

    if(existColour==1){
        *colours = aux;
    }else{
        free(aux);
        *colours = NULL;
    }
}

int parseModel(xmlNodePtr cur, xmlChar * file, xmlChar *texture, Points *m, int textureCount) {
	int fd,x,i=0,j=0,w=0,ret=0;
	char buffer[213];
	float coord[3];
	char* aux;
        
    fd = open(file,O_RDONLY);

    if(fd>0){
        x=readln(fd,buffer,213);
        if(x>0){
            Points auxM=(Points)malloc(sizeof(struct modelPoints));
            buffer[x-1]=0;
            auxM->size=atoi(buffer);
            auxM->points = (float*)malloc(sizeof(float)*auxM->size*3);
            auxM->normals = (float*)malloc(sizeof(float)*auxM->size*3);
            if(texture){ 
                auxM->textureC = (float*)malloc(sizeof(float)*auxM->size*2);
                auxM->textureId=loadTexture(texture);
                auxM->colours=NULL;
            }
            else{ 
                auxM->textureC=NULL;
                auxM->textureId=0;
                getColours(cur,&auxM->colours); 
            }
            auxM->next = NULL;
            while(j++<auxM->size*3 && (x=readln(fd,buffer,213))>0){
                //vertex
                aux = strtok(buffer," ");
                coord[0] = atof(aux);
                aux = strtok (NULL, " ");
                coord[1] = atof(aux);
                aux = strtok (NULL, " ");
                coord[2] = atof(aux);
                auxM->points[i]=coord[0];
                auxM->points[i+1]=coord[1];
                auxM->points[i+2]=coord[2];
                //normal vector
                aux = strtok(NULL," ");
                coord[0] = atof(aux);
                aux = strtok (NULL, " ");
                coord[1] = atof(aux);
                aux = strtok (NULL, " ");
                coord[2] = atof(aux);
                auxM->normals[i]=coord[0];
                auxM->normals[i+1]=coord[1];
                auxM->normals[i+2]=coord[2];
                //texture
                if(texture){
                    aux = strtok(NULL," ");
                    coord[0] = atof(aux);
                    aux = strtok (NULL, " ");
                    coord[1] = atof(aux);
                    auxM->textureC[w]=coord[0];
                    auxM->textureC[w+1]=coord[1];
                    w+=2;
                }
                i+=3;
            }
            *m=auxM;
            ret = 1;
        }
    }
    else printf("Couldn't open file %s\n", file);
    return ret;
}

Points *parseModels(xmlNodePtr cur, Points *m, Transforms *t, int textureCount){
    int models=0;
    while(cur){
        if(!xmlStrcmp(cur->name,(const xmlChar*)"model")){
            if(parseModel(cur,xmlGetProp(cur,(const xmlChar*)"file"),xmlGetProp(cur, (const xmlChar*)"texture"),m, textureCount)){
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

void parseLight(xmlNodePtr cur, Lights *l){
    char *aux;
    int i;
    Lights auxL=(Lights)malloc(sizeof(struct light));
    
    if(xmlGetProp(cur,(const xmlChar*)"type")!=NULL){
       aux = (char*)xmlGetProp(cur,(const xmlChar*)"type");
       
       if(strcmp(aux,"POINT")==0){
           auxL->t='p';
           auxL->args =(float*)malloc(sizeof(float)*8);
       }else if(strcmp(aux,"DIRECTIONAL")==0){
           auxL->t='d';
           auxL->args =(float*)malloc(sizeof(float)*8);
       }else if(strcmp(aux,"SPOT")==0){
           auxL->t='s';
           auxL->args =(float*)malloc(sizeof(float)*12);
       }
    }
   
    //lights with position
    if(auxL->t=='p' || auxL->t=='s'){
            auxL->args[0]= (xmlGetProp(cur,(const xmlChar*)"posX")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"posX")) : 0;
            auxL->args[1]= (xmlGetProp(cur,(const xmlChar*)"posY")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"posY")) : 0;
            auxL->args[2]= (xmlGetProp(cur,(const xmlChar*)"posZ")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"posZ")) : 0;
            auxL->args[3]=1.f;
    }

    //all lights have diffuse color
    auxL->args[4]= (xmlGetProp(cur,(const xmlChar*)"diffR")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"diffR")) : 0;
    auxL->args[5]= (xmlGetProp(cur,(const xmlChar*)"diffG")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"diffG")) : 0;
    auxL->args[6]= (xmlGetProp(cur,(const xmlChar*)"diffB")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"diffB")) : 0;
    auxL->args[7]= (xmlGetProp(cur,(const xmlChar*)"diffA")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"diffA")) : 0;
    
    //lights with direction
    if(auxL->t=='d' || auxL->t=='s'){
            if(auxL->t=='s'){
                i=8;
                auxL->args[11]= (xmlGetProp(cur,(const xmlChar*)"cutoff")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"cutoff")) : 0;
            }else{
                i=0;
                auxL->args[3]=0.f;
            }
            
            auxL->args[i]= (xmlGetProp(cur,(const xmlChar*)"dirX")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"dirX")) : 0;
            auxL->args[i+1]= (xmlGetProp(cur,(const xmlChar*)"dirY")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"dirY")) : 0;
            auxL->args[i+2]= (xmlGetProp(cur,(const xmlChar*)"dirZ")!=NULL) ? atof(xmlGetProp(cur,(const xmlChar*)"dirZ")) : 0;
    }

    auxL->next=NULL;
    *l=auxL;
}

Transforms *parseGroup(xmlNodePtr cur, Points *m, Transforms *t, int textureCount){
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
            m = parseModels(cur -> xmlChildrenNode, m, t, textureCount);
            t = &((*t)->next);
        }
        if(!xmlStrcmp(cur->name,(const xmlChar*)"group")){
            t = parseGroup(cur -> xmlChildrenNode, m, t, textureCount);
            t = &((*t)->next);
        }
        cur = cur -> next;
    }
    pop(t);
    return t;
} 

void parseLights(xmlNodePtr cur, Lights *l){
    while(cur){
        if(!xmlStrcmp(cur->name,(const xmlChar*)"light")){
            parseLight(cur,l);
            numLights++;
            l = &((*l)->next);
        }
        cur = cur -> next;   
    }
}

void parseNodes(xmlNodePtr cur, Points *ml, Transforms *tl, Lights *ll){
    Points * m=ml;
    Transforms *t=tl;
    Lights *l=ll;
    int textureCount=0;
    
    while(cur){
        if(!xmlStrcmp(cur->name,(const xmlChar*)"group")){
            t = parseGroup(cur -> xmlChildrenNode, m, t, textureCount);
            t = &((*t)->next);
        }
        if(!xmlStrcmp(cur->name,(const xmlChar*)"lights")){
            parseLights(cur->xmlChildrenNode,l);
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
                lights=(Lights *)malloc(sizeof(void*));
                *lights=NULL;
                parseNodes(cur,models,transforms,lights);
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
    float dX,dY,dZ,dMod,rX,rZ;
    dX=xLookAt-xLocation; 
    dY=yLookAt-yLocation; 
    dZ=zLookAt-zLocation; 

    if(fwd){
        xLocation += 0.1*dX*fwd;
        yLocation += 0.1*dY*fwd;
        zLocation += 0.1*dZ*fwd;
        xLookAt += 0.1*dX*fwd;
        yLookAt += 0.1*dY*fwd;
        zLookAt += 0.1*dZ*fwd;
    }else if(lat){
        rX=-dZ; 
        rZ=dX; 
        xLocation += 0.1*rX*lat;
        zLocation += 0.1*rZ*lat;
        xLookAt += 0.1*rX*lat;
        zLookAt += 0.1*rZ*lat;
    }else if(abs(rot)==1){ //yy rotation
        alfafpc = alfafpc + rot;
        xLookAt = xLocation + sin(alfafpc * 3.14 /180);
        zLookAt = zLocation + cos(alfafpc * 3.14 /180);
    }else if(abs(rot)==2){ //xx or zz rotation
        betafpc = betafpc + rot/2;
        yLookAt = yLocation + cos(betafpc * 3.14 /180);
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

void colorClear(int t){
    float black[4]={0,0,0,0};
    float white[4]={1,1,1,1};
    
    if(t==1){
        glMaterialfv(GL_FRONT, GL_DIFFUSE, black);
        glColor3f(0,0,0);
    }else{
        glMaterialfv(GL_FRONT, GL_DIFFUSE, white);
        glColor3f(1.0,1.0,1.0);
    }
    glMaterialf(GL_FRONT,GL_SHININESS,0);
    glMaterialfv(GL_FRONT, GL_SPECULAR, black);
    glMaterialfv(GL_FRONT, GL_EMISSION, black);
    glMaterialfv(GL_FRONT, GL_AMBIENT, black);
}

void drawModels(int begin, int end){
    Points auxM=*models;
    int i=0;
    
    while(auxM && i<begin) {
        auxM = auxM -> next;
        i++;
    }

    while(auxM && i<end){
        //clean colors but define diffuse as white
        colorClear(0);
        if(auxM->colours && numLights>0){
            if(auxM->colours[0]!=-1){
                float color[4]={auxM->colours[0],auxM->colours[1],auxM->colours[2],auxM->colours[3]};
                glMaterialfv(GL_FRONT, GL_DIFFUSE, color);
            }
            if(auxM->colours[4]!=-1){
                float color[4]={auxM->colours[4],auxM->colours[5],auxM->colours[6],auxM->colours[7]};
                glMaterialf(GL_FRONT,GL_SHININESS,auxM->colours[8]);
                glMaterialfv(GL_FRONT, GL_SPECULAR, color);
            }
            if(auxM->colours[9]!=-1){
                float color[4]={auxM->colours[9],auxM->colours[10],auxM->colours[11],auxM->colours[12]};
                glMaterialfv(GL_FRONT, GL_EMISSION, color);
            }
            if(auxM->colours[13]!=-1){
                float color[4]={auxM->colours[13],auxM->colours[14],auxM->colours[15],auxM->colours[16]};
                glMaterialfv(GL_FRONT, GL_AMBIENT, color);
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);
        glVertexPointer(3,GL_FLOAT,0,0);
        glBufferData(GL_ARRAY_BUFFER,(auxM->size)*3*sizeof(float),auxM->points,GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER,buffers[1]);
        glNormalPointer(GL_FLOAT,0,0);
        glBufferData(GL_ARRAY_BUFFER,(auxM->size)*3*sizeof(float),auxM->normals,GL_STATIC_DRAW);

        if(auxM->textureC){
            glBindBuffer(GL_ARRAY_BUFFER,buffers[2]); 
            glTexCoordPointer(2,GL_FLOAT,0,0);
            glBufferData(GL_ARRAY_BUFFER,auxM->size*2*sizeof(float),auxM->textureC,GL_STATIC_DRAW);
            glBindTexture(GL_TEXTURE_2D, auxM->textureId); 
        }

        glDrawArrays(GL_TRIANGLES,0,auxM->size);
        glBindTexture(GL_TEXTURE_2D, 0);
        auxM = auxM->next;
        i++;
        //clean colors
        colorClear(1);
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
    indices[0] = (index+pointCount-1)%pointCount;   
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
    int init = 0, Npush = 0;
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
                time = glutGet(GLUT_ELAPSED_TIME);
                gt =  fmod(time,auxT->args[0] * 1000) / (auxT->args[0] * 1000);
                renderCatmullRomCurve((long)auxT->args[1],&(auxT->args[2]));
                getGlobalCatmullRomPoint(gt,pos,deriv,(long)auxT->args[1],&(auxT->args[2]));
                normalize(deriv);
                normalize(lY);
                cross(deriv,lY,z);
                normalize(z);
                cross(z,deriv,lY);
                buildRotMatrix(deriv,lY,z,m);
                glTranslatef(pos[0],pos[1],pos[2]);
                glPushMatrix();
                Npush++;
                glMultMatrixf(m);
                break;
            case 'i':
            	time = glutGet(GLUT_ELAPSED_TIME);
                gt =  fmod(time,auxT->args[0] * 1000) / (auxT->args[0] * 1000);
                glRotatef(360 * gt,auxT->args[1],auxT->args[2],auxT->args[3]);
                break;
            case 'm':
                drawModels(init,init+(int)auxT->args[0]);
                init= init + (int)auxT->args[0];
                break;
            case 'o':
                for(int i=0;i<Npush; i++) glPopMatrix();
                Npush=0;
                glPopMatrix();
                break;
            default:
                break;
        }
        auxT= auxT -> next;
    }
}

void drawLights(){
    Lights auxL=*lights;

    for(int i=0;i<8 && i<numLights; i++, auxL=auxL->next){
        if(auxL->t=='p' || auxL->t=='d'){
            glLightfv(GL_LIGHT0+i, GL_POSITION, auxL->args);
        }else if(auxL->t=='s'){
            glLightfv(GL_LIGHT0+i, GL_POSITION, auxL->args);
            glLightfv(GL_LIGHT0+i, GL_SPOT_DIRECTION, auxL->args+8);
            glLightf(GL_LIGHT0+i, GL_SPOT_CUTOFF, auxL->args[11]);
        }
        glLightfv(GL_LIGHT0+i, GL_DIFFUSE, auxL->args+4);
    }
}

void renderScene(void) {
    
    glClearColor(0.0f,0.0f,0.0f,0.0f);
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
	    
    glPolygonMode(GL_FRONT_AND_BACK, mode); //change mode
    
    //draw lights
    drawLights();

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
            if(fpc) updateFstPrsn(0,0,-2);
            else if(beta < 1.5f) beta += change;
			break;
		case GLUT_KEY_DOWN:
            if(fpc) updateFstPrsn(0,0,2);
            else if(beta > -1.5f) beta -= change;
			break;
	}
	glutPostRedisplay();
}

void glEnableLights(){
    //openGl only have 8 lights
    if(numLights>0){
        glEnable(GL_LIGHTING);
        for(int i=0; i<8 && i<numLights; i++){
            glEnable(GL_LIGHT0+i);
        }
    }   
}

int main(int argc, char **argv) {
    if(argc==2){
	    
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
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);        

        glEnable(GL_TEXTURE_2D);

        glGenBuffers(3,buffers);

        xmlParser(argv[1]);
        glEnableLights();

	    glutMainLoop();
    }else fprintf(stderr, "Wrong number of arguments.\n");
	
    return 1;
}
