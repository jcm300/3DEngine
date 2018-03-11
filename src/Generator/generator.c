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

typedef struct point{
    float x,y,z;    
}Point;


int writeConfig(int, char**);
int generatePlane(int, char*,char*);
char* ftoa(float);
int generateBox(int, char*, char*, char*, char*);
int generateCone(int, char*, char*, char*, char*);
int generateSphere(int, char *, char *, char *);
void printLine(int, char*, float, float, float);
void genSlice(int, int,float,float, float, float,int);
void genWalls(int,int,float,float,float,float,float,float);
Point normalize(Point,float);

int main(int argc, char *argv[]){

    if(argc<3){
        fprintf(stderr,"Invalid no. of arguments");
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
         	if (nParams==6) generateBox(fd,params[1],params[2],params[3],params[4]);
			else if (nParams==5) generateBox(fd,params[1],params[2],params[3],"1"); //without stacks
			else fprintf(stderr,"Incorrect box parameters\n");
        }
        else if (strcmp(params[0],"sphere") == 0){ //sphere
            if (nParams==5) generateSphere(fd,params[1],params[2],params[3]);
            else fprintf(stderr,"Incorrect sphere parameters\n");
        }
        else if (strcmp(params[0],"cone") == 0){ //cone
            if (nParams==6) generateCone(fd,params[1],params[2],params[3],params[4]);
            else fprintf(stderr,"Incorrect cone parameters\n");
        }
        else fprintf(stderr,"Primitive not recognized\n");
        close(fd);
    }
    return 1;
}

int generatePlane(int fd, char *l, char *c){
    float lf=atof(l),cf=atof(c);
    char arr[70];
    lf/=2.0f; 
    cf/=2.0f; 
    write(fd,"6\n",2);
    printLine(fd,arr,-lf,0.f,-cf);
    printLine(fd,arr,-lf,0.f,cf);
    printLine(fd,arr,lf,0.f,cf);
    printLine(fd,arr,-lf,0.f,-cf);
    printLine(fd,arr,lf,0.f,cf);
    printLine(fd,arr,lf,0.f,-cf);
}

void printLine(int fd, char* array, float x, float y, float z){
    sprintf(array, "%f %f %f\n", x, y, z);
    write(fd,array,strlen(array));
}

void faceXZ(int fd, char* array, float x1, float x2, float y, float z){
	printLine(fd,array,x2,y,z);
    if (y>0) printLine(fd,array,x2,y,-z);
    else printLine(fd,array,x1,y,z); 
    printLine(fd,array,x1,y,-z);
    
    printLine(fd,array,x2,y,z);
    printLine(fd,array,x1,y,-z);
    if (y>0) printLine(fd,array,x1,y,z);
    else printLine(fd,array,x2,y,-z);
}

void faceYZ(int fd, char* array, float x, float y1, float y2, float z){
    printLine(fd,array,x, y1, -z);
    if (x>0) printLine(fd,array,x, y2, -z);
    else printLine(fd,array,x, y1, z);
    printLine(fd,array,x, y2, z);
    
    printLine(fd,array,x, y1, -z);
    printLine(fd,array,x, y2, z);
    if (x>0) printLine(fd,array,x, y1, z);
    else printLine(fd,array,x, y2, -z);
}

void faceXY(int fd, char* array, float x, float y1, float y2, float z){
	printLine(fd,array,x, y1, z);
    if (z>0) printLine(fd,array,x, y2, z);
    else printLine(fd,array,-x,y1, z);
    printLine(fd,array,-x, y2, z);
    
    printLine(fd,array,x, y1, z);
    printLine(fd,array,-x, y2, z);
    if (z>0) printLine(fd,array,-x, y1, z);
    else printLine(fd,array,x, y2, z);
}

int generateBox(int fd, char *xx, char *yy, char *zz, char *dd){
    float x = (float) atof(xx);
    float y = (float) atof(yy);
    float z = (float) atof(zz);
    int d = atoi(dd);
    float var_y = y/d;
    float var_x = x/d;
    x/=2.0f;
    y/=2.0f;
    z/=2.0f;
    char array[70];
    float y1 = -y;
    float y2 = y1 + var_y;
    float x1 = -x;
    float x2 = x1 + var_x;
    int i;

    sprintf(array,"%d\n",(2 * d + 4 * d) * 6);
    write(fd,array,strlen(array));

	for(i=0;i<d;i++,y1+=var_y,y2+=var_y,x1+=var_x,x2+=var_x){
		faceXZ(fd,array,x1,x2,-y,z);
		faceXZ(fd,array,x1,x2,y,z);
        
        faceYZ(fd,array,-x,y1,y2,z);
        faceYZ(fd,array,x,y1,y2,z);
        
        faceXY(fd,array,x,y1,y2,z);
		faceXY(fd,array,x,y1,y2,-z);
    }
}

int generateSphere(int fd, char *rds, char *slc, char *stks){
    int slices=atoi(slc), stacks=atoi(stks)/2,i;
    float radius=atof(rds),curRadius=radius,curHeight=0.f;
    float step=radius/stacks;
   	float angle = (2*M_PI)/slices;
    float sliceSide=2*curRadius*sin(angle/2);
    char array[70];
    sprintf(array,"%d\n", slices*stacks*12);
    write(fd,array,strlen(array));
    
    for(i=0;i<stacks;i++){
        genWalls(fd,slices,radius,curRadius,curHeight,angle,step,-1.f);
        curRadius-=step;
        curHeight+=step;
    }
    
    curHeight=-step;
    curRadius=radius-step;
    for(i=1;i<=stacks;i++){
        genWalls(fd,slices,radius,curRadius,curHeight,angle,step,0.f);
        curRadius-=step;
        curHeight-=step;
    }
}




int generateCone(int fd, char *radiuss , char *heights, char *slicess, char *stackss){
    float radius = (float) atof(radiuss);
    float height = (float) atof(heights);
    float slices = (float) atof(slicess);
    float stacks = (float) atof(stackss);
    int numVert =  slices*stacks*6 + slices *3,i;
    char array[65];
    float curAngle=0.f;
    float deltaAngle = (2*M_PI)/slices;
    float curHeight=0.f;
    float deltaH =height/stacks;
    float curRadius = radius;
    float deltaR = radius/stacks;

    sprintf(array, "%d\n", numVert);
    write(fd,array,strlen(array));
    
    for(i=0;i<stacks;i++,curHeight+=deltaH, curRadius-=deltaR){
        genWalls(fd,slices,radius,curRadius,curHeight,deltaAngle,deltaR,deltaH);
    }
    
    //base
    for(i=0;i<slices;i++,curAngle+=deltaAngle){
        printLine(fd,array,0.f,0.f,0.f);
        printLine(fd,array,radius*sin(curAngle+deltaAngle),0.f,radius*cos(curAngle+deltaAngle));
        printLine(fd,array,radius*sin(curAngle),0.f,radius*cos(curAngle));         
    }
}

void genWalls(int fd, int sliceCount, float radius, float curRadius, float y, float angle, float delta, float top){
    char array[70];
    int i;
    float curAngle;
    float nextStackY=delta+y, nextRadius=curRadius-delta;
    Point curP;
    
    if(top==0.f) nextRadius=curRadius+delta;
    else if(top>0.f) nextStackY=y+top;

    for(i=0,curAngle=0.f;i<sliceCount;i++,curAngle+=angle){
        curP.x=nextRadius*sin(curAngle);
        curP.y=nextStackY;
        curP.z=nextRadius*cos(curAngle);
        if(top<=0.f)curP=normalize(curP,radius);
        printLine(fd,array,curP.x,curP.y,curP.z);

        curP.x=curRadius*sin(curAngle);
        curP.y=y;
        curP.z=curRadius*cos(curAngle);
        if(top<=0.f)curP=normalize(curP,radius);
        printLine(fd,array,curP.x,curP.y,curP.z);

        curP.x=curRadius*sin(curAngle+angle);
        curP.y=y;
        curP.z=curRadius*cos(curAngle+angle);
        if(top<=0.f)curP=normalize(curP,radius);
        printLine(fd,array,curP.x,curP.y,curP.z);

        curP.x=nextRadius*sin(curAngle);
        curP.y=nextStackY;
        curP.z=nextRadius*cos(curAngle);
        if(top<=0.f)curP=normalize(curP,radius);
        printLine(fd,array,curP.x,curP.y,curP.z);

        curP.x=curRadius*sin(curAngle+angle);
        curP.y=y;
        curP.z=curRadius*cos(curAngle+angle);
        if(top<=0.f)curP=normalize(curP,radius);
        printLine(fd,array,curP.x,curP.y,curP.z);

        curP.x=nextRadius*sin(curAngle+angle);
        curP.y=nextStackY;
        curP.z=nextRadius*cos(curAngle+angle);
        if(top<=0.f)curP=normalize(curP,radius);
        printLine(fd,array,curP.x,curP.y,curP.z);
    }

}


Point normalize(Point cur,float radius){
    Point normPoint;     
    float dist=sqrt(cur.x*cur.x+cur.y*cur.y+cur.z*cur.z);
    float dx,dy,dz;
    dx=(cur.x*radius)/dist;
    dy=(cur.y*radius)/dist;
    dz=(cur.z*radius)/dist;
    normPoint.x=dx;
    normPoint.y=dy;
    normPoint.z=dz;
    return normPoint;
}

//Converts a float to an array of chars
char *ftoa(float fl){
    char *c=(char*)calloc(20,sizeof(char));
    int i=0,temp=(int)fl,dec=5;
    
    //fl is now the decimal part
    fl-=temp;
    do{
        *(c+i)=48+temp%10;
        temp/=10;
        i ++;
    }while(temp>0 && i<18);
    if(i==0){
        *(c+i)='0';
        i ++;
    }
    *(c+i)='.';
    i ++;
    temp=(int)fl*1000000;
    do{
        *(c+i+dec)=48+temp%10;
        temp/=10;
        dec --;
    }while(dec>=0 && temp>0);
    if(dec==5)
        *(c+i)='0';

    return c;
}
