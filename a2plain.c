/******************************************************************************
*       a2plain.h
*       By: Aidan and Hannah*
*       2/13/2023
*
*       Comp40 Project 1: locality
*   
*       This file contains struct definitons and functions for instances of
*       A2Methods_UArray2.
*   
******************************************************************************/

#include <string.h>
#include <a2plain.h>
#include "uarray2.h"

typedef A2Methods_UArray2 A2;

/*
*       Description: Takes the dimensions width and height and element size 
*       respectively along with initializes a A2Methods_UArray2 struct. UArray2
*       allocates memory for the struct and members. 
*   
*       In/Out Expectations: Under the hood, UArray2 new holds neccesary 
*       assertions.
*/
static A2Methods_UArray2 new(int width, int height, int size)
{
        return UArray2_new(width, height, size);
}

/*
*       Description: Takes the dimensions width and height and element size and 
*       blocksize and initializes a A2Methods_UArray2 struct. UArray2
*       allocates memory for the struct and members. The blocksize is voided as 
*       it is not needed for instances of a2plain.
*   
*       In/Out Expectations: Under the hood, UArray2 new holds neccesary 
*       assertions.
*/
static A2Methods_UArray2 new_with_blocksize(int width, int height, int size,
                                            int blocksize)
{
        (void) blocksize;
        return UArray2_new(width, height, size);
}


/* TODO: ...many more private (static) definitions follow */

static void map_row_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_row_major(uarray2, (UArray2_applyfun*)apply, cl);
}

static void map_col_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_col_major(uarray2, (UArray2_applyfun*)apply, cl);
}

struct small_closure {
        A2Methods_smallapplyfun *apply; 
        void                    *cl;
};

static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
        struct small_closure *cl = vcl;
        (void)i;
        (void)j;
        (void)uarray2;
        cl->apply(elem, cl->cl);
}

static void small_map_row_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_row_major(a2, apply_small, &mycl);
}

static void small_map_col_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_col_major(a2, apply_small, &mycl);
}

static int width(A2 array2)
{
        return UArray2_width(array2);
}
static int height(A2 array2)
{
        return UArray2_height(array2);
}
static int size(A2 array2)
{
        return UArray2_size(array2);
}

static void a2free(A2 * array2p)
{
        UArray2_free((UArray2_T *) array2p);
}

static A2Methods_Object *at(A2 array2, int i, int j)
{
        return UArray2_at(array2, i, j);
}


/*
*       Description: defines a struct that holds the functionality that can be
*       performed with an instance of A2Methods_T when a2plain is inherited. 
*       Block major mapping is voided as it's not used in this interface.
*   
*       In/Out Expectations: Member variables hold functions.
*/
static struct A2Methods_T uarray2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        NULL,                   // blocksize
        at,
        map_row_major,
        map_col_major,
        NULL,                           // map_block_major
        map_row_major,                  // map_block_major (default)
        small_map_row_major,
        small_map_col_major,
        NULL,                           // small_map_block_major
        small_map_row_major,            // small_map_block_major (default)
};

// finally the payoff: here is the exported pointer to the struct

A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
