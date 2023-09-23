#ifndef IPC_H
#define IPC_H

#include <QString>

namespace mu::ipc {
static QString serverName()
{
    // Make this application version dependent, as in to having devel communicate with others
    return "ipc-server-jeetee";
}

static const int TIMEOUT_MSEC(500);

static const char* REPLY_ACK_OK { "ACK_OK" };
static const char* REPLY_NACK_NOK { "NACK_NOK" };
static const char* REQ_IS_CHILD { "IS_CHILD" };
static const char* REQ_MAKE_CHILD { "MAKE_CHILD" };
}

#endif // IPC_H
