// MIT License

// Copyright (c) 2024 CLECIO JUNG <clecio.jung@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//------------------------------------------------------------------------------
// HEADER
//------------------------------------------------------------------------------

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef __DYNAMIC_ARRAY
#define __DYNAMIC_ARRAY

// This file is a header only library. In order to include it's implementation,
// define the macro DYNAMIC_ARRAY_IMPLEMENTATION before including this file

// It is a classic implementation of dynamic arrays in C
// Be carefull, this implementation uses a lot of macros, and therefore,
// is not entirely type safe
// It can be used as a custom allocator, where the chunks of memory are all of the same size
// This approach is simple, fast, saves memory and avoids memory fragmentation

#define HEADER_SIZE                 (2*sizeof(size_t))

#define DEFAULT_INITIAL_CAPACITY    (512)

#define array_size(array)           ((size_t *)(array))[-1]
#define array_capacity(array)       ((size_t *)(array))[-2]

#define array_new_with_capacity(type,initial_capacity) \
    _array_new(sizeof(type), (initial_capacity))
#define array_resize(array, new_capacity) \
    _array_resize((array), sizeof(*(array)), (new_capacity))

#define array_new(type)     array_new_with_capacity(type, DEFAULT_INITIAL_CAPACITY)
#define array_clear(array)  (array_size(array) = 0)
#define array_delete(array) (free((void *)((char *)(array) - HEADER_SIZE)))

#define array_index_is_valid(array, index)      ((index) < array_size(array))
#define array_index_is_invalid(array, index)    ((index) >= array_size(array))
#define array_is_empty(array)                   (array_size(array) == 0)
#define array_is_not_empty(array)               (array_size(array) > 0)
#define array_at(array, index)                  ((array)[(index)])
#define array_last(array)                       (array_at((array),(array_size(array) - 1)))
#define array_pop(array)                        (array_at((array),(--array_size(array))))

#define array_push(array, value) \
    do { \
        array_size(array)++; \
        (array) = (array_size(array) >= array_capacity(array)) ? array_resize((array), 2*array_capacity(array)) : (array); \
        if ((array) != NULL) { \
            array_last(array) = (value); \
        } \
    } while (0)

// Remove element from the beginning
#define array_shift(array) \
    do { \
        if (array_is_not_empty(array)) { \
            memmove((array), &array_at((array), 1), (array_size(array) - 1)*sizeof(*(array))); \
            array_size(array)--; \
        } \
    } while (0)

// Insert element in the beginning
#define array_unshift(array, value) \
    do { \
        array_size(array)++; \
        (array) = (array_size(array) >= array_capacity(array)) ? array_resize((array), 2*array_capacity(array)) : (array); \
        if ((array) != NULL) { \
            memmove(&array_at((array) ,1), (array), (array_size(array) - 1)*sizeof(*(array))); \
            array_at(array, 0) = (value); \
        } \
    } while (0)

#define array_insert_at(array, index, value) \
    do { \
        size_t new_size = array_index_is_valid((array), (index)) ? (array_size(array)+1) : ((index)+1); \
        if (new_size >= array_capacity(array)) { \
            size_t new_capacity = array_capacity(array); \
            while (new_size >= new_capacity) new_capacity *= 2; \
            (array) = array_resize((array), new_capacity); \
            if ((array) != NULL) { \
                memset(&array_last(array), 0, (new_size - (array_size(array)+1))*sizeof(*(array))); \
            } \
        } \
        if ((array) != NULL) { \
            if (array_index_is_valid((array), (index))) { \
                memmove(&array_at((array),((index)+1)), &array_at((array),(index)), (array_size(array) - ((index)))*sizeof(*(array))); \
            } \
            array_size(array) = new_size; \
            array_at((array), (index)) = (value); \
        } \
    } while (0)

#define array_remove_at(array, index) \
    do { \
        if (array_index_is_valid((array), (index))) { \
            memmove(&array_at((array), (index)), &array_at((array), (index)+1), (array_size(array) - ((index)+1))*sizeof(*(array))); \
            array_size(array)--; \
        } \
    } while (0)

// Function prototypes
// This functions shouldn't be called directly, insted use the macros defined above
void *_array_new(size_t element_size, size_t initial_capacity) __attribute__((warn_unused_result));
void *_array_resize(void *array, size_t element_size, size_t new_capacity) __attribute__((warn_unused_result));

#endif  // __DYNAMIC_ARRAY

//------------------------------------------------------------------------------
// SOURCE
//------------------------------------------------------------------------------

#ifdef DYNAMIC_ARRAY_IMPLEMENTATION

// This function shouldn't be called directly, insted use the macros array_new or array_new_with_capacity
void *_array_new(size_t element_size, size_t initial_capacity) {
    void *p = malloc(initial_capacity * element_size + HEADER_SIZE);
    if (p == NULL) {
        return(NULL);
    }
    void *array = (char *)p + HEADER_SIZE;
    array_size(array) = 0;
    array_capacity(array) = initial_capacity;
    return(array);
}

// This function shouldn't be called directly, insted use the macro array_resize
void *_array_resize(void *array, size_t element_size, size_t new_capacity) {
    void *p = ((char *)array - HEADER_SIZE);
    void *new_p = realloc(p, new_capacity * element_size + HEADER_SIZE);
    if (new_p == NULL) {
        free(p);
        return(NULL);
    }
    void *new_array = (char *)new_p + HEADER_SIZE;
    array_capacity(new_array) = new_capacity;
    return(new_array);
}

#endif // DYNAMIC_ARRAY_IMPLEMENTATION

//------------------------------------------------------------------------------
// END
//------------------------------------------------------------------------------

// MIT License

// Copyright (c) 2024 CLECIO JUNG <clecio.jung@gmail.com>

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
