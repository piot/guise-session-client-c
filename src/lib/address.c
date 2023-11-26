/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/guise-session-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <guise-sessions-client/address.h>
#include <stdio.h>
#include <tiny-libc/tiny_libc.h>

#if defined TORNADO_OS_WINDOWS
#include <Ws2tcpip.h>
#endif

int guiseSclAddressEqual(const GuiseSclAddress* a, const GuiseSclAddress* b)
{
    return (a->sin_addr.s_addr == b->sin_addr.s_addr && a->sin_port == b->sin_port);
}

const char* guiseSclAddressToString(const GuiseSclAddress* self, char* temp, size_t maxCount)
{
    const char* converted = inet_ntop(AF_INET, &self->sin_addr, temp, (socklen_t) (maxCount - 6));

    size_t len = tc_strlen(converted);

    sprintf(&temp[len], ":%d", self->sin_port);

    return temp;
}
