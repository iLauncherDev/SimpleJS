#include <lib/realpath.h>
#include <stdlib.h>

pchar simplejs_platform_realpath(char *relative_path)
{
    return _fullpath(NULL, relative_path, 0);
}
