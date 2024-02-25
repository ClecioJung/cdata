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

#ifndef CDATA_NO_STDLIB
#include <stddef.h> // size_t
#include <stdlib.h> // realloc, free, qsort
#include <string.h> // memset, memmove, memcpy, strlen
#endif // CDATA_NO_STDLIB

#ifndef __CDATA_HEADER_ONLY_LIBRARY
#define __CDATA_HEADER_ONLY_LIBRARY

// This file is a header only library. In order to include it's implementation,
// define the macro CDATA_IMPLEMENTATION before including this file

// It is a classic implementation of dynamic arrays and hash tables in C
// Be carefull, this implementation uses a lot of macros, and therefore,
// is not entirely type safe

//------------------------------------------------------------------------------
// General Configuration

#ifndef GROWTH_FACTOR
#define GROWTH_FACTOR               (2)
#endif
#if (GROWTH_FACTOR <= 1)
#error "The GROWTH_FACTOR should be greater than one!"
#endif

#ifndef ARRAY_DEFAULT_CAPACITY
#define ARRAY_DEFAULT_CAPACITY            (512)
#endif
#if (ARRAY_DEFAULT_CAPACITY <= 1)
#error "The ARRAY_DEFAULT_CAPACITY should be greater than one!"
#endif

#ifndef HASH_TABLE_DEFAULT_CAPACITY
#define HASH_TABLE_DEFAULT_CAPACITY            (512)
#endif
#if (HASH_TABLE_DEFAULT_CAPACITY <= 1)
#error "The HASH_TABLE_DEFAULT_CAPACITY should be greater than one!"
#endif

// Load factor for hash tables
#ifndef LOAD_FACTOR_NUMERATOR
#define LOAD_FACTOR_NUMERATOR       (1)
#endif
#ifndef LOAD_FACTOR_DENOMINATOR
#define LOAD_FACTOR_DENOMINATOR     (2)
#endif
#if (LOAD_FACTOR_NUMERATOR <= 0)
#error "The LOAD_FACTOR_NUMERATOR should be greater than zero!"
#endif
#if (LOAD_FACTOR_DENOMINATOR <= 0)
#error "The LOAD_FACTOR_DENOMINATOR should be greater than zero!"
#endif
#if (LOAD_FACTOR_NUMERATOR >= LOAD_FACTOR_DENOMINATOR)
#error "The load factor (LOAD_FACTOR_NUMERATOR/LOAD_FACTOR_DENOMINATOR) should be lower than one!"
#endif

// The default is quadratic probing
#if !defined(QUADRATIC_PROBING) && !defined(LINEAR_PROBING)
#define QUADRATIC_PROBING
#endif

#ifndef ARENA_DEFAULT_REGION_CAPACITY
#define ARENA_DEFAULT_REGION_CAPACITY       4096
#endif

// Custom function modifier
#ifndef CDATA_FCN_DEF
#define CDATA_FCN_DEF
#endif

#ifndef CDATA_ASSERT
#ifdef CDATA_DEBUG
#include <assert.h>
#define CDATA_ASSERT(x)             assert(x)
#else
#define CDATA_ASSERT(x)             ((void)0)
#endif
#endif

#ifndef CDATA_NO_STDLIB

#if (defined(CDATA_REALLOC) != defined(CDATA_FREE))
#error "You should define both CDATA_REALLOC and CDATA_FREE!"
#endif
#ifndef CDATA_REALLOC
#define CDATA_REALLOC(ptr,size)     realloc((ptr),(size))
#endif
#ifndef CDATA_FREE
#define CDATA_FREE(ptr)             free(ptr)
#endif

#ifndef CDATA_QSORT
#define CDATA_QSORT(ptr,n,size,cmp) qsort((ptr),(n),(size),(cmp)) 
#endif

#ifndef CDATA_MEMSET
#define CDATA_MEMSET(str,c,size)    memset((str),(c),(size))
#endif
#ifndef CDATA_MEMMOVE
#define CDATA_MEMMOVE(dst,src,size) memmove((dst),(src),(size))
#endif
#ifndef CDATA_MEMCPY
#define CDATA_MEMCPY(dst,src,size)  memcpy((dst),(src),(size))
#endif
#ifndef CDATA_STRLEN
#define CDATA_STRLEN(str)           strlen((str))
#endif

#else // CDATA_NO_STDLIB

#ifndef CDATA_REALLOC
#error "CDATA_REALLOC must be defined if CDATA_NO_STDLIB is used!"
#endif
#ifndef CDATA_FREE
#error "CDATA_FREE must be defined if CDATA_NO_STDLIB is used!"
#endif

#ifndef CDATA_MEMSET
#error "CDATA_MEMSET must be defined if CDATA_NO_STDLIB is used!"
#endif
#ifndef CDATA_MEMMOVE
#error "CDATA_MEMMOVE must be defined if CDATA_NO_STDLIB is used!"
#endif
#ifndef CDATA_MEMCPY
#error "CDATA_MEMCPY must be defined if CDATA_NO_STDLIB is used!"
#endif
#ifndef CDATA_STRLEN
#error "CDATA_STRLEN must be defined if CDATA_NO_STDLIB is used!"
#endif

#endif // CDATA_NO_STDLIB

#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(a)
#define warn_unused_result
#define nonnull
#endif

#if defined(__GNUC__) || defined(__clang__)
#define CDATA_TYPEOF_SUPPORTED 
#endif

//------------------------------------------------------------------------------
// General Definitions

// Useful macro to get the size of static arrays
#define STATIC_ARRAY_SIZE(array)    (sizeof(array)/sizeof(array[0]))

#define INT_DIV_ROUND_UP(num,den)   (((num) + (den) - 1)/(den))
#define INT_ROUND_UP(value,unit)    (INT_DIV_ROUND_UP((value),(unit))*(unit))
#define INT_MIN(a,b)                ((a)<(b)?(a):(b))
#define INT_MAX(a,b)                ((a)>(b)?(a):(b))

// Bit operations
#define TEST_BIT(value,bit)         ((value) &  (1L << (bit)))
#define SET_BIT(value,bit)          ((value) |= (1L << (bit)))
#define CLEAR_BIT(value,bit)        ((value) &= (~(1L << (bit))))
#define TOOGLE_BIT(value,bit)       ((value) ^  (1L << (bit)))

#ifdef __cplusplus
#define ERROR(msg)                  static_assert(0, (msg))
#else
#define ERROR(msg)                  _Static_assert(0, (msg))
#endif

// This function type is used for comparing elements in both sorting and search functions.
// It should return an integer less than zero if the first argument is considered smaller,
// zero if they are deemed equal, and greater than zero if the first argument is greater
// than the second.
typedef int (*Compare_Fcn)(const void *, const void *);

//------------------------------------------------------------------------------
// Dynamic array

#define ARRAY_HEADER_SIZE                       (2*sizeof(size_t))

#define array_size(array)                       ((size_t *)(array))[-1]
#define array_capacity(array)                   ((size_t *)(array))[-2]

#define array_clear(array) \
    do { \
        if ((array) != NULL) { \
            array_size(array) = 0; \
        } \
    } while (0)
#define array_delete(array) \
    do { \
        if ((array) != NULL) { \
            CDATA_FREE((void *)((char *)(array) - ARRAY_HEADER_SIZE)); \
        } \
    } while (0)

#define array_index_is_valid(array,index)       (((array) != NULL) && ((index) < array_size(array)))
#define array_index_is_invalid(array,index)     (((array) == NULL) || ((index) >= array_size(array)))
#define array_is_empty(array)                   (((array) == NULL) || (array_size(array) == 0))
#define array_is_not_empty(array)               (((array) != NULL) && (array_size(array) > 0))
#define array_at(array,index)                   ((array)[(index)])
#define array_last(array)                       (array_at((array),(array_size(array) - 1)))
#define array_for(array,index)                  for (size_t (index) = 0; ((array) != NULL) && ((index) < array_size(array)); (index)++)

#ifdef CDATA_TYPEOF_SUPPORTED
#define array_for_each(array,it)                for (__typeof__(array) (it) = (array); ((array) != NULL) && ((it) <= &array_last(array)); (it)++)
#else
#define array_for_each(array,it)                ERROR("array_for_each is not supported for this compiler!")
#endif

// Remove element at the end of the array
#define array_pop(array)                        (array_at((array),(--array_size(array))))

// Insert element at the end of the array
#define array_push(array,value) \
    (((array) = _array_resize_if_needed((array), sizeof(*array), 1)), \
    CDATA_ASSERT((array) != NULL), \
    array_size(array)++, \
    (array_last(array) = (value)))

#define array_push_raw(array,value,element_size) \
    do { \
        ((array) = _array_resize_if_needed((array), (element_size), 1)), \
        CDATA_ASSERT((array) != NULL); \
        array_size(array)++, \
        CDATA_MEMCPY(array_compute_address_at((array), (element_size), array_size(array)-1), (value), element_size); \
    } while (0)

// Remove element from the beginning of the array (nothing is returned)
#define array_shift(array) \
    do { \
        if (array_is_not_empty(array)) { \
            CDATA_MEMMOVE((array), &array_at((array), 1), (array_size(array) - 1)*sizeof(*(array))); \
            array_size(array)--; \
        } \
    } while (0)

// Insert element in the beginning of the array
#define array_unshift(array, value) \
    (((array) = _array_resize_if_needed((array), (sizeof(*array)), 1)), \
    CDATA_ASSERT((array) != NULL), \
    array_size(array)++, \
    CDATA_MEMMOVE(&array_at((array) ,1), (array), (array_size(array) - 1)*sizeof(*(array))), \
    array_at(array, 0) = (value))

// Insert element at a specified position in the array
#define array_insert_at(array, index, value) \
    ((array) = _array_insert_zero_at((array), sizeof(*(array)), (index)), \
    CDATA_ASSERT((array) != NULL), \
    array_at((array), (index)) = (value))

// Remove element at a specified position in the array (nothing is returned)
#define array_remove_at(array, index) \
    do { \
        if (array_index_is_valid((array), (index))) { \
            CDATA_MEMMOVE(&array_at((array), (index)), &array_at((array), (index)+1), (array_size(array) - ((index)+1))*sizeof(*(array))); \
            array_size(array)--; \
        } \
    } while (0)

#define array_sequential_search(array,key,compare) \
    _array_sequential_search((array), sizeof(*(array)), (key), (compare)) 
#define array_binary_search(array,key,compare) \
    _array_binary_search((array), sizeof(*(array)), (key), (compare))

#if defined(CDATA_NO_STDLIB) && !defined(CDATA_QSORT)
#define array_qsort(array,compare)      ERROR("CDATA_QSORT was not defined!")
#else
#define array_qsort(array,compare) \
    CDATA_QSORT((array), array_size(array), sizeof(*array), (compare))
#endif

// Insert a new element in the array while keeping it sorted
// It returns 1 if a new element was inserted, and 0 if that element is already in the array
#define array_insert_sorted(array,value,compare,index) \
    _array_insert_sorted((void **)&(array), sizeof(*(array)), (value), (compare), (index))

#define array_address_at(array,index) \
    array_compute_address_at((array),sizeof(*(array)),(index))
#define array_compute_address_at(array,element_size,index) \
    ((void *)((size_t)(array) + (index)*(element_size)))

#ifdef __cplusplus
extern "C" {
#endif

// This functions shouldn't be called directly, insted use the macros defined above
CDATA_FCN_DEF size_t round_up_2(size_t value)
    __attribute__((warn_unused_result));
CDATA_FCN_DEF void *_array_resize(void *array, size_t element_size, size_t header_size, size_t new_capacity)
    __attribute__((warn_unused_result));
CDATA_FCN_DEF void *_array_resize_if_needed(void *array, size_t element_size, size_t size_to_add)
    __attribute__((warn_unused_result));
CDATA_FCN_DEF size_t _array_sequential_search(const void *array, size_t element_size, const void *key, Compare_Fcn compare)
    __attribute__((warn_unused_result, nonnull(3,4)));
CDATA_FCN_DEF size_t _array_binary_search(const void *array, size_t element_size, const void *key, Compare_Fcn compare)
    __attribute__((warn_unused_result, nonnull(3,4)));
CDATA_FCN_DEF int _array_insert_sorted(void **array, size_t element_size, const void *element, Compare_Fcn compare, size_t *const user_index)
    __attribute__((nonnull(1,3)));

#ifdef __cplusplus
}
#endif

//------------------------------------------------------------------------------
// Hash Table using open adressing (with linear or quadratic probing)

#define hash_table_capacity(hash_table)             array_capacity(hash_table)
#define hash_table_size(hash_table)                 array_size(hash_table)
#define hash_table_address_at(hash_table,index)     array_address_at(hash_table,index)
#define hash_table_compute_address_at(hash_table,element_size,index) \
    array_compute_address_at(hash_table,element_size,index)

#define hash_table_occupied_length_from_capacity(capacity) \
    (INT_DIV_ROUND_UP((capacity), (8*sizeof(size_t)))*sizeof(size_t))
#define hash_table_occupied_length(hash_table) \
    hash_table_occupied_length_from_capacity(hash_table_capacity(hash_table))
#define hash_table_header_size_from_capacity(capacity) \
    (ARRAY_HEADER_SIZE + sizeof(Hash_Fcn) + sizeof(Compare_Fcn) + \
    hash_table_occupied_length_from_capacity(capacity))
#define hash_table_header_size(hash_table) \
    (hash_table_header_size_from_capacity(hash_table_capacity(hash_table)))
#define hash_table_occupied_pointer(hash_table) \
    ((size_t *)((size_t)(hash_table) - hash_table_header_size(hash_table)))

#define hash_table_new_with_capacity(type,hash_function,compare_key,initial_capacity) \
    (type *)_hash_table_new(sizeof(type), (hash_function), (compare_key), (initial_capacity))
#define hash_table_new(type,hash_function,compare_key) \
    hash_table_new_with_capacity(type, (hash_function), (compare_key), HASH_TABLE_DEFAULT_CAPACITY)
#define hash_table_delete(hash_table) \
    (CDATA_FREE((void *)hash_table_occupied_pointer(hash_table)))

#define hash_table_should_resize(hash_table) \
    (LOAD_FACTOR_DENOMINATOR*hash_table_size(hash_table) >= LOAD_FACTOR_NUMERATOR*hash_table_capacity(hash_table))

#define hash_table_hash_function(hash_table) \
    (*(Hash_Fcn *)((size_t)(hash_table) - ARRAY_HEADER_SIZE - sizeof(Hash_Fcn)))
#define hash_table_compute_hash(hash_table,data) \
    (hash_table_hash_function(hash_table)(data) % hash_table_capacity(hash_table))

#define hash_table_compare_function(hash_table) \
    (*(Compare_Fcn *)((size_t)(hash_table) - ARRAY_HEADER_SIZE - sizeof(Hash_Fcn) - sizeof(Compare_Fcn)))
#define hash_table_compare_keys(hash_table,data1,data2) \
    (hash_table_compare_function(hash_table)(data1,data2))

#define hash_table_is_occupied(hash_table,index) \
    TEST_BIT(hash_table_occupied_pointer(hash_table)[((index)/(sizeof(size_t)*8))], ((index) % (sizeof(size_t)*8)))
#define hash_table_set_occupied(hash_table,index) \
    SET_BIT(hash_table_occupied_pointer(hash_table)[((index)/(sizeof(size_t)*8))], ((index) % (sizeof(size_t)*8)))
#define hash_table_clear_occupied(hash_table,index) \
    CLEAR_BIT(hash_table_occupied_pointer(hash_table)[((index)/(sizeof(size_t)*8))], ((index) % (sizeof(size_t)*8)))

#define hash_table_get(hash_table,key) \
    _hash_table_get((hash_table), sizeof(*hash_table), (key))
#define hash_table_insert(hash_table,value,address) \
    _hash_table_insert((void **)&(hash_table), sizeof(*(hash_table)), (value), (void **const)(address))

#ifdef CDATA_TYPEOF_SUPPORTED
#define hash_table_for_each(hash_table,index,it) \
    for (size_t (index) = 0, keep = 1; (index) < hash_table_capacity(hash_table); (index)++, keep = 1) \
        for (__typeof__(hash_table) (it) = hash_table_address_at(hash_table,index); \
            keep && hash_table_is_occupied((hash_table),(index)); keep = 0)
#else
#define hash_table_for_each(hash_table,index,it) \
    ERROR("hash_table_for_each is not supported for this compiler")
#endif

#define hash_table_to_array(hash_table) (_hash_table_to_array((hash_table), sizeof(*(hash_table))))

#ifdef __cplusplus
extern "C" {
#endif

// Hash functions
typedef size_t (*Hash_Fcn)(const void *);

// This functions shouldn't be called directly, insted use the macros defined above
CDATA_FCN_DEF size_t djb2(const char *str)
    __attribute__((warn_unused_result, nonnull));
CDATA_FCN_DEF void *_hash_table_new(size_t element_size, Hash_Fcn hash_function, Compare_Fcn compare_key, size_t initial_capacity)
    __attribute__((warn_unused_result));
CDATA_FCN_DEF size_t _hash_table_get_index(void *hash_table, size_t element_size, const void *key)
    __attribute__((warn_unused_result, nonnull));
CDATA_FCN_DEF void *_hash_table_get(void *hash_table, size_t element_size, const void *key)
    __attribute__((warn_unused_result, nonnull));
CDATA_FCN_DEF void *_hash_table_resize(void *hash_table, size_t element_size, size_t new_capacity)
    __attribute__((warn_unused_result, nonnull));
CDATA_FCN_DEF void *_hash_table_resize_if_needed(void *hash_table, size_t element_size)
    __attribute__((warn_unused_result, nonnull));
CDATA_FCN_DEF int _hash_table_insert(void **hash_table, size_t element_size, const void *value, void **const user_address)
    __attribute__((nonnull(1,3)));
CDATA_FCN_DEF void *_hash_table_to_array(const void *hash_table, size_t element_size)
    __attribute__((warn_unused_result, nonnull));

#ifdef __cplusplus
}
#endif

//------------------------------------------------------------------------------
// Arena allocator

#ifdef __cplusplus
extern "C" {
#endif

struct _Region;
typedef struct _Region {
    size_t capacity;
    size_t occupied;
    struct _Region *next;
    char data[];
} Region;

typedef struct {
    Region *first;
    Region *current;
} Arena;

// TODO: add arena_snapshot, arena_rewind

CDATA_FCN_DEF void *arena_alloc(Arena *arena, size_t size)
    __attribute__((warn_unused_result, nonnull));
CDATA_FCN_DEF char *arena_strdup(Arena *arena, const char *str)
    __attribute__((warn_unused_result, nonnull));
CDATA_FCN_DEF char *arena_strndup(Arena *arena, const char *str, size_t len)
    __attribute__((warn_unused_result, nonnull));
CDATA_FCN_DEF void arena_delete(Arena *arena)
    __attribute__((nonnull));

#ifdef __cplusplus
}
#endif

#endif  // __CDATA_HEADER_ONLY_LIBRARY

//------------------------------------------------------------------------------
// SOURCE
//------------------------------------------------------------------------------

#ifdef CDATA_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif

// Function stolen from https://handwiki.org/wiki/Quadratic_probing
CDATA_FCN_DEF size_t round_up_2(size_t value) {
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    value++;
    return value;
}

CDATA_FCN_DEF void *_array_resize(void *array, size_t element_size, size_t header_size, size_t new_capacity) {
    void *new_p = NULL;
    if (array == NULL) {
        new_p = CDATA_REALLOC(NULL, new_capacity * element_size + header_size);
        CDATA_MEMSET(new_p, 0, header_size);
        if (new_p == NULL) {
            return(NULL);
        }
    } else {
        void *p = (void *)((size_t)array - header_size);
        new_p = CDATA_REALLOC(p, new_capacity * element_size + header_size);
        if (new_p == NULL) {
            CDATA_FREE(p);
            return(NULL);
        }
    }
    void *new_array = (char *)new_p + header_size;
    void *address = array_compute_address_at(new_array, element_size, array_capacity(new_array));
    size_t length = (new_capacity - array_capacity(new_array));
    CDATA_MEMSET(address, 0, length * element_size);
    array_capacity(new_array) = new_capacity;
    return(new_array);
}

CDATA_FCN_DEF void *_array_resize_if_needed(void *array, size_t element_size, size_t size_to_add) {
    if (array == NULL) {
        size_t new_capacity = INT_MAX(size_to_add, ARRAY_DEFAULT_CAPACITY);
        new_capacity = round_up_2(new_capacity);
        array = _array_resize(NULL, element_size, ARRAY_HEADER_SIZE, new_capacity);
        if (array == NULL) {
            return NULL;
        }
    }
    size_t new_size = array_size(array) + size_to_add;
    if (new_size >= array_capacity(array)) {
        size_t new_capacity = array_capacity(array);
        while (new_size >= new_capacity) {
            new_capacity *= GROWTH_FACTOR;
        }
        new_capacity = round_up_2(new_capacity);
        array = _array_resize(array, element_size, ARRAY_HEADER_SIZE, new_capacity);
    }
    return array;
}

CDATA_FCN_DEF void *_array_insert_zero_at(void *array, size_t element_size, size_t index) {
    size_t old_size = (array == NULL) ? 0 : array_size(array);
    size_t size_to_add = array_index_is_valid(array, index) ? 1 : ((index) + 1 - old_size);
    array = _array_resize_if_needed(array, element_size, size_to_add);
    if (array != NULL) {
        void *actual = array_compute_address_at(array, element_size, index);
        if (index < old_size) {
            void *next = array_compute_address_at(array, element_size, (index+1));
            size_t length = old_size - index;
            CDATA_MEMMOVE(next, actual, length*element_size);
        }
        CDATA_MEMSET(actual, 0, element_size);
        array_size(array) = old_size + size_to_add;
    }
    return(array);
}

// This function returns an invalid index in case the key wasn't found
CDATA_FCN_DEF size_t _array_sequential_search(const void *array, size_t element_size, const void *key, Compare_Fcn compare) {
    if (array != NULL) {
        for (size_t i = 0; i < array_size(array); i++) {
            const void *it = array_compute_address_at(array, element_size, i);
            if (!compare(key, it)) {
                return(i);
            }
        }
    }
    return((size_t)-1);
}

// This function returns an invalid index in case the key wasn't found
CDATA_FCN_DEF size_t _array_binary_search(const void *array, size_t element_size, const void *key, Compare_Fcn compare) {
    if (array == NULL) {
        return((size_t)-1);
    }
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
            return(index);
        }
    }
    // Didn't found the key
    // Return an invalid index, but with enought information so that _array_insert_sorted
    // can recover the ideal position for the element to be inserted
    index = (size_t)-1*(low+1);
    return(index);
}

// This function returns 1 if a new element was inserted, and 0 if that element is already in the array
CDATA_FCN_DEF int _array_insert_sorted(void **array, size_t element_size, const void *element, Compare_Fcn compare, size_t *const user_index) {
    if (*array == NULL) {
        array_push_raw(*array, element, element_size);
        if (user_index != NULL) {
            *user_index = 0;
        }
        return(1);
    }
    size_t index = _array_binary_search(*array, element_size, element, compare);
    if (array_index_is_valid(*array, index)) {
        if (user_index != NULL) {
            *user_index = index;
        }
        return(0);
    }
    // Insert new element
    index = (size_t)(-1L*(long)index - 1);
    *array = _array_insert_zero_at(*array, element_size, index);
    if (*array == NULL) {
        return(-1);
    }
    void *it = array_compute_address_at(*array, element_size, index);
    CDATA_MEMCPY(it, element, element_size);
    if (user_index != NULL) {
        *user_index = index;
    }
    return(1);
}

CDATA_FCN_DEF size_t djb2(const char *str) {
    size_t hash = 5381;
    for (; *str; ++str) {
        /* hash * 33 + c */
        hash = ((hash << 5) + hash) + (size_t)*str;
    }
    return hash;
}

CDATA_FCN_DEF void *_hash_table_new(size_t element_size, Hash_Fcn hash_function, Compare_Fcn compare_key, size_t initial_capacity) {
    size_t header_size = hash_table_header_size_from_capacity(initial_capacity);
    void *hash_table = _array_resize(NULL, element_size, header_size, initial_capacity);
    if (hash_table != NULL) {
        hash_table_hash_function(hash_table) = hash_function;
        hash_table_compare_function(hash_table) = compare_key;
    }
    return hash_table;
}

// This function returns the index of the key if it is present in the hash table.
// If it is not present, the function returns the next non-occupied index (if there is one).
CDATA_FCN_DEF size_t _hash_table_get_index(void *hash_table, size_t element_size, const void *key) {
    size_t index = hash_table_compute_hash(hash_table, key);
    size_t i = 0;
    for (; i < hash_table_capacity(hash_table); i++) {
        if (!hash_table_is_occupied(hash_table, index)) {
            break;
        }
        void *it = hash_table_compute_address_at(hash_table,element_size,index);
        if (hash_table_compare_keys(hash_table, it, key) == 0) {
            break;
        }
#ifdef QUADRATIC_PROBING
        index = (index + i*i) % hash_table_capacity(hash_table);
#else // LINEAR_PROBING
        index = (index + 1) % hash_table_capacity(hash_table);
#endif
    }
    if (i >= hash_table_capacity(hash_table)) {
        // The table is full and the key wasn't found
        return((size_t)-1);
    }
    return(index);
}

CDATA_FCN_DEF void *_hash_table_get(void *hash_table, size_t element_size, const void *key) {
    size_t index = _hash_table_get_index(hash_table, element_size, key);
    if (index >= hash_table_capacity(hash_table)) {
        return(NULL);
    }
    if (hash_table_is_occupied(hash_table, index)) {
        void *it = hash_table_compute_address_at(hash_table,element_size,index);
        return(it);
    }
    return(NULL);
}

CDATA_FCN_DEF void *_hash_table_resize(void *hash_table, size_t element_size, size_t new_capacity) {
    void *new_hash_table = _hash_table_new(element_size,
        hash_table_hash_function(hash_table),
        hash_table_compare_function(hash_table),
        new_capacity);
    if (new_hash_table == NULL) {
        return(NULL);
    }
    for (size_t i = 0; i < hash_table_capacity(hash_table); i++) {
        if (hash_table_is_occupied(hash_table, i)) {
            // Insert every element from the old hash table into the new one
            // OBS: the indexes may change
            void *it = hash_table_compute_address_at(hash_table, element_size, i);
            size_t index = _hash_table_get_index(new_hash_table, element_size, it);
            void *address = hash_table_compute_address_at(new_hash_table, element_size, index);
            CDATA_MEMCPY(address, it, element_size);
            hash_table_set_occupied(new_hash_table, index);
        }
        hash_table_size(new_hash_table) = hash_table_size(hash_table);
    }
    hash_table_delete(hash_table);
    return(new_hash_table);
}

CDATA_FCN_DEF void *_hash_table_resize_if_needed(void *hash_table, size_t element_size) {
    if (LOAD_FACTOR_DENOMINATOR*hash_table_size(hash_table) >= LOAD_FACTOR_NUMERATOR*hash_table_capacity(hash_table)) {
        size_t new_capacity = GROWTH_FACTOR*hash_table_capacity(hash_table);
        new_capacity = round_up_2(new_capacity);
        hash_table = _hash_table_resize(hash_table, element_size, new_capacity);
    }
    return(hash_table);
}

CDATA_FCN_DEF int _hash_table_insert(void **hash_table, size_t element_size, const void *value, void **const user_address) {
    *hash_table = _hash_table_resize_if_needed(*hash_table, element_size);
    size_t index = _hash_table_get_index(*hash_table, element_size, value);
    void *address = hash_table_compute_address_at(*hash_table, element_size, index);
    if (hash_table_is_occupied(*hash_table, index)) {
        // The element is already present in the hash table
        if (user_address != NULL) {
            *user_address = address;
        }
        return(0);
    }
    CDATA_MEMCPY(address, value, element_size);
    hash_table_set_occupied(*hash_table, index);
    hash_table_size(*hash_table)++;
    if (user_address != NULL) {
        *user_address = address;
    }
    return(1);
}

CDATA_FCN_DEF void *_hash_table_to_array(const void *hash_table, size_t element_size) {
    void *array = _array_resize(NULL, element_size, ARRAY_HEADER_SIZE, hash_table_capacity(hash_table));
    if (array == NULL) {
        return(NULL);
    }
    for (size_t i = 0; i < hash_table_capacity(hash_table); i++) {
        if (hash_table_is_occupied(hash_table, i)) {
            void *it = hash_table_compute_address_at(hash_table, element_size, i);
            array_push_raw(array,it,element_size);
        }
    }
    return(array);
}

CDATA_FCN_DEF void *arena_alloc(Arena *arena, size_t size) {
    size = INT_ROUND_UP(size, sizeof(void *));
    for (Region *current = arena->current; current != NULL; current = current->next) {
        arena->current = current;
        size_t available = current->capacity - current->occupied;
        if (available > size) {
            void *data = (void *)((size_t)current->data + current->occupied);
            current->occupied += size;
            return data;
        }
    }
    size_t capacity = INT_MAX(size, ARENA_DEFAULT_REGION_CAPACITY);
    Region *new_region = CDATA_REALLOC(NULL, capacity + sizeof(Region));
    if (new_region == NULL) {
        return NULL;
    }
    new_region->next = NULL;
    new_region->capacity = capacity;
    new_region->occupied = size;
    if (arena->current != NULL) {
        arena->current->next = new_region;
    }
    arena->current = new_region;
    if (arena->first == NULL) {
        arena->first = new_region;
    }
    return new_region->data;
}

CDATA_FCN_DEF char *arena_strdup(Arena *arena, const char *str) {
    size_t len = CDATA_STRLEN(str) + 1;
    char *dup = arena_alloc(arena, len);
    if (dup == NULL) {
        return NULL;
    }
    CDATA_MEMCPY(dup, str, len);
    return dup;
}

CDATA_FCN_DEF char *arena_strndup(Arena *arena, const char *str, size_t len) {
    char *dup = arena_alloc(arena, len + 1);
    if (dup == NULL) {
        return NULL;
    }
    CDATA_MEMCPY(dup, str, len);
    dup[len] = '\0';
    return dup;
}

CDATA_FCN_DEF void arena_free_all(Arena *arena) {
    arena->current = arena->first;
    Region *region = arena->first;
    while (region != NULL) {
        region->occupied = 0;
        region = region->next;
    }
}

CDATA_FCN_DEF void arena_delete(Arena *arena) {
    Region *region = arena->first;
    while (region != NULL) {
        Region *next = region->next;
        CDATA_FREE(region);
        region = next;
    }
    CDATA_MEMSET(arena, 0, sizeof(*arena));
}

#ifdef __cplusplus
}
#endif

#endif // CDATA_IMPLEMENTATION

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
