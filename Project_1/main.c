/**
 * @author JP Labadie
 * Created September 11, 2016
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

static const size_t INIT_BUFF_SIZE = 255;
static const char cli_help_text[] =
    "ppmrw [format] [input file] [output file]\n"
    "Description -- Cross-converts PPM formats P6 and P3\n";

void binToAscii(char *, size_t, size_t, FILE *, FILE *);
void asciiToBin(char *, size_t, FILE *, FILE *);
void copy(char *buffer, size_t size, FILE *, FILE *);

size_t getImageSizeBin(size_t, size_t, FILE *);
size_t getImageSizeAscii(size_t, size_t, char *, size_t, FILE *);
size_t sizeFit(char *buffer, size_t size, FILE *const file);

int main(int argc, char *argv[]) {
  if (argc == 1) {
        printf("%s", cli_help_text);
    }

    if (argc != 4) {
        printf("ERROR: Incorrect number of parameters\n");
        exit(0);
    }

    char *format = malloc(3);
    if (!strcmp(argv[1], "3"))
        format = "P3";
    else if (!strcmp(argv[1], "6"))
        format = "P6";

    char *inPath = argv[2];
    char *outPath = argv[3];

    FILE *inFile = fopen(inPath, "rb");
        if ( !inFile ) {
            fprintf( stderr, "ERROR: Failed to open file %s\n", inPath);
            fclose(inFile);
            return -1;
        }

    FILE *outFile = fopen(outPath, "w");
    char *buffer = (char *)malloc(sizeof(char) * INIT_BUFF_SIZE);

    if ( !buffer ) {
      fprintf(stderr, "ERROR: Failed to allocate buffer\n");
      fclose(outFile);
      fclose(inFile);
      return -1;
    }

    // get the magic number
    fgets(buffer, INIT_BUFF_SIZE, inFile);
    buffer[2] = '\0';

    char *file_format = malloc(strlen(buffer));
    strcpy(file_format, buffer);
    printf("File Format = %s -> %s\n", file_format, format);

    if (strcmp(file_format, "P3") && strcmp(file_format, "P6")) {
      fprintf(stderr, "ERROR: This is not a valid format\n");
      fclose(outFile);
      fclose(inFile);
      return -1;
    }

    fgets(buffer, INIT_BUFF_SIZE, inFile);
    while (buffer[0] == '#') {
      fgets(buffer, INIT_BUFF_SIZE, inFile);
    }

   char *split = strtok(buffer, " ");
    if (!split) {
      fprintf(stderr, "ERROR: bad width");
      fclose(outFile);
      fclose(inFile);
      return -1;
    }

    size_t width = strtol(split, NULL, 10);
    split = strtok(NULL, " ");
    if (!split) {
      fprintf(stderr, "ERROR: bad height");
      fclose(outFile);
      fclose(inFile);
      return -1;
    }

    size_t height = strtol(split, NULL, 10);
  printf("Dimensions = %ld %ld\n", width, height);

  fgets(buffer, INIT_BUFF_SIZE, inFile);
  size_t maxVal = strtol(buffer, NULL, 10);

  printf("Maximum Value = %ld\n", maxVal);

  if (maxVal > 255) {
    fprintf(stderr, "ERROR: This image requires multi-byte channels (unsupported)");
    return -1;
  }

  if (maxVal < 1) {
    fprintf(stderr, "ERROR: Max value must be > 1\n");
    fclose(outFile);
    fclose(inFile);
    return -1;
  }

  // realloc buffer to read one line at a time
  size_t data_buffer_size = width * 3;
  buffer = realloc(buffer, data_buffer_size);
  if (!buffer) {
    fprintf(stderr, "ERROR: Not enough memory to open image\n");
    fclose(outFile);
    fclose(inFile);
    return -1;
  }

  // check if file size is correct ------------------------------
  size_t size = 0;

  if (strcmp(file_format, "P6") == 0) {
    if (!(size = getImageSizeBin(width, height, inFile))) {
      fprintf(stderr, "ERROR: Real image size does not match header.\n");
      fclose(outFile);
      fclose(inFile);
      return -1;
    }
  }

  if (strcmp(file_format, "P3") == 0) {
    if (!(size = getImageSizeAscii(width, height, buffer, INIT_BUFF_SIZE,
                                   inFile))) {
      fprintf(stderr, "ERROR: Real image size does not match header.\n");
      fclose(outFile);
      fclose(inFile);
      return -1;
    }
  }
  fputs(format, outFile);
  sprintf(buffer, "\n%ld %ld\n", width, height);
  fputs(buffer, outFile);
  sprintf(buffer, "%ld\n", maxVal);
  fputs(buffer, outFile);

  // attach pixel data
  if (strcmp(file_format, format) == 0) {
        copy(buffer, data_buffer_size, inFile, outFile);
  } else if (strcmp(format, "P3") == 0) {
        binToAscii(buffer, width, height, inFile, outFile);
  } else if (strcmp(format, "P6") == 0) {
      asciiToBin(buffer, data_buffer_size, inFile, outFile);
  } else {
    printf("ERROR:Invalid format (P3/P6 only)\n");
  }
}

size_t getImageSizeBin(size_t width, size_t height, FILE *file_to_check) {
  fpos_t original_pos = 0;
  fpos_t end_pos = 0;

  fgetpos(file_to_check, &original_pos);
  fseek(file_to_check, 0, SEEK_END);
  fgetpos(file_to_check, &end_pos);

  if (end_pos < original_pos || end_pos - original_pos != width * height * 3) {
    printf("ERROR: Image does not have a correct size\n");
    return (int)NULL;
  }

  fsetpos(file_to_check, &original_pos);
  return end_pos - original_pos;
}

size_t getImageSizeAscii(size_t width, size_t height, char *buffer,
                         size_t size, FILE *file_to_check) {
  fpos_t original_pos = 0;
  char *token = NULL;
  size_t num_toks = 0;

  fgetpos(file_to_check, &original_pos);
    sizeFit(buffer, size, file_to_check);

  while (fgets(buffer, size, file_to_check) != NULL) {
    token = strtok(buffer, " \n");
    while (token) {
      num_toks++;
      token = strtok(NULL, " \n");
    } // end of while loop
  }   // end of outer while loop

  fsetpos(file_to_check, &original_pos);

  if (num_toks != width * height * 3) {
    return 0;
  } else {
    return num_toks;
  }
}
size_t sizeFit(char *buffer, size_t size, FILE *const file) {
  fpos_t pos = 0;
  int c = 0;
  int count = 0;
  count = 0;

  fgetpos(file, &pos);
  while (true) {
    c = fgetc(file);
    if (c == EOF || c == '\n')
      break;
    ++count;
  }
  fsetpos(file, &pos);

  if (count > size) {
    buffer = realloc(buffer, count + 1);
    size = count + 1;
  }

  return size;
}

void binToAscii(char *buffer, size_t width, size_t height, FILE *input_file, FILE *output_file) {

  unsigned char *unint8_buffer = (unsigned char *)buffer;

  for (size_t i = 0; i < height; i++) {
    fread(unint8_buffer, width * 3, 1, input_file);
    for (size_t j = 0; j < width; j++) {
      sprintf(buffer, "%u ", unint8_buffer[3 * j]);
      fputs(buffer, output_file);
      sprintf(buffer, "%u ", unint8_buffer[3 * j + 1]);
      fputs(buffer, output_file);
      sprintf(buffer, "%u\n", unint8_buffer[3 * j + 2]);
      fputs(buffer, output_file);
    }
  }
}

void asciiToBin(char *buffer, size_t size, FILE *input_file,
                FILE *output_file) {
  char *token = NULL;
  static unsigned char val = 0;

  size = sizeFit(buffer, size, input_file);

  while (fgets(buffer, size, input_file)) {
    token = strtok(buffer, " \n");
    while (token) {
      val = (unsigned char)atoi(token);
      fputc(val, output_file);
      token = strtok(NULL, "  \n");
    }
  }
}

void copy(char *buffer, size_t size, FILE *input_file, FILE *output_file) {
  while (fread(buffer, size, 1, input_file))
    fwrite(buffer, size, 1, output_file);
}