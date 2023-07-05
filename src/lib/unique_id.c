#include <guise-sessions-client/unique_id.h>

size_t guiseSclUniqueIdGetIndex(GuiseSclUnique unique)
{
    return unique & 0xff;
}
