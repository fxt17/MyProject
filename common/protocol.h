#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MSG_SENSOR  1
#define MSG_DEVICE  2

typedef struct
{
    long type;

    union
    {
        struct
        {
            int temperature;
            int humidity;
        }sensor;

        struct
        {
            int led;
        }device;

    }data;

}msg_t;

#endif