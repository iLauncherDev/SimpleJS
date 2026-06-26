#include <lib/realpath.h>
#include <limits.h>

pchar simplejs_platform_realpath(char *relative_path)
{
    return realpath(relative_path, NULL);
}
