/**
 * generator.c
 * Purpose: Generates a config file with defining the given geometrical primitives, passed as parameter.
 *
 * @author João Vieira 
 * @author José Martins
 * @author Miguel Quaresma
 * @author Simão Barbosa
 */

#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h> 
#include<sys/fcntl.h>
#include<sys/unistd.h>
#include<unistd.h>

int writeConfig(int, char**);
void generatePlane(int, char*,char*);
void generateBox(int, char*, char*, char*, char*);
void generateCone(int, char*, char*, char*, char*);
void generateSphere(int, char *, char *, char *);
void generateBezierPatch(int, char *, char *);
void printLine(int, char*, float[], float[], float[]);
void genSlice(int, int,float,float, float, float,int);
void genWalls(int,int,float,float,float,float,float,float,int,float);
void normalize(float[],float);
void crossProduct(float *, float *, float *);

int main(int argc, char *argv[]){
    if(argc<3){
        fprintf(stderr,"Invalid no. of arguments\n");
    }else{
        writeConfig(argc-1,++argv);
    }
}

int writeConfig(int nParams, char **params){
    char *fileName=params[nParams-1];
    int fd=open(fileName,O_CREAT | O_WRONLY,0777);
    if(fd!=-1){
        if (strcmp(params[0],"plane") == 0){ //plane
            if (nParams==4) generatePlane(fd,params[1],params[2]);
            else fprintf(stderr,"Incorrect plane parameters\n");
        }
        else if (strcmp(params[0],"box") == 0){ //box
         	if (nParams==6){
                generateBox(fd,params[1],params[2],params[3],params[4]);
            }else if (nParams==5){
                generateBox(fd,params[1],params[2],params[3],"1"); //without stacks
			}else fprintf(stderr,"Incorrect box parameters\n");
        }
        else if (strcmp(params[0],"sphere") == 0){ //sphere
            if (nParams==5) generateSphere(fd,params[1],params[2],params[3]);
            else fprintf(stderr,"Incorrect sphere parameters\n");
        }
        else if (strcmp(params[0],"cone") == 0){ //cone
            if (nParams==6) generateCone(fd,params[1],params[2],params[3],params[4]);
            else fprintf(stderr,"Incorrect cone parameters\n");
        }else if(strcmp(params[0],"bezierPatch") == 0){ //bezierPatch
            if (nParams==4) generateBezierPatch(fd,params[1],params[2]);
            else fprintf(stderr,"Incorrect bezier patch parameters\n");
        }
        else fprintf(stderr,"Primitive not recognized\n");
        close(fd);
    }
    return 1;
}

void generatePlane(int fd, char *l, char *c){
    float lf=atof(l),cf=atof(c);
    float pos[3], normal[3], texture[2];
    char arr[213];
    lf/=2.0f; 
    cf/=2.0f;

    write(fd,"6\n",2);
    
    normal[0]=0.f;
    normal[1]=1.f;
    normal[2]=0.f;

    pos[0]=-lf; pos[1]=0.f; pos[2]=-cf;
    texture[0]=0.f; texture[1]=1.f;
    printLine(fd,arr,pos,normal,texture);

    pos[0]=-lf; pos[1]=0.f; pos[2]=cf;
    texture[0]=0.f; texture[1]=0.f;
    printLine(fd,arr,pos,normal,texture);

    pos[0]=lf; pos[1]=0.f; pos[2]=cf;
    texture[0]=1.f; texture[1]=0.f;
    printLine(fd,arr,pos,normal,texture);
    
    pos[0]=lf; pos[1]=0.f; pos[2]=-cf;
    texture[0]=1.f; texture[1]=1.f;
    printLine(fd,arr,pos,normal,texture);

    pos[0]=-lf; pos[1]=0.f; pos[2]=-cf;
    texture[0]=0.f; texture[1]=1.f;
    printLine(fd,arr,pos,normal,texture);

    pos[0]=lf; pos[1]=0.f; pos[2]=cf;
    texture[0]=1.f; texture[1]=0.f;
    printLine(fd,arr,pos,normal,texture);
}

void printLine(int fd, char* array, float pos[], float normal[], float texture[]){
    sprintf(array, "%f %f %f %f %f %f %f %f\n", pos[0], pos[1], pos[2], normal[0], normal[1], normal[2], texture[0], texture[1]);
    write(fd,array,strlen(array));
}

void faceXZ(int fd, char* array, float x1, float x2, float y, float z, float xLength){
	float pos[3], normal[3], texture[2];
   
    normal[0]=0.f;
    if(y>0) normal[1]=1.f;
    else normal[1]=-1.f;
    normal[2]=0.f;

    pos[0]=x1; pos[1]=y; pos[2]=-z;
    texture[0]=x1/xLength;texture[1]=0.f;
    printLine(fd,array,pos,normal,texture);

    pos[0]=x2; pos[1]=y; pos[2]=z;
    texture[0]=x2/xLength;texture[1]=1.f;
    printLine(fd,array,pos,normal,texture);

    if (y>0){
        pos[0]=x2; pos[1]=y; pos[2]=-z;
        texture[0]=x2/xLength;texture[1]=0.f;
        printLine(fd,array,pos,normal,texture);
    }else{
        pos[0]=x1; pos[1]=y; pos[2]=z;
        texture[0]=x1/xLength;texture[1]=1.f;
        printLine(fd,array,pos,normal,texture); 
    }

    pos[0]=x2; pos[1]=y; pos[2]=z;
    texture[0]=x2/xLength;texture[1]=1.f;
    printLine(fd,array,pos,normal,texture);

    pos[0]=x1; pos[1]=y; pos[2]=-z;
    texture[0]=x1/xLength;texture[1]=0.f;
    printLine(fd,array,pos,normal,texture);

    if (y>0){
        pos[0]=x1; pos[1]=y; pos[2]=z;
        texture[0]=x1/xLength;texture[1]=1.f;
        printLine(fd,array,pos,normal,texture);
    }else{
        pos[0]=x2; pos[1]=y; pos[2]=-z;
        texture[0]=x2/xLength;texture[1]=0.f;
        printLine(fd,array,pos,normal,texture);
    }
}

void faceYZ(int fd, char* array, float x, float y1, float y2, float z, float yLength){
    float pos[3], normal[3], texture[2];
   
    if(x>0) normal[0]=1.f;
    else normal[0]=-1.f;
    normal[1]=0.f;
    normal[2]=0.f;
    
    pos[0]=x; pos[1]=y2; pos[2]=z;
    texture[0]=y2/yLength;texture[1]=1.f;
    printLine(fd,array,pos,normal,texture);

    pos[0]=x; pos[1]=y1; pos[2]=-z;
    texture[0]=y1/yLength;texture[1]=0.f;
    printLine(fd,array,pos,normal,texture);

    if (x>0){
        pos[0]=x; pos[1]=y2; pos[2]=-z;
        texture[0]=y2/yLength;texture[1]=0.f;
        printLine(fd,array,pos,normal,texture);
    }else{
        pos[0]=x; pos[1]=y1; pos[2]=z;
        texture[0]=y1/yLength;texture[1]=1.f;
        printLine(fd,array,pos,normal,texture);
    }

    pos[0]=x; pos[1]=y1; pos[2]=-z;
    texture[0]=y1/yLength;texture[1]=0.f;
    printLine(fd,array,pos,normal,texture);

    pos[0]=x; pos[1]=y2; pos[2]=z;
    texture[0]=y2/yLength;texture[1]=1.f;
    printLine(fd,array,pos,normal,texture);
    if (x>0){
        pos[0]=x; pos[1]=y1; pos[2]=z;
        texture[0]=y1/yLength;texture[1]=1.f;
        printLine(fd,array,pos,normal,texture);
    }else{
        pos[0]=x; pos[1]=y2; pos[2]=-z;
        texture[0]=y2/yLength;texture[1]=0.f;
        printLine(fd,array,pos,normal,texture);
    }
}

void faceXY(int fd, char* array, float x, float y1, float y2, float z, float yLength){
	float pos[3], normal[3], texture[2];
    
    normal[0]=0.f;
    normal[1]=0.f;
    if(z>0) normal[2]=1.f;
    else normal[2]=-1.f;
    
    pos[0]=-x; pos[1]=y2; pos[2]=z;
    texture[0]=y2/yLength;texture[1]=0.f;
    printLine(fd,array,pos,normal,texture);

    pos[0]=x; pos[1]=y1; pos[2]=z;
    texture[0]=y1/yLength;texture[1]=1.f;
    printLine(fd,array,pos,normal,texture);

    if (z>0){
        pos[0]=x; pos[1]=y2; pos[2]=z;
        texture[0]=y2/yLength;texture[1]=1.f;
        printLine(fd,array,pos,normal,texture);
    }else{
        pos[0]=-x; pos[1]=y1; pos[2]=z;
        texture[0]=y1/yLength;texture[1]=0.f;
        printLine(fd,array,pos,normal,texture);
    }

    pos[0]=x; pos[1]=y1; pos[2]=z;
    texture[0]=y1/yLength;texture[1]=1.f;
    printLine(fd,array,pos,normal,texture);

    pos[0]=-x; pos[1]=y2; pos[2]=z;
    texture[0]=y2/yLength;texture[1]=0.f;
    printLine(fd,array,pos,normal,texture);

    if (z>0){
        pos[0]=-x; pos[1]=y1; pos[2]=z;
        texture[0]=y1/yLength;texture[1]=0.f;
        printLine(fd,array,pos,normal,texture);
    }else{
        pos[0]=x; pos[1]=y2; pos[2]=z;
        texture[0]=y2/yLength;texture[1]=1.f;
        printLine(fd,array,pos,normal,texture);
    }
}

void generateBox(int fd, char *xx, char *yy, char *zz, char *dd){
    float x = (float) atof(xx);
    float y = (float) atof(yy);
    float z = (float) atof(zz);
    int d = atoi(dd);
    float var_y = y/d;
    float var_x = x/d;
    float yLength=y, xLength=x;
    x/=2.0f;
    y/=2.0f;
    z/=2.0f;
    char array[213];
    float y1 = -y;
    float y2 = y1 + var_y;
    float x1 = -x;
    float x2 = x1 + var_x;
    int i;

    sprintf(array,"%d\n",(2 * d + 4 * d) * 6);
    write(fd,array,strlen(array));

	for(i=0;i<d;i++,y1+=var_y,y2+=var_y,x1+=var_x,x2+=var_x){
		faceXZ(fd,array,x1,x2,-y,z,xLength);
		faceXZ(fd,array,x1,x2,y,z,xLength);
        
        faceYZ(fd,array,-x,y1,y2,z,yLength);
        faceYZ(fd,array,x,y1,y2,z,yLength);
        
        faceXY(fd,array,x,y1,y2,z,yLength);
		faceXY(fd,array,x,y1,y2,-z,yLength);
    }
}

void generateSphere(int fd, char *rds, char *slc, char *stks){
    int slices=atoi(slc), stacks=atoi(stks)/2,i;
    float radius=atof(rds),curRadius=radius,curHeight=0.f;
    float step=radius/stacks;
   	float angle = (2*M_PI)/slices;
    float sliceSide=2*curRadius*sin(angle/2);
    char array[213];
    sprintf(array,"%d\n", 6*slices*(2*stacks-1));
    write(fd,array,strlen(array));

    for(i=0;i<stacks-1;i++){
        genWalls(fd,slices,radius,curRadius,curHeight,angle,step,-1.f,0,0);
        curRadius-=step;
        curHeight+=step;
    }

    genWalls(fd,slices,radius,curRadius,curHeight,angle,step,-1.f,1,0);


    curHeight=-step;
    curRadius=radius-step;
    for(i=0;i<stacks-1;i++){
        genWalls(fd,slices,radius,curRadius,curHeight,angle,step,0.f,0,0);
        curRadius-=step;
        curHeight-=step;
    }

    genWalls(fd,slices,radius,curRadius,curHeight,angle,step,0.f,-1,0);
}

void generateCone(int fd, char *radiuss , char *heights, char *slicess, char *stackss){
    float radius = (float) atof(radiuss);
    float height = (float) atof(heights);
    float slices = (float) atof(slicess);
    float stacks = (float) atof(stackss);
    int numVert =  6*slices*stacks,i;
    char array[213];
    float curAngle;
    float deltaAngle = (2*M_PI)/slices;
    float curHeight;
    float deltaH =height/stacks;
    float curRadius = radius;
    float deltaR = radius/stacks;
    float pos[3], normal[3], texture[2];

    sprintf(array, "%d\n", numVert);
    write(fd,array,strlen(array));

    for(i=0,curHeight=0.f;i<stacks-1;i++,curHeight+=deltaH, curRadius-=deltaR){
        genWalls(fd,slices,radius,curRadius,curHeight,deltaAngle,deltaR,deltaH,0,height);
    }

    genWalls(fd,slices,radius,curRadius,curHeight,deltaAngle,deltaR,deltaH,1,height);

    //base
    normal[0]=0.f;
    normal[1]=-1.f;
    normal[2]=0.f;
    for(i=0, curAngle=0.f;i<slices;i++,curAngle+=deltaAngle){
        pos[0]=0.f; pos[1]=0.f; pos[2]=0.f;
        texture[0]=curAngle/(2*M_PI); texture[1]=0.f;
        printLine(fd,array,pos,normal,texture);

        pos[0]=radius*sin(curAngle+deltaAngle); pos[1]=0.f; pos[2]=radius*cos(curAngle+deltaAngle);
        texture[0]=(curAngle+deltaAngle)/(2*M_PI); texture[1]=1.f;
        printLine(fd,array,pos,normal,texture);

        pos[0]=radius*sin(curAngle); pos[1]=0.f; pos[2]=radius*cos(curAngle);
        texture[0]=curAngle/(2*M_PI); texture[1]=1.f;
        printLine(fd,array,pos,normal,texture);
    }
}

void sectionWall(int fd, float radius, float pRadius, float y, float angle, float pAngle, float top, float height){
    char array[213];
    float curP[3], normal[3], texture[2], vector[3], vector2[3]; 
    
    curP[0]=pRadius*sin(pAngle+angle);
    curP[1]=y;
    curP[2]=pRadius*cos(pAngle+angle);
    if(top<=0.f){
        normalize(curP,radius);
        normal[0]=curP[0];
        normal[1]=curP[1];
        normal[2]=curP[2];
        texture[0]=(pAngle+angle)/(2*M_PI);
        texture[1]=0.5f+0.5f*y/radius;
    }else{
        vector[0]=radius*sin(pAngle+angle);
        vector[1]=-height;
        vector[2]=radius*cos(pAngle+angle);
        vector2[0]=pRadius*sin(pAngle+angle+M_PI/2);
        vector2[1]=0;
        vector2[2]=pRadius*cos(pAngle+angle+M_PI/2);
        texture[0]=(angle+pAngle)/(2*M_PI);
        texture[1]=y/height;
        
        crossProduct(vector,vector2,normal);
        normalize(normal,1);
    }
    printLine(fd,array,curP,normal,texture);
}

void genWalls(int fd, int sliceCount, float radius, float curRadius, float y, float angle, float delta, float top,int tpbt, float height){
    int i;
    float curAngle;
    float nextStackY=delta+y, nextRadius=curRadius-delta;
   
    if(top==0.f) nextRadius=curRadius+delta;
    else if(top>0.f) nextStackY=y+top;

    for(i=0,curAngle=0.f;i<sliceCount;i++,curAngle+=angle){
        if(tpbt>=0){
            sectionWall(fd,radius,curRadius,y,angle,curAngle,top,height);
            sectionWall(fd,radius,nextRadius,nextStackY,0,curAngle,top,height);
            sectionWall(fd,radius,curRadius,y,0,curAngle,top,height);
        }
        if(tpbt<=0){
            sectionWall(fd,radius,nextRadius,nextStackY,angle,curAngle,top,height);
            sectionWall(fd,radius,nextRadius,nextStackY,0,curAngle,top,height);
            sectionWall(fd,radius,curRadius,y,angle,curAngle,top,height);
        }
    }
}


void normalize(float cur[], float radius){
    float dist=sqrt(cur[0]*cur[0]+cur[1]*cur[1]+cur[2]*cur[2]);
    
    cur[0]=(cur[0]*radius)/dist;
    cur[1]=(cur[1]*radius)/dist;
    cur[2]=(cur[2]*radius)/dist;
}

//get number of Patches/Control Points
int getNumber(int fd){
    int i=0;
    char line[20], buf;

    while(read(fd, &buf, 1) && buf!='\n'){
      line[i++]=buf;
    }
    line[i]='\0';
    return atoi(line);
}

//get the indices of the patches and save in patches
void getPatches(int fd, int *patches, int numPatches){
    char buf;
    char line[20]; 
    int i, w;       
    
    for(int j=0; j<numPatches; j++){
        i=0;
        w=0;

        while(read(fd, &buf, 1) && buf!='\n'){
            if(buf==','){
                line[i]='\0';
                i=0;
                patches[j*16+w++]=atoi(line);
            }
            else if(buf!=' ') line[i++]=buf;
        }
        line[i]='\0';
        patches[j*16+w]=atoi(line);
    }
}

//get control points and save in controlPoints
void getControlPoints(int fd, float *controlPoints, int numControlPoints){ 
    char buf;
    char line[20];
    int i, w;

    for(int j=0; j<numControlPoints; j++){
        i=0;
        w=0;

        while(read(fd, &buf, 1) && buf!='\n'){
            if(buf==','){
                line[i]='\0';
                i=0;
                controlPoints[j*3+w++]=atof(line);
            }
            else if(buf!=' ') line[i++]=buf;
        }
        line[i]='\0';
        controlPoints[j*3+w]=atof(line);
    }
}

//produto externo
void crossProduct(float *a, float *b, float *res) {
	res[0] = a[1]*b[2] - a[2]*b[1];
	res[1] = a[2]*b[0] - a[0]*b[2];
	res[2] = a[0]*b[1] - a[1]*b[0];
}

//multiply matrix M by the control points and then by M transposed
void mMultCpMultM(float m[4][4], float cp[4][4][3], float res[4][4][3]){
    float aux[4][4][3];

    for(int i=0; i<4; i++){
        for(int j=0; j<4; j++){
            aux[i][j][0] = m[i][0]*cp[0][j][0] + m[i][1]*cp[1][j][0] + m[i][2]*cp[2][j][0] + m[i][3]*cp[3][j][0];
            aux[i][j][1] = m[i][0]*cp[0][j][1] + m[i][1]*cp[1][j][1] + m[i][2]*cp[2][j][1] + m[i][3]*cp[3][j][1];
            aux[i][j][2] = m[i][0]*cp[0][j][2] + m[i][1]*cp[1][j][2] + m[i][2]*cp[2][j][2] + m[i][3]*cp[3][j][2];
        }
    }

    for(int i=0; i<4; i++){
        for(int j=0; j<4; j++){
            res[i][j][0] = aux[i][0][0]*m[0][j] + aux[i][1][0]*m[1][j] + aux[i][2][0]*m[2][j] + aux[i][3][0]*m[3][j];
            res[i][j][1] = aux[i][0][1]*m[0][j] + aux[i][1][1]*m[1][j] + aux[i][2][1]*m[2][j] + aux[i][3][1]*m[3][j];
            res[i][j][2] = aux[i][0][2]*m[0][j] + aux[i][1][2]*m[1][j] + aux[i][2][2]*m[2][j] + aux[i][3][2]*m[3][j];
        }
    }
}

//calcula a posição do ponto e a normal
void calcPoint(float u, float v, float m[4][4][3], float *p, float *n){
    float aux[4][3];
    float dU[3]; //derivada parcial por u
    float dV[3]; //derivada parcial por v

    //[3u² 2u 1 0]*m
    for(int i=0;i<4;i++){
        aux[i][0] = 3*u*u*m[0][i][0] + 2*u*m[1][i][0] + m[2][i][0];
        aux[i][1] = 3*u*u*m[0][i][1] + 2*u*m[1][i][1] + m[2][i][1];
        aux[i][2] = 3*u*u*m[0][i][2] + 2*u*m[1][i][2] + m[2][i][2];
    }
    
    //[3u² 2u 1 0]*m*[[v³][v²][v][1]]
    dU[0] = aux[0][0]*v*v*v + aux[1][0]*v*v + aux[2][0]*v + aux[3][0];
    dU[1] = aux[0][1]*v*v*v + aux[1][1]*v*v + aux[2][1]*v + aux[3][1];
    dU[2] = aux[0][2]*v*v*v + aux[1][2]*v*v + aux[2][2]*v + aux[3][2];

    //[u³ u² u 1]*m
    for(int i=0;i<4;i++){
        aux[i][0] = u*u*u*m[0][i][0] + u*u*m[1][i][0] + u*m[2][i][0] + m[3][i][0];
        aux[i][1] = u*u*u*m[0][i][1] + u*u*m[1][i][1] + u*m[2][i][1] + m[3][i][1];
        aux[i][2] = u*u*u*m[0][i][2] + u*u*m[1][i][2] + u*m[2][i][2] + m[3][i][2];
    }

    //[u³ u² u 1]*m*[[3v²][2v][1][0]]
    dV[0] = aux[0][0]*3*v*v + aux[1][0]*2*v + aux[2][0];
    dV[1] = aux[0][1]*3*v*v + aux[1][1]*2*v + aux[2][1];
    dV[2] = aux[0][2]*3*v*v + aux[1][2]*2*v + aux[2][2];
    
    //calculo da posição do ponto
    //[u³ u² u 1]*m*[[v³][v²][v][1]]
    p[0] = aux[0][0]*v*v*v + aux[1][0]*v*v + aux[2][0]*v + aux[3][0];
    p[1] = aux[0][1]*v*v*v + aux[1][1]*v*v + aux[2][1]*v + aux[3][1];
    p[2] = aux[0][2]*v*v*v + aux[1][2]*v*v + aux[2][2]*v + aux[3][2];

    //calculo da normal do ponto
    crossProduct(dV,dU,n);
}

//buscar a controlPoints os pontos de controlo deste patch
void getPatchPoints(int *patch, float *cp, float res[4][4][3]){
    int i=0;

    for(int j=0;j<4;j++){
        for(int k=0;k<4;k++){
            res[j][k][0]=cp[patch[i]*3];
            res[j][k][1]=cp[patch[i]*3 +1];
            res[j][k][2]=cp[patch[i]*3 +2];
            i++;
        }
    }
}

void generateBezierPatch(int fd, char *file, char *tessLevel){
    int fdI=open(file,O_RDONLY,0777);
    int numPatches, numControlPoints;
    float cp[4][4][3], con[4][4][3];
    char array[213];
    //need to be at least 4
    int tessLvl = atoi(tessLevel);
    
    //matriz de Bezier
    float m[4][4] = {{-1.f,  3.f, -3.f, 1.f},
                     { 3.f, -6.f,  3.f, 0.f},
                     {-3.f,  3.f,  0.f, 0.f},
                     { 1.f,  0.f,  0.f, 0.f}};

    if(fdI!=-1){
        numPatches = getNumber(fdI);
        int patches[numPatches*16];
        getPatches(fdI,patches,numPatches);
        
        numControlPoints = getNumber(fdI);
        float controlPoints[numControlPoints*3];
        getControlPoints(fdI,controlPoints,numControlPoints);
        
        float points[tessLvl][tessLvl][3], normal[tessLvl][tessLvl][3], texture[tessLvl][tessLvl][2];
       
        //print number of points
        sprintf(array,"%d\n", numPatches*(tessLvl-1)*(tessLvl-1)*6);
        write(fd,array,strlen(array));

        for(int i=0;i<numPatches;i++){
            getPatchPoints(patches+i*16,controlPoints,cp);
            mMultCpMultM(m,cp,con);
            
            //gerar os pontos para cada patch
            float u=0,v;
            for(int t=0; t<tessLvl; t++){
                v=0;
                for(int q=0; q<tessLvl; q++){
                    calcPoint(u,v,con,points[t][q],normal[t][q]);
                    texture[t][q][0]=u;
                    texture[t][q][1]=v;
                    v+=(1.f/(tessLvl-1));
                }
                u+=(1.f/(tessLvl-1));
            }
            
            //desenhar os triangulos a partir dos pontos gerados
            for(int t=0; t<tessLvl-1; t++){
                for(int q=0; q<tessLvl-1; q++){
                    printLine(fd,array,points[t][q],normal[t][q],texture[t][q]);
                    printLine(fd,array,points[t][q+1],normal[t][q+1],texture[t][q+1]);
                    printLine(fd,array,points[t+1][q],normal[t+1][q],texture[t+1][q]);
                    
                    printLine(fd,array,points[t+1][q],normal[t+1][q],texture[t+1][q]);
                    printLine(fd,array,points[t][q+1],normal[t][q+1],texture[t][q+1]);
                    printLine(fd,array,points[t+1][q+1],normal[t+1][q+1],texture[t+1][q+1]);
                }
            }
        }
        close(fdI);
    }else fprintf(stderr,"Wrong bezier file\n");
}

void createVec(float *a, float *b, float *res) {
	res[0] = b[0] - a[0];
	res[1] = b[1] - a[1];
	res[2] = b[2] - a[2];
}

void computeNormal(float v1[], float v2[], float v3[], float v4[], float res[]) {
    float aux1[3], aux2[3];

    createVec(v1,v2,aux1);
    createVec(v3,v4,aux2);

    crossProduct(aux1,aux2,res);
    normalize(res,1);
}
