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

/* 连接回调 */
void on_connect(struct mosquitto *mosq, void *userdata, int rc)
{
    if(rc == 0)
    {
        printf("Connected to MQTT Broker\n");

        /* 订阅控制主题 */
        mosquitto_subscribe(mosq, NULL, SUB_TOPIC, 0);
    }
    else
    {
        printf("Connect failed: %d\n", rc);
    }
}

/* 接收消息回调 */
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg)
{
    printf("Receive topic: %s\n", msg->topic);
    printf("Payload: %s\n", (char *)msg->payload);

    if(strcmp((char*)msg->payload, "LED_ON") == 0)
        printf("LED on\n");
    else if(strcmp((char*)msg->payload, "LED_OFF") == 0)
        printf("LED off\n");
}

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
    if(mosquitto_username_pw_set(mosq, MQTT_USER, MQTT_PASS) != MOSQ_ERR_SUCCESS)
    {
        printf("mosquitto_username_pw_set failed\n");
        return -1;
    }// set username and password

    /* 设置回调 */
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);

    /* 自动重连 */
    mosquitto_reconnect_delay_set(mosq, 2, 10, true);//最小重连等待时间2s，指数退避为true时重连时间递增至10s，若为false则固定2s重连

    /* 连接服务器 */
    if(mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 60) != MOSQ_ERR_SUCCESS)
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