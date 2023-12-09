#include<stdio.h>
#include<stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include "processLib.h"



void showProcessCode() {
  int status;
  pid_t wpid;
  while ((wpid = wait( & status)) > 0) {
    if (WIFEXITED(status)) {
      printf("S-a încheiat procesul cu pid-ul %d și codul %d(ce reprezinta nr de linii scrise) la parent %d\n", wpid, WEXITSTATUS(status), getpid());
    } else {
      printf("Procesul cu pid-ul %d a fost încheiat în mod neașteptat\n", wpid);
    }
  }
}

void goThroughDirectory(char * dirPath, char * dirIesire, char *character) {

  DIR * dir = opendir(dirPath);
  if (dir == NULL) {
    finishWithError("Eroare la deschiderea directorului!");
  }
  pid_t pid = fork();// a procces for the given directory
  if (pid < 0) {
    finishWithError("Error while forking!");
  } else if (pid == 0) {
    int nrOfLines=processFile(dirPath, dirIesire);
    exit(nrOfLines);
  } else {
    showProcessCode();

    struct dirent * currentEntry;

    while ((currentEntry = readdir(dir)) != NULL) //cat timp intrarile nu sunt epuizate
    {
      char filePath[PATH_MAX];
      snprintf(filePath, PATH_MAX, "%s/%s", dirPath, currentEntry -> d_name);

      if (strcmp(currentEntry -> d_name, ".") != 0 && strcmp(currentEntry -> d_name, "..") != 0) //nu aplica logica pentru legaturile default la directorul curent si directorul parinte
      {

        if (currentEntry -> d_type == DT_REG || currentEntry -> d_type == DT_LNK || currentEntry -> d_type == DT_DIR) { // link or directory case
          pid_t pid = fork();
          if (pid < 0) {
            finishWithError("Error while forking!");
          } else if (pid == 0) {
            int nrOfLines=processFile(filePath, dirIesire);
            exit(nrOfLines);
          } else {
            showProcessCode();
          }
        }

        if (currentEntry -> d_type == DT_REG) {
					if (strstr(filePath, ".bmp")) {   // bmp regular file case         
          	pid_t pid = fork(); // proess for processing file
          	if (pid < 0) {
              finishWithError("Error while forking!");
            } else if (pid == 0) {
              int nrOfLines=processFile(filePath, dirIesire);
              exit(nrOfLines);
            } else {
              showProcessCode();
            }
            
            pid = fork();// process for converting photo 
            if (pid < 0) {
              finishWithError("Error while forking!");
            } else if (pid == 0) {
              convert_to_grayscale(filePath);
              exit(0);
            } else {
              showProcessCode();
            }
          }
          
           else{ // regular file case(non-bmp)
          
          	int pipeChildren[2];//pipe-ul pt comunicarea intre copii
          	int pipe2WithParent[2]; //pepe-ul pt comunicarea intre copilul 2 si parinte
          	if(pipe(pipeChildren)==-1 || pipe(pipe2WithParent)==-1)
          	{
          		finishWithError("Eroare cand s-au creat pipe-urile!");
          	}
          	
          	pid_t pid = fork();
            if (pid < 0) {
              finishWithError("Error while forking!");
            } else if (pid == 0) {

								close(pipeChildren[0]);//se inchide capatul de citire 
								dup2(pipeChildren[1],STDOUT_FILENO);//redirectam capatul de scriere cu iesirea standard(unde o sa scrie proccesFile)
								close(pipeChildren[1]);
								int nrOfLines=processFile(filePath, dirIesire);
           	  	exit(nrOfLines);
            } else {
              showProcessCode();
            }
            
            pid = fork();
            if (pid < 0) {
              finishWithError("Error while forking!");
            } else if (pid == 0) {

								close(pipeChildren[1]);//se inchide capatul de scriere 
								dup2(pipeChildren[0],STDIN_FILENO);//redirectam capatul de citire cu intrarea standard(tot ce s-a primit -de la stdout- se pune pe stdin pt functia de regex
								close(pipeChildren[0]);//inchidere si a capatului de citire
								
								close(pipe2WithParent[0]);//inchide capatul de citire
								dup2(pipe2WithParent[1],STDOUT_FILENO);//redirecteaza capatul de scriere cu ce se primeste de la iesirea standard unde se scriu nr de match-uri
								close(pipe2WithParent[1]);//se inchide si capatul de scriere
								printRegexMatches(character);
           	  	exit(0);
            } else {
            	close(pipeChildren[0]);//inchidem capatul de citire
            	close(pipeChildren[1]);//inchidem capatul de scriere
            	close(pipe2WithParent[1]);//inchidem capatul de scriere;
            	
            	char buff[4096];//512*8
            	ssize_t bytes;
            	int nr=0;
            	// se citeste ce s-a primit pe pipe-ul de citire -de la stdout-
            	while((bytes=read(pipe2WithParent[0],buff,sizeof(buff)))>0){
            		 nr=atoi(buff);
            	}
            	
            	close(pipe2WithParent[0]);//inchidere capat de citire
            	printf("Au fost identificate in total <%d> propozitii corecte care contin caracterul %s\n", nr,character);//to be mod
              showProcessCode();
            }
					}
        }
      }

    }

  }
}

int main(int argc, char * argv[]) {

  if (argc != 4) {
    printf("Usage ./program <director_intrare><director_iesire> <c>");
    exit(-1);
  }

  goThroughDirectory(argv[1], argv[2],argv[3]);

  return 0;
}
