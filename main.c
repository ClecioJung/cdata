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

typedef Word *(*Init_Fcn)(void);
typedef Word *(*Process_Word_Fcn)(Word *, const Word);
typedef Word *(*Post_Process_Fcn)(Word *const);
typedef void (*Display_Results_Fcn)(Word *const);
typedef void (*Deinit_Fcn)(Word *);

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

Word *array_sort_words_descending_by_count(Word *const array) {
    array_qsort(array, compare_words_by_count);
    return array;
}

Word *convert_hash_table_to_sorted_array(Word *hash_table) {
    Word *array = hash_table_to_array(hash_table);
    hash_table_delete(hash_table);
    return array_sort_words_descending_by_count(array);
}

size_t djb2(const char *str) {
    size_t hash = 5381;
    for (; *str; ++str) {
        /* hash * 33 + c */
        hash = ((hash << 5) + hash) + (size_t)*str;
    }
    return hash;
}

size_t word_hash(const void *data) {
    const Word *word = data;
    size_t value = djb2(word->word);
    return value;
}

Word *array_init(void) {
    Word *array = array_new(Word);
    return array;
}

Word *hash_table_init(void) {
    Word *hash_table = hash_table_new(Word, word_hash, compare_words);
    return hash_table;
}

void array_deinit(Word *array) {
    array_for_each(array, it) {
        free(it->word);
    }
    array_delete(array);
}

void array_display_results(Word *const array) {
    printf("  unique words: %zu\n", array_size(array));
    const size_t number_of_words = MIN(10, array_size(array));
    if (number_of_words > 0) {
        printf("  top %zu words:\n", number_of_words);
        for (size_t i = 0; i < number_of_words; i++) {
            printf("    %02zu. %-15s %6zu\n", (i+1), array[i].word, array[i].count);
        }
    }
}

Word *sequential_algorithm(Word *array, const Word word) {
    const size_t index = array_sequential_search(array, &word, compare_words);
    if (array_index_is_valid(array, index)) {
        array_at(array, index).count++;
    } else {
        Word new_word = {
            .word = strdup(word.word),
            .count = 1,
        };
        array_push(array, new_word);
    }
    return array;
}

Word *sorted_algorithm(Word *array, const Word word) {
    size_t index = (size_t)-1;
    if (array_insert_sorted(array, &word, compare_words, &index)) {
        array_at(array, index) = (Word) {
            .word = strdup(word.word),
            .count = 1,
        };
    } else {
        array_at(array, index).count++;
    }
    return array;
}

Word *hash_algorithm(Word *hash_table, const Word word) {
    Word *stored = NULL;
    if (hash_table_insert(hash_table, &word, &stored)) {
        *stored = (Word) {
            .word = strdup(word.word),
            .count = 1,
        };
    } else {
        stored->count++;
    }
    return hash_table;
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
    {
        .name = "hash table",
        .init = hash_table_init,
        .process_word = hash_algorithm,
        .post_process = convert_hash_table_to_sorted_array,
        .display_results = array_display_results,
        .deinit = array_deinit,
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
    Word *data = algorithm.init();
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
                // TODO: convert all words to lowercase
                assert(word_len < sizeof(tmp_buffer));
                strncpy(tmp_buffer, str, word_len);
                tmp_buffer[word_len] = '\0';
                Word word_found = { .word = tmp_buffer };
                data = algorithm.process_word(data, word_found);
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
    data = algorithm.post_process(data);
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
    // TODO: Add command line arguments to select which algorithm to use
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
