#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <mosquitto.h>
#include "../common/protocol.h"
#include "../common/msg_queue.h"

#define MQTT_HOST "sh-5-mqtt.iot-api.com"
#define MQTT_PORT 1883
#define MQTT_USER "zj98082da66alcoz"
#define MQTT_PASS "DNNnDVmJAB"
#define MQTT_TOPIC "attributes"

int main()
{
    struct mosquitto *mosq;
    int usr_pw_set, connect;

    /* 初始化mosquitto库 */
    mosquitto_lib_init();

    mosq = mosquitto_new(NULL, true, NULL);
    if(!mosq)
    {
        printf("mosquitto_new failed\n");
        return -1;
    }

    /* 设置用户名密码 */
    usr_pw_set = mosquitto_username_pw_set(mosq, MQTT_USER, MQTT_PASS);// set username and password
    if(usr_pw_set != MOSQ_ERR_SUCCESS)
    {
        printf("mosquitto_username_pw_set failed\n");
        return -1;
    }

    /* 连接服务器 */
    connect = mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60);
    if(connect != MOSQ_ERR_SUCCESS)
    {
        printf("mosquitto_connect failed\n");
        return -1;
    }
    printf("MQTT connected\n");

    int msgid = msg_create();
    msg_t msg;  // buffer for storing msg

    while(1)
    {
        if(msgrcv(msgid, &msg, sizeof(msg.data), MSG_SENSOR, 0) == -1)
        {
            printf("msgrcv failed\n");
            return -1;
        }//read dates from sensor by IPC
        int temp = msg.data.sensor.temperature;
        int hum  = msg.data.sensor.humidity;
        /* 生成JSON */
        char payload[128];
        snprintf(payload, sizeof(payload),
                 "{\"temperature\":%d,\"humidity\":%d}",
                 temp, hum);
        printf("publish: %s\n", payload);

        /* publish */
        mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(payload), payload, 0, false);

        /* 处理网络 */
        mosquitto_loop(mosq, 100, 1);
    }

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}