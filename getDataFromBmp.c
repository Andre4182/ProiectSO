#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include<aio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include<libgen.h>
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


void processFile(char *filePath,char* dirIesire){

	int input=open(filePath,O_RDONLY);
	if(input==-1){
		finishWithError("Eroare la deschiderea fisierului de intrare!");
	}
	
	struct stat file_stat;
	if(lstat(filePath,&file_stat)== -1){
		finishWithError("Eroare la obtinerea informatiilor despre fisier!");
	}
	char *fileName=basename(filePath);
		
	char time[20];
	extractLastModificationTime(&file_stat,time);
	
	char accessRights[10];//mutare pt ca la link sunt drepturile legaturii

	
	char statistica[260];
	
	if (S_ISLNK(file_stat.st_mode))
    	{
		struct stat link_stat;
		if(fstat(input,&link_stat)== -1)
		{
			finishWithError("Eroare la obtinerea informatiilor despre fisierul target!");}

	        extractAccessRights(link_stat.st_mode,accessRights);

        	sprintf(statistica, "\nnume legatura: %s\ndimensiune legatura: %ld\ndimensiune fisier target: %ld\ndrepturi de acces user legatura: %c%c%c\ndrepturi de acces grup legatura: %c%c%c\ndrepturi de acces altii legatura: %c%c%c\n",
                fileName, file_stat.st_size, link_stat.st_size, accessRights[0], accessRights[1], accessRights[2], accessRights[3], accessRights[4], accessRights[5], accessRights[6], accessRights[7], accessRights[8]);
    }
	else if (S_ISREG(file_stat.st_mode))
	{
	    	extractAccessRights(file_stat.st_mode,accessRights);
       		if (strstr(fileName, ".bmp"))
        	{
            		int height, width;
            		getHeightAndWidth(input, &height, &width);

            		sprintf(statistica, "\nnume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c\n",
                    fileName, height, width, file_stat.st_size, file_stat.st_uid, time, file_stat.st_nlink, accessRights[0], accessRights[1], accessRights[2], accessRights[3], accessRights[4], accessRights[5], accessRights[6], accessRights[7], accessRights[8]);
      		}
        	else
        	{
            		sprintf(statistica, "\nnume fisier: %s\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c\n",
                    fileName, file_stat.st_size, file_stat.st_uid, time, file_stat.st_nlink, accessRights[0], accessRights[1], accessRights[2], accessRights[3], accessRights[4], accessRights[5], accessRights[6], accessRights[7], accessRights[8]);
        	}
    	}
    	else if (S_ISDIR(file_stat.st_mode))
    	{extractAccessRights(file_stat.st_mode,accessRights);
        	sprintf(statistica,"\nnume director: %s\nidentificatorul utilizatorului: %d\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c\n",
                fileName, file_stat.st_uid, accessRights[0], accessRights[1], accessRights[2], accessRights[3], accessRights[4], accessRights[5], accessRights[6], accessRights[7], accessRights[8]);
  	}
char outputPath[200];
  	//strcat(dirPath,"/");
  	//strcat(dirPath,fileName);
  	//strcat(dirPath,"_statistica.txt");
  	snprintf(outputPath,PATH_MAX,"%s/%s_statistica.txt",dirIesire,fileName);
	int output=creat(outputPath, S_IWUSR | S_IRUSR);
	if(output==-1){
		finishWithError("Eroare la crearea de fisier output:statistica.txt");
	}
    ssize_t bytes = write(output, statistica, strlen(statistica));
    if (bytes == -1)
    {
        finishWithError("Error writing to output file");
    }

    close(input);
    	int close_output=close(output);
	if(close_output==-1){
		perror("Eroare la inchidere!");
		exit(-3);
	}
}


void goThroughDirectory(char *dirPath,char *dirIesire){

	DIR *dir=opendir(dirPath);
	if(dir==NULL){
		finishWithError("Eroare la deschiderea directorului!");
	}
	//processFile(dirPath,dirIesire);
	struct dirent *currentEntry;
	
	while((currentEntry=readdir(dir))!=NULL)//cat timp intrarile nu sunt epuizate
	{	
		char filePath[PATH_MAX];
		snprintf(filePath,PATH_MAX,"%s/%s",dirPath,currentEntry->d_name);
		
		if(strcmp(currentEntry->d_name,".")!=0 && strcmp(currentEntry->d_name,"..")!=0)//nu aplica logica pentru legaturile default la directorul curent si directorul parinte
		{
			pid_t pid = fork();
            		if (pid < 0) {
                		finishWithError("Error while forking!");
            		} 
            		else if (pid == 0) 
            		{


			
				if(currentEntry->d_type==DT_REG || currentEntry->d_type==DT_LNK|| currentEntry->d_type==DT_DIR){
					processFile(filePath,dirIesire);
				
				}

				 exit(0);
			}
			
			else
			{
				int status;
    				pid_t wpid;
				while ((wpid = wait(&status)) > 0) {
        				if (WIFEXITED(status)) {
           					 printf("S-a încheiat procesul cu pid-ul %d și codul %d\n la parent %d\n", wpid, WEXITSTATUS(status),getpid());
        				} else {
            					printf("Procesul cu pid-ul %d a fost încheiat în mod neașteptat\n", wpid);
      					 }
    				}
			
			}
			if(currentEntry->d_type==DT_DIR){
					goThroughDirectory(filePath,dirIesire);
			}
		}
	}
}

int main(int argc,char *argv[]){

	if(argc!=3){
		printf("Usage ./program <director_intrare><director_iesire>");
		exit(-1);
	}
	//int output=creat("statistica.txt", S_IWUSR | S_IRUSR);
	//if(output==-1){
	//	finishWithError("Eroare la crearea de fisier output:statistica.txt");
	//}

	goThroughDirectory(argv[1],argv[2]);
	
	//int close_output=close(output);
	//if(close_output==-1){
	//	perror("Eroare la inchidere!");
	//	exit(-3);
	//}
return 0;
}



