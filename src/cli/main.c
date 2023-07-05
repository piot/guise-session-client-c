/*---------------------------------------------------------------------------------------------
 *  Copyright (c) Peter Bjorklund. All rights reserved.
 *  Licensed under the MIT License. See LICENSE in the project root for license information.
 *--------------------------------------------------------------------------------------------*/
#include "guise-sessions-client/address.h"
#include "guise-sessions-client/user_session.h"
#include <_types/_uint8_t.h>
#include <clog/console.h>
#include <flood/in_stream.h>
#include <flood/out_stream.h>
#include <flood/text_in_stream.h>
#include <guise-client/client.h>
#include <guise-client/network_realizer.h>
#include <guise-serialize/commands.h>
#include <guise-serialize/parse_text.h>
#include <guise-sessions-client/client.h>
#include <imprint/default_setup.h>
#include <inttypes.h>
#include <stdio.h>
#include <udp-client/udp_client.h>
#include <unistd.h>

clog_config g_clog;
char g_clog_temp_str[CLOG_TEMP_STR_SIZE];

static ssize_t clientReceive(void* _self, uint8_t* data, size_t size)
{
    UdpClientSocket* self = _self;

    return udpClientReceive(self, data, size);
}

static int clientSend(void* _self, const uint8_t* data, size_t size)
{
    UdpClientSocket* self = _self;

    return udpClientSend(self, data, size);
}

typedef struct Secret {
    GuiseSerializeUserId userId;
    GuiseSerializeUserName userName;
    uint64_t passwordHash;
} Secret;

static int readFirstUserInUsersTxt(Secret* secret)
{
    CLOG_DEBUG("reading users file")
    FILE* fp = fopen("users.txt", "r");
    if (fp == 0) {
        CLOG_ERROR("could not find users.txt")
        return -4;
    }

    size_t usersRead = 0;
    char line[1024];
    char* ptr = fgets(line, 1024, fp);
    if (ptr == 0) {
        return -39;
    }
    fclose(fp);

    FldTextInStream textInStream;
    FldInStream inStream;

    fldInStreamInit(&inStream, (const uint8_t*) line, tc_strlen(line));
    fldTextInStreamInit(&textInStream, &inStream);

    GuiseSerializeUserInfo userInfo;
    int err = guiseTextStreamReadUser(&textInStream, &userInfo);
    if (err < 0) {
        return err;
    }

    secret->passwordHash = userInfo.passwordHash;
    secret->userId = userInfo.userId;
    secret->userName = userInfo.userName;

    return 0;
}

int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    g_clog.log = clog_console;
    g_clog.level = CLOG_TYPE_VERBOSE;

    CLOG_VERBOSE("example start")
    CLOG_VERBOSE("initialized")

    FldOutStream outStream;

    uint8_t buf[1024];
    fldOutStreamInit(&outStream, buf, 1024);

    GuiseClientRealize clientRealize;
    GuiseClientRealizeSettings settings;

    ImprintDefaultSetup memory;

    DatagramTransport transportInOut;

    imprintDefaultSetupInit(&memory, 16 * 1024 * 1024);

    int startupErr = udpClientStartup();
    if (startupErr < 0) {
        return startupErr;
    }

    const char* hostToConnectTo = "127.0.0.1";

    if (argc > 1) {
        hostToConnectTo = argv[1];
    }

    UdpClientSocket udpClientSocket;
    udpClientInit(&udpClientSocket, hostToConnectTo, 27004);

    transportInOut.self = &udpClientSocket;
    transportInOut.receive = clientReceive;
    transportInOut.send = clientSend;

    Secret secret;
    int err = readFirstUserInUsersTxt(&secret);
    if (err < 0) {
        CLOG_ERROR("could not read: %d", err)
    }

    CLOG_INFO("first user is '%s' userId: %" PRIx64, secret.userName.utf8, secret.userId)

    settings.memory = &memory.tagAllocator.info;
    settings.transport = transportInOut;
    settings.userId = secret.userId;
    settings.secretPasswordHash = secret.passwordHash;
    Clog guiseClientLog;
    guiseClientLog.config = &g_clog;
    guiseClientLog.constantPrefix = "GuiseClient";
    settings.log = guiseClientLog;

    guiseClientRealizeInit(&clientRealize, &settings);
    guiseClientRealizeReInit(&clientRealize, &settings);

    clientRealize.state = GuiseClientRealizeStateInit;
    clientRealize.targetState = GuiseClientRealizeStateLogin;

    GuiseClientState reportedState;
    reportedState = GuiseClientStateIdle;

    GuiseSclClient sclClient;
    Clog guiseSclClientLog;
    guiseSclClientLog.config = &g_clog;
    guiseSclClientLog.constantPrefix = "GuiseSclClient";

    bool sclClientIsInitialized = false;
    uint8_t dataBuf[DATAGRAM_TRANSPORT_MAX_SIZE];

    while (true) {
        usleep(16 * 1000);

        MonotonicTimeMs now = monotonicTimeMsNow();

        guiseClientRealizeUpdateOut(&clientRealize, now);


        if (reportedState != clientRealize.client.state) {
            reportedState = clientRealize.client.state;
            if (reportedState == GuiseClientStateLoggedIn) {
                if (!sclClientIsInitialized) {
                    guiseSclClientInit(&sclClient, transportInOut, clientRealize.client.mainUserSessionId,
                                       guiseSclClientLog);
                    sclClientIsInitialized = true;
                }
            }
        }
        if (sclClientIsInitialized) {
            const GuiseSclUserSession* foundSession;
            GuiseSclAddress addressToCheck;
            guiseSerializeAddressToSclAddress(&addressToCheck, &clientRealize.client.mainNetworkAddress);
            GuiseSerializeUserSessionId sessionIdToCheck = clientRealize.client.mainUserSessionId;
            int lookupErr = guiseSclClientLookup(&sclClient, &addressToCheck, sessionIdToCheck, &foundSession);
            if (lookupErr > 0) {
                CLOG_INFO("Has info! '%s'", foundSession->userName.utf8);
            }
        }

        ssize_t octetsFound = datagramTransportReceive(&transportInOut, dataBuf, DATAGRAM_TRANSPORT_MAX_SIZE);
        if (octetsFound <= 0) {
            continue;
        }

        if (dataBuf[0] == guiseSerializeCmdConfirmInfoResponse) {
            guiseSclClientFeed(&sclClient, dataBuf, (size_t) octetsFound);
        } else {
            guiseClientFeed(&clientRealize.client, dataBuf, (size_t) octetsFound);
        }
    }

    imprintDefaultSetupDestroy(&memory);
}
