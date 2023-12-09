#ifndef __PROCESSLIB_H
#define __PROCESSLIB_H
#pragma pack(push, 1) // 1-byte alignment
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


struct PixelBMP {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
};
#pragma pack(pop) //get back to default structure alignment
int processFile(char * filePath, char * dirIesire);
void convert_to_grayscale(char * file);
void finishWithError(const char * errorMessage);
void printRegexMatches(char *c);
#endif

