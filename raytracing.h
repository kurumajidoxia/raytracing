#ifndef __RAYTRACING_H
#define __RAYTRACING_H

#include "objects.h"
#include <stdint.h>
#include <pthread.h>

#define ROWS 512
#define COLS 512

typedef struct {
    uint8_t *pixels;
    color background_color;
    rectangular_node rectangulars;
    sphere_node spheres;
    light_node lights;
    const viewpoint *view;
    int width_begin;
    int width_end;
    int height_begin;
    int height_end;
} raytracing_para;

void raytracing(void *para);

/*void raytracing(uint8_t *pixels, color background_color,
                rectangular_node rectangulars, sphere_node spheres,
                light_node lights, const viewpoint *view,
                int width,int height);*/
#endif
