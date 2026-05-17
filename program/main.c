#include <default.h>
#include <mm/gc.h>
#include <plugin.h>
#include <simplejs/lib/thread.h>
#include <simplejs/lib/shared_lib.h>
#include <simplejs/lib/sleep.h>
#include <compiler.h>
#include <vm.h>
#include <builtin_object/dynamic_object.h>

#include <time.h>

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
        else if (!simplejs_strcasecmp(string, "-plugin"))
        {
            skips = 2;

            if (remainding_i < skips)
            {
                printf("not enough parameters\n");
                return 1;
            }

            plugin_entry_t *entry = malloc(sizeof(*entry));
            if (!entry)
            {
                printf("cannot allocate plugin entry\n");
                return 1;
            }

            memclr(entry, sizeof(*entry));

            entry->name = argv_window[1];
            simplejs_init_list_entry(&entry->list_entry, entry);
            simplejs_insert_tail_list(&plugin_list, &entry->list_entry);

            if (!SIMPLEJS_SUCCESS(simplejs_load_shared_lib(entry->name, &entry->shared_lib)))
            {
                printf("cannot load plugin lib\n");
                return 1;
            }

            if (!SIMPLEJS_SUCCESS(simplejs_shared_lib_get_proc_address(entry->shared_lib, SIMPLEJS_PLUGIN_ENTRY_NAME, (void **)&entry->plugin_function)))
            {
                printf("cannot load plugin function\n");
                return 1;
            }
        }
    }

    return 0;
}

int SIMPLEJS_HOOK_API get_memory_amount();

volatile bool run_gc_thread = true;

uintptr_t gc_thread_callback(simplejs_thread_t *thread)
{
    while (run_gc_thread)
    {
        printf("executing GC event!\n");

        simplejs_gc_event();

        simplejs_sleep(5000);
    }

    simplejs_gc_event();
    printf("closed GC thread\n");

    return 0;
}

int main(int argc, char **argv)
{
    simplejs_token_ctx_t *token_ctx = NULL;
    simplejs_parser_ctx_t *parser_ctx = NULL;
    simplejs_compiler_ctx_t *compiler_ctx = NULL;
    simplejs_bytecode_vm_t *vm = NULL;
    simplejs_object_t *global_object = NULL;
    simplejs_utf8_string_t *code = NULL;
    simplejs_thread_t *gc_thread = NULL;

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_init_list_entry(&plugin_list, &plugin_list);

    status = simplejs_init();
    if (!SIMPLEJS_SUCCESS(status))
    {
        printf("cannot init simplejs\n");
        goto result;
    }

    int result = processArgs(argc, argv);
    if (result)
        return result;

    if (!file_path)
    {
        printf("no file path was specified!\n");
        return 1;
    }

    code = readFile(file_path);
    if (!code)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    status = simplejs_create_thread(gc_thread_callback, NULL, &gc_thread);
    if (!SIMPLEJS_SUCCESS(status))
    {
        printf("cannot create gc thread\n");
        goto result;
    }

    status = simplejs_tokenize(code, &token_ctx);
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

    if (export_bytecode_path)
    {
        FILE *file = fopen(export_bytecode_path, "wb");

        fseek(file, 0, SEEK_SET);
        fwrite(compiler_ctx->executable, compiler_ctx->executable_size, 1, file);
        fclose(file);
    }

    status = simplejs_builtin_create_dynamic_object(&global_object);
    if (!SIMPLEJS_SUCCESS(status))
    {
        printf("simplejs_builtin_create_dynamic_object error\n");
        goto result;
    }

    simplejs_object_reference(global_object);

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

    status = simplejs_create_vm(&vm);
    if (!SIMPLEJS_SUCCESS(status))
    {
        printf("simplejs_create_vm error\n");
        goto result;
    }

    simplejs_variable_t global_variable;
    global_variable.type = SIMPLEJS_VARIABLE_TYPE_OBJECT;
    global_variable.value.object = global_object;

    simplejs_variable_assign(&vm->state.global_variable, &global_variable);
    simplejs_gc_add_object(global_object);

    simplejs_bytecode_header_t *header = (void *)compiler_ctx->executable;
    uint32_t header_size = ((uint32_t)header->size_low << 0) | ((uint32_t)header->size_high << 8);

    vm->state.instruction_pointer = (uintptr_t)compiler_ctx->executable + header_size;

    printf("vm started!\n");

    clock_t start_time, end_time;
    double elapsed_time;

    start_time = clock();

    while (SIMPLEJS_SUCCESS(status))
    {
        status = simplejs_execute_vm(vm);
    }

    end_time = clock();
    elapsed_time = (double)(end_time - start_time) / (double)CLOCKS_PER_SEC;

    printf("vm exited with '%s' on %f seconds\n", simplejs_get_status_string(status), elapsed_time);

result:
    if (global_object)
        simplejs_object_dereference(global_object);

    if (vm)
        simplejs_destroy_vm(vm);

    if (compiler_ctx)
        simplejs_free_compiler_ctx(compiler_ctx);

    if (parser_ctx)
        simplejs_free_parser_ctx(parser_ctx);

    if (token_ctx)
        simplejs_free_token_ctx(token_ctx);

    if (gc_thread)
    {
        run_gc_thread = false;
        simplejs_destroy_thread(gc_thread);
    }

    if (code)
        free(code);

    free_plugin_list();

    simplejs_uninit();

    printf("pointers allocated %d\n", get_memory_amount());
    return 0;
}
