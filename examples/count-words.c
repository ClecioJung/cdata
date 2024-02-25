#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define CDATA_IMPLEMENTATION
#include "cdata.h"

typedef struct {
    char *word;
    size_t count;
} Word;

typedef Word *(*Init_Fcn)(void);
typedef Word *(*Process_Word_Fcn)(Word *, const Word);
typedef Word *(*Post_Process_Fcn)(Word *const);
typedef void (*Display_Results_Fcn)(Word *const, size_t);
typedef void (*Deinit_Fcn)(Word *);

typedef struct {
    char *name;
    char arg_option; // Command line argument to activate this algorithm
    char *help_msg;
    Init_Fcn init;
    Process_Word_Fcn process_word;
    Post_Process_Fcn post_process;
    Display_Results_Fcn display_results;
    Deinit_Fcn deinit;
} Algorithm;

// Global arena allocator
static Arena arena = { 0 };

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
    array_delete(array);
}

void array_display_results(Word *const array, size_t number_of_words) {
    printf("    unique words: %zu\n", array_size(array));
    number_of_words = INT_MIN(number_of_words, array_size(array));
    if (number_of_words > 0) {
        printf("    top %zu words:\n", number_of_words);
        for (size_t i = 0; i < number_of_words; i++) {
            printf("      %02zu. %-15s %6zu\n", (i+1), array[i].word, array[i].count);
        }
    }
}

Word *sequential_algorithm(Word *array, const Word word) {
    const size_t index = array_sequential_search(array, &word, compare_words);
    if (array_index_is_valid(array, index)) {
        array_at(array, index).count++;
    } else {
        Word new_word = {
            .word = arena_strdup(&arena, word.word),
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
            .word = arena_strdup(&arena, word.word),
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
            .word = arena_strdup(&arena, word.word),
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
        .arg_option = 'd',
        .help_msg = "Uses dynamic array with sequential search algorithm",
        .init = array_init,
        .process_word = sequential_algorithm,
        .post_process = array_sort_words_descending_by_count,
        .display_results = array_display_results,
        .deinit = array_deinit,
    },
    {
        .name = "sorted dynamic array",
        .arg_option = 's',
        .help_msg = "Uses sorted dynamic array algorithm",
        .init = array_init,
        .process_word = sorted_algorithm,
        .post_process = array_sort_words_descending_by_count,
        .display_results = array_display_results,
        .deinit = array_deinit,
    },
    {
        .name = "hash table",
        .arg_option = 't',
        .help_msg = "Uses hash table algorithm",
        .init = hash_table_init,
        .process_word = hash_algorithm,
        .post_process = convert_hash_table_to_sorted_array,
        .display_results = array_display_results,
        .deinit = array_deinit,
    },
};

void copy_sized_str_to_cstr_lowercase(char *dst, const char *src, size_t len) {
    for (size_t i = 0; (src[i] != '\0') && (i < len); i++) {
        dst[i] = (char)tolower(src[i]);
    }
    dst[len] = '\0';
}

int process_file(const char *const filename, const Algorithm algorithm, int print_header, size_t number_of_words) {
    clock_t tic = clock();
    FILE *const file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open file \"%s\": %s\n", filename, strerror(errno));
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
                assert(word_len < sizeof(tmp_buffer));
                copy_sized_str_to_cstr_lowercase(tmp_buffer, str, word_len);
                Word word_found = { .word = tmp_buffer };
                data = algorithm.process_word(data, word_found);
            }
            str += word_len;
        }
        lines++;
    }
    int result = EXIT_SUCCESS;
    if (!feof(file)) {
        fprintf(stderr, "Error: Didn't reach the end of file \"%s\"\n", filename);
        result = EXIT_FAILURE;
    }
    data = algorithm.post_process(data);
    clock_t toc = clock();
    if (result == EXIT_SUCCESS) {
        if (print_header) {
            printf("File: %s\n", filename);
            printf("  lines: %zu\n", lines);
            printf("  chars: %zu\n", chars);
            printf("  words: %zu\n", words);
        }
        printf("  algorithm: %s\n", algorithm.name);
        printf("    execution time: %gs\n", (double)(toc - tic) / CLOCKS_PER_SEC);
        algorithm.display_results(data, number_of_words);
    }
    algorithm.deinit(data);
    arena_free_all(&arena);
    fclose(file);
    return result;
}

int parse_uint(const char *const str, size_t *const number) {
    char *endptr = NULL;
    unsigned long parsed_number = strtoul(str, &endptr, 10);
    if (*endptr != '\0') {
        return EXIT_FAILURE;
    }
    *number = (size_t)parsed_number;
    return EXIT_SUCCESS;
}

size_t algorithm_option(char arg) {
    for (size_t i = 0; i < STATIC_ARRAY_SIZE(algorithms); i++) {
        if (arg == algorithms[i].arg_option) {
            return i;
        }
    }
    return (size_t)-1;
}

void usage(FILE *const stream, const char *const  program_name) {
    fprintf(stream, "Usage: %s [options] file...\n", program_name);
    fprintf(stream, "Options:\n");
    for (size_t i = 0; i < STATIC_ARRAY_SIZE(algorithms); i++) {
        const Algorithm algorithm = algorithms[i];
        fprintf(stream, "  -%c                       %s\n", algorithm.arg_option, algorithm.help_msg);
    }
    fprintf(stream, "  -n  <unsigned integer>   Specifies the number of most used words to display\n");
    fprintf(stream, "  -h                       Display this help message\n");
}

int main(const int argc, const char *const argv[])
{
    assert(argc > 0);
    const char *const program_name = argv[0];
    if (argc == 1) {
        fprintf(stderr, "Error: no argument was provided!\n");
        usage(stderr, program_name);
        return EXIT_FAILURE;
    }
    int active_algorithms = 0 ;
    char **filenames = array_new(char *);
    assert(filenames != NULL);
    size_t number_of_words = 10;
    // First, process all arguments, and append all filenames to a list, for later processing
    for (int i = 1; i < argc; i++) {
        const char *const arg = argv[i];
        if (arg[0] != '-') {
            array_push(filenames, (char *)arg);
            continue;
        }
        if (strlen(arg) != 2) {
            fprintf(stderr, "Error: Unrecognized argument: %s\n", arg);
            usage(stderr, program_name);
            return EXIT_FAILURE;
        }
        // Check if it was an algorithm option
        size_t found_algorithm = algorithm_option(arg[1]);
        if (found_algorithm < STATIC_ARRAY_SIZE(algorithms)) {
            SET_BIT(active_algorithms, found_algorithm);
            continue;
        }
        // Check other argument options
        switch (arg[1]) {
        case 'n': {
            if (++i == argc) {
                fprintf(stderr, "Error: Argument %s should be followed by a integer number\n", arg);
                usage(stderr, program_name);
                return EXIT_FAILURE;
            }
            if (parse_uint(argv[i], &number_of_words) == EXIT_FAILURE) {
                fprintf(stderr, "Error: %s is not a valid integer number\n", argv[i]);
                usage(stderr, program_name);
                return EXIT_FAILURE;
            }
        } break;
        case 'h':
            usage(stdout, program_name);
            return EXIT_SUCCESS;
        default:
            fprintf(stderr, "Error: Unrecognized argument: %s\n", arg);
            usage(stderr, program_name);
            return EXIT_FAILURE;
        }
    }
    // If no algorithm was specified, enable all of them
    if (active_algorithms == 0) {
        active_algorithms = 0xFF;
    }
    if (array_size(filenames) == 0) {
        fprintf(stderr, "Error: No file was specified...\n");
        usage(stderr, program_name);
        return EXIT_FAILURE;
    }
    // Process each file using the specified algorithms
    for (size_t i = 0; i < array_size(filenames); i++) {
        int print_header = 1;
        const char *const filename = filenames[i];
        for (size_t j = 0; j < STATIC_ARRAY_SIZE(algorithms); j++) {
            if (!TEST_BIT(active_algorithms, j)) {
                continue;
            }
            if (process_file(filename, algorithms[j], print_header, number_of_words)) {
                // The error was already reported in process_file
                return EXIT_FAILURE;
            }
            print_header = 0;
        }
    }
    array_delete(filenames);
    arena_delete(&arena);
    return EXIT_SUCCESS;
}
