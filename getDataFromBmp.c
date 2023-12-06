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
#include "processLib.h"

struct BITMAPINFOHEADER info;

void showProcessCode(){
  int status;
  pid_t wpid;
                while ((wpid = wait( & status)) > 0) {
                    if (WIFEXITED(status)) {
                        printf("S-a încheiat procesul cu pid-ul %d și codul %d\n la parent %d\n", wpid, WEXITSTATUS(status), getpid());
                    } else {
                        printf("Procesul cu pid-ul %d a fost încheiat în mod neașteptat\n", wpid);
                    }
                }
}

void goThroughDirectory(char * dirPath, char * dirIesire) {

    DIR * dir = opendir(dirPath);
    if (dir == NULL) {
        finishWithError("Eroare la deschiderea directorului!");
    }
     pid_t pid = fork();
     if (pid < 0) {
              	  finishWithError("Error while forking!");
     } else if (pid == 0) {
	    processFile(dirPath,dirIesire);
            exit(0);
      } else {
      	showProcessCode();

     
    struct dirent * currentEntry;

    while ((currentEntry = readdir(dir)) != NULL) //cat timp intrarile nu sunt epuizate
    {
        char filePath[PATH_MAX];
        snprintf(filePath, PATH_MAX, "%s/%s", dirPath, currentEntry -> d_name);

        if (strcmp(currentEntry -> d_name, ".") != 0 && strcmp(currentEntry -> d_name, "..") != 0) //nu aplica logica pentru legaturile default la directorul curent si directorul parinte
        {
            
                if (currentEntry -> d_type == DT_REG || currentEntry -> d_type == DT_LNK || currentEntry -> d_type == DT_DIR) {
                pid_t pid = fork();
            	if (pid < 0) {
              	  finishWithError("Error while forking!");
            	} else if (pid == 0) {

                   	 processFile(filePath, dirIesire);
                   	 exit(0);
                 } else {
			showProcessCode();

          	  }
          	}
		
           	 if (currentEntry -> d_type == DT_REG && strstr(filePath, ".bmp"))
           	  {
                        pid_t pid = fork();
                        if (pid < 0) {
                            finishWithError("Error while forking!");
                        } else if (pid == 0) {
                            read_bmp_header(filePath);
                            convert_to_grayscale(filePath, info.width, info.height);
                        } else {
				showProcessCode();
                            }
                     }
              }

         }


     }       
}

int main(int argc, char * argv[]) {

    if (argc != 3) {
        printf("Usage ./program <director_intrare><director_iesire>");
        exit(-1);
    }

    goThroughDirectory(argv[1], argv[2]);


    return 0;
}
