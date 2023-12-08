#ifndef __PROCESSLIB_H
#define __PROCESSLIB_H
#pragma pack(push, 1) // Ensure struct uses 1-byte alignment
struct BITMAPFILEHEADER {
    uint16_t signature;
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
};

struct BITMAPINFOHEADER {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t image_size;
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t colors_important;
};


struct Pixel {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
};
#pragma pack(pop) // Restore default structure alignment
int processFile(char * filePath, char * dirIesire);
void convert_to_grayscale(char * file);
void read_bmp(char * file_path);
void finishWithError(const char * errorMessage);
void printRegexMatches(char *c);
#endif

