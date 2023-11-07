#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<aio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <libgen.h>
void finishWithError(const char* errorMessage){
	perror(errorMessage);
	exit(-1);
}

void extractLastModificationTime(struct stat*file_stat,char *time){
	struct tm* timeinfo= localtime(&file_stat->st_atime);
	strftime(time,20,"%d.%m.%Y",timeinfo);
}

void extractAccessRights(mode_t mode, char *accessRights){
	accessRights[0]=(mode & S_IRUSR) ?'R':'-';
    accessRights[1]=(mode & S_IWUSR) ?'W':'-';
    accessRights[2]=(mode & S_IXUSR) ?'X':'-';
    accessRights[3]=(mode & S_IRGRP) ?'R':'-';
    accessRights[4]=(mode & S_IWGRP) ?'W':'-';
    accessRights[5]=(mode & S_IXGRP) ?'X':'-';
    accessRights[6]=(mode & S_IROTH) ?'R':'-';
    accessRights[7]=(mode & S_IWOTH) ?'W':'-';
    accessRights[8]=(mode & S_IXOTH) ?'X':'-';
    accessRights[9]='\0';
}
int isFileBMP(int fd){
	char signature[3];
	read(fd,signature,2);
	signature[2]='\0';
	char *expected="BM\0";
	if(strcmp(signature,expected)==0)
		return 1;
	else 
		return 0;
}

void getHeightAndWidth(int fd,int *height,int *width){
	if(lseek(fd,18,SEEK_SET)==-1)
	{
		finishWithError("Eroare la citire!");
	}
	ssize_t bytes=0;
	bytes=read(fd,width,4);
	if(bytes==-1){
		finishWithError("Eroare la citire!");
	}

	bytes=read(fd,height,4);
	if(bytes==-1){
		perror("Eroare la citire!");
		exit(-6);
	}
}
int main(int argc,char *argv[]){

	if(argc!=2){
		printf("Usage ./program <fisier_intrare>");
		exit(-1);
	}
	
	int input=open(argv[1],O_RDONLY);
	if(input==-1){
		finishWithError("Eroare la deschiderea fisierului de intrare!");
	}
	
	if(isFileBMP(input)==0)
	{
		printf("Fisierul nu este BMP!");
		exit(-1);
	}
	struct stat file_stat;
	if(fstat(input,&file_stat)== -1){
		finishWithError("Eroare la obtinerea informatiilor despre fisier!");
	}
	
	char time[20];
	extractLastModificationTime(&file_stat,time);
	
	char accessRights[10];
	extractAccessRights(file_stat.st_mode,accessRights);
	
	char *fileName=basename(argv[1]);
	int idOwner=file_stat.st_uid;
	long file_size=(long)file_stat.st_size;
	long nrOfLinks=(long)file_stat.st_nlink;
	
	int output=creat("statistica.txt", S_IWUSR);
	if(output==-1){
		perror("Eroare la crearea de fisier output:statistica.txt");
		int inchidere=close(input);
		if(inchidere==-1){
			finishWithError("Eroare la inchiderea fisierului de intrare!");
		}
		exit(-1);
	}
	
	int height=0,width=0;
	getHeightAndWidth(input,&height,&width);
	char statistica[260];
    sprintf(statistica, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c",
            fileName,height,width,file_size,idOwner,time,nrOfLinks,
            accessRights[0],accessRights[1],accessRights[2],
            accessRights[3],accessRights[4],accessRights[5],
            accessRights[6],accessRights[7],accessRights[8]);

	ssize_t bytes= write(output,statistica,strlen(statistica));
    if (bytes== -1) {
        finishWithError("Eroare la scriere in fisierul de iesire!");
    }
	int close_output=close(input);
	if(close_output==-1){
		perror("Eroare la inchidere!");
		exit(-3);
	}
return 0;
}
