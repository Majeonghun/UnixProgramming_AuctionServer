// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define KEY 0x11110000

typedef struct {
    long mtype;
    char content[256];
} mymsgbuf;

int client_id;
int msgid;

// Thread to receive winner notification
void* winner_listener(void* arg) {
    mymsgbuf msg;

    while (1) {
        if (msgrcv(msgid, &msg, sizeof(msg.content), client_id, 0) >= 0) {
            printf("\n===== AUCTION RESULT =====\n");
            printf("%s\n", msg.content);
            printf("==========================\n");
            exit(0);
        }
    }
    return NULL;
}

int main() {
    mymsgbuf msg;
    int bid;
    pthread_t listener;

    msgid = msgget(KEY, 0666);
    if (msgid < 0) {
        perror("msgget");
        exit(1);
    }

    printf("Enter your Client ID: ");
    scanf("%d", &client_id);
    getchar();

    pthread_create(&listener, NULL, winner_listener, NULL);

    printf("Auction started. You can bid anytime.\n");

    while (1) {
        printf("\nEnter your bid (0 to keep previous): ");
        if (scanf("%d", &bid) != 1) {
            while (getchar() != '\n');
            continue;
        }
        getchar();

        if (bid < 0) {
            printf("Bid cannot be negative.\n");
            continue;
        }

        msg.mtype = 2;
        snprintf(msg.content, sizeof(msg.content), "%d %d", client_id, bid);

        if (msgsnd(msgid, &msg, sizeof(msg.content), 0) >= 0) {
            printf("Bid submitted: %d\n", bid);
        }
    }

    return 0;
}
