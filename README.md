# cdata

## Overview

**cdata** is a simple library for data-structures implemented in the C programming language. The main goal is to simplify the programming experience in this language. It posseses the following characteristics:

- Compatible with C99;
- It is a [Header Only Library](https://github.com/nothings/stb);
- It was tested only on Linux (compatible with GNU GCC or clang);
- It is highly customizable;

## Usage

Example of usage of dynamic arrays:

```c
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define CDATA_IMPLEMENTATION
#include "cdata.h"

int compare_strings(const void *a, const void *b)
{
  return strcmp(*(char **)a, *(char **)b);
}

int main(void)
{
  // Creates a new array
  char **array = array_new(char *);
  assert(array != NULL);
  
  // Insert element at the end of the array
  array_push(array, "alpha");
  array_push(array, "beta");
  array_push(array, "gamma");

  // Insert element in the beginning of the array
  array_unshift(array, "zeta");

  // Iterates over the entire array
  printf("Array:\n");
  for (size_t i = 0; i < array_size(array); i++) {
    printf("  %s\n", array[i]);
  }

  // Sequential search
  char *key = "beta";
  printf("Found \"%s\" at position %zu\n", key, array_sequential_search(array, &key, compare_strings));

  // Sort the array using quicksort
  array_qsort(array, compare_strings);

  // Insert a new element in the array while keeping it sorted
  key = "theta";
  array_insert_sorted(array, &key, compare_strings, NULL);

  // For each loop
  printf("Array sorted:\n");
  array_for_each(array, it) {
    // it is a pointer to each element in the array
    printf("  %s\n", *it);
  }

  // Binary search for sorted arrays
  key = "gamma";
  printf("Found \"%s\" at position %zu\n", key, array_binary_search(array, &key, compare_strings));

  // Remove element at the end of the array
  printf("Array pop: %s\n", array_pop(array));

  // Deallocates the array
  array_delete(array);
  return 0;
}
```

Example of usage of hash tables (it uses open adressing with linear or quadratic probing):

```c
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define CDATA_IMPLEMENTATION
#include "cdata.h"

typedef struct {
  char *key;
  char *value;
} Entry;

int compare_entries(const void *a, const void *b)
{
  const Entry *entry_a = a;
  const Entry *entry_b = b;
  return strcmp(entry_a->key, entry_b->key);
}

size_t hash_entry(const void *data) {
  const Entry *entry = data;
  // String hash function available with the library
  return djb2(entry->key);
}

int main(void)
{
  // Creates a new hash table
  Entry *hash_table = hash_table_new(Entry, hash_entry, compare_entries);
  assert(hash_table != NULL);
  
  // Insert element in the hash table
  Entry entry = (Entry){
    .key = "alpha",
    .value = "69",
  };
  hash_table_insert(hash_table, &entry, NULL);
  entry = (Entry){
    .key = "beta",
    .value = "420",
  };
  hash_table_insert(hash_table, &entry, NULL);
  entry = (Entry){
    .key = "gamma",
    .value = "???",
  };
  hash_table_insert(hash_table, &entry, NULL);

  // Finds an element in the hash table
  entry = (Entry){ .key = "alpha" };
  Entry *found = hash_table_get(hash_table, &entry);
  if (found != NULL) {
    printf("The key = \"%s\" has vlaue = \"%s\"\n", found->key, found->value);
  } else {
    printf("The key = \"%s\" was NOT found in the hash table\n", entry.key);
  }

  // For each loop
  printf("Hash table:\n");
  hash_table_for_each(hash_table, index, it) {
    // it is a pointer to each element in the hash table
    printf("  %s: %s\n", it->key, it->value);
  }

  // Deallocates the hash table
  hash_table_delete(hash_table);
  return 0;
}
```

More complete examples can be found in the folder `./examples`. Check the next section for more information on how to use them.

## Examples

To compile and run the examples, follow these steps:

- Download this project
- Navigate to its folder
- Run the make command to compile the examples:

```console
$ make
gcc -pedantic -W -Wall -Wextra -Wconversion -Wswitch-enum -Werror -std=c99 -O0 -g -I.  examples/count-words.c -o examples/count-words
```

- Just run the examples. The `count-words` example was developed to compare the performance of hash tables and dynamic arrays. It determines the most used words in a text file:

```console
$ ./examples/count-words examples/The\ Divine\ Comedy.txt examples/Shakespeare.txt 
File: examples/The Divine Comedy.txt
  lines: 6390
  chars: 220001
  words: 38039
  algorithm: dynamic array
    execution time: 1.78038s
    unique words: 7783
    top 10 words:
      01. the               2213
      02. and               1483
      03. of                 854
      04. to                 806
      05. i                  779
      06. that               651
      07. he                 450
      08. in                 441
      09. a                  436
      10. with               397
  algorithm: sorted dynamic array
    execution time: 0.039635s
    unique words: 7783
    top 10 words:
      01. the               2213
      02. and               1483
      03. of                 854
      04. to                 806
      05. i                  779
      06. that               651
      07. he                 450
      08. in                 441
      09. a                  436
      10. with               397
  algorithm: hash table
    execution time: 0.020831s
    unique words: 7783
    top 10 words:
      01. the               2213
      02. and               1483
      03. of                 854
      04. to                 806
      05. i                  779
      06. that               651
      07. he                 450
      08. in                 441
      09. a                  436
      10. with               397
File: examples/Shakespeare.txt
  lines: 124455
  chars: 5458198
  words: 900987
  algorithm: dynamic array
    execution time: 188.265s
    unique words: 58580
    top 10 words:
      01. the              27627
      02. and              26063
      03. i                19593
      04. to               18996
      05. of               18011
      06. a                14589
      07. my               12476
      08. in               10682
      09. you              10642
      10. that             10498
  algorithm: sorted dynamic array
    execution time: 1.30934s
    unique words: 58580
    top 10 words:
      01. the              27627
      02. and              26063
      03. i                19593
      04. to               18996
      05. of               18011
      06. a                14589
      07. my               12476
      08. in               10682
      09. you              10642
      10. that             10498
  algorithm: hash table
    execution time: 0.462562s
    unique words: 58580
    top 10 words:
      01. the              27627
      02. and              26063
      03. i                19593
      04. to               18996
      05. of               18011
      06. a                14589
      07. my               12476
      08. in               10682
      09. you              10642
      10. that             10498
```

Feel free to reach out to us if you need further assistance or have any questions. Enjoy using the library!
