#ifndef _PPLIB_H_
#define _PPLIB_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * define a Pixel to be a struct of 3 unsigned rgb integer values (RGB - red, green, blue)
 */
typedef struct { uint8_t r, g, b; } Pixel;

/**
 * Writes a given pixel buffer to an output binary file
 * (essentially creates a screenshot of a given pixel buffer state)
 *
 * @param buffer input pixel buffer handle
 * @param width image width in pixels
 * @param height image height in pixels
 * @param output_file the output file handle
 */
void bufferToBinary(Pixel *buffer, size_t width, size_t height,
                    FILE *output_file) {
  fprintf(output_file, "P6\n");
  fprintf(output_file, "%u %u\n", width, height);
  fprintf(output_file, "255\n");
  for (int i = 0; i < width * height; i++) {
    fputc(buffer[i].r, output_file);
    fputc(buffer[i].g, output_file);
    fputc(buffer[i].b, output_file);
  }
}

#endif
