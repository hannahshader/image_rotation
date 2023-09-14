/******************************************************************************
*       ppmtrans.c
*       By: Aidan and Hannah*
*       2/24/2023
*
*       Comp40 Project 1: locality
*   
*       This is file uses blocked, row major, and column major mapps to 
*       perform various rotations on ppm images. Uses interfaces for storing
*       2d UArrays to access and rotate pixels in a Ppm. Times these
*       transformations and writes the resulting images to output.
*   
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "cputiming.h"

#define SET_METHODS(METHODS, MAP, WHAT) do {                    \
        methods = (METHODS);                                    \
        assert(methods != NULL);                                \
        map = methods->MAP;                                     \
        if (map == NULL) {                                      \
                fprintf(stderr, "%s does not support "          \
                                WHAT "mapping\n",               \
                                argv[0]);                       \
                exit(1);                                        \
        }                                                       \
} while (false)

static void
usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[-{row,col,block}-major] [filename]\n",
                        progname);
        exit(1);
}

void reinsert_pixel_0(int i, int j, A2Methods_UArray2 array2, 
                     A2Methods_Object *ptr, void *cl);
void reinsert_pixel_90(int i, int j, A2Methods_UArray2 array2, 
                      A2Methods_Object *ptr, void *cl);
void reinsert_pixel_180(int i, int j, A2Methods_UArray2 array2, 
                       A2Methods_Object *ptr, void *cl);
void reinsert_pixel_270(int i, int j, A2Methods_UArray2 array2, 
                       A2Methods_Object *ptr, void *cl);
void flip_pixel_horizontal(int i, int j, A2Methods_UArray2 array2, 
                          A2Methods_Object *ptr, void *cl);
void flip_pixel_vertical(int i, int j, A2Methods_UArray2 array2, 
                        A2Methods_Object *ptr, void *cl);
void flip_pixel_transpose(int i, int j, A2Methods_UArray2 array2, 
                         A2Methods_Object *ptr, void *cl);
void print_pixel(Pnm_ppm holdMethods, 
                A2Methods_UArray2 array, int col, int row);
void pop_new_array(Pnm_ppm *new_ppm, int arrwidth, int arr_height, int size, 
                  Pnm_ppm old_instance, int denominator);
void functionality_start_timer(FILE *time_file);

/*
*       Description: Parses the command line to rotate an image with a 
*       specified rotation. If specified, times the transformation. 
*
*       In/Out Expectations: takes a command line. Exits failure if files
*       don't open, otherwise, does nothing. Returns void. Writes transformed
*       file to standard output. Writes timing data from a specified file. 
*       Successful execution returns EXIT_SUCCESS.
*       
*/
int main(int argc, char *argv[]) 
{
        char *time_file_name = NULL;
        int   rotation       = 0;
        int   flip           = 0;
        int i;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map);

        FILE *fp = NULL;
        int time_file_provided = 0;
        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-row-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                } else if (strcmp(argv[i], "-flip-horizontal") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                        flip = 1;
                } else if (strcmp(argv[i], "-flip-vertical") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                        flip = 2;
                } else if (strcmp(argv[i], "-transpose") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                        flip = 3;
                } else if (strcmp(argv[i], "-col-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_col_major, 
                                    "column-major");
                } else if (strcmp(argv[i], "-block-major") == 0) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                } else if (strcmp(argv[i], "-rotate") == 0) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }
                        char *endptr;
                        rotation = strtol(argv[++i], &endptr, 10);
                        if (!(rotation == 0 || rotation == 90 ||
                            rotation == 180 || rotation == 270)) {
                                fprintf(stderr, 
                                        "Rotation must be 0, 90 180 or 270\n");
                                usage(argv[0]);
                        }
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }
                } else if (strcmp(argv[i], "-time") == 0) {
                        time_file_name = argv[++i];
                        time_file_provided = 1;
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
                                argv[i]);
                        usage(argv[0]);
                } else if (argc - i > 1) {
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {
                        fp = fopen(argv[i], "r");
                        
                        if (fp == NULL) {
                                fprintf(stderr, "Cannot open file\n");
                                exit(EXIT_FAILURE);
                        }
                        break;
                }
        }
        
        if (fp == NULL) fp = stdin;

        Pnm_ppm old_instance = Pnm_ppmread(fp, methods);
        int width = (old_instance->methods)->width(old_instance->pixels);
        int height = (old_instance->methods)->height(old_instance->pixels);
        int size = (old_instance->methods)->size(old_instance->pixels);
        int denominator = old_instance->denominator;
        
        Pnm_ppm new_ppm = malloc(sizeof(struct Pnm_ppm));
        
        CPUTime_T timer;
        timer = CPUTime_New();
        double time_used; 
        FILE *time_file = NULL;


        if (time_file_provided == 1){
                time_file = fopen(time_file_name, "w");
                assert(time_file);
                functionality_start_timer(time_file);
        }
        
        if (rotation == 90 || rotation == 270 || (flip == 3 && rotation == 0)) {
                pop_new_array(&new_ppm, height, width, size, old_instance, 
                             denominator);
                if (rotation == 90) {
                        CPUTime_Start(timer);
                        map(old_instance->pixels, reinsert_pixel_90, new_ppm);
                        time_used = CPUTime_Stop(timer);
                } else if (rotation == 270) {
                        CPUTime_Start(timer);
                        map(old_instance->pixels, reinsert_pixel_270, new_ppm);
                        time_used = CPUTime_Stop(timer);
                } else if (flip == 3 && rotation == 0) {
                        CPUTime_Start(timer);
                        map(old_instance->pixels, flip_pixel_transpose, 
                           new_ppm);
                        time_used = CPUTime_Stop(timer);
                }
        }
        else {
                pop_new_array(&new_ppm, width, height, size, old_instance, 
                             denominator);
                if (rotation == 180) {
                        CPUTime_Start(timer);
                        map(old_instance->pixels, reinsert_pixel_180, new_ppm);
                        time_used = CPUTime_Stop(timer);
                } else if (rotation == 0 && flip == 0) {
                        CPUTime_Start(timer);
                        map(old_instance->pixels, reinsert_pixel_0, new_ppm);
                        time_used = CPUTime_Stop(timer);
                } else if (flip == 1 && rotation == 0) {
                        CPUTime_Start(timer);
                        map(old_instance->pixels, flip_pixel_horizontal, 
                           new_ppm);
                        time_used = CPUTime_Stop(timer);
                } else if (flip == 2 && rotation == 0) {
                        CPUTime_Start(timer);
                        map(old_instance->pixels, flip_pixel_vertical, new_ppm);
                        time_used = CPUTime_Stop(timer);
                }
        }
        if (time_file_provided == 1) {
                fprintf(time_file, "Computed in %.0f nanoseconds\n", time_used);
                int total_pixels = width * height;
                fprintf(time_file, "Time/Pixel: %.0f nanoseconds\n", 
                time_used / total_pixels);
                CPUTime_Free(&timer);
                fclose(time_file);
        }
        
        FILE *op = stdout;
        Pnm_ppmwrite(op, new_ppm);
        Pnm_ppmfree(&old_instance);
        Pnm_ppmfree(&new_ppm);
        fclose(fp);
        return EXIT_SUCCESS;

}

/*
*       Description: Asserts the timer file opens. 
*
*       In/Out Expectations: takes a file. Exits failure if there the file
*       doesn't open, otherwise, does nothing. Returns void.
*/
void functionality_start_timer(FILE *time_file) {
        if (time_file == NULL) {
                fprintf(stderr, "Could not open file for reading");
                exit(EXIT_FAILURE);
        }
}

/*
*       Description: A function that populates a new Pnm_ppm type with
*       specified data fields from an old instance of a Pnm_ppm type 
*
*       In/Out Expectations: takes the new and old Pnm_ppms, and widths,
*       heights, and sizes from the old Pnm_ppm. Returns void.
*/
void pop_new_array(Pnm_ppm *new_ppm, int arr_width, int arr_height, int size, 
Pnm_ppm old_instance, int denominator) {
        A2Methods_UArray2 new_array = 
        old_instance->methods->new(arr_width, arr_height, size);
        (*new_ppm)->width = arr_width;
        (*new_ppm)->height = arr_height;
        (*new_ppm)->pixels = new_array;
        (*new_ppm)->methods = old_instance->methods;
        (*new_ppm)->denominator = denominator;
}

/*
*       Description: A function that maps an element of an old UArray2 to the
*       pixels member of a Pnm_ppm struct with a 0 degree rotation 
*       transformation 
*
*       In/Out Expectations: expects to take in the row and col of the old 
*       UArray2, where element to be transformed lies, the element to replace,
*       the old UArray2, and the new instance of Pnm_ppm with a UArray where 
*       the element will be transformed. Returns void.
*/
void reinsert_pixel_0(int i, int j, A2Methods_UArray2 array2, 
                     A2Methods_Object *ptr, void *cl) {
        Pnm_ppm new_array_ppm = (Pnm_ppm)cl;
        Pnm_rgb location_new_arr = new_array_ppm->methods->
                                   at(new_array_ppm->pixels, i, j);
        *location_new_arr = *((Pnm_rgb) ptr);
        (void)array2;
}

/*
*       Description: A function that maps an element of an old UArray2 to the
*       pixels member of a Pnm_ppm struct with a 90 degree rotation 
*       transformation 
*
*       In/Out Expectations: expects to take in the row and col of the old 
*       UArray2, where element to be transformed lies, the element to replace,
*       the old UArray2, and the new instance of Pnm_ppm with a UArray where 
*       the element will be transformed. Returns void.
*/
void reinsert_pixel_90(int i, int j, A2Methods_UArray2 array2, 
A2Methods_Object *ptr, void *cl) {
        Pnm_ppm new_array_ppm = (Pnm_ppm)cl;
        
        A2Methods_UArray2 new_array = (new_array_ppm)->pixels;
        A2Methods_UArray2 old_array = array2;

        int arr_width = new_array_ppm->methods->width(old_array);
        int arr_height = new_array_ppm->methods->height(old_array);

        Pnm_rgb location_new_arr = 
        new_array_ppm->methods->at(new_array, (arr_height - j - 1), i);
        *location_new_arr = *((Pnm_rgb) ptr);
        
        (void)arr_width;
}

/*
*       Description: A function that maps an element of an old UArray2 to the
*       pixels member of a Pnm_ppm struct with a 180 degree rotation 
*       transformation 
*
*       In/Out Expectations: expects to take in the row and col of the old 
*       UArray2, where element to be transformed lies, the element to replace,
*       the old UArray2, and the new instance of Pnm_ppm with a UArray where 
*       the element will be transformed. Returns void.
*/
void reinsert_pixel_180(int i, int j, A2Methods_UArray2 array2, 
                        A2Methods_Object *ptr, void *cl) {
        Pnm_ppm new_array_ppm = (Pnm_ppm)cl;
        A2Methods_UArray2 new_array = (new_array_ppm)->pixels;
        A2Methods_UArray2 old_array = array2;
        int arr_width = new_array_ppm->methods->width(old_array);
        int arr_height = new_array_ppm->methods->height(old_array);
        Pnm_rgb location_new_arr = new_array_ppm->methods->at(new_array, 
        (arr_width - i - 1), (arr_height - j - 1));
        *location_new_arr = *((Pnm_rgb) ptr);
}

/*
*       Description: A function that maps an element of an old UArray2 to the
*       pixels member of a Pnm_ppm struct with a horizontal rotation 
*       transformation 
*
*       In/Out Expectations: expects to take in the row and col of the old 
*       UArray2, where element to be transformed lies, the element to replace,
*       the old UArray2, and the new instance of Pnm_ppm with a UArray where 
*       the element will be transformed. Returns void.
*/
void flip_pixel_horizontal(int i, int j, A2Methods_UArray2 array2, 
                                A2Methods_Object *ptr, void *cl) {
        Pnm_ppm new_array_ppm = (Pnm_ppm)cl;
        A2Methods_UArray2 new_array = (new_array_ppm)->pixels;
        A2Methods_UArray2 old_array = array2;
        int arr_width = new_array_ppm->methods->width(old_array);
        int arr_height = new_array_ppm->methods->height(old_array);
        Pnm_rgb location_new_arr = new_array_ppm->methods->at(new_array, 
        (arr_width - i - 1), j);
        *location_new_arr = *((Pnm_rgb) ptr);
        (void) arr_height;
}

/*
*       Description: A function that maps an element of an old UArray2 to the
*       pixels member of a Pnm_ppm struct with a vertical rotation 
*       transformation 
*
*       In/Out Expectations: expects to take in the row and col of the old 
*       UArray2, where element to be transformed lies, the element to replace,
*       the old UArray2, and the new instance of Pnm_ppm with a UArray where 
*       the element will be transformed. Returns void.
*/
void flip_pixel_vertical(int i, int j, A2Methods_UArray2 array2, 
A2Methods_Object *ptr, void *cl) {
        Pnm_ppm new_array_ppm = (Pnm_ppm)cl;
        A2Methods_UArray2 new_array = (new_array_ppm)->pixels;
        A2Methods_UArray2 old_array = array2;
        int arr_width = new_array_ppm->methods->width(old_array);
        int arr_height = new_array_ppm->methods->height(old_array);
        Pnm_rgb location_new_arr = 
        new_array_ppm->methods->at(new_array, i, (arr_height - j - 1));
        *location_new_arr = *((Pnm_rgb) ptr);
        (void) arr_width;
}

/*
*       Description: A function that maps an element of an old UArray2 to the
*       pixels member of a Pnm_ppm struct with a transpose rotation 
*       transformation 
*
*       In/Out Expectations: expects to take in the row and col of the old 
*       UArray2, where element to be transformed lies, the element to replace,
*       the old UArray2, and the new instance of Pnm_ppm with a UArray where 
*       the element will be transformed. Returns void.
*/
void flip_pixel_transpose(int i, int j, A2Methods_UArray2 array2, 
A2Methods_Object *ptr, void *cl) {
        Pnm_ppm new_array_ppm = (Pnm_ppm)cl;
        A2Methods_UArray2 new_array = (new_array_ppm)->pixels;
        A2Methods_UArray2 old_array = array2;
        int arr_width = new_array_ppm->methods->width(old_array);
        int arr_height = new_array_ppm->methods->height(old_array);
        Pnm_rgb location_new_arr = new_array_ppm->methods->at(new_array, j, i);
        *location_new_arr = *((Pnm_rgb) ptr);
        (void) arr_width;
        (void) arr_height;
}

/*
*       Description: A function that maps an element of an old UArray2 to the
*       pixels member of a Pnm_ppm struct with a 270 degree rotation 
*       transformation 
*
*       In/Out Expectations: expects to take in the row and col of the old 
*       UArray2, where element to be transformed lies, the element to replace,
*       the old UArray2, and the new instance of Pnm_ppm with a UArray where 
*       the element will be transformed. Returns void.
*/
void reinsert_pixel_270(int i, int j, A2Methods_UArray2 array2, 
A2Methods_Object *ptr, void *cl) {
        Pnm_ppm new_array_ppm = (Pnm_ppm)cl;
        
        A2Methods_UArray2 new_array = (new_array_ppm)->pixels;
        A2Methods_UArray2 old_array = array2;

        int arr_width = new_array_ppm->methods->width(old_array);
        int arr_height = new_array_ppm->methods->height(old_array);

        Pnm_rgb location_new_arr = 
        new_array_ppm->methods->at(new_array, j, (arr_width - i - 1));
        *location_new_arr = *((Pnm_rgb) ptr);
        
        (void)arr_height;
}
