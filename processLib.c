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
struct BITMAPFILEHEADER file_header;
struct BITMAPINFOHEADER info_header;

void convert_to_grayscale(char * file) {
printf("----------------%d\n",info_header.bit_count);
  int fd = open(file, O_RDWR);
  if (fd == -1) {
    printf("Error opening file.\n");
    return;
  }

  lseek(fd, 54, SEEK_CUR);
   struct Pixel pixel;
if(info_header.bit_count>8){
 
  for (int y = 0; y < info_header.height; y++) {
    for (int x = 0; x < info_header.width; x++) {
      int pixel_position = lseek(fd, 0, SEEK_CUR); // Get the current position
      ssize_t bytes = read(fd, & pixel, sizeof(struct Pixel));
      if (bytes == -1) {
        perror("Bad reading");
      }
      // Convert to grayscale using the provided formula
      uint8_t grayscale = (uint8_t)((0.299 * pixel.red) + (0.587 * pixel.green) + (0.114 * pixel.blue));

      // Override original pixel values with grayscale values
      pixel.red = grayscale;
      pixel.green = grayscale;
      pixel.blue = grayscale;

      lseek(fd, pixel_position, SEEK_SET);
      bytes = write(fd, & pixel, sizeof(struct Pixel));
      if (bytes == -1) {
        perror("Bad writting");

      }

    }
  }
}else{

	for(int i=0;i<info_header.colors_used;i++){
	 int pixel_position = lseek(fd, 0, SEEK_CUR); // Get the current position
      ssize_t bytes = read(fd, & pixel, sizeof(struct Pixel));
      if (bytes == -1) {
        perror("Bad reading");
      }
      uint8_t reserved=0;
      bytes = read(fd, &reserved, sizeof(uint8_t));
      if (bytes == -1) {
        perror("Bad reading");
      }
      // Convert to grayscale using the provided formula
      uint8_t grayscale = (uint8_t)((0.299 * pixel.red) + (0.587 * pixel.green) + (0.114 * pixel.blue));

      // Override original pixel values with grayscale values
      pixel.red = grayscale;
      pixel.green = grayscale;
      pixel.blue = grayscale;

      lseek(fd, pixel_position, SEEK_SET);
      bytes = write(fd, & pixel, sizeof(struct Pixel));
      if (bytes == -1) {
        perror("Bad writting");

      }
      bytes = write(fd, &reserved, sizeof(uint8_t));
      if (bytes == -1) {
        perror("Bad writting");

      }
	}

}
  close(fd);
}

void read_bmp(char * file_path) {
  int fd = open(file_path, O_RDWR);
  if (fd == -1) {
    printf("Error opening file.\n");
    return;
  }

  if (read(fd, & file_header, sizeof(file_header)) != sizeof(file_header)) {
    printf("Error reading file header.\n");
    close(fd);
    return;
  }

  printf("Signature: %c%c\n", (char)(file_header.signature & 0xFF), (char)((file_header.signature >> 8) & 0xFF));
  printf("File Size: %u bytes\n", file_header.file_size);
  printf("Data Offset: %u bytes\n", file_header.data_offset);

  lseek(fd, sizeof(file_header), SEEK_SET);

  if (read(fd, & info_header, sizeof(info_header)) != sizeof(info_header)) {
    printf("Error reading info header.\n");
    close(fd);
    return;
  }

  close(fd);
}

void finishWithError(const char * errorMessage) {
  perror(errorMessage);
  exit(-1);
}

void extractLastModificationTime(struct stat * file_stat, char * time) {
  struct tm * timeinfo = localtime( & file_stat -> st_atime);
  strftime(time, 20, "%d.%m.%Y", timeinfo);
}

void extractAccessRights(mode_t mode, char * accessRights) {
  accessRights[0] = (mode & S_IRUSR) ? 'R' : '-';
  accessRights[1] = (mode & S_IWUSR) ? 'W' : '-';
  accessRights[2] = (mode & S_IXUSR) ? 'X' : '-';
  accessRights[3] = (mode & S_IRGRP) ? 'R' : '-';
  accessRights[4] = (mode & S_IWGRP) ? 'W' : '-';
  accessRights[5] = (mode & S_IXGRP) ? 'X' : '-';
  accessRights[6] = (mode & S_IROTH) ? 'R' : '-';
  accessRights[7] = (mode & S_IWOTH) ? 'W' : '-';
  accessRights[8] = (mode & S_IXOTH) ? 'X' : '-';
  accessRights[9] = '\0';
}

void getHeightAndWidth(int fd, int * height, int * width) {
  if (lseek(fd, 18, SEEK_SET) == -1) {
    finishWithError("Eroare la citire!");
  }
  ssize_t bytes = 0;
  bytes = read(fd, width, 4);
  if (bytes == -1) {
    finishWithError("Eroare la citire!");
  }

  bytes = read(fd, height, 4);
  if (bytes == -1) {
    perror("Eroare la citire!");
    exit(-6);
  }
}

void processFile(char * filePath, char * dirIesire) {

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

  char accessRights[10]; //mutare pt ca la link sunt drepturile legaturii

  char statistica[260];

  if (S_ISLNK(file_stat.st_mode)) {
    struct stat link_stat;
    if (fstat(input, & link_stat) == -1) {
      finishWithError("Eroare la obtinerea informatiilor despre fisierul target!");
    }

    extractAccessRights(link_stat.st_mode, accessRights);

    sprintf(statistica, "\nnume legatura: %s\ndimensiune legatura: %ld\ndimensiune fisier target: %ld\ndrepturi de acces user legatura: %c%c%c\ndrepturi de acces grup legatura: %c%c%c\ndrepturi de acces altii legatura: %c%c%c\n",
      fileName, file_stat.st_size, link_stat.st_size, accessRights[0], accessRights[1], accessRights[2], accessRights[3], accessRights[4], accessRights[5], accessRights[6], accessRights[7], accessRights[8]);
  } else if (S_ISREG(file_stat.st_mode)) {
    extractAccessRights(file_stat.st_mode, accessRights);
    if (strstr(fileName, ".bmp")) {
      int height, width;
      getHeightAndWidth(input, & height, & width);
      sprintf(statistica, "\nnume fisier: %s\ninaltime: %d\nlungime: %d\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c\n",
        fileName, height, width, file_stat.st_size, file_stat.st_uid, time, file_stat.st_nlink, accessRights[0], accessRights[1], accessRights[2], accessRights[3], accessRights[4], accessRights[5], accessRights[6], accessRights[7], accessRights[8]);
    } else {
      sprintf(statistica, "\nnume fisier: %s\ndimensiune: %ld\nidentificatorul utilizatorului: %d\ntimpul ultimei modificari: %s\ncontorul de legaturi: %ld\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c\n",
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
    sprintf(statistica, "\nnume director: %s\nidentificatorul utilizatorului: %d\ndrepturi de acces user: %c%c%c\ndrepturi de acces grup: %c%c%c\ndrepturi de acces altii: %c%c%c\n",
      fileName, file_stat.st_uid, accessRights[0], accessRights[1], accessRights[2], accessRights[3], accessRights[4], accessRights[5], accessRights[6], accessRights[7], accessRights[8]);
  }
  char outputPath[200];
  //strcat(dirPath,"/");
  //strcat(dirPath,fileName);
  //strcat(dirPath,"_statistica.txt");
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
}


void printRegexMatches(char *c){

execlp("bash","bash","./shell.sh", c, (char * )NULL);

}
