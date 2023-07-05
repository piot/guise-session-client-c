#include <guise-sessions-client/user_session.h>

void guiseSclUserSessionInit(GuiseSclUserSession* self, GuiseSerializeUserSessionId userSessionId,
                             GuiseSerializeUserId userId, GuiseSerializeUserName userName,
                             const GuiseSclAddress* address)
{
    self->userSessionId = userSessionId;
    self->userId = userId;
    self->userName = userName;
    self->address = *address;
}
