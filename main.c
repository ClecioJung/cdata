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
    // TODO: Remove this - simple hack used to implement simple hash table
    int occupied;
} Word;

typedef void *(*Init_Fcn)(void);
typedef void (*Process_Word_Fcn)(void **, const Word);
typedef void (*Post_Process_Fcn)(void *const);
typedef void (*Display_Results_Fcn)(void *const);
typedef void (*Deinit_Fcn)(void *);

typedef struct {
    char *name;
    Init_Fcn init;
    Process_Word_Fcn process_word;
    Post_Process_Fcn post_process;
    Display_Results_Fcn display_results;
    Deinit_Fcn deinit;
} Algorithm;

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

void *array_init(void) {
    Word *array = array_new(Word);
    return array;
}

void array_deinit(void *data) {
    Word *array = data;
    array_for_each(array, it) {
        free(it->word);
    }
    array_delete(array);
}

void hash_table_deinit(void *data) {
    Word *array = data;
    for (size_t i = 0; i < array_capacity(array); i++) {
        if (array[i].occupied) {
            free(array[i].word);
        }
    }
    array_delete(array);
}

int compare_words(const void *a, const void *b) {
    Word *word_a = (Word *)a;
    Word *word_b = (Word *)b;
    return strcmp(word_a->word, word_b->word);
}

int compare_words_by_count(const void *a, const void *b) {
    Word *word_a = (Word *)a;
    Word *word_b = (Word *)b;
    return ((int)word_b->count - (int)word_a->count);
}

void array_sort_words_descending_by_count(void *const data) {
    Word *const array = data;
    array_qsort(array, compare_words_by_count);
}

void array_display_results(void *const data) {
    Word *const array = data;
    printf("  unique words: %zu\n", array_size(array));
    const size_t number_of_words = MIN(10, array_size(array));
    if (number_of_words > 0) {
        printf("  top %zu words:\n", number_of_words);
        for (size_t i = 0; i < number_of_words; i++) {
            printf("    %02zu. %-15s %6zu\n", (i+1), array[i].word, array[i].count);
        }
    }
}

void sequential_algorithm(void **data, const Word word) {
    Word **array = (Word **)data;
    const size_t index = array_sequential_search(*array, &word, compare_words);
    if (array_index_is_valid(*array, index)) {
        array_at(*array, index).count++;
    } else {
        Word new_word = {
            .word = strdup(word.word),
            .count = 1,
        };
        array_push(*array, new_word);
    }
}

void sorted_algorithm(void **data, const Word word) {
    Word **array = (Word **)data;
    size_t index;
    if (array_insert_sorted(*array, &word, compare_words, &index)) {
        array_at(*array, index) = (Word) {
            .word = strdup(word.word),
            .count = 1,
        };
    } else {
        array_at(*array, index).count++;
    }
}

unsigned int djb2(const char *str) {
    unsigned int hash = 5381;
    for (; *str; ++str) {
        /* hash * 33 + c */
        hash = ((hash << 5) + hash) + (unsigned int)*str;
    }
    return hash;
}

void hash_algorithm(void **data, const Word word) {
    Word **array = (Word **)data;
    size_t index = djb2(word.word) % array_capacity(*array);
    size_t i = 0;
    for (; i < array_capacity(*array) && 
        (array_at(*array, index).occupied &&
        strcmp(word.word, array_at(*array, index).word) != 0);
        i++) {
        index = (index + 1) % array_capacity(*array);
    }
    assert(i < array_capacity(*array)); // TODO: Review this
    if (array_at(*array, index).occupied) {
        array_at(*array, index).count++;
    } else {
        array_at(*array, index) = (Word) {
            .word = strdup(word.word),
            .count = 1,
            .occupied = 1,
        };
        array_size(*array)++;
    }
}

static const Algorithm algorithms[] = {
    {
        .name = "dynamic array",
        .init = array_init,
        .process_word = sequential_algorithm,
        .post_process = array_sort_words_descending_by_count,
        .display_results = array_display_results,
        .deinit = array_deinit,
    },
    {
        .name = "sorted dynamic array",
        .init = array_init,
        .process_word = sorted_algorithm,
        .post_process = array_sort_words_descending_by_count,
        .display_results = array_display_results,
        .deinit = array_deinit,
    },
    { // TODO: implement generic hash table
        .name = "hash table",
        .init = array_init,
        .process_word = hash_algorithm,
        .post_process = array_sort_words_descending_by_count,
        .display_results = array_display_results,
        .deinit = hash_table_deinit,
    },
};

int process_file(const char *const filename, const Algorithm algorithm) {
    clock_t tic = clock();
    FILE *const file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file %s: %s\n", filename, strerror(errno));
        return EXIT_FAILURE;
    }
    char tmp_buffer[4096];
    char buffer[4096];
    void *data = algorithm.init();
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
                assert(word_len < sizeof(tmp_buffer));
                strncpy(tmp_buffer, str, word_len);
                tmp_buffer[word_len] = '\0';
                Word word_found = { .word = tmp_buffer };
                algorithm.process_word(&data, word_found);
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
    algorithm.post_process(data);
    clock_t toc = clock();
    if (result == EXIT_SUCCESS) {
        printf("File: %s\n", filename);
        printf("  algorithm: %s\n", algorithm.name);
        printf("  execution time: %gs\n", (double)(toc - tic) / CLOCKS_PER_SEC);
        printf("  lines: %zu\n", lines);
        printf("  chars: %zu\n", chars);
        printf("  words: %zu\n", words);
        algorithm.display_results(data);
    }
    algorithm.deinit(data);
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
        for (size_t j = 0; j < STATIC_ARRAY_LEN(algorithms); j++) {
            if (process_file(filename, algorithms[j])) {
                return EXIT_FAILURE;
            }
        }
    }
    return EXIT_SUCCESS;
}
