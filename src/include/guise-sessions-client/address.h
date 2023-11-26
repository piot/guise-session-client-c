/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/guise-session-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#ifndef GUISE_SESSION_CLIENT_ADDRESS_H
#define GUISE_SESSION_CLIENT_ADDRESS_H

#if defined TORNADO_OS_WINDOWS
#include <WinSock2.h>
#include <Windows.h>
#else
#include <netinet/in.h>
#endif

typedef struct sockaddr_in GuiseSclAddress;
int guiseSclAddressEqual(const GuiseSclAddress* a, const GuiseSclAddress* b);
const char* guiseSclAddressToString(const GuiseSclAddress* self, char* temp, size_t maxCount);

#endif
