/*----------------------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved. https://github.com/piot/guise-session-client-c
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------------------*/
#include "guise-sessions-client/address.h"
#include <flood/in_stream.h>
#include <flood/out_stream.h>
#include <guise-serialize/client_out.h>
#include <guise-serialize/query_client_in.h>
#include <guise-serialize/query_client_out.h>
#include <guise-sessions-client/client.h>
#include <guise-sessions-client/user_session.h>
#include <inttypes.h>

int guiseSclClientInit(GuiseSclClient* self, DatagramTransport transport, GuiseSerializeUserSessionId toUseForQueries,
                       Clog log)
{
    guiseSclUserSessionsInit(&self->sessions, log);
    self->transport = transport;
    self->toUseForQueries = toUseForQueries;
    CLOG_ASSERT(toUseForQueries != 0, "user session id zero is reserved")
    self->log = log;

    return 0;
}

void guiseSclAddressToSerializeAddress(GuiseSerializeAddress* target, const GuiseSclAddress* address)
{
    target->address.ipv4[0] = address->sin_addr.s_addr >> 24;
    target->address.ipv4[1] = (address->sin_addr.s_addr >> 16) & 0xff;
    target->address.ipv4[2] = (address->sin_addr.s_addr >> 8) & 0xff;
    target->address.ipv4[3] = address->sin_addr.s_addr & 0xff;
    target->port = address->sin_port;
    target->type = GuiseSerializeAddressTypeV4;
}

void guiseSerializeAddressToSclAddress(GuiseSclAddress* target, const GuiseSerializeAddress* address)
{
    switch (address->type) {
        case GuiseSerializeAddressTypeV4:
            target->sin_addr.s_addr = ((in_addr_t) address->address.ipv4[0] << 24) |
                                      ((in_addr_t) address->address.ipv4[1] << 16) |
                                      ((in_addr_t) address->address.ipv4[2] << 8) |
                                      ((in_addr_t) address->address.ipv4[3]);
            target->sin_port = address->port;
            break;
        case GuiseSerializeAddressTypeV6:
            CLOG_ERROR("not supported yet v6")
    }
}

static void sendRequestToServer(GuiseSclClient* self, GuiseSerializeUserSessionId sessionIdToLookup,
                                const GuiseSclAddress* address)
{
    FldOutStream outStream;

    fldOutStreamInit(&outStream, self->buf, DATAGRAM_TRANSPORT_MAX_SIZE);

    GuiseSerializeAddress serializeAddress;

    guiseSclAddressToSerializeAddress(&serializeAddress, address);

    guiseSerializeClientOutRequestInfo(&outStream, self->toUseForQueries, sessionIdToLookup, &serializeAddress);
    CLOG_C_INFO(&self->log,
                "sending session id query to server. userID that requests %" PRIX64 " for session %" PRIx64 " port: %d",
                self->toUseForQueries, sessionIdToLookup, address->sin_port)

    self->transport.send(self->transport.self, outStream.octets, outStream.pos);
}

/// Either returns an existing information about the session, or sends a request to the guise server for information
/// @param self scl client
/// @param address address
/// @param sessionId sessionId
/// @param outSession outSession
/// @return negative on error
int guiseSclClientLookup(GuiseSclClient* self, const GuiseSclAddress* address, GuiseSerializeUserSessionId sessionId,
                         const struct GuiseSclUserSession** outSession)
{
    size_t index = sessionId & 0xffff;
    if (index >= self->sessions.capacity) {
        CLOG_C_SOFT_ERROR(&self->log, "index from session id is outside of capacity %zu out of %zu", index,
                          self->sessions.capacity)
        return -4;
    }

    GuiseSclUserSession* userSession = &self->sessions.sessions[index];
    if (userSession->isConfirmed) {
        *outSession = userSession;
        return 0;
    }

    userSession->userSessionId = sessionId;
    userSession->userId = 0;
    userSession->isConfirmed = false;
    userSession->address = *address;

    sendRequestToServer(self, sessionId, address);
    *outSession = 0;

    return -1;
}

static int onReceiveConfirmResponse(GuiseSclClient* self, const uint8_t* data, size_t octetCount)
{
    GuiseSerializeConfirmResponse response;

    FldInStream inStream;
    fldInStreamInit(&inStream, data, octetCount);

    int err = guiseSerializeReadGetInfoResponse(&response, &inStream);
    if (err < 0) {
        return err;
    }

    GuiseSclUserSession* outSession;

    GuiseSclAddress sclAddress;

    guiseSerializeAddressToSclAddress(&sclAddress, &response.address);

    guiseSclUserSessionsConfirm(&self->sessions, response.userSessionId, &sclAddress, response.userId,
                                response.userName, &outSession);

    return 0;
}

int guiseSclClientFeed(GuiseSclClient* self, const uint8_t* payload, size_t octetCountReceived)
{
    switch (payload[0]) {
        case guiseSerializeCmdConfirmInfoResponse: {
            int err = onReceiveConfirmResponse(self, payload + 1, (size_t) (octetCountReceived - 1));
            if (err < 0) {
                return err;
            }
            break;
        }
        default:
            CLOG_C_NOTICE(&self->log, "unknown command %02X", payload[0])
            return -44;
    }

    return 0;
}

static int readAllDatagrams(GuiseSclClient* self)
{
    for (size_t i = 0; i < 30; ++i) {
        ssize_t octetCountReceived = datagramTransportReceive(&self->transport, self->buf, DATAGRAM_TRANSPORT_MAX_SIZE);
        if (octetCountReceived <= 0) {
            return (int) octetCountReceived;
        }

        int err = guiseSclClientFeed(self, self->buf, (size_t) octetCountReceived);
        if (err < 0) {
            return err;
        }
    }

    return 0;
}

int guiseSclClientUpdate(GuiseSclClient* self)
{
    return readAllDatagrams(self);
}
