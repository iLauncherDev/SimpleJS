#include <lib/file.h>

int64_t simplejs_platform_file_tell(simplejs_file_t *file)
{
    return (int64_t)_ftelli64((FILE *)file);
}

int simplejs_platform_file_seek(simplejs_file_t *file, int64_t offset, int whence)
{
    return _fseeki64((FILE *)file, offset, whence);
}
