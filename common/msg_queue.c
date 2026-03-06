#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include "msg_queue.h"

/**
 * 
 */

int msg_create()
{
    key_t key = ftok("/tmp", 1);

    int msgid = msgget(key, IPC_CREAT | 0666);

    if(msgid < 0)
    {
        perror("msgget");
    }

    return msgid;
}
