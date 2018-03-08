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
#define _USE_MATH_DEFINES
#include<math.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h> 
#include<sys/fcntl.h>
#include<sys/unistd.h>


int writeConfig(int, char**);
int generatePlane(int, char*,char*);
char* ftoa(float);
int generateBox(int, char*, char*, char*, char*);
int generateCone(int, char*, char*, char*, char*);
int generateSphere(int, char *, char *, char *);
void printLine(int, char*, float, float, float);
void genSlice(int, int, float, float, float);

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
        switch(**params){
            case 'p': //plane
                generatePlane(fd,params[1],params[2]);
                break;
            case 'b': //box
            	if (nParams==6)
					generateBox(fd,params[1],params[2],params[3],params[4]);
				else if (nParams==5){
					char* noDivision = "1";
					generateBox(fd,params[1],params[2],params[3],noDivision);
				}
				else 
					fprintf(stderr,"Incorrect box parameters\n");
                break;
            case 's': //sphere
                generateSphere(fd,params[1],params[2],params[3]);
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
    char array[70];
    float y1 = -y;
    float y2 = y1 + var;
    int i;

	faceXZ(fd,array,x,-y,z);
	faceXZ(fd,array,x,y,z);

	for(i=0;i<d;i++,y1+=var,y2+=var){
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
    
    for(i=0;i<stacks;i++){
        genSlice(fd,slices,curRadius,curHeight,angle);
        curRadius-=step;
        curHeight+=step;
    }
    
    curHeight=-step;
    curRadius=radius-step;
    for(i=1;i<stacks;i++){
        genSlice(fd,slices,curRadius,curHeight,angle);
        curRadius-=step;
        curHeight-=step;
    }
}




int generateCone(int fd, char *radiuss , char *heights, char *slicess, char *stackss){
    float radius = (float) atof(radiuss);
    float height = (float) atof(heights);
    float slices = (float) atof(slicess);
    float stacks = (float) atof(stackss);

    int numVert =  slices*stacks*6 + slices *6 ;
    char str[150];
    sprintf(str, "%d\n", numVert);
    write(fd,str,strlen(str));

    char array[5000];
    int s = 0;
    int st =0;
    float angle = (2*M_PI)/slices;
    float angleV = 0;

    float i_heig =height/stacks;
    float i_radi = radius/stacks;


    float height_top = i_heig;
    float radius_top = radius- i_radi;
    float height_bot = 0;
    float radius_bot = radius;

    while (st<stacks) {
         while(s<slices){
/*
                //base cima
                printLine(fd,array,radius_top*sin(angleV),height_top,radius_top*cos(angleV));
                printLine(fd,array,radius_top*sin(angleV+angle),height_top,radius_top*cos(angleV+angle));
                printLine(fd,array,0,height_top,0);
                
                //base baixo
                printLine(fd,array,0,height_bot,0);
                printLine(fd,array,radius_bot*sin(angleV+angle),height_bot,radius_bot*cos(angleV+angle));
                printLine(fd,array,radius_bot*sin(angleV),height_bot,radius_bot*cos(angleV));
*/
                //triangulo diagonal esq
                printLine(fd,array,radius_top*sin(angleV),height_top,radius_top*cos(angleV));
                printLine(fd,array,radius_bot*sin(angleV),height_bot,radius_bot*cos(angleV));
                printLine(fd,array,radius_top*sin(angleV+angle),height_top,radius_top*cos(angleV+angle));
                
                //triangulo diagonal dir
                printLine(fd,array,radius_top*sin(angleV+angle),height_top,radius_top*cos(angleV+angle));
                printLine(fd,array,radius_bot*sin(angleV),height_bot,radius_bot*cos(angleV));
                printLine(fd,array,radius_bot*sin(angleV+angle),height_bot,radius_bot*cos(angleV+angle));
                angleV+=angle;
                s++;
         }
         radius_bot-=i_radi;
         height_bot+= i_heig;
         radius_top-=i_radi;
         height_top+= i_heig;
         st++;
         s=0;
    }

    angle = (2*M_PI)/slices;
    angleV = 0;
    s=0;
    height_bot=height - i_heig;
    height_top= height;
    radius_bot+=i_radi;
    radius_top=0;
    
    while(s<slices){

                //base baixo
                printLine(fd,array,0,0,0);
                printLine(fd,array,radius*sin(angleV+angle),0,radius*cos(angleV+angle));
                printLine(fd,array,radius*sin(angleV),0,radius*cos(angleV));

                //bico cima
                printLine(fd,array,radius_top*sin(angleV+angle),height_top,radius_top*cos(angleV+angle));
                printLine(fd,array,radius_bot*sin(angleV),height_bot,radius_bot*cos(angleV));
                printLine(fd,array,radius_bot*sin(angleV+angle),height_bot,radius_bot*cos(angleV+angle));
                
                angleV+=angle;
                s++;
    }



}

void genSlice(int fd, int sliceCount, float radius, float y, float angle){
    char array[70];
    int i;
    float curAngle;

    for(i=0,curAngle=0.f;i<sliceCount;i++,curAngle+=angle){
        printLine(fd,array,0.f,y,0.f);
        printLine(fd,array,radius*sin(curAngle+angle),y,radius*cos(curAngle+angle));
        printLine(fd,array,radius*sin(curAngle),y,radius*cos(curAngle));
    }

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
