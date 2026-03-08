#include <stdio.h>
#include <sys/msg.h>
#include "../common/protocol.h"
#include "../common/msg_queue.h"

int main()
{
    int msgid = msg_create();

    msg_t msg;  // buffer for storing msg

    while(1)
    {
        msgrcv(msgid, &msg, sizeof(msg.data), MSG_SENSOR, 0); //

        printf("mqtt recv sensor: temp=%d hum=%d\n",
               msg.data.sensor.temperature,
               msg.data.sensor.humidity);
    }
}