#ifndef _VECTOR_MATH_H_
#define _VECTOR_MATH_H_

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "VectorMath.h"
#include "RayTracer.h"

int line = 1;
const uint8_t INIT_NUM_OBJ = 64;

/**
 * Wrapper for the getc() func, adds error checking and line-number maintainance
 * @param json
 * @return
 */
int nextC(FILE *json) {
  int c = fgetc(json);
#ifdef DEBUG
  printf("nextC: '%c'\n", c);
#endif
  if (c == '\n') {
    line += 1;
  }
  if (c == EOF) {
    fprintf(stderr, "Error: Unexpected end of file on line number %d.\n", line);
    exit(1);
  }
  return c;
}

/**
 * Verifies that the next number is d, or throws an error and exits
 *
 * @param json
 * @param d the next expected number
 */
void expectC(FILE *json, int d) {
  int c = nextC(json);
  if (c == d)
    return;
  fprintf(stderr, "Error: Expected '%c' on line %d.\n", d, line);
  exit(1);
}

/**
 * Skips all whitespace at the current position in the file
 *
 * @param json
 */
void skipWs(FILE *json) {
  int c = nextC(json);
  while (isspace(c)) {
    c = nextC(json);
  }
  ungetc(c, json);
}

/**
 * Returns the next numerical string from the file, or throws an error and exits
 * @param json
 * @return
 */
char *nextString(FILE *json) {
  char buffer[129];
  int c = nextC(json);
  if (c != '"') {
    fprintf(stderr, "Error: Expected string on line %d.\n", line);
    exit(1);
  }
  c = nextC(json);
  int i = 0;
  while (c != '"') {
    if (i >= 128) {
      fprintf(stderr, "Error: Strings > 128 characters in length are not supported.\n");
      exit(1);
    }
    if (c == '\\') {
      fprintf(stderr, "Error: Strings containing escape codes are not supported.\n");
      exit(1);
    }
    if (c < 32 || c > 126) {
      fprintf(stderr, "Error: Strings containing non-ascii characters are not supported.\n");
      exit(1);
    }
    buffer[i] = c;
    i += 1;
    c = nextC(json);
  }
  buffer[i] = 0;
  return strdup(buffer);
}

/**
 * Returns the next number encountered in the input file
 * (no error checking implemented)
 *
 * @param json
 * @return
 */
double nextNumber(FILE *json) {
  double value;
  fscanf(json, "%lf", &value);
  //Todo: Error handling
  return value;
}

/**
 * Returns the next Vector value encountered in the input File
 * (no error checking implemented)
 *
 * @param json
 * @return
 */
double *nextVector(FILE *json) {
  double *v = malloc(3 * sizeof(double));
    expectC(json, '[');
    skipWs(json);
  v[0] = nextNumber(json);
    skipWs(json);
    expectC(json, ',');
    skipWs(json);
  v[1] = nextNumber(json);
    skipWs(json);
    expectC(json, ',');
    skipWs(json);
  v[2] = nextNumber(json);
    skipWs(json);
    expectC(json, ']');
  return v;
    //Todo: Error handling
}

/**
 * Loads the scene from a given file into an object struct array as defined in tracer.h
 *
 * @param filename
 * @return
 */
Object **readScene(char *filename) {
  int c;
  FILE *json = fopen(filename, "r");
  if (json == NULL) {
    fprintf(stderr, "Error: Could not open file \"%s\"\n", filename);
    exit(1);
  }

  Object **objArray;
  objArray = malloc(sizeof(Object *) * INIT_NUM_OBJ);

    skipWs(json);

  // Find the beginning of the list
    expectC(json, '[');
    skipWs(json);

  size_t curr_obj = 0;
  while (1) {
    objArray[curr_obj] = malloc(sizeof(Object));

    c = fgetc(json);
    if (c == ']') {
      fprintf(stderr, "Error: This is the worst scene file EVER.\n");
      fclose(json);
      return NULL;
    }
    if (c == '{') {
        skipWs(json);

      // Parse the object
      char *key = nextString(json);
      if (strcmp(key, "type") != 0) {
        fprintf(stderr, "Error: Expected \"type\" key on line number %d.\n",
                line);
        exit(1);
      }

        skipWs(json);
        expectC(json, ':');
        skipWs(json);

      char *value = nextString(json);

      if (strcmp(value, "camera") == 0) {
        objArray[curr_obj]->type = CAMERA;
      } else if (strcmp(value, "sphere") == 0) {
        objArray[curr_obj]->type = SPHERE;
      } else if (strcmp(value, "plane") == 0) {
        objArray[curr_obj]->type = PLANE;
      } else {
        fprintf(stderr, "Error: Unknown type, \"%s\", on line number %d.\n",
                value, line);
        exit(1);
      }

        skipWs(json);

      while (1) {
        c = nextC(json);
        if (c == '}') {
          // stop parsing this object
          break;
        } else if (c == ',') {
          // read another field
            skipWs(json);
          char *key = nextString(json);
            skipWs(json);
            expectC(json, ':');
            skipWs(json);

          // scalar types
          if (strcmp(key, "width") == 0) {
            double value = nextNumber(json);
            if (value < 0) {
              fprintf(stderr, "Error: Width cannot be less than 0. Found %lf "
                              "on line number %d.\n",
                      value, line);
              exit(1);
            }

            objArray[curr_obj]->Camera.width = value;

          } else if (strcmp(key, "height") == 0) {
            double value = nextNumber(json);
            if (value < 0) {
              fprintf(stderr, "Error: Height cannot be less than 0. Found %lf "
                              "on line number %d.\n",
                      value, line);
              exit(1);
            }

            objArray[curr_obj]->Camera.height = value;
          } else if (strcmp(key, "radius") == 0) {
            double value = nextNumber(json);
            if (value < 0) {
              fprintf(stderr, "Error: Radius cannot be less than 0. Found %lf "
                              "on line number %d.\n",
                      value, line);
              exit(1);
            }

            objArray[curr_obj]->Sphere.radius = value;
          }

          else if (strcmp(key, "color") == 0) {
            double *value = nextVector(json);
            objArray[curr_obj]->color.r = value[0];
            if (value[0] < 0 || value[1] < 0 || value[2] < 0) {
              fprintf(stderr, "Error: color values cannot be less than 0. "
                              "On line number %d.\n",
                      line);
              exit(1);
            }

            objArray[curr_obj]->color.g = value[1];
            objArray[curr_obj]->color.b = value[2];
          } else if (strcmp(key, "position") == 0) {
            double *value = nextVector(json);

            if (objArray[curr_obj]->type == PLANE) {
                vectorCopy(objArray[curr_obj]->Plane.position, value);
            } else if (objArray[curr_obj]->type == SPHERE) {
                vectorCopy(objArray[curr_obj]->Sphere.position, value);
            } else {
              fprintf(stderr, "Error: Unknown type, \"%d\", on line %d.\n",
                      objArray[curr_obj]->type, line);
              exit(1);
            }

          } else if (strcmp(key, "normal") == 0) {
            double *value = nextVector(json);
              vectorCopy(objArray[curr_obj]->Plane.normal, value);
          } else {
            fprintf(stderr, "Error: Unknown property, \"%s\", on line %d.\n",
                    key, line);
            exit(1);
          }

            skipWs(json);
        } else {
          fprintf(stderr, "Error: Unexpected value on line %d\n", line);
          exit(1);
        }
      }

        skipWs(json);
      c = nextC(json);
      if (c == ',') {
          skipWs(json);
      } else if (c == ']') {
        fclose(json);
        objArray[curr_obj + 1] = 0;
        return objArray;
      } else {
        fprintf(stderr, "Error: Expecting ',' or ']' on line %d.\n", line);
        exit(1);
      }
    }
    curr_obj++;
  }
}

#endif
