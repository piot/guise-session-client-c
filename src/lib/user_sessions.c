/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/guise-session-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include <clog/clog.h>
#include <flood/in_stream.h>
#include <guise-serialize/serialize.h>
#include <guise-sessions-client/address.h>
#include <guise-sessions-client/unique_id.h>
#include <guise-sessions-client/user_session.h>
#include <guise-sessions-client/user_sessions.h>
#include <inttypes.h>
#include <stdbool.h>

/// Initialize the user session collection
/// @param self user sessions
/// @param log target log
void guiseSclUserSessionsInit(GuiseSclUserSessions* self, Clog log)
{
    self->log = log;
    self->capacity = 1024;
    self->sessions = tc_malloc_type_count(GuiseSclUserSession, self->capacity);
    tc_mem_clear_type_n(self->sessions, self->capacity);
}

void guiseSclUserSessionsReset(GuiseSclUserSessions* self)
{
    for (size_t i = 0; i < self->capacity; ++i) {
        GuiseSclUserSession* session = &self->sessions[i];
        session->userId = 0;
    }
}

void guiseSclUserSessionsDestroy(GuiseSclUserSessions* self)
{
    self->capacity = 0;
    tc_free(self->sessions);
}

/*
int guiseSclUserSessionsPrepare(GuiseSclUserSessions* self, GuiseSerializeUserSessionId userSessionId,
                                const GuiseSclAddress* address, GuiseSerializeUserId userId,
                                GuiseSerializeUserName userName, GuiseSclUserSession** outSession)
{
}
*/
int guiseSclUserSessionsConfirm(GuiseSclUserSessions* self, GuiseSerializeUserSessionId userSessionId,
                                const GuiseSclAddress* address, GuiseSerializeUserId userId,
                                GuiseSerializeUserName userName, GuiseSclUserSession** outSession)
{
    *outSession = 0;

    size_t index = userSessionId & 0xffff;
    if (index >= self->capacity) {
        CLOG_C_SOFT_ERROR(&self->log, "index from session id is outside of capacity %zu out of %zu", index,
                          self->capacity)
        return -4;
    }

    GuiseSclUserSession* session = &self->sessions[index];

    if (session->userSessionId != userSessionId || !guiseSclAddressEqual(&session->address, address)) {
        CLOG_C_NOTICE(&self->log, "we got a reply from the guise server that don't add up. ignoring")
        return -5;
    }

    session->userName = userName;
    session->isConfirmed = true;
    session->userId = userId;

    *outSession = session;

    return -1;
}

static int guiseSclUserSessionsFind(const GuiseSclUserSessions* self, GuiseSerializeUserSessionId uniqueId,
                                    const GuiseSclAddress* addr, const GuiseSclUserSession** outSession)
{
    size_t index = guiseSclUniqueIdGetIndex(uniqueId);
    if (index >= self->capacity) {
        return -2;
    }

    GuiseSclUserSession* foundSession = &self->sessions[index];
    if (foundSession->userId == 0) {
        CLOG_C_NOTICE(&self->log, "session id is not valid or destroyed %" PRIx64, uniqueId)
        return -1;
    }

    if (foundSession->userId != uniqueId) {
        CLOG_C_SOFT_ERROR(&self->log, "wrong user session id, got %" PRIX64 " but wanted %" PRIX64, uniqueId,
                          foundSession->userId)

    }
    if (!guiseSclAddressEqual(addr, &foundSession->address)) {
        CLOG_EXECUTE(char addrTemp[64];)
        CLOG_C_SOFT_ERROR(&self->log, "wrong address %s vs %s", guiseSclAddressToString(addr, addrTemp, 64),
                          guiseSclAddressToString(&foundSession->address, addrTemp, 64))
        *outSession = 0;
        return -3;
    }

    *outSession = foundSession;

    return 0;
}

int guiseSclUserSessionsReadAndFind(const GuiseSclUserSessions* self, const GuiseSclAddress* address,
                                    FldInStream* stream, const GuiseSclUserSession** outSession)
{

    GuiseSerializeUserSessionId userSessionId;
    guiseSerializeReadUserSessionId(stream, &userSessionId);

    int errorCode = guiseSclUserSessionsFind(self, userSessionId, address, outSession);
    if (errorCode < 0) {
        CLOG_C_WARN(&self->log, "couldn't find user session %" PRIx64, userSessionId)
        return errorCode;
    }

    return 0;
}
