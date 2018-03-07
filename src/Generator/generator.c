/**
 * generator.c
 * Purpose: Generates a config file with defining the given geometrical primitives, passed as parameter.
 *
 * @author João Vieira 
 * @author José Martins
 * @author Miguel Quaresma
 * @author Simão Barbosa
 */

#ifdef __WIN32__
    #include<io.h>
#else 
    #include<unistd.h>
#endif
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>
#include<sys/fcntl.h>
#include<sys/unistd.h>


int writeConfig(int, char**);
int generatePlane(int, char*,char*);
char* ftoa(float);
int generateBox(int, char*, char*, char*, char*);

int main(int argc, char *argv[]){

    if(argc<3){
        fprintf(stderr,"Invalid no. of arguments");
    }else{
        writeConfig(argc-1,++argv);
    }
}

int writeConfig(int nParams, char **params){
    char *fileName=params[nParams-1];
    int fd=open(fileName,O_CREAT | O_WRONLY,9999);
    if(fd!=-1){
        switch(**params){
            case 'p': //plane
                generatePlane(fd,params[2],params[3]);
                break;
            case 'b': //box
                generateBox(fd,params[1],params[2],params[3],params[4]);
                break;
            case 's': //sphere

                break;
            case 'c': //cone
				generateCone(fd,params[1],params[2],params[3],params[4]);
                break;
            default:
                fprintf(stderr,"Primitive not recognized\n");
                break;
        }
        close(fd);
    }
    return 1;
}

int generatePlane(int fd, char *l, char *c){
    float li=atof(l),ci=atof(c);
    li/=2.0f; 
    ci/=2.0f; 

}

void printLine(int fd, char* array, float x, float y, float z){
	sprintf(array, "%f %f %f\n", x, y, z);
    write(fd,array,strlen(array));
}

void faceXZ(int fd, char* array, float x, float y, float z){
	printLine(fd,array,x,y,z);
    if (y>0) printLine(fd,array,x,y,-z);
    else printLine(fd,array,-x,y,z); 
    printLine(fd,array,-x,y,-z);
    
    printLine(fd,array,x,y,z);
    printLine(fd,array,-x,y,-z);
    if (y>0) printLine(fd,array,-x,y,z);
    else printLine(fd,array,x,y,-z);
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
    float d = (float) atof(dd);
    float var = y/d;
    x/=2.0f;
    y/=2.0f;
    z/=2.0f;
    char array[100];
    float y1 = -y;
    float y2 = y1 + var;

	faceXZ(fd,array,x,-y,z);
	faceXZ(fd,array,x,y,z);

	for (int i=0;i<d;i++,y1+=var,y2+=var){
        faceYZ(fd,array,-x,y1,y2,z);
        faceYZ(fd,array,x,y1,y2,z);
        faceXY(fd,array,x,y1,y2,z);
		faceXY(fd,array,x,y1,y2,-z);
    }
}

int generateSphere(){

}


int generateCone(int fd, char *radiuss , char *heights, char *slicess, char *stackss){
    float radius = (float) atof(radiuss);
    float height = (float) atof(heights);
    float slices = (float) atof(slicess);
    float stacks = (float) atof(stackss);
    char array[100];
    int s = 0;
   	float angle = (2*M_PI)/10;
    float angleV = 0;

    while(s<slices){
            
            printLine(fd,array,0,0,0);
            printLine(fd,array,radius*sin(angleV+angle),0,radius*cos(angleV+angle));
            printLine(fd,array,radius*sin(angleV),0,radius*cos(angleV));

        	printLine(fd,array,0,height,0);
      	    printLine(fd,array,radius*sin(angleV),0,radius*cos(angleV));
            printLine(fd,array,radius*sin(angleV+angle),0,radius*cos(angleV+angle));

            angleV+=angle;
            s++;
    }
}

//Converts a float to an array of chars
char *ftoa(float fl){
    char *c=(char*)calloc(20,sizeof(char));
    int i=0;
    float intPart=0.f, back=fl;
    if(fl<=0.f){
        fl*=-1;
        *(c+i)='-';
        i++;
    }
    while(back>=1.f){
        intPart+=fmod(back,10.f);
        back/=10.f;
    }
    //fl is now the decimal part
    fl-=intPart;
    do{
        if(intPart>=1.f){
            *(c+i)=48+fmod(intPart,10.f);
            intPart/=10.f;
        }else{
            fl*=10.f;
            *(c+i)=48+fmod(fl,10);
        }
        i ++;
    }while(fl+intPart>=0.0f);
    return c;
}
