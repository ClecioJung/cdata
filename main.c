#define _GNU_SOURCE

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DYNAMIC_ARRAY_IMPLEMENTATION
#include "dynamic_array.h"

typedef struct {
    char *word;
    size_t count;
} Word;

typedef void (*Process_Word_Fcn)(Word **, const char *const, const size_t);
typedef void (*Post_Process_Words)(Word *const);
typedef struct {
    char *name;
    Process_Word_Fcn process_word;
    Post_Process_Words post_process;
} Algorithm;

#define ARRAY_LEN(array) (sizeof(array)/sizeof(array[0]))

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

int compare_words(const void *a, const void *b) {
    Word *word_a = (Word *)a;
    Word *word_b = (Word *)b;
    return strcmp(word_a->word, word_b->word);
}

int compare_words_by_count(const void *a, const void *b) {
    Word *word_a = (Word *)a;
    Word *word_b = (Word *)b;
    return (word_b->count - word_a->count);
}

void sort_words_descending_by_count(Word *const array) {
    qsort(array, array_size(array), sizeof(array[0]), compare_words_by_count);
}

int compare_sized_str_to_word(const char *const str, const size_t str_len, const char *const word) {
    const size_t word_len = strlen(word);
    const size_t min_len = MIN(word_len, str_len);
    const int comp = strncmp(str, word, min_len);
    if (!comp) {
        return ((int)str_len - (int)word_len);
    }
    return comp;
}

size_t sequential_search(const Word *const array, const char *const word, const size_t word_len) {
    for (size_t i = 0; i < array_size(array); i++) {
        if (!compare_sized_str_to_word(word, word_len, array[i].word)) {
            return i;
        }
    }
    return (size_t)-1;
}

// This function returns non-zero if found the variable in the array and index
// stores the corresponding position in the array.
// Otherwise, the index points to position for the element to be inserted keeping the array sorted.
int binary_search(const Word *const array, const char *const word, const size_t word_len, size_t *const index) {
    size_t low = 0;
    size_t high = array_size(array) - 1;
    while ((low <= high) && array_index_is_valid(array, high)) {
        *index = low + (high - low) / 2; // middle
        const int comp = compare_sized_str_to_word(word, word_len, array[*index].word);
        if (comp < 0) {
            high = *index - 1;
        } else if (comp > 0) {
            low = *index + 1;
        } else {
            return 1;
        }
    }
    *index = low;
    return 0;
    
    /*
    char buffer[4096];
    assert(word_len < sizeof(buffer));
    strncpy(buffer, word, word_len);
    buffer[word_len] = '\0';
    const Word key = { .word = buffer };
    const Word *const found = bsearch(&key, array, array_size(array), sizeof(array[0]), compare_words);
    if (found == NULL) {
        return (size_t)-1;
    }
    return (found - array);
    */
}

void sequential_algorithm(Word **array, const char *const word, const size_t word_len) {
    const size_t index = sequential_search(*array, word, word_len);
    if (array_index_is_valid(*array, index)) {
        array_at(*array, index).count++;
    } else {
        Word new_word = {
            .word = strndup(word, word_len),
            .count = 1,
        };
        array_push(*array, new_word);
    }
}

void sorted_algorithm(Word **array, const char *const word, const size_t word_len) {
    size_t index = 0;
    if (binary_search(*array, word, word_len, &index)) {
        array_at(*array, index).count++;
    } else {
        Word new_word = {
            .word = strndup(word, word_len),
            .count = 1,
        };
        /*array_push(*array, new_word);
        qsort(*array, array_size(*array), sizeof((*array)[0]), compare_words);*/
        array_insert_at(*array, index, new_word);
    }
}

static const Algorithm algorithms[] = {
    {.name = "sequential", .process_word = sequential_algorithm, .post_process = sort_words_descending_by_count, },
    {.name = "sorted", .process_word = sorted_algorithm, .post_process = sort_words_descending_by_count, },
};

int process_file(const char *const filename, const Algorithm algorithm) {
    clock_t tic = clock();
    FILE *const file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file %s: %s\n", filename, strerror(errno));
        return EXIT_FAILURE;
    }
    char buffer[4096];
    Word *array = array_new(Word);
    size_t lines = 0, chars = 0, words = 0;
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        char *str = buffer;
        chars += strlen(str);
        while (*str) {
            while (isspace(*str) || ispunct(*str)) {
                str++;
            }
            size_t word_len = 0;
            while (*str && !isspace(str[word_len]) && !ispunct(*str)) {
                word_len++;
            }
            if (word_len > 0) {
                words++;
                algorithm.process_word(&array, str, word_len);
            }
            str += word_len;
        }
        lines++;
    }
    int result = EXIT_SUCCESS;
    if (!feof(file)) {
        fprintf(stderr, "Didn't reach the end of file for %s\n", filename);
        result = EXIT_FAILURE;
    }
    algorithm.post_process(array);
    clock_t toc = clock();
    if (result == EXIT_SUCCESS) {
        printf("File: %s\n", filename);
        printf("  algorithm: %s\n", algorithm.name);
        printf("  execution time: %gs\n", (double)(toc - tic) / CLOCKS_PER_SEC);
        printf("  lines: %zu\n", lines);
        printf("  chars: %zu\n", chars);
        printf("  words: %zu\n", words);
        printf("  unique words: %zu\n", array_size(array));
        const size_t number_of_words = MIN(10, array_size(array));
        if (number_of_words > 0) {
            printf("  top %zu words:\n", number_of_words);
            for (size_t i = 0; i < number_of_words; i++) {
                printf("    %02zu. %-15s %6zu\n", (i+1), array[i].word, array[i].count);
            }
        }
    }
    for (size_t i = 0; i < array_size(array); i++) {
        free(array[i].word);
    }
    array_delete(array);
    fclose(file);
    return result;
}

void usage(FILE *const stream, const char *const  program_name) {
    fprintf(stream, "Usage: %s [file]\n", program_name);
}

int main(const int argc, const char *const argv[])
{
    assert(argc > 0);
    const char *const program_name = argv[0];
    if (argc == 1) {
        usage(stderr, program_name);
        return EXIT_FAILURE;
    }
    for (int i = 1; i < argc; i++) {
        const char *const filename = argv[i];
        for (size_t j = 0; j < ARRAY_LEN(algorithms); j++) {
            if (process_file(filename, algorithms[j])) {
                return EXIT_FAILURE;
            }
        }
        
    }
    return EXIT_SUCCESS;
}
