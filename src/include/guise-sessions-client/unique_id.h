/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef GUISE_SESSIONS_CLIENT_UNIQUE_H
#define GUISE_SESSIONS_CLIENT_UNIQUE_H

#include <stdint.h>
#include <stddef.h>

typedef uint64_t GuiseSclUnique;

size_t guiseSclUniqueIdGetIndex(GuiseSclUnique unique);

#endif
