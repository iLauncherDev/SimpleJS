#include <lib/file.h>

psimplejs_file_t SIMPLEJS_PLATFORM_API simplejs_file_open(const char *file_path, const char *modes)
{
    return (simplejs_file_t *)fopen(file_path, modes);
}

int64_t SIMPLEJS_PLATFORM_API simplejs_file_tell(simplejs_file_t *file)
{
    return simplejs_platform_file_tell(file);
}

int SIMPLEJS_PLATFORM_API simplejs_file_seek(simplejs_file_t *file, int64_t offset, int whence)
{
    return simplejs_platform_file_seek(file, offset, whence);
}

uint32_t SIMPLEJS_PLATFORM_API simplejs_file_read(simplejs_file_t *file, void *buffer, uint32_t length)
{
    return (uint32_t)fread(buffer, 1, length, (FILE *)file);
}

uint32_t SIMPLEJS_PLATFORM_API simplejs_file_write(simplejs_file_t *file, void *buffer, uint32_t length)
{
    return (uint32_t)fwrite(buffer, 1, length, (FILE *)file);
}

void SIMPLEJS_PLATFORM_API simplejs_file_close(simplejs_file_t *file)
{
    fclose((FILE *)file);
}
