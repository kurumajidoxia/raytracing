#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "primitives.h"
#include "raytracing.h"

#define OUT_FILENAME "out.ppm"

static void write_to_ppm(FILE *outfile, uint8_t *pixels,
                         int width, int height)
{
    fprintf(outfile, "P6\n%d %d\n%d\n", width, height, 255);
    fwrite(pixels, 1, height * width * 3, outfile);
}

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

int main(int argc,char *argv[])
{
    if(!argv[1]) exit(-2);
    int P_NUM=atoi(argv[1]);
    uint8_t *pixels;
    light_node lights = NULL;
    rectangular_node rectangulars = NULL;
    sphere_node spheres = NULL;
    //color background = { 0.0, 0.1, 0.1 };
    struct timespec start, end;
    pthread_t thread[P_NUM];
    raytracing_para *raytracing_arg[P_NUM];

#include "use-models.h"

    /* allocate by the given resolution */
    pixels = malloc(sizeof(unsigned char) * ROWS * COLS * 3);
    if (!pixels) exit(-1);
    for(int i=0; i<P_NUM; ++i) {
        raytracing_arg[i]= (raytracing_para*)malloc(sizeof(raytracing_para));
        if(!raytracing_arg[i]) exit(-1);
    }
    /*give parameters of each thread*/
    for(int i=0; i<P_NUM; ++i) {
        raytracing_arg[i]->pixels=pixels;
        raytracing_arg[i]->background_color[0]=0.0;
        raytracing_arg[i]->background_color[1]=0.1;
        raytracing_arg[i]->background_color[2]=0.1;
        raytracing_arg[i]->rectangulars=rectangulars;
        raytracing_arg[i]->spheres=spheres;
        raytracing_arg[i]->lights=lights;
        raytracing_arg[i]->view=&view;
        raytracing_arg[i]->width_begin=0;
        raytracing_arg[i]->width_end=ROWS;
        raytracing_arg[i]->height_begin=i*(COLS/P_NUM);
        raytracing_arg[i]->height_end=(i+1)*(COLS/P_NUM);
    }
    printf("# Rendering scene\n");
    /* do the ray tracing with the given geometry */
    clock_gettime(CLOCK_REALTIME, &start);
    /*create threads*/
    for(int i=0; i<P_NUM; ++i) {
        if(pthread_create(&thread[i],NULL,(void *)&raytracing,(void *)(raytracing_arg[i]))) {
            printf("error creating pthread[%d]\n",i);
            exit(EXIT_FAILURE);
        }
    }

    for(int i=0; i<P_NUM; ++i) {
        pthread_join(thread[i],NULL);
    }

    clock_gettime(CLOCK_REALTIME, &end);
    {
        FILE *outfile = fopen(OUT_FILENAME, "wb");
        write_to_ppm(outfile, pixels, ROWS, COLS);
        fclose(outfile);
    }

    delete_rectangular_list(&rectangulars);
    delete_sphere_list(&spheres);
    delete_light_list(&lights);
    free(pixels);
    for(int i=0; i<P_NUM; ++i) {
        free(raytracing_arg[i]);
    }
    printf("Done!\n");
    printf("Execution time of raytracing() : %lf sec\n", diff_in_second(start, end));
    return 0;
}
