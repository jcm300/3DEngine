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

int generateBox(int fd, char *xx, char *yy, char *zz, char *dd){
    float x = (float) atof(xx);
    float y = (float) atof(yy);
    float z = (float) atof(zz);
    float d = (float) atof(dd);
    x/=2.0f;
    z/=2.0f;
    char array[100];

    sprintf(array, "%f %f %f\n", -x, 0.f, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, 0.f, -z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, 0.f, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", -x, 0.f, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", -x, 0.f, -z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, 0.f, -z);
    write(fd,array,strlen(array));

    sprintf(array, "%f %f %f\n", -x, y, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, y, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, y, -z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", -x, y, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, y, -z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", -x, y, -z);
    write(fd,array,strlen(array));

    sprintf(array, "%f %f %f\n", -x, 0.f, -z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", -x, 0.f, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", -x, y, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", -x, y, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", -x, y, -z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", -x, 0.f, -z);
    write(fd,array,strlen(array));

    sprintf(array, "%f %f %f\n", -x, 0.f, -z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", -x, y, -z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, 0.f, -z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, 0.f, -z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", -x, y, -z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, y, -z);
    write(fd,array,strlen(array));

    sprintf(array, "%f %f %f\n", x, y, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, 0.f, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, 0.f, -z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, 0.f, -z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, y, -z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, y, z);
    write(fd,array,strlen(array));

    sprintf(array, "%f %f %f\n", x, 0.f, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", -x, y, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", -x, 0.f, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, 0.f, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", x, y, z);
    write(fd,array,strlen(array));
    sprintf(array, "%f %f %f\n", -x, y, z);
    write(fd,array,strlen(array));
}

int generateSphere(){

}

int generateCone(){

}

/*
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
*/