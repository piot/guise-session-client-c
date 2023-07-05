#ifndef GUISE_SCL_CLIENT_H
#define GUISE_SCL_CLIENT_H

#include <clog/clog.h>
#include <datagram-transport/transport.h>
#include <datagram-transport/types.h>
#include <guise-serialize/client_out.h>
#include <guise-sessions-client/address.h>
#include <guise-sessions-client/user_sessions.h>

struct GuiseSclUserSession;

typedef struct GuiseSclClient {
    GuiseSclAddress address;
    GuiseSclUserSessions sessions;
    DatagramTransport transport;
    GuiseSerializeUserSessionId toUseForQueries;
    Clog log;
    uint8_t buf[DATAGRAM_TRANSPORT_MAX_SIZE];
} GuiseSclClient;

int guiseSclClientInit(GuiseSclClient* self, DatagramTransport transport, GuiseSerializeUserSessionId toUseForQueries,
                       Clog log);
int guiseSclClientLookup(GuiseSclClient* self, const GuiseSclAddress* address, GuiseSerializeUserSessionId sessionId,
                         const struct GuiseSclUserSession** outSession);
int guiseSclClientUpdate(GuiseSclClient* self);

void guiseSclAddressToSerializeAddress(GuiseSerializeAddress* target, const GuiseSclAddress* address);
void guiseSerializeAddressToSclAddress(GuiseSclAddress* target, const GuiseSerializeAddress* address);
int guiseSclClientFeed(GuiseSclClient* self, const uint8_t* payload, size_t octetCountReceived);

#endif
