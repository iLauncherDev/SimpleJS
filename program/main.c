#include <default.h>
#include <simplejs/mm/gc.h>
#include <simplejs/plugin.h>
#include <simplejs/lib/realpath.h>
#include <simplejs/lib/thread.h>
#include <simplejs/lib/shared_lib.h>
#include <simplejs/lib/time.h>
#include <simplejs/lib/generic_sort.h>

#include <simplejs/compiler.h>
#include <simplejs/vm.h>
#include <simplejs/builtin_object/dynamic_object.h>
#include <simplejs/lib/map_buffer_file.h>

uint32_t minimum_vm_memory_size = 1 * 1024 * 1024;
uint32_t vm_memory_size = 32 * 1024 * 1024;

char *file_path = NULL;
char *export_bytecode_path = NULL;

typedef struct plugin_entry
{
    char *name;

    simplejs_shared_lib_t *shared_lib;
    simplejs_plugin_entry_f plugin_function;

    simplejs_list_entry_t list_entry;
} plugin_entry_t;

simplejs_list_entry_t plugin_list;

void free_plugin_list()
{
    simplejs_list_entry_t *end_plugin = &plugin_list;
    simplejs_list_entry_t *current_plugin = end_plugin->next;

    while (current_plugin != end_plugin)
    {
        simplejs_list_entry_t *next_plugin = current_plugin->next;
        plugin_entry_t *plugin = simplejs_get_list_entry_structure(current_plugin);

        if (plugin->shared_lib)
            simplejs_close_shared_lib(plugin->shared_lib);

        free(plugin);

        current_plugin = next_plugin;
    }
}

simplejs_utf8_string_t *readFile(char *path)
{
    size_t bytes = 0;
    long position = 0;
    FILE *file;
    simplejs_utf8_string_t *string = NULL;

    file = fopen(path, "rb");
    if (!file)
    {
        printf("file not found!\n");
        goto errorL;
    }

    fseek(file, 0, SEEK_END);

    position = ftell(file);

    fseek(file, 0, SEEK_SET);

    string = malloc(sizeof(*string) + position);
    if (!string)
    {
        printf("cannot allocate string\n");
        goto errorL;
    }

    string->buffer = (void *)((uint8_t *)string + sizeof(*string));
    string->max_size = position;

    bytes = fread(string->buffer, 1, position, file);
    if (bytes != string->max_size)
    {
        printf("cannot read string\n");
        goto errorL;
    }

    string->valid_size = simplejs_strnlen(string->buffer, bytes);

    printf("bytes: %llu, size: %ld\n", (long long)bytes, position);

    return string;

errorL:
    if (file)
        fclose(file);
    if (string)
        free(string);

    return NULL;
}

int processArgs(int argc, char **argv)
{
    int skips = 1;

    for (int i = 0; i < argc; i += skips)
    {
        int remainding_i = argc - i;
        skips = 1;

        char **argv_window = &argv[i];
        char *string = argv_window[0];

        if (!simplejs_strcasecmp(string, "-file"))
        {
            skips = 2;

            if (remainding_i < skips)
            {
                printf("not enough parameters\n");
                return 1;
            }

            file_path = argv_window[1];
        }
        else if (!simplejs_strcasecmp(string, "-export-bytecode"))
        {
            skips = 2;

            if (remainding_i < skips)
            {
                printf("not enough parameters\n");
                return 1;
            }

            export_bytecode_path = argv_window[1];
        }
        else if (!simplejs_strcasecmp(string, "-vm_memory"))
        {
            skips = 2;

            if (remainding_i < skips)
            {
                printf("not enough parameters\n");
                return 1;
            }

            vm_memory_size = (uint32_t)(strtof(argv_window[1], NULL) * 1024.0f * 1024.0f);
            if (minimum_vm_memory_size > vm_memory_size)
                vm_memory_size = minimum_vm_memory_size;
        }
        else if (!simplejs_strcasecmp(string, "-plugin"))
        {
            skips = 2;

            if (remainding_i < skips)
            {
                printf("not enough parameters\n");
                return 1;
            }

            char *plugins = argv_window[1];
            int len = 0;
            int i = 0;

            while (true)
            {
                char current_char = plugins[i];

                if ((current_char == '\0' || current_char == ',') &&
                    len > 0)
                {
                    char *temp_string = &plugins[i - len];
                    char temp_string_terminator = temp_string[len];

                    temp_string[len] = '\0';

                    plugin_entry_t *entry = malloc(sizeof(*entry));
                    if (!entry)
                    {
                        printf("cannot allocate plugin entry\n");
                        return 1;
                    }

                    memclr(entry, sizeof(*entry));

                    entry->name = temp_string;
                    simplejs_init_list_entry(&entry->list_entry, entry);
                    simplejs_insert_tail_list(&plugin_list, &entry->list_entry);

                    if (!SIMPLEJS_SUCCESS(simplejs_load_shared_lib(entry->name, &entry->shared_lib)))
                    {
                        printf("cannot load '%s' plugin\n", entry->name);
                        return 1;
                    }

                    if (!SIMPLEJS_SUCCESS(simplejs_shared_lib_get_proc_address(entry->shared_lib, SIMPLEJS_PLUGIN_ENTRY_NAME, (void **)&entry->plugin_function)))
                    {
                        printf("cannot load '%s' plugin function\n", entry->name);
                        return 1;
                    }

                    temp_string[len] = temp_string_terminator;
                    len = -1;
                }

                if (current_char == '\0')
                    break;

                len++, i++;
            }
        }
    }

    return 0;
}

int SIMPLEJS_HOOK_API get_memory_amount();

volatile bool run_gc_thread = true;

uintptr_t gc_thread_callback(simplejs_thread_t *thread)
{
    int tick = 500;
    int delay = 5000 / tick;

    int current_tick = 0;

    while (run_gc_thread)
    {
        simplejs_gc_event(false);

        simplejs_sleep(10);
    }

    simplejs_gc_event(true);
    printf("closed GC thread\n");

    return 0;
}

typedef struct
{
    bool has_initialized;

    int *array;
    int length;
    int current_index;

    char *name;
} int_array_context_t;

int test_array[] = {9, 1, 8, 5, 0, 3, 2, 6, 4, 7};
int test_sorted[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int test_reverse[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
int test_random[] = {42, 7, 91, 3, 56, 12, 88, 1, 34, 67};
int test_duplicates[] = {5, 2, 8, 2, 5, 1, 8, 1, 5, 2};
int test_all_equal[] = {7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
int test_alternating[] = {0, 9, 1, 8, 2, 7, 3, 6, 4, 5};
int test_nearly_sorted[] = {0, 1, 2, 3, 5, 4, 6, 7, 8, 9};
int test_negative[] = {-5, 3, -10, 0, 8, -1, 4, -7, 2, -3};
int test_extremes[] = {
    2147483647, -2147483647 - 1, 0, 1, -1,
    2147483646, -2147483647, 100, -100, 42};
int test_small[] = {2, 1};

typedef struct test_list
{
    char *name;
    int *array;
    int length;
} test_list_t;

#define TEST_LIST_ENTRY(test_array) {.name = #test_array, .array = test_array, .length = sizeof(test_array) / sizeof(*test_array)}

test_list_t test_arrays[] = {
    TEST_LIST_ENTRY(test_sorted),
    TEST_LIST_ENTRY(test_reverse),
    TEST_LIST_ENTRY(test_random),
    TEST_LIST_ENTRY(test_duplicates),
    TEST_LIST_ENTRY(test_all_equal),
    TEST_LIST_ENTRY(test_alternating),
    TEST_LIST_ENTRY(test_nearly_sorted),
    TEST_LIST_ENTRY(test_negative),
    TEST_LIST_ENTRY(test_extremes),
    TEST_LIST_ENTRY(test_small),
};

void dump_array(int_array_context_t *int_ctx)
{
    printf("int_ctx->array (%s) = {", int_ctx->name);

    for (int i = 0; i < int_ctx->length; i++)
    {
        printf("%d", int_ctx->array[i]);

        if ((i + 1) < int_ctx->length)
            printf(", ");
    }

    printf("}\n");
}

size_t int_sort_get_entry_count(simplejs_generic_sort_ctx_t *sort_ctx)
{
    const int_array_context_t *int_ctx = simplejs_generic_sort_get_context(sort_ctx);

    return int_ctx->length;
}

void int_sort_goto_pointer(simplejs_generic_sort_ctx_t *sort_ctx, bool direction)
{
    int_array_context_t *temp_int_array_ctx = simplejs_generic_sort_get_temp_context(sort_ctx);
    memcpy(temp_int_array_ctx, simplejs_generic_sort_get_context(sort_ctx), simplejs_generic_sort_get_context_size(sort_ctx));
    temp_int_array_ctx->has_initialized = true;

    temp_int_array_ctx->current_index = (temp_int_array_ctx->length - 1) * (int)direction;
}

bool int_sort_check_pointer(simplejs_generic_sort_ctx_t *sort_ctx)
{
    int_array_context_t *temp_int_array_ctx = simplejs_generic_sort_get_temp_context(sort_ctx);

    return temp_int_array_ctx->has_initialized &&
           temp_int_array_ctx->current_index >= 0 && temp_int_array_ctx->current_index < temp_int_array_ctx->length;
}

void int_sort_prev_pointer(simplejs_generic_sort_ctx_t *sort_ctx)
{
    int_array_context_t *temp_int_array_ctx = simplejs_generic_sort_get_temp_context(sort_ctx);

    if (temp_int_array_ctx->has_initialized &&
        temp_int_array_ctx->current_index > 0)
        temp_int_array_ctx->current_index--;
}

void int_sort_next_pointer(simplejs_generic_sort_ctx_t *sort_ctx)
{
    int_array_context_t *temp_int_array_ctx = simplejs_generic_sort_get_temp_context(sort_ctx);

    if (temp_int_array_ctx->has_initialized &&
        temp_int_array_ctx->current_index < temp_int_array_ctx->length)
        temp_int_array_ctx->current_index++;
}

float int_sort_diff_f32(simplejs_generic_sort_ctx_t *sort_a_ctx, simplejs_generic_sort_ctx_t *sort_b_ctx)
{
    int_array_context_t *a_int_array_ctx = simplejs_generic_sort_get_temp_context(sort_a_ctx);
    int_array_context_t *b_int_array_ctx = simplejs_generic_sort_get_temp_context(sort_b_ctx);

    if (a_int_array_ctx->current_index >= a_int_array_ctx->length ||
        b_int_array_ctx->current_index >= b_int_array_ctx->length)
        return 0;

    int backup_a = a_int_array_ctx->array[a_int_array_ctx->current_index];
    int backup_b = b_int_array_ctx->array[b_int_array_ctx->current_index];

    return (float)backup_a - (float)backup_b;
}

void int_sort_swap_entries(simplejs_generic_sort_ctx_t *sort_a_ctx, simplejs_generic_sort_ctx_t *sort_b_ctx)
{
    int_array_context_t *a_int_array_ctx = simplejs_generic_sort_get_temp_context(sort_a_ctx);
    int_array_context_t *b_int_array_ctx = simplejs_generic_sort_get_temp_context(sort_b_ctx);

    if (a_int_array_ctx->current_index >= a_int_array_ctx->length ||
        b_int_array_ctx->current_index >= b_int_array_ctx->length)
        return;

    int backup_a = a_int_array_ctx->array[a_int_array_ctx->current_index];
    int backup_b = b_int_array_ctx->array[b_int_array_ctx->current_index];

    a_int_array_ctx->array[a_int_array_ctx->current_index] = backup_b;
    b_int_array_ctx->array[b_int_array_ctx->current_index] = backup_a;
}

void test_generic_sort()
{
    int_array_context_t int_ctx;

    simplejs_generic_sort_ctx_t *generic_sort = NULL;
    simplejs_alloc_generic_sort(&generic_sort, &int_ctx, sizeof(int_ctx));

    SIMPLEJS_ASSERT(generic_sort != NULL);

    for (int i = 0; i < sizeof(test_arrays) / sizeof(*test_arrays); i++)
    {
        test_list_t *test_entry = &test_arrays[i];

        memclr(&int_ctx, sizeof(int_ctx));
        int_ctx.name = test_entry->name;
        int_ctx.array = test_entry->array;
        int_ctx.length = test_entry->length;

        simplejs_generic_sort_define_get_entry_count_callback(generic_sort, int_sort_get_entry_count);
        simplejs_generic_sort_define_goto_pointer_callback(generic_sort, int_sort_goto_pointer);
        simplejs_generic_sort_define_check_pointer_callback(generic_sort, int_sort_check_pointer);
        simplejs_generic_sort_define_prev_pointer_callback(generic_sort, int_sort_prev_pointer);
        simplejs_generic_sort_define_next_pointer_callback(generic_sort, int_sort_next_pointer);
        simplejs_generic_sort_define_diff_f32_callback(generic_sort, int_sort_diff_f32);
        simplejs_generic_sort_define_swap_entries_callback(generic_sort, int_sort_swap_entries);

        printf("before: ");
        dump_array(&int_ctx);

        uint64_t iterations = 0;
        simplejs_selection_sort_half_f32(generic_sort, &iterations);

        printf("after (%llu iterations): ", (long long)iterations);
        dump_array(&int_ctx);
    }

    simplejs_free_generic_sort(generic_sort);
}

int main(int argc, char **argv)
{
    char *abs_file_path = NULL;
    simplejs_map_buffer_t *source_code = NULL;
    simplejs_thread_t *gc_thread = NULL;
    simplejs_linemap_ctx_t *linemap_ctx = NULL;
    simplejs_token_ctx_t *token_ctx = NULL;
    simplejs_parser_ctx_t *parser_ctx = NULL;
    simplejs_compiler_ctx_t *compiler_ctx = NULL;
    simplejs_vm_memory_t *vm_memory = NULL;
    simplejs_vm_t *vm = NULL;
    simplejs_object_t *global_object = NULL;

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_init_list_entry(&plugin_list, &plugin_list);

    status = simplejs_init();
    if (!SIMPLEJS_SUCCESS(status))
    {
        printf("cannot init simplejs\n");
        goto result;
    }

#if 0
    test_generic_sort();

    return 0;
#endif

    int result = processArgs(argc, argv);
    if (result)
        return result;

    if (!file_path)
    {
        printf("no file path was specified!\n");
        return 1;
    }

    abs_file_path = simplejs_realpath(file_path);
    if (!abs_file_path)
    {
        printf("cannot get absolute file path\n");

        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    status = simplejs_create_map_buffer_file(&source_code, abs_file_path, SIMPLEJS_MAP_BUFFER_FILE_FLAG_READ);
    if (!SIMPLEJS_SUCCESS(status))
    {
        printf("cannot create map buffer file\n");
        goto result;
    }

    status = simplejs_create_thread(gc_thread_callback, NULL, &gc_thread);
    if (!SIMPLEJS_SUCCESS(status))
    {
        printf("cannot create gc thread\n");
        goto result;
    }

    status = simplejs_generate_linemap(abs_file_path, source_code, &linemap_ctx);
    if (!SIMPLEJS_SUCCESS(status))
    {
        printf("simplejs_generate_linemap error\n");
        goto result;
    }

    status = simplejs_tokenize(linemap_ctx, &token_ctx);
    if (!SIMPLEJS_SUCCESS(status))
    {
        printf("simplejs_tokenize error\n");
        goto result;
    }

    simplejs_tokenize_dump_tokens(token_ctx);

    printf("(simplejs_tokenize_dump_tokens) pointers allocated %d\n", get_memory_amount());

    status = simplejs_tokens_to_ast(token_ctx, &parser_ctx);
    if (!SIMPLEJS_SUCCESS(status))
    {
        printf("simplejs_tokens_to_ast error\n");
        goto result;
    }

    printf("(simplejs_tokens_to_ast) pointers allocated %d\n", get_memory_amount());

    status = simplejs_ast_to_bytecode(parser_ctx, &compiler_ctx);
    if (!SIMPLEJS_SUCCESS(status))
    {
        printf("simplejs_ast_to_bytecode error\n");
        goto result;
    }

    void *executable = NULL;
    uint32_t executable_size = 0;

    simplejs_compiler_ctx_get_executable(compiler_ctx, &executable, &executable_size);

    if (export_bytecode_path)
    {
        FILE *file = fopen(export_bytecode_path, "wb");

        fseek(file, 0, SEEK_SET);
        fwrite(executable, executable_size, 1, file);
        fclose(file);
    }

    status = simplejs_builtin_create_dynamic_object(&global_object);
    if (!SIMPLEJS_SUCCESS(status))
    {
        printf("simplejs_builtin_create_dynamic_object error\n");
        goto result;
    }

    {
        simplejs_list_entry_t *end_plugin = &plugin_list;
        simplejs_list_entry_t *current_plugin = end_plugin->next;

        while (current_plugin != end_plugin)
        {
            plugin_entry_t *plugin = simplejs_get_list_entry_structure(current_plugin);

            status = plugin->plugin_function(global_object);
            if (!SIMPLEJS_SUCCESS(status))
            {
                printf("plugin->plugin_function failed with '%s'\n", simplejs_get_status_string(status));
                goto result;
            }

            current_plugin = current_plugin->next;
        }
    }

    status = simplejs_create_vm_memory(vm_memory_size, &vm_memory);
    if (!SIMPLEJS_SUCCESS(status))
    {
        printf("simplejs_create_vm_memory error\n");
        goto result;
    }

    simplejs_vm_executable_t *vm_executable = simplejs_vm_upload_executable(vm_memory, linemap_ctx, executable, executable_size);
    if (!vm_executable)
    {
        printf("vm_executable is too big!\n");

        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    simplejs_printf("start bloat memory iteration\n");
    for (int i = 0; i < 2; i++)
    {
        simplejs_printf("bloat memory iteration %d\n", i);

        uintptr_t bloat_mem_size = 16 * 1024 * 1024;
        void *bloat_mem = simplejs_vm_memory_alloc(vm_memory, bloat_mem_size);
        SIMPLEJS_ASSERT(bloat_mem != NULL);

        memclr(bloat_mem, bloat_mem_size);
        simplejs_vm_memory_free(vm_memory, bloat_mem);
    }
    simplejs_printf("end bloat memory iteration\n");

    status = simplejs_create_vm(&vm);
    if (!SIMPLEJS_SUCCESS(status))
    {
        printf("simplejs_create_vm error\n");
        goto result;
    }

    simplejs_vm_set_memory(vm, vm_memory);

    printf("vm started!\n");

    double start_time, end_time;
    double elapsed_time;

    start_time = simplejs_get_timestamp_f64();

    simplejs_variable_t return_variable;
    simplejs_variable_init_undefined(&return_variable);

    simplejs_variable_t global_variable;
    simplejs_variable_init_object(&global_variable, global_object, 0);

    simplejs_vm_set_global_variable(vm, &global_variable);
    simplejs_gc_add_object(global_object);

    void *sandbox_executable;
    size_t sandbox_executable_size;
    simplejs_vm_executable_get_pointer(vm_executable, &sandbox_executable, &sandbox_executable_size);

    simplejs_function_t main_function = {
        .type = SIMPLEJS_FUNCTION_TYPE_NATIVE,
        .value = {
            .instruction_pointer = simplejs_compiler_get_executable_entry_point(sandbox_executable, sandbox_executable_size),
        },
    };

    status = simplejs_vm_call_function(vm, &main_function, &return_variable, NULL, 0, true);

    while (SIMPLEJS_SUCCESS(status))
    {
        status = simplejs_execute_vm(vm);
    }

    end_time = simplejs_get_timestamp_f64();
    elapsed_time = end_time - start_time;

    printf("vm exited with '%s' on %f seconds\n", simplejs_get_status_string(status), elapsed_time);

    simplejs_variable_dereference(&return_variable);

result:
    if (vm)
        simplejs_destroy_vm(vm);

    if (vm_memory)
        simplejs_destroy_vm_memory(vm_memory);

    if (compiler_ctx)
        simplejs_free_compiler_ctx(compiler_ctx);

    if (parser_ctx)
        simplejs_free_parser_ctx(parser_ctx);

    if (token_ctx)
        simplejs_free_token_ctx(token_ctx);

    if (linemap_ctx)
        simplejs_free_linemap_ctx(linemap_ctx);

    if (gc_thread)
    {
        run_gc_thread = false;
        simplejs_destroy_thread(gc_thread);
    }

    if (source_code)
        simplejs_destroy_map_buffer(source_code);

    if (abs_file_path)
        free(abs_file_path);

    free_plugin_list();

    simplejs_uninit();

    printf("pointers allocated %d\n", get_memory_amount());
    return 0;
}
