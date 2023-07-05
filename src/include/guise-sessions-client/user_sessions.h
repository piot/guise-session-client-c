/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#ifndef GUISE_SERVER_GUISE_SESSIONS_H
#define GUISE_SERVER_GUISE_SESSIONS_H

#include <clog/clog.h>
#include <guise-sessions-client/address.h>
#include <stdlib.h>

struct GuiseSclUserSession;
struct sockaddr_in;
struct FldInStream;
struct GuiseSclUser;

typedef struct GuiseSclUserSessions
{
    struct GuiseSclUserSession *sessions;
    size_t capacity;
    size_t count;
    Clog log;
    char prefix[32];
} GuiseSclUserSessions;

void guiseSclUserSessionsInit(GuiseSclUserSessions *self, Clog log);
void guiseSclUserSessionsDestroy(GuiseSclUserSessions *self);
void guiseSclUserSessionsReset(GuiseSclUserSessions *self);
int guiseSclUserSessionsCreate(GuiseSclUserSessions *sessions, struct GuiseSclUser *user, const GuiseSclAddress *address,
                               struct GuiseSclUserSession **outSession);
int guiseSclUserSessionsReadAndFind(const GuiseSclUserSessions *self, const GuiseSclAddress *address, struct FldInStream *stream,
                                    const struct GuiseSclUserSession **outSession);

int guiseSclUserSessionsConfirm(GuiseSclUserSessions* self, GuiseSerializeUserSessionId userSessionId,
                                const GuiseSclAddress* address, GuiseSerializeUserId userId,
                                GuiseSerializeUserName userName, struct GuiseSclUserSession** outSession);


#endif
