#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "mem.h"
#include <assert.h>
#include "uarray.h"
#include "uarray2.h"
#include "uarray2b.h"

/*
*       Description: Defines the struct for a UArray2b_T.
*       Initializes parameters.
*/
struct UArray2b_T {
    int width, height, size, blocksize;
    UArray2_T UArray2;
};

/*
*       Description: Takes the dimensions width and height and element size 
*       and blocksize and innitializes a new UArray2b_T. Sets the underlying
*       Uarray2 to an array of arrays with the correct sizes.
*   
*       In/Out Expectations: The function uses asserts to ensure that the
*       parameters are valid. Results in an innitialized UArray2b_T.
*/
extern UArray2b_T UArray2b_new(int width, int height, int size, int blocksize) 
{

        assert(blocksize > 1 && width >= 0 && height >= 0 && 
                size > 0 && blocksize > 0);
        //create a UArray2 that holds the blocks
        int num_blocks_width = (width / blocksize) + 1; // add 1 to account for
                                                        // when division 
                                                        // rounds down
        int num_blocks_height = (height / blocksize) + 1; // add 1 to account 
                                                        // for when division 
                                                        // rounds down

        UArray2_T UArray2 = UArray2_new(num_blocks_width, num_blocks_height, 
                                        sizeof(UArray_T));
        assert(UArray2 != NULL);
        // fprintf(stderr, "get here");
        for (int width_iterator = 0; width_iterator < num_blocks_width; 
        width_iterator++) {
                for (int height_iterator = 0; height_iterator < 
                num_blocks_height; height_iterator++) {
                        //initializes each slot of UArray2 with an empty UArray
                        // (later to be filled with each block)
                        UArray_T *new_arr_location = UArray2_at(UArray2, 
                        width_iterator, height_iterator);
                        assert(new_arr_location != NULL);
                        *new_arr_location = UArray_new(blocksize * blocksize, 
                        size);
                        assert(new_arr_location != NULL);
                }
        }

        UArray2b_T Array = malloc(sizeof(struct UArray2b_T));
        assert(Array != NULL);
        Array->width = width;
        Array->height = height;
        Array->size = size;
        Array->blocksize = blocksize;
        // Array->counter = blocksize * blocksize;
        Array->UArray2 = UArray2;

        return Array; 
}

/*
*       Description: Takes the dimensions width and height and element size 
*       and innitializes a new UArray2b_T with a 64k blocksi. Sets the
*       underlying Uarray2 to an array of arrays with the correct sizes.
*       Uses Uarray2b_new.
*   
*       In/Out Expectations: The function uses asserts to ensure that the
*       parameters are valid. Results in an innitialized UArray2b_T.
*/
extern UArray2b_T UArray2b_new_64K_block(int width, int height, int size) {
        assert(width >= 0 && height >= 0 && size > 0);
        int total_bytes_block = 64 * 1024;
        int side_length_bytes = sqrt(total_bytes_block);
        int side_length_unit_size = side_length_bytes / size;
        UArray2b_T new_64_k = UArray2b_new(width, height, size, 
                                        side_length_unit_size);
        assert(new_64_k != NULL);
        return new_64_k;
}

/*
*       Description: Frees memory associated with a UArray2b struct. 
*   
*       In/Out Expectations: Takes a UArray2b, frees memory at each
*       slot in the array. Returns nothing
*/
extern void  UArray2b_free(UArray2b_T *array2b) {
        assert(array2b != NULL);
        UArray2_T UArray2 = (*array2b)->UArray2;
        for (int width_iterator = 0; width_iterator < 
        UArray2_width((*array2b)->UArray2); width_iterator++) {
        for (int height_iterator = 0; height_iterator < 
                UArray2_height((*array2b)->UArray2); height_iterator++) {
                UArray_T *new_arr_location = 
                UArray2_at(UArray2, width_iterator, height_iterator);
                UArray_free(new_arr_location);
        }
        }
        UArray2_free(&((*array2b)->UArray2));
        FREE(*array2b);
}

/*
*       Description: Provides the width of the 2D Blocked Array.
*   
*       In/Out Expectations: Takes a UArray2b_T and returns the width. It is a
*       checked runtime error for the array to be NULL.
*/
extern int   UArray2b_width    (UArray2b_T  array2b) {
    assert(array2b != NULL);
    return array2b->width;
}

/*
*       Description: Provides the height of the 2D Blocked Array.
*   
*       In/Out Expectations: Takes a UArray2b_T and returns the height. It is a
*       checked runtime error for the array to be NULL.
*/
extern int   UArray2b_height   (UArray2b_T  array2b) {
    assert(array2b != NULL);
    return array2b->height;
}

/*
*       Description: Provides the size of the cells of a 2D Blocked Array.
*   
*       In/Out Expectations: Takes a UArray2b_T and returns the int size. It is
*       a checked runtime error for the array to be NULL.
*/
extern int   UArray2b_size   (UArray2b_T  array2b) {
    assert(array2b != NULL);
    return array2b->size;
}

/*
*       Description: Provides the length of one block of the 2D Blocked Array.
*   
*       In/Out Expectations: Takes a UArray2b_T and returns the length. It is a
*       checked runtime error for the array to be NULL.
*/
extern int   UArray2b_blocksize(UArray2b_T  array2b) {
    assert(array2b != NULL);
    return array2b->blocksize;
}


/*
*       Description: gets the address of the element at a given row and column
*       of a Uarray2b.
*   
*       In/Out Expectations: Takes in UArray2b_T and a row and column index 
*       respectively and asserts that the array inputed is not null. Asserts 
*       row and column indexes are not less than 0. Returns the address of the
*       array2b at that index.
*/
extern void *UArray2b_at(UArray2b_T array2b, int column, int row) {
    assert(array2b != NULL);
    assert(column >= 0 && row >= 0);
    int blocksize = array2b->blocksize;
    int idx_in_blockarr_row = row / blocksize;
    int idx_in_blockarr_col = column / blocksize;

    UArray_T block_arr = *((UArray_T*) (UArray2_at(array2b->UArray2, 
    idx_in_blockarr_col, idx_in_blockarr_row)));

    return UArray_at(block_arr, 
    (column % blocksize) + (row % blocksize) * blocksize);
}


/*
*       Description: A map function which runs through the UArray2B_T provided 
*       in block order, meaning visiting in row major order all of the 
*       elements in a smaller square of the whole array, before visiting the 
*       next one (moving left to right, top to bottom).
*   
*       In/Out Expectations: Asserts the array is not null. Applies passed 
*       though function. Returns nothing.
*/
extern void  UArray2b_map(UArray2b_T array2b, 
                          void apply(int col, int row, UArray2b_T array2b,
                                     void *elem, void *cl), void *cl){
                                        
    assert(array2b != NULL);
    int ArrayRow, ArrayCol;
    int blocksize = array2b->blocksize;

    UArray2_T arr_of_blocks = array2b->UArray2;
    UArray_T one_block;


    for (int row = 0; row < UArray2_height(arr_of_blocks); row++) {
        for (int col = 0; col < UArray2_width(arr_of_blocks); col++) {
            one_block = *((UArray_T*) UArray2_at(arr_of_blocks, col, row));
            for (int blkIdx = 0; blkIdx < UArray_length(one_block); blkIdx++) {

                ArrayRow = (row * blocksize) + (blkIdx / blocksize);
                ArrayCol = (col * blocksize) + (blkIdx % blocksize);

                if (ArrayRow < UArray2b_height(array2b) &&
                    ArrayCol < UArray2b_width(array2b)) {
                        apply(ArrayCol, ArrayRow, array2b, UArray2b_at(array2b,
                             ArrayCol, ArrayRow), cl);
                }
            }
        }
    }
}