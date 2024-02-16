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

#define array_index_is_valid(array,index)       ((index) < array_size(array))
#define array_index_is_invalid(array,index)     ((index) >= array_size(array))
#define array_is_empty(array)                   (array_size(array) == 0)
#define array_is_not_empty(array)               (array_size(array) > 0)
#define array_at(array, index)                  ((array)[(index)])
#define array_last(array)                       (array_at((array),(array_size(array) - 1)))
#define array_for(array,index)                  for (size_t (index) = 0; (index) < array_size(array); (index)++)
#define array_for_each(array,it)                for (__typeof__(array) (it) = (array); (it) <= &array_last(array); (it)++)

// Remove element at the end of the array
#define array_pop(array)                        (array_at((array),(--array_size(array))))

// Insert element at the end of the array
#define array_push(array, value) \
    (((array) = (++array_size(array) >= array_capacity(array)) ? array_resize((array), 2*array_capacity(array)) : (array)), \
    (array_last(array) = (value)))


// Remove element from the beginning of the array (nothing is returned)
#define array_shift(array) \
    do { \
        if (array_is_not_empty(array)) { \
            memmove((array), &array_at((array), 1), (array_size(array) - 1)*sizeof(*(array))); \
            array_size(array)--; \
        } \
    } while (0)


// Insert element in the beginning of the array
#define array_unshift(array, value) \
    (((array) = (++array_size(array) >= array_capacity(array)) ? array_resize((array), 2*array_capacity(array)) : (array)), \
    memmove(&array_at((array) ,1), (array), (array_size(array) - 1)*sizeof(*(array))), \
    array_at(array, 0) = (value))

// Insert element at a specified position in the array
#define array_insert_at(array, index, value) \
    ((array) = _array_insert_zero_at((array), sizeof(*(array)), (index)), \
    array_at((array), (index)) = (value))

// Remove element at a specified position in the array (nothing is returned)
#define array_remove_at(array, index) \
    do { \
        if (array_index_is_valid((array), (index))) { \
            memmove(&array_at((array), (index)), &array_at((array), (index)+1), (array_size(array) - ((index)+1))*sizeof(*(array))); \
            array_size(array)--; \
        } \
    } while (0)

// This function type is used for comparing elements in both sorting and search functions.
// It should return an integer less than zero if the first argument is considered smaller,
// zero if they are deemed equal, and greater than zero if the first argument is greater
// than the second.
typedef int (*Compare_Fcn)(const void *, const void *);

#define array_sequential_search(array,key,compare) \
    _array_sequential_search((array), sizeof(*(array)), (key), (compare)) 
#define array_binary_search(array,key,compare) \
    _array_binary_search((array), sizeof(*(array)), (key), (compare))
#define array_qsort(array,compare) \
    qsort((array), array_size(array), sizeof(*array), (compare))

// Insert a new element in the array while keeping it sorted
// It returns 1 if a new element was inserted, and 0 if that element is already in the array
#define array_insert_sorted(array,element,compare,index) \
    _array_insert_sorted((void **)&(array), sizeof(*(array)), (element), (compare), (index))

#define array_address_at(array,index) \
    array_compute_address_at((array),sizeof(*(array)),(index))
#define array_compute_address_at(array,element_size,index) \
    ((void *)((size_t)(array) + (index)*(element_size)))

/*** Function prototypes ***/

// This functions shouldn't be called directly, insted use the macros defined above
void *_array_new(size_t element_size, size_t initial_capacity)
    __attribute__((warn_unused_result));
void *_array_resize(void *array, size_t element_size, size_t new_capacity)
    __attribute__((warn_unused_result));
size_t _array_sequential_search(const void *array, size_t element_size, const void *key, Compare_Fcn compare)
    __attribute__((warn_unused_result));
size_t _array_binary_search(const void *array, size_t element_size, const void *key, Compare_Fcn compare)
    __attribute__((warn_unused_result));

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

// This function shouldn't be called directly, insted use the macro array_insert_at
void *_array_insert_zero_at(void *array, size_t element_size, size_t index) {
    size_t new_size = array_index_is_valid(array, index) ? (array_size(array)+1) : ((index)+1);
    if (new_size >= array_capacity(array)) {
        size_t new_capacity = array_capacity(array);
        while (new_size >= new_capacity) {
            new_capacity *= 2;
        }
        array = _array_resize(array, element_size, new_capacity);
        if (array != NULL) {
            void *end = array_compute_address_at(array, element_size, array_size(array));
            size_t length = (new_size - (array_size(array)+1));
            memset(end, 0, length*element_size);
        }
    }
    if (array != NULL) {
        void *actual = array_compute_address_at(array, element_size, index);
        if (array_index_is_valid((array), (index))) {
            void *next = array_compute_address_at(array, element_size, (index+1));
            size_t length = (array_size(array) - ((index)));
            memmove(next, actual, length*element_size);
        }
        memset(actual, 0, element_size);
        array_size(array) = new_size;
    }
    return array;
}

// This function shouldn't be called directly, insted use the macro array_sequential_search
// It returns an invalid index in case the key wasn't found
size_t _array_sequential_search(const void *array, size_t element_size, const void *key, Compare_Fcn compare) {
    for (size_t i = 0; i < array_size(array); i++) {
        const void *it = array_compute_address_at(array, element_size, i);
        if (!compare(key, it)) {
            return i;
        }
    }
    return (size_t)-1;
}

// This function shouldn't be called directly, insted use the macro array_binary_search
// It returns an invalid index in case the key wasn't found
size_t _array_binary_search(const void *array, size_t element_size, const void *key, Compare_Fcn compare) {
    size_t index;
    size_t low = 0;
    size_t high = array_size(array) - 1;
    while ((low <= high) && array_index_is_valid(array, high)) {
        index = low + (high - low) / 2; // middle
        const void *it = array_compute_address_at(array, element_size, index);
        const int comp = compare(key, it);
        if (comp < 0) {
            high = index - 1;
        } else if (comp > 0) {
            low = index + 1;
        } else {
            return index;
        }
    }
    // Didn't found the key
    // Return an invalid index, but with enought information so that _array_insert_sorted
    // can recover the ideal position for the element to be inserted
    index = (size_t)-1*(low+1);
    return index;
}

// This function shouldn't be called directly, insted use the macro array_insert_sorted
// It returns 1 if a new element was inserted, and 0 if that element is already in the array
int _array_insert_sorted(void **array, size_t element_size, const void *element, Compare_Fcn compare, size_t *const user_index) {
    size_t index = _array_binary_search(*array, element_size, element, compare);
    if (array_index_is_valid(*array, index)) {
        if (user_index != NULL) {
            *user_index = index;
        }
        return 0;
    }
    // Insert new element
    index = (size_t)(-1L*(long)index - 1);
    *array = _array_insert_zero_at(*array, element_size, index);
    void *it = array_compute_address_at(*array, element_size, index);
    memcpy(it, element, element_size);
    if (user_index != NULL) {
        *user_index = index;
    }
    return 1;
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
