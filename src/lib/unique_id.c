/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/guise-session-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <guise-sessions-client/unique_id.h>

size_t guiseSclUniqueIdGetIndex(GuiseSclUnique unique)
{
    return unique & 0xff;
}
