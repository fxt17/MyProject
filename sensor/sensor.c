#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include "../common/protocol.h"
#include "../common/msg_queue.h"

int main()
{
    int msgid = msg_create();

    msg_t msg;

    msg.type = MSG_SENSOR;

    while(1)
    {
        msg.data.sensor.temperature = rand()%10 + 20;
        msg.data.sensor.humidity = rand()%20 + 40;

        msgsnd(msgid, &msg, sizeof(msg.data), 0);

        printf("sensor send: temp=%d hum=%d\n",
               msg.data.sensor.temperature,
               msg.data.sensor.humidity);

        sleep(5);
    }
}