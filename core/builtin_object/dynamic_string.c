#include <builtin_object/dynamic_string.h>
#include <simplejs/lib/pool.h>
#include <simplejs/lib/utf8.h>

const uint32_t dynamic_string_block_size = 16;
simplejs_proxy_t *dynamic_string_proxy = NULL;
simplejs_pool_t *dynamic_string_pool = NULL;

static inline size_t align_to_dynamic_string_block(size_t value)
{
    return (value + (dynamic_string_block_size - 1)) & ~(size_t)(dynamic_string_block_size - 1);
}

simplejs_status_t simplejs_dynamic_string_get_hash_string(simplejs_proxy_context_t context, simplejs_hash_string_ctx_t *out, simplejs_proxy_get_string_mode_t mode)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_string_raw_t *dynamic_string = context.pointer;

    bool is_open = mode == SIMPLEJS_PROXY_GET_STRING_MODE_OPEN;
    bool is_close = mode == SIMPLEJS_PROXY_GET_STRING_MODE_CLOSE;

    if (is_open)
        simplejs_spinlock_acquire(&dynamic_string->buffer_lock, true);

    if (dynamic_string->string)
    {
        if (out)
        {
            if (dynamic_string->is_dirty)
            {
                dynamic_string->is_dirty = false;

                dynamic_string->string_hash = simplejs_hash_string_generate_hash(dynamic_string->string);
            }

            out->hash = dynamic_string->string_hash;
            out->string = dynamic_string->string;
        }
    }

    if (is_close)
        simplejs_spinlock_release(&dynamic_string->buffer_lock);
    return status;
}

simplejs_status_t simplejs_dynamic_string_get_string(simplejs_proxy_context_t context, const char **out, simplejs_proxy_get_string_mode_t mode)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_string_raw_t *dynamic_string = context.pointer;

    bool is_open = mode == SIMPLEJS_PROXY_GET_STRING_MODE_OPEN;
    bool is_close = mode == SIMPLEJS_PROXY_GET_STRING_MODE_CLOSE;

    if (is_open)
        simplejs_spinlock_acquire(&dynamic_string->buffer_lock, true);

    if (dynamic_string->string)
    {
        if (out)
            *out = dynamic_string->string;
    }

    if (is_close)
        simplejs_spinlock_release(&dynamic_string->buffer_lock);
    return status;
}

static inline size_t relocate_offset_to_valid_char(simplejs_dynamic_string_raw_t *dynamic_string, size_t offset)
{
    size_t new_offset = offset;

    if (dynamic_string->string && offset < dynamic_string->valid_size)
    {
        while (new_offset > 1)
        {
            char chr1 = dynamic_string->string[new_offset];
            if (!simplejs_utf8_is_continuation_byte(chr1))
                break;

            new_offset--;
        }
    }

    return new_offset;
}

static simplejs_status_t simplejs_dynamic_string_on_valid_size_change(simplejs_dynamic_string_raw_t *dynamic_string, size_t old_value)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    if (dynamic_string->std_flags & (SIMPLEJS_PROXY_STD_FLAG_PERMA_LOCK | SIMPLEJS_PROXY_STD_FLAG_READ_ONLY))
        goto result;

    dynamic_string->valid_size = relocate_offset_to_valid_char(dynamic_string, dynamic_string->valid_size);

    if (dynamic_string->valid_size > old_value)
        dynamic_string->valid_size = old_value;
    else if (dynamic_string->valid_size != old_value)
        dynamic_string->is_dirty = true;

    dynamic_string->string[dynamic_string->valid_size] = '\0';

result:
    return status;
}

static simplejs_status_t simplejs_dynamic_string_on_allocated_size_change(simplejs_dynamic_string_raw_t *dynamic_string, size_t old_value)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    if (dynamic_string->std_flags & (SIMPLEJS_PROXY_STD_FLAG_PERMA_LOCK | SIMPLEJS_PROXY_STD_FLAG_READ_ONLY))
        goto result;

    size_t new_offset = relocate_offset_to_valid_char(dynamic_string, dynamic_string->allocated_size);
    size_t allocation_size = align_to_dynamic_string_block(new_offset + 1);
    size_t copy_start_size = dynamic_string->string ? dynamic_string->valid_size : 0;
    if (copy_start_size > new_offset)
        copy_start_size = new_offset;

    dynamic_string->allocated_size = old_value;

    if (dynamic_string->allocated_size != allocation_size)
    {
        char *new_string = simplejs_pool_malloc(dynamic_string_pool, allocation_size);
        if (!new_string)
        {
            status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
            goto result;
        }

        if (dynamic_string->string)
            memcpy(new_string, dynamic_string->string, copy_start_size + 1);

        if (dynamic_string->is_heap_allocated && dynamic_string->string)
            simplejs_pool_mfree(dynamic_string_pool, dynamic_string->string);

        dynamic_string->is_heap_allocated = true;
        dynamic_string->string = new_string;
        dynamic_string->allocated_size = allocation_size;
    }

    if (dynamic_string->valid_size != copy_start_size)
    {
        dynamic_string->is_dirty = true;

        dynamic_string->valid_size = copy_start_size;
        dynamic_string->string[dynamic_string->valid_size] = '\0';
    }

result:
    return status;
}

simplejs_status_t SIMPLEJS_API simplejs_builtin_dynamic_string_get_size(simplejs_object_t *object, size_t *out_valid_size, size_t *out_allocated_size)
{
    SIMPLEJS_ASSERT(out_valid_size != NULL || out_allocated_size != NULL);

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_object_lock_property_list(object, 0);

    if (simplejs_object_get_proxy(object) != dynamic_string_proxy)
    {
        status = SIMPLEJS_STATUS_INVALID_PARAMETER;
        goto result;
    }

    simplejs_dynamic_string_raw_t *dynamic_string = simplejs_object_get_raw_object(object);

    if (out_valid_size)
        *out_valid_size = dynamic_string->valid_size;
    if (out_allocated_size)
        *out_allocated_size = dynamic_string->allocated_size;

result:
    simplejs_object_unlock_property_list(object, 0);
    return status;
}

simplejs_status_t simplejs_builtin_dynamic_string_write_ex(simplejs_object_t *object, const char *string, size_t offset, bool avoid_lock)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_string_raw_t *dynamic_string = NULL;

    if (!avoid_lock)
        simplejs_object_lock_property_list(object, 0);

    if (simplejs_object_get_proxy(object) != dynamic_string_proxy)
    {
        status = SIMPLEJS_STATUS_INVALID_PARAMETER;
        goto result;
    }

    dynamic_string = simplejs_object_get_raw_object(object);
    simplejs_spinlock_acquire(&dynamic_string->buffer_lock, true);

    if (dynamic_string->std_flags & (SIMPLEJS_PROXY_STD_FLAG_PERMA_LOCK | SIMPLEJS_PROXY_STD_FLAG_READ_ONLY))
        goto result;

    offset = relocate_offset_to_valid_char(dynamic_string, offset);

    char discard_buffer[5];
    uint32_t unicode_old, unicode_out;

    size_t i, j;
    size_t chr_size = 0;
    size_t req_chr_size = 0;
    size_t additional_size = 0;

    size_t copy_start_size = dynamic_string->string ? dynamic_string->valid_size : 0;

    i = offset, j = 0;
    while (string[j] != '\0')
    {
        uint8_t old_len, new_len, out_len;

        simplejs_utf8_decode_char(&dynamic_string->string[i], &dynamic_string->string[copy_start_size], &old_len, &unicode_old);
        simplejs_utf8_decode_char(&string[j], (const char *)((uintptr_t)-1), &new_len, &unicode_out);
        out_len = simplejs_utf8_get_unicode_char_size(unicode_out);

        int diff_sign = (int)out_len - (int)old_len;

        if (i < copy_start_size)
        {
            additional_size += diff_sign;
        }
        else
        {
            additional_size = 0;
        }

        chr_size += new_len;
        req_chr_size += out_len;
        i += old_len, j += new_len;
    }

    size_t required_size = additional_size != 0 ? copy_start_size + additional_size : offset + req_chr_size;
    size_t required_allocation = align_to_dynamic_string_block(required_size + 1);

    if (!dynamic_string->string || required_allocation > dynamic_string->allocated_size)
    {
        size_t old_allocated_size = dynamic_string->allocated_size;
        dynamic_string->allocated_size = required_allocation;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_dynamic_string_on_allocated_size_change(dynamic_string, old_allocated_size), result, status);
    }

    if (additional_size != 0 || required_size > dynamic_string->valid_size)
    {
        dynamic_string->is_dirty = true;
        dynamic_string->valid_size = required_size;

        dynamic_string->string[dynamic_string->valid_size] = '\0';
    }

    if (offset > copy_start_size)
        memset(&dynamic_string->string[copy_start_size], ' ', offset - copy_start_size);

    i = offset, j = 0;
    while (j < chr_size)
    {
        uint8_t old_len, new_len, out_len;

        simplejs_utf8_decode_char(&dynamic_string->string[i], &dynamic_string->string[dynamic_string->valid_size], &old_len, &unicode_old);
        simplejs_utf8_decode_char(&string[j], (const char *)((uintptr_t)-1), &new_len, &unicode_out);
        simplejs_utf8_encode_char(unicode_out, discard_buffer, &discard_buffer[(sizeof(discard_buffer) / sizeof(*discard_buffer)) - 1], &out_len);

        dynamic_string->is_dirty |= unicode_old != unicode_out;

        int diff_sign = (int)out_len - (int)old_len;
        int diff_mask = -(diff_sign < 0);
        int diff_len = ((-diff_sign) & diff_mask) | (diff_sign & ~diff_mask);

        if (additional_size != 0)
        {
            dynamic_string->is_dirty |= diff_len != 0;

            if (out_len > old_len)
            {
                memcpy(&dynamic_string->string[i + diff_len], &dynamic_string->string[i], dynamic_string->valid_size - (i + diff_len));
            }
            else if (out_len < old_len)
            {
                memcpy(&dynamic_string->string[i], &dynamic_string->string[i + diff_len], dynamic_string->valid_size - (i + diff_len));
            }
        }

        memcpy(&dynamic_string->string[i], discard_buffer, out_len);

        i += out_len;
        j += new_len;
    }

result:
    if (dynamic_string)
        simplejs_spinlock_release(&dynamic_string->buffer_lock);

    if (!avoid_lock)
        simplejs_object_unlock_property_list(object, 0);
    return status;
}

simplejs_status_t SIMPLEJS_API simplejs_builtin_dynamic_string_write(simplejs_object_t *object, const char *string, size_t offset)
{
    return simplejs_builtin_dynamic_string_write_ex(object, string, offset, false);
}

simplejs_status_t simplejs_dynamic_string_lock_property_list(simplejs_proxy_context_t context)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_string_raw_t *dynamic_string = context.pointer;

    simplejs_spinlock_acquire(&dynamic_string->property_lock, true);

    return status;
}

simplejs_status_t simplejs_dynamic_string_unlock_property_list(simplejs_proxy_context_t context)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_string_raw_t *dynamic_string = context.pointer;

    simplejs_spinlock_release(&dynamic_string->property_lock);

    return status;
}

simplejs_status_t simplejs_dynamic_string_release(simplejs_proxy_context_t context)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_string_raw_t *dynamic_string = context.pointer;

    simplejs_dynamic_string_lock_property_list(context);

    if (dynamic_string->is_heap_allocated && dynamic_string->string)
        simplejs_pool_mfree(dynamic_string_pool, dynamic_string->string);

    simplejs_hook_mfree(dynamic_string);
    return status;
}

simplejs_status_t simplejs_dynamic_string_set_std_flags(simplejs_proxy_context_t context, uint32_t std_flags)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_string_raw_t *dynamic_string = context.pointer;
    if (dynamic_string->std_flags & SIMPLEJS_PROXY_STD_FLAG_PERMA_LOCK)
        goto result;

    dynamic_string->std_flags |= std_flags;

result:
    return status;
}

simplejs_status_t simplejs_dynamic_string_clear_std_flags(simplejs_proxy_context_t context, uint32_t std_flags)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_string_raw_t *dynamic_string = context.pointer;
    if (dynamic_string->std_flags & SIMPLEJS_PROXY_STD_FLAG_PERMA_LOCK)
        goto result;

    dynamic_string->std_flags &= ~std_flags;

result:
    return status;
}

static bool simplejs_dynamic_string_find_property(simplejs_dynamic_string_raw_t *dynamic_string, simplejs_variable_t *property, simplejs_dynamic_string_property_t *out)
{
    bool status = true;

    memclr(out, sizeof(*out));

    if (property->type == SIMPLEJS_VARIABLE_TYPE_NUMBER)
    {
        size_t offset = simplejs_number_get_intptr(&property->value.number);

        out->type = SIMPLEJS_DYNAMIC_STRING_PROPERTY_TYPE_OFFSET;
        out->offset = offset;
    }
    else
    {
        char tempBuffer[4096];
        simplejs_hash_string_ctx_t hash_property;
        simplejs_variable_to_hash_string(property, tempBuffer, sizeof(tempBuffer), &hash_property, SIMPLEJS_PROXY_GET_STRING_MODE_OPEN);

        for (size_t i = 0; i < dynamic_string->property_list_count; i++)
        {
            simplejs_dynamic_string_property_t *entry = &dynamic_string->property_list[i];

            if (entry->hash == hash_property.hash)
            {
                *out = *entry;
                goto result;
            }
        }

        simplejs_variable_to_hash_string(property, NULL, 0, &hash_property, SIMPLEJS_PROXY_GET_STRING_MODE_CLOSE);

        status = false;
    }

result:
    return status;
}

simplejs_status_t simplejs_dynamic_string_get_property_value(simplejs_proxy_context_t context, simplejs_variable_t *property, simplejs_variable_t *out, simplejs_variable_t *out_object_level)
{
    if (out_object_level)
    {
        simplejs_variable_t tmp_var;
        simplejs_variable_init_object(&tmp_var, context.object, context.object_value);
        simplejs_variable_assign(out_object_level, &tmp_var);
    }

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_string_raw_t *dynamic_string = context.pointer;

    simplejs_object_lock_property_list(context.object, context.object_value);

    simplejs_variable_t tmp_out;
    simplejs_variable_init_undefined(&tmp_out);

    simplejs_dynamic_string_property_t dynamic_string_property;
    if (!simplejs_dynamic_string_find_property(dynamic_string, property, &dynamic_string_property))
        goto result;

    simplejs_number_t tmp_num = {0};
    simplejs_function_t tmp_func = {0};

    switch (dynamic_string_property.type)
    {
    case SIMPLEJS_DYNAMIC_STRING_PROPERTY_TYPE_OFFSET:
    {
        size_t string_offset = relocate_offset_to_valid_char(dynamic_string, dynamic_string_property.offset);

        if (string_offset < dynamic_string->valid_size)
        {
            uint8_t discard_len;

            simplejs_spinlock_acquire(&dynamic_string->buffer_lock, true);

            tmp_num.type = SIMPLEJS_NUMBER_TYPE_UNICODE;
            simplejs_utf8_decode_char(&dynamic_string->string[string_offset], &dynamic_string->string[dynamic_string->valid_size], &discard_len, &tmp_num.value.unicode);
            simplejs_variable_init_number(&tmp_out, &tmp_num);

            simplejs_spinlock_release(&dynamic_string->buffer_lock);
        }

        break;
    }

    case SIMPLEJS_DYNAMIC_STRING_PROPERTY_TYPE_FIELD_OFFSET_SIZE_T:
    {
        size_t *ptr_value = (void *)((uint8_t *)dynamic_string + dynamic_string_property.offset);

        tmp_num.type = SIMPLEJS_NUMBER_TYPE_UIPTR;
        tmp_num.value.uiptr = *ptr_value;
        simplejs_variable_init_number(&tmp_out, &tmp_num);
        break;
    }

    case SIMPLEJS_DYNAMIC_STRING_PROPERTY_TYPE_FUNCTION_PROXY:
    {
        tmp_func.type = SIMPLEJS_FUNCTION_TYPE_PROXY;
        tmp_func.value.proxy = dynamic_string_property.function_proxy;
        simplejs_variable_init_function(&tmp_out, &tmp_func);
        break;
    }

    default:
        break;
    }

result:
    simplejs_object_unlock_property_list(context.object, context.object_value);

    simplejs_variable_assign(out, &tmp_out);
    return status;
}

simplejs_status_t simplejs_dynamic_string_set_property_value(simplejs_proxy_context_t context, simplejs_variable_t *property, simplejs_variable_t *in)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_string_raw_t *dynamic_string = context.pointer;

    simplejs_object_lock_property_list(context.object, context.object_value);

    if (dynamic_string->std_flags & (SIMPLEJS_PROXY_STD_FLAG_PERMA_LOCK | SIMPLEJS_PROXY_STD_FLAG_READ_ONLY))
        goto result;

    simplejs_dynamic_string_property_t dynamic_string_property;
    if (!simplejs_dynamic_string_find_property(dynamic_string, property, &dynamic_string_property))
        goto result;

    size_t input_value = simplejs_number_get_intptr(&in->value.number);

    switch (dynamic_string_property.type)
    {
    case SIMPLEJS_DYNAMIC_STRING_PROPERTY_TYPE_OFFSET:
    {
        size_t string_offset = relocate_offset_to_valid_char(dynamic_string, dynamic_string_property.offset);

        if (in->type != SIMPLEJS_VARIABLE_TYPE_NUMBER)
        {
            char tempBuffer[4096];
            const char *in_string;

            simplejs_variable_to_string(in, tempBuffer, sizeof(tempBuffer), &in_string, SIMPLEJS_PROXY_GET_STRING_MODE_OPEN);
            if (in_string != dynamic_string->string)
                simplejs_builtin_dynamic_string_write_ex(context.object, in_string, string_offset, true);
            simplejs_variable_to_string(in, tempBuffer, sizeof(tempBuffer), &in_string, SIMPLEJS_PROXY_GET_STRING_MODE_CLOSE);
        }
        else
        {
            uint8_t discard_len;
            char utf8_buffer[5] = {0};
            simplejs_utf8_encode_char(input_value, utf8_buffer, &utf8_buffer[sizeof(utf8_buffer) - 1], &discard_len);

            simplejs_builtin_dynamic_string_write_ex(context.object, utf8_buffer, string_offset, true);
        }

        break;
    }

    case SIMPLEJS_DYNAMIC_STRING_PROPERTY_TYPE_FIELD_OFFSET_SIZE_T:
    {
        if (in->type != SIMPLEJS_VARIABLE_TYPE_NUMBER)
            break;

        size_t *ptr_value = (void *)((uint8_t *)dynamic_string + dynamic_string_property.offset);
        size_t old_value = *ptr_value;

        (*ptr_value) = input_value;

        if (dynamic_string_property.on_change_f)
            dynamic_string_property.on_change_f(dynamic_string, old_value);
        break;
    }

    default:
        break;
    }

result:
    simplejs_object_unlock_property_list(context.object, context.object_value);
    return status;
}

simplejs_status_t simplejs_dynamic_string_delete_property(simplejs_proxy_context_t context, simplejs_variable_t *property)
{
    return SIMPLEJS_STATUS_SUCCESS;
}

simplejs_status_t simplejs_dynamic_string_proxy_write(simplejs_function_header_t *function_header)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    if (function_header->argument_count < 2)
        goto result;

    simplejs_variable_t *string_argument = &function_header->arguments[0];
    simplejs_variable_t *offset_argument = &function_header->arguments[1];

    void *object;
    simplejs_variable_get_object(&function_header->this_variable, &object, NULL);

    size_t offset = simplejs_number_get_intptr(&offset_argument->value.number);

    char tempBuffer[4096];
    const char *temp_string;

    simplejs_variable_to_string(string_argument, tempBuffer, sizeof(tempBuffer), &temp_string, SIMPLEJS_PROXY_GET_STRING_MODE_OPEN);
    simplejs_builtin_dynamic_string_write(object, temp_string, offset);
    simplejs_variable_to_string(string_argument, NULL, 0, &temp_string, SIMPLEJS_PROXY_GET_STRING_MODE_CLOSE);

result:
    return status;
}

simplejs_status_t simplejs_dynamic_string_proxy_charLen(simplejs_function_header_t *function_header)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    if (function_header->argument_count < 1)
        goto result;

    simplejs_variable_t *offset_argument = &function_header->arguments[0];

    void *object;
    simplejs_variable_get_object(&function_header->this_variable, &object, NULL);

    size_t offset = simplejs_number_get_intptr(&offset_argument->value.number);

    if (simplejs_object_get_proxy(object) != dynamic_string_proxy)
        goto result;

    uint32_t unicode;
    uint8_t length = 0;
    char tempBuffer[4];

    simplejs_dynamic_string_raw_t *dynamic_string = simplejs_object_get_raw_object(object);

    simplejs_object_lock_property_list(object, 0);
    simplejs_spinlock_acquire(&dynamic_string->buffer_lock, true);

    offset = relocate_offset_to_valid_char(dynamic_string, offset);
    simplejs_utf8_decode_char(&dynamic_string->string[offset], &dynamic_string->string[dynamic_string->valid_size], &length, &unicode);

    simplejs_spinlock_release(&dynamic_string->buffer_lock);
    simplejs_object_unlock_property_list(object, 0);

    simplejs_number_t tmp_num;
    tmp_num.type = SIMPLEJS_NUMBER_TYPE_UI32;
    tmp_num.value.ui32 = length;

    simplejs_variable_t tmp_var;
    simplejs_variable_init_number(&tmp_var, &tmp_num);
    simplejs_variable_assign(function_header->return_variable, &tmp_var);

result:
    return status;
}

simplejs_dynamic_string_property_t builtin_propertries[] = {
    {
        .name = "length",
        .type = SIMPLEJS_DYNAMIC_STRING_PROPERTY_TYPE_FIELD_OFFSET_SIZE_T,
        .offset = (uintptr_t)(&((simplejs_dynamic_string_raw_t *)0)->valid_size),
        .on_change_f = simplejs_dynamic_string_on_valid_size_change,
    },

    {
        .name = "valid_size",
        .type = SIMPLEJS_DYNAMIC_STRING_PROPERTY_TYPE_FIELD_OFFSET_SIZE_T,
        .offset = (uintptr_t)(&((simplejs_dynamic_string_raw_t *)0)->valid_size),
        .on_change_f = simplejs_dynamic_string_on_valid_size_change,
    },

    {
        .name = "allocated_size",
        .type = SIMPLEJS_DYNAMIC_STRING_PROPERTY_TYPE_FIELD_OFFSET_SIZE_T,
        .offset = (uintptr_t)(&((simplejs_dynamic_string_raw_t *)0)->allocated_size),
        .on_change_f = simplejs_dynamic_string_on_allocated_size_change,
    },

    {
        .name = "write",
        .type = SIMPLEJS_DYNAMIC_STRING_PROPERTY_TYPE_FUNCTION_PROXY,
        .function_proxy = simplejs_dynamic_string_proxy_write,
    },

    {
        .name = "charLen",
        .type = SIMPLEJS_DYNAMIC_STRING_PROPERTY_TYPE_FUNCTION_PROXY,
        .function_proxy = simplejs_dynamic_string_proxy_charLen,
    },
};

simplejs_status_t SIMPLEJS_API simplejs_create_builtin_dynamic_string(simplejs_object_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_string_raw_t *dynamic_string = NULL;
    simplejs_object_t *object = NULL;

    dynamic_string = simplejs_hook_malloc(sizeof(*dynamic_string));
    if (!dynamic_string)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }
    memclr(dynamic_string, sizeof(*dynamic_string));

    dynamic_string->property_list = builtin_propertries;
    dynamic_string->property_list_count = sizeof(builtin_propertries) / sizeof(*builtin_propertries);

    simplejs_init_spinlock(&dynamic_string->property_lock);
    simplejs_init_spinlock(&dynamic_string->buffer_lock);

    dynamic_string->is_dirty = true;
    dynamic_string->string = "";
    dynamic_string->allocated_size = 1;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_object(dynamic_string, dynamic_string_proxy, &object), result, status);

    *out = object;
result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (dynamic_string)
            simplejs_hook_mfree(dynamic_string);

        if (object)
            simplejs_free_object(object);
    }

    return status;
}

simplejs_status_t simplejs_builtin_init_dynamic_string()
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_proxy(&dynamic_string_proxy), result, status);

    simplejs_proxy_define_release_callback(dynamic_string_proxy, simplejs_dynamic_string_release);

    simplejs_proxy_define_set_std_flags_callback(dynamic_string_proxy, simplejs_dynamic_string_set_std_flags);
    simplejs_proxy_define_clear_std_flags_callback(dynamic_string_proxy, simplejs_dynamic_string_clear_std_flags);

    simplejs_proxy_define_lock_property_list_callback(dynamic_string_proxy, simplejs_dynamic_string_lock_property_list);
    simplejs_proxy_define_unlock_property_list_callback(dynamic_string_proxy, simplejs_dynamic_string_unlock_property_list);
    // simplejs_proxy_define_query_property_callback(dynamic_string_proxy, simplejs_dynamic_string_query_property);

    simplejs_proxy_define_get_property_value_callback(dynamic_string_proxy, simplejs_dynamic_string_get_property_value);
    simplejs_proxy_define_set_property_value_callback(dynamic_string_proxy, simplejs_dynamic_string_set_property_value);
    simplejs_proxy_define_delete_property_callback(dynamic_string_proxy, simplejs_dynamic_string_delete_property);

    simplejs_proxy_define_get_hash_string_callback(dynamic_string_proxy, simplejs_dynamic_string_get_hash_string);
    simplejs_proxy_define_get_string_callback(dynamic_string_proxy, simplejs_dynamic_string_get_string);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_create_pool(dynamic_string_block_size, &dynamic_string_pool), result, status);

    for (int i = 0; i < sizeof(builtin_propertries) / sizeof(*builtin_propertries); i++)
    {
        simplejs_dynamic_string_property_t *entry = &builtin_propertries[i];

        entry->hash = simplejs_hash_string_generate_hash(entry->name);
    }

result:
    return status;
}

void simplejs_builtin_uninit_dynamic_string()
{
    if (dynamic_string_proxy)
        simplejs_free_proxy(dynamic_string_proxy);

    if (dynamic_string_pool)
        simplejs_destroy_pool(dynamic_string_pool);
}