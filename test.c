#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <string.h>

#define MAX_WORKERS 10
#define GENERIC_MSG_TYPE 1  // Generic message type for all containers

typedef struct {
    long msg_type;  // Message type for distribution (used generically here)
    int id;
    int weight;
    int status;
    int altitude;
} Container;

int send_container_to_queue(int msqid, const Container *container) {
    Container new_container = *container;
    new_container.msg_type = GENERIC_MSG_TYPE;  // Use a generic message type for all containers
    if (msgsnd(msqid, &new_container, sizeof(Container) - sizeof(long), 0) == -1) {
        perror("msgsnd failed");
        return -1;
    }
    return 0;
}

int main() {

    key_t key = ftok("test.c", 'b');
    int msg_queue_id = msgget(key, 0666 | IPC_CREAT);
    if (msg_queue_id == -1) {
        perror("msgget failed");
        exit(EXIT_FAILURE);
    }

    int container_id = 1;  // Container ID starts at 1
    int i = 0;

    while (i < 100) {
        Container container = {
            .msg_type = GENERIC_MSG_TYPE,
            .id = container_id,
            .weight = 5,
            .status = container_id % 3,
            .altitude = 0
        };
        if (send_container_to_queue(msg_queue_id, &container) == -1) {
            msgctl(msg_queue_id, IPC_RMID, NULL); // Clean up the message queue
            exit(EXIT_FAILURE);
        }
        printf("Container %d sent successfully.\n", container.id);
        fflush(stdout);
        
        sleep(1);  // Sleep for 1 second before sending the next container
        container_id++;  // Increment container ID for the next container
        i++;
    }

    // Cleanup message queue at the end of the operation
    msgctl(msg_queue_id, IPC_RMID, NULL);

    return 0;
}
