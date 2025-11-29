// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define KEY 0x11110000

typedef struct {
    long mtype;
    char content[256];
} mymsgbuf;

int main() {
    int msgid;
    mymsgbuf msg;
    int client_id;
    int bid;

    // Connect to the message queue
    msgid = msgget(KEY, 0666);
    if(msgid < 0) {
        perror("msgget");
        exit(1);
    }

    // Enter client ID
    printf("Enter your Client ID (integer): ");
    if(scanf("%d", &client_id) != 1) {
        printf("Invalid ID\n");
        exit(1);
    }
    getchar(); // consume newline

    printf("\nAuction started! You can bid anytime by entering a value.\n");
    printf("Format: <bid_amount>\n");
    printf("Example: 100\n");

    while(1) {
        printf("\nEnter your bid (or 0 to skip): ");
        if(scanf("%d", &bid) != 1) {
            printf("Invalid input. Try again.\n");
            while(getchar() != '\n'); // clear input buffer
            continue;
        }
        getchar(); // consume newline

        if(bid < 0) {
            printf("Bid cannot be negative.\n");
            continue;
        }

        // Prepare message: "<ClientID> <Bid>"
        msg.mtype = 2;
        snprintf(msg.content, sizeof(msg.content), "%d %d", client_id, bid);

        if(msgsnd(msgid, &msg, sizeof(msg.content), 0) < 0) {
            perror("msgsnd");
        } else {
            printf("Bid submitted: %d\n", bid);
        }
    }

    return 0;
}
