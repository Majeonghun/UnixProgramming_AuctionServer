// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_CLIENTS 10
#define KEY 0x11110000

typedef struct {
    long mtype;
    char content[256];
} mymsgbuf;

typedef struct {
    int client_id;
    int bid;
} client_t;

client_t clients[MAX_CLIENTS];
int client_count = 0;

int last_highest_bid = 0;
int last_highest_client_id = -1;

int auction_over = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int msgid;

// Find highest bidder
int highest_bidder(int* highest) {
    int i;
    int max_bid = 0;
    int winner_id = -1;
    int tie_count = 0;

    pthread_mutex_lock(&lock);
    for (i = 0; i < client_count; i++) {
        if (clients[i].bid > max_bid) {
            max_bid = clients[i].bid;
            winner_id = clients[i].client_id;
            tie_count = 1;
        }
        else if (clients[i].bid == max_bid && max_bid > 0) {
            tie_count++;
        }
    }
    pthread_mutex_unlock(&lock);

    *highest = max_bid;

    if (tie_count == 1 && max_bid > 0)
        return winner_id;
    else
        return -1;
}

// Print round results
void print_round(int round) {
    int i;
    printf("\n===== Round %d =====\n", round);
    printf("Client ID | Bid\n");
    printf("----------------\n");

    pthread_mutex_lock(&lock);
    for (i = 0; i < client_count; i++) {
        printf("%9d | %d\n", clients[i].client_id, clients[i].bid);
    }
    pthread_mutex_unlock(&lock);

    printf("----------------\n");
}

// Receive thread for bids
void* receive_thread(void* arg) {
    mymsgbuf msg;
    int cid, bid;
    int i;
    int found;

    while (!auction_over) {
        if (msgrcv(msgid, &msg, sizeof(msg.content), 2, 0) < 0) {
            continue;
        }

        if (sscanf(msg.content, "%d %d", &cid, &bid) == 2) {
            pthread_mutex_lock(&lock);
            found = 0;

            for (i = 0; i < client_count; i++) {
                if (clients[i].client_id == cid) {
                    clients[i].bid = bid;
                    found = 1;
                    break;
                }
            }

            if (!found && client_count < MAX_CLIENTS) {
                clients[client_count].client_id = cid;
                clients[client_count].bid = bid;
                client_count++;
            }

            pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}

int main() {
    int i;
    int round = 1;

    char item_name[128];
    char item_desc[256];

    // Reset message queue
    msgid = msgget(KEY, 0666);
    if (msgid >= 0) msgctl(msgid, IPC_RMID, NULL);

    msgid = msgget(KEY, IPC_CREAT | 0666);
    if (msgid < 0) {
        perror("msgget");
        exit(1);
    }

    // Input item info
    printf("Enter item name: ");
    fgets(item_name, sizeof(item_name), stdin);
    item_name[strcspn(item_name, "\n")] = 0;

    printf("Enter item description: ");
    fgets(item_desc, sizeof(item_desc), stdin);
    item_desc[strcspn(item_desc, "\n")] = 0;

    printf("\nItem: %s\nDescription: %s\n", item_name, item_desc);
    printf("Auction will begin.\n");

    pthread_t tid;
    pthread_create(&tid, NULL, receive_thread, NULL);

    while (!auction_over) {
        printf("\n--- Round %d start ---\n", round);

        // Reset only for Round 1
        if (round == 1) {
            pthread_mutex_lock(&lock);
            for (i = 0; i < client_count; i++) {
                clients[i].bid = 0;
            }
            pthread_mutex_unlock(&lock);
        }

        sleep(30);  // Round duration

        print_round(round);

        {
            int highest;
            int winner_id = highest_bidder(&highest);

            if (winner_id == -1) {
                printf("No unique highest bidder. Next round.\n");
                last_highest_bid = 0;
                last_highest_client_id = -1;
            }
            else {
                if (winner_id == last_highest_client_id &&
                    highest == last_highest_bid) {

                    printf("\n***** AUCTION OVER *****\n");
                    printf("Winner: Client %d with bid %d\n", winner_id, highest);

                    // ★ Send winning message to the winner
                    mymsgbuf winmsg;
                    winmsg.mtype = winner_id;
                    snprintf(winmsg.content, sizeof(winmsg.content),
                        "You won the auction with bid %d!", highest);

                    msgsnd(msgid, &winmsg, sizeof(winmsg.content), 0);

                    auction_over = 1;
                }
                else {
                    printf("Highest bidder this round: Client %d with bid %d\n",
                        winner_id, highest);
                    last_highest_bid = highest;
                    last_highest_client_id = winner_id;
                }
            }
        }

        round++;
    }

    pthread_join(tid, NULL);
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
