#ifndef GuiseScl_SERVER_LIB_USER_SESSIONS_H
#define GuiseScl_SERVER_LIB_USER_SESSIONS_H

#include <guise-serialize/types.h>
#include <guise-sessions-client/address.h>
#include <stdbool.h>

typedef struct GuiseSclUserSession {
    GuiseSerializeUserSessionId userSessionId;
    GuiseSerializeUserId userId;
    GuiseSerializeUserName userName;
    GuiseSclAddress address;
    char prefix[32];
    bool isConfirmed;
} GuiseSclUserSession;

void guiseSclUserSessionInit(GuiseSclUserSession* self, GuiseSerializeUserSessionId userSessionId,
                          GuiseSerializeUserId userId, GuiseSerializeUserName userName, const GuiseSclAddress* address);

#endif
