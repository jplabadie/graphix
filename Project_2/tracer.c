#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "PixTool.h"
#include "JSONParser.h"
#include "RayTracer.h"
#include "VectorMath.h"

/**
 * Finds the intersection with a plane by the formula provided in class/text
 *
 * @param Ro
 * @param Rd
 * @param position
 * @param normal
 * @return double representing the intersection
 */
double planeIntersection(double *Ro, double *Rd, double *position, double *normal) {
    // distance = dot(Po-Lo,N)/dot(L,N)
    V3 temp = malloc(sizeof(double) * 3);
    vectorSubtract(Ro, position, temp);
    double distance = vectorDot(normal, temp);

    double denominator = vectorDot(normal, Rd);
    distance = -(distance / denominator);
    if (distance > 0)
        return distance;

    return 0;
}

/**
 *  Finds the intersection with a sphere using the formula provided in class/text
 * @param Ro
 * @param Rd
 * @param Center
 * @param r
 * @return double representing the intersection
 */
double sphereIntersection( double *Ro, double *Rd, double *Center, double r ) {
  double a = (sqr(Rd[0]) + sqr(Rd[1]) + sqr(Rd[2]));
  double b = (2 * (Ro[0] * Rd[0] - Rd[0] * Center[0] + Ro[1] * Rd[1] -
                   Rd[1] * Center[1] + Ro[2] * Rd[2] - Rd[2] * Center[2]));
  double c = sqr(Ro[0]) - 2 * Ro[0] * Center[0] + sqr((Center[1]) + Ro[1]) -
             2 * Ro[1] * Center[1] + sqr(Center[1]) + sqr(Ro[2]) -
             2 * Ro[2] * Center[2] + sqr(Center[2]) - sqr(r);

  double det = sqr(b) - 4 * a * c;
  if (det < 0)
    return -1;

  det = sqrt(det);

  double t0 = (-b - det) / (2 * a);
  if (t0 > 0)
    return t0;

  double t1 = (-b + det) / (2 * a);
  if (t1 > 0)
    return t1;

  return -1;
}


/**
 *  Main
 *
 * @param argc
 * @param argv
 * @return 0 if success with all other values representing failure modes
 */
int main(int argc, char *argv[]) {

  // check if required args are present
  if (argc < 5) {
    fprintf(stderr, "Error: Not enough arguments\n");
    exit(1);
  }

  // get the width and height
  int imgWidth = strtol(argv[1], (char **)NULL, 10);
  int imgHeight = strtol(argv[2], (char **)NULL, 10);
  char *inputJson = argv[3];

  // open the output file
  FILE *outputPPM = fopen(argv[4], "wb");
  if (!outputPPM) {
    fprintf(stderr, "ERROR: Failed to open file %s\n", argv[4]);
    fclose(outputPPM);
    return -1;
  }

  // define/init colored pixel objects
  Pixel color = {.r = 0, .g = 0, .b = 0};
  Pixel black = {.r = 0, .g = 0, .b = 0};

  // populate the array of object pointers using readScene
  Object **objects = readScene(inputJson);

  double h = 2;
  double w = 2;
  int num_cams = 0;

  // discover the camera and read its values
  for (int i = 0; objects[i] != 0; i++) {
    if (objects[i]->type == 0) {
      w = objects[i]->Camera.width;
      h = objects[i]->Camera.height;
      num_cams++;
    }
  }

  // ensure 1 and only 1 camera
  if ( num_cams != 1 ) {
    fprintf( stderr, "ERROR: Incorrect number of cameras specified, must have exactly 1. Found: %d\n", num_cams );
  }

  double cx = 0;
  double cy = 0;

  int M = imgWidth;
  int N = imgHeight;

  Pixel *buffer = malloc(M * N * sizeof(Pixel));

  double pixheight = h / M;
  double pixwidth = w / N;

  for (int y = 0; y < M; y += 1) {
      for (int x = 0; x < N; x += 1) {
          double Ro[3] = {0, 0, 0};
          // Rd = normalize(P - Ro)
          double Rd[3] = {cx - (w / 2) + pixwidth * (x + 0.5),
                          -(cy - (h / 2) + pixheight * (y + 0.5)), 1};
          normalize(Rd);

          double max = INFINITY;
          bool shape_detected = false;
          for (int i = 0; objects[i] != 0; i++) {
              if (shape_detected)
                  break;

              double t = 0;
              switch (objects[i]->type) {

                  // Case when encountering a PLANE
                  case 2:
                      t = planeIntersection(Ro, Rd, objects[i]->Plane.position, objects[i]->Plane.normal);
                      if (t > 0 && t < max) {
                          shape_detected = true;
                          color = objects[i]->color;
                      }
                      break;

                      // Case when encountering a SPHERE
                  case 1:
                      t = sphereIntersection(Ro, Rd, objects[i]->Sphere.position, objects[i]->Sphere.radius);
                      if (t > 0 && t < max) {
                          shape_detected = true;
                          color = objects[i]->color;
                      }
                      break;

                      // Case when encountering the CAMERA
                  case 0:
                      break;

                  default: // switch should never default out (error)
                      exit(1);
              }

              if (t > 0 && t < max)
                  max = t;

              if (max > 0 && max != INFINITY) {
                  buffer[y * M + x] = color;
              } else {
                  buffer[y * M + x] = black;
              }
          }
      }
  }
    //write the resultant scene to file as a PPM image (this could be a frame in another context)
    bufferToBinary(buffer, M, N, outputPPM);
    return 0;
}
