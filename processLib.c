#include<stdio.h>
#include<stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include<libgen.h>
#include "processLib.h"

struct BITMAPINFOHEADER info_header;

void finishWithError(const char * errorMessage) {
  perror(errorMessage);
  exit(-1);
}

int returnNumberOfLines(char *text){

    char scriptExecute[300];
    FILE* fp;
    sprintf(scriptExecute, "./scriptNrOfLines.sh '%s'", text);

    fp = popen(scriptExecute, "r");
    if (fp == NULL) {
        finishWithError("Failed to run scriptNrOfLines shell script!");
    }
    int lines;
    fscanf(fp, "%d", &lines);
    pclose(fp);
    return lines;
}

void read_bmp(int fd) {
	lseek(fd,14,SEEK_SET);//skips over file header

  if ((read(fd, & info_header, sizeof(info_header))) != sizeof(info_header)) {
    finishWithError("Error reading info header.\n");
    close(fd);
  }
}

void convert_to_grayscale(char * file) {

  int fd = open(file, O_RDWR);
  if (fd == -1) {
    finishWithError("Error opening bmp image file");
  }
	read_bmp(fd);// pozitia cursorului va fi dupa primii 54 de bytes in urma citirii
   struct PixelBMP pixel;
if(info_header.bit_count>8){ 
 
  for (int y = 0; y < info_header.height; y++) {
    for (int x = 0; x < info_header.width; x++) {
      int pixel_position = lseek(fd, 0, SEEK_CUR); // Get the current position
      ssize_t bytes = read(fd, & pixel, sizeof(struct PixelBMP));
      if (bytes == -1) {
        finishWithError("Bad reading");
      }
      // calculam valoarea de gri cu ajutorul valorilor curente
      uint8_t grayscale = (uint8_t)((0.299 * pixel.red)+(0.587 * pixel.green)+(0.114 * pixel.blue));

      // suprascriem valorile cu valoarea de gri
      pixel.red = grayscale;
      pixel.green = grayscale;
      pixel.blue = grayscale;

      lseek(fd, pixel_position, SEEK_SET);
      bytes = write(fd, & pixel, sizeof(struct PixelBMP));
      if (bytes == -1) {
        finishWithError("Bad overriding of pixel!");
      }

    }
  }
}else{

	for(int i=0;i<info_header.colors_used;i++){
		int pixel_position = lseek(fd, 0, SEEK_CUR); // Get the current position
		ssize_t bytes = read(fd, & pixel, sizeof(struct PixelBMP));
      	if (bytes == -1) {
        	finishWithError("Bad reading");
      	}
      uint8_t reserved=0;
      bytes = read(fd, &reserved, sizeof(uint8_t));
      if (bytes == -1) {
        finishWithError("Bad reading");
      }
      // Convert to grayscale using the provided formula
      uint8_t grayscale = (uint8_t)((0.299 * pixel.red) + (0.587 * pixel.green) + (0.114 * pixel.blue));

      // Override original pixel values with grayscale values
      pixel.red = grayscale;
      pixel.green = grayscale;
      pixel.blue = grayscale;

      lseek(fd, pixel_position, SEEK_SET);
      bytes = write(fd, & pixel, sizeof(struct PixelBMP));
      if (bytes == -1) {
        finishWithError("Bad writting");

      }
      bytes = write(fd, &reserved, sizeof(uint8_t));
      if (bytes == -1) {
        finishWithError("Bad writting");

      }
    }
    
}
  close(fd);
}

void extractLastModificationTime(struct stat * file_stat, char * time) {
  struct tm * timeinfo = localtime( & file_stat -> st_atime);
  strftime(time, 20, "%d.%m.%Y", timeinfo);//coverts the time object into a string
}

void extractAccessRights(mode_t mode, char * accessRights) {
  accessRights[0]=(mode & S_IRUSR)?'R':'-';
  accessRights[1]=(mode & S_IWUSR)?'W':'-';
  accessRights[2]=(mode & S_IXUSR)?'X':'-';
  accessRights[3]=(mode & S_IRGRP)?'R':'-';
  accessRights[4]=(mode & S_IWGRP)?'W':'-';
  accessRights[5]=(mode & S_IXGRP)?'X':'-';
  accessRights[6]=(mode & S_IROTH)?'R':'-';
  accessRights[7]=(mode & S_IWOTH)?'W':'-';
  accessRights[8]=(mode & S_IXOTH)?'X':'-';
}

int processFile(char * filePath, char * dirIesire) {

  int input = open(filePath, O_RDONLY);
  if (input == -1) {
    finishWithError("Eroare la deschiderea fisierului de intrare!");
  }
  struct stat file_stat;
  if (lstat(filePath, & file_stat) == -1) {
    finishWithError("Eroare la obtinerea informatiilor despre fisier!");
  }
  char * fileName = basename(filePath);

  char time[20];
  extractLastModificationTime( & file_stat, time);

  char accessRights[10];

  char statistica[260];//ce va fi pus la output 

  if (S_ISLNK(file_stat.st_mode)) {
    struct stat link_stat;
    if (fstat(input, & link_stat) == -1) {
      finishWithError("Eroare la obtinerea informatiilor despre fisierul target!");
    }

    extractAccessRights(link_stat.st_mode, accessRights);

    sprintf(statistica, "nume legatura: %s\ndimensiune legatura: %ld\ndimensiune fisier target: %ld\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c",
      fileName, file_stat.st_size, link_stat.st_size, accessRights[0], accessRights[1], accessRights[2], accessRights[3], accessRights[4], accessRights[5], accessRights[6], accessRights[7], accessRights[8]);
  } else if (S_ISREG(file_stat.st_mode)) {
    extractAccessRights(file_stat.st_mode, accessRights);
    if (strstr(fileName, ".bmp")) {
			read_bmp(input);
      sprintf(statistica, "nume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c",
        fileName, info_header.height, info_header.width, file_stat.st_size, file_stat.st_uid, time, file_stat.st_nlink, accessRights[0], accessRights[1], accessRights[2], accessRights[3], accessRights[4], accessRights[5], accessRights[6], accessRights[7], accessRights[8]);
    } else {
      sprintf(statistica, "nume fisier: %s\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c",
        fileName, file_stat.st_size, file_stat.st_uid, time, file_stat.st_nlink, accessRights[0], accessRights[1], accessRights[2], accessRights[3], accessRights[4], accessRights[5], accessRights[6], accessRights[7], accessRights[8]);
    char buff[4096];
    ssize_t bytes = 0;

    	while ((bytes = read(input, buff, sizeof(buff))) > 0)
    	{
        	write(STDOUT_FILENO, buff, bytes);
    	} 	

    }
  } else if (S_ISDIR(file_stat.st_mode)) {
    extractAccessRights(file_stat.st_mode, accessRights);
    sprintf(statistica, "nume director: %s\nidentificatorul utilizatorului: %d\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c",
      fileName, file_stat.st_uid, accessRights[0], accessRights[1], accessRights[2], accessRights[3], accessRights[4], accessRights[5], accessRights[6], accessRights[7], accessRights[8]);
  }
  char outputPath[200];

  snprintf(outputPath, PATH_MAX, "%s/%s_statistica.txt", dirIesire, fileName);
  int output = creat(outputPath, S_IWUSR | S_IRUSR);
  if (output == -1) {
    finishWithError("Eroare la crearea de fisier output:statistica.txt");
  }
  ssize_t bytes = write(output, statistica, strlen(statistica));
  if (bytes == -1) {
    finishWithError("Error writing to output file");
  }
   
  close(input);
  int close_output = close(output);
  if (close_output == -1) {
    perror("Eroare la inchidere!");
    exit(-3);
  }
  return returnNumberOfLines(statistica);
}


void printRegexMatches(char *c){

execlp("bash","bash","./shell.sh", c, (char * )NULL);
}


