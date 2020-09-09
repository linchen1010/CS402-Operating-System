#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "cs402.h"
#include "my402list.h"

typedef struct tagPacket {
    int pktID;
    int numToken;
    int serviceTime;
    double pktIntervalArrivalTIme;
    double arrivalTime;
    double q1EnterTime, q1LeaveTime;
    double q2EnterTime, q2LeaveTime;
    double startServerTime;
    double quitTime;
} Packet;

/* ----------------------- global variables ----------------------- */
My402List queue1, queue2;
int tokenCount = 0;
int tokenInBucket = 0;
int tokenDrop = 0;
int pktOrder = 1;
char *tsfile;
FILE *fp = NULL;
// Time stamps variables
double arrStart, tokStart;
double emulationStart, emulationEnd;
///
int arrivalThreadWorking = TRUE;
int tokenThreadWorking = TRUE;
int time_to_quit = 0;
// default to Deterministic mode, if specified tfile, mode = 'T'
char emulationMode = 'D';
// simulation parameters
double lambda, mu, r;
int B, P;
int num_packets;
/* ----------------------- pthread varaibles ----------------------- */
pthread_t arrThread, tokThread, ser1Thread, ser2Thread;
pthread_mutex_t mutex;
pthread_cond_t queue_cv;

/* ----------------------- function part  ----------------------- */

double getInstantTime() {
    struct timeval currTime;
    gettimeofday(&currTime, NULL);
    return 1000 * (currTime.tv_sec) + (currTime.tv_usec) / 1000.0;
}

void Usage() {
    fprintf(stderr,
            "\nUsage: ./warmup2 [-lambda lambda] [-mu mu] [-r r] [-B B] [-P P] "
            "[-n num] "
            "[-t tsfile] \n");
}

void readCommandLine(int argc, char *argv[]) {
    if (argc > 15) {
        fprintf(stderr, "Error: Invalid command!");
        Usage();
        exit(0);
    }

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-lambda") == 0) {
            if (i + 1 < argc) {
                if (sscanf(argv[i + 1], "%lf", &lambda) != 1) {
                    fprintf(stderr,
                            "Error: Invalid command, the amount of lambda "
                            "\"%s\" is not correct!",
                            argv[i + 1]);
                    Usage();
                    exit(0);
                } else {
                    sscanf(argv[i + 1], "%lf", &lambda);
                }
                if (lambda < 0) {
                    fprintf(stderr, "Error: lambda can't not be negative!\n");
                    exit(0);
                }
            } else {
                fprintf(stderr,
                        "Error: Invalid command, missing some arguments!");
                Usage();
                exit(0);
            }
        } else if (strcmp(argv[i], "-mu") == 0) {
            if (i + 1 < argc) {
                if (sscanf(argv[i + 1], "%lf", &mu) != 1) {
                    fprintf(stderr,
                            "Error: Invalid command, the amount of mu "
                            "\"%s\" is not correct!",
                            argv[i + 1]);
                    Usage();
                    exit(0);
                } else {
                    sscanf(argv[i + 1], "%lf", &mu);
                }
                if (mu < 0) {
                    fprintf(stderr, "Error: mu can't not be negative!\n");
                    exit(0);
                }
            } else {
                fprintf(stderr,
                        "Error: Invalid command, missing some arguments!");
                Usage();
                exit(0);
            }
        } else if (strcmp(argv[i], "-r") == 0) {
            if (i + 1 < argc) {
                if (sscanf(argv[i + 1], "%lf", &r) != 1) {
                    fprintf(stderr,
                            "Error: Invalid command, the amount of r "
                            "\"%s\" is not correct!",
                            argv[i + 1]);
                    Usage();
                    exit(0);
                } else {
                    sscanf(argv[i + 1], "%lf", &r);
                }
                if (r < 0) {
                    fprintf(stderr, "Error: r can't not be negative!\n");
                    exit(0);
                }
            } else {
                fprintf(stderr,
                        "Error: Invalid command, missing some arguments!");
                Usage();
                exit(0);
            }
        } else if (strcmp(argv[i], "-B") == 0) {
            if (i + 1 < argc) {
                if (sscanf(argv[i + 1], "%d", &B) != 1) {
                    fprintf(stderr,
                            "Error: Invalid command, the amount of B "
                            "\"%s\" is not correct!",
                            argv[i + 1]);
                    Usage();
                    exit(0);
                } else {
                    sscanf(argv[i + 1], "%d", &B);
                }
                if (B < 0) {
                    fprintf(stderr, "Error: B can't not be negative!\n");
                    exit(0);
                }
            } else {
                fprintf(stderr,
                        "Error: Invalid command, missing some arguments!");
                Usage();
                exit(0);
            }
        } else if (strcmp(argv[i], "-P") == 0) {
            if (i + 1 < argc) {
                if (sscanf(argv[i + 1], "%d", &P) != 1) {
                    fprintf(stderr,
                            "Error: Invalid command, the amount of P "
                            "\"%s\" is not correct!",
                            argv[i + 1]);
                    Usage();
                    exit(0);
                } else {
                    sscanf(argv[i + 1], "%d", &P);
                }
                if (P < 0) {
                    fprintf(stderr, "Error: P can't not be negative!\n");
                    exit(0);
                }
            } else {
                fprintf(stderr,
                        "Error: Invalid command, missing some arguments!");
                Usage();
                exit(0);
            }
        } else if (strcmp(argv[i], "-n") == 0) {
            if (i + 1 < argc) {
                if (sscanf(argv[i + 1], "%d", &num_packets) != 1) {
                    fprintf(stderr,
                            "Error: Invalid command, the amount of num "
                            "\"%s\" is not correct!",
                            argv[i + 1]);
                    Usage();
                    exit(0);
                } else {
                    sscanf(argv[i + 1], "%d", &num_packets);
                }
                if (num_packets > 2147483647) {
                    fprintf(stderr,
                            "Error: num (of packets) could not be greater than "
                            "2147483647! \n");
                    exit(0);
                } else if (num_packets < 0) {
                    fprintf(stderr,
                            "Error: num (of packet) could not be negative! \n");
                    exit(0);
                }
            } else {
                fprintf(stderr,
                        "Error: Invalid command, missing some arguments!");
                Usage();
                exit(0);
            }
        } else if (strcmp(argv[i], "-t") == 0) {
            if (i + 1 < argc) {
                tsfile = argv[i + 1];
                emulationMode = 'T';
            } else {
                fprintf(stderr, "Error: Missing tsfile Name!");
                Usage();
                exit(0);
            }
        } else {
            fprintf(stderr,
                    "Error: Invalid command (incorrect input arguments or "
                    "missing arguments)!");
            Usage();
            exit(0);
        }
    }
}

void readFile(char *tsfile) {
    fp = fopen(tsfile, "r");
    if (fp == NULL) {
        fprintf(stderr,
                "Error: Malformed command or input file \"%s\" "
                "does not exist ...\n",
                tsfile);
        Usage();
        exit(0);
    }
    char buf[1024];
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fprintf(stderr, "Error: The tsfile is empty!\n");
        exit(0);
    } else {
        char *start_ptr = buf;
        char *tab_ptr = strchr(start_ptr, '\t');
        if (tab_ptr != NULL) {
            fprintf(
                stderr,
                "Error: First line indicates the number of packets, it should "
                "only be one nubmer! \n");
            exit(0);
        }
        num_packets = atoi(start_ptr);
        if (num_packets <= 0) {
            fprintf(stderr,
                    "Error: %s is not in the right format! (num of packets "
                    "is not valid)\n",
                    tsfile);
            exit(0);
        }
    }
}

// need to generate packet in the specified way, then create a packet and
// add it to Q1 if this thread is moving a packet into Q2, it needs to wake
// up a sleeping server thread -> call pthread_cond_broadcase()
void *pktArrivalThread(void *id) {
    // Packet *myPacket = malloc(sizeof(myPacket));
    int sleepTime = 0;
    double prevPktArrTime = 0;
    // fprintf(stdout, "I'm in arrThread!\n");
    for (int i = 0; i < num_packets; i++) {
        Packet *myPacket = (Packet *)malloc(sizeof(Packet));
        // Trace-driven mode
        if (emulationMode == 'T') {
            char buf[1024];
            if (fgets(buf, sizeof(buf), fp) != NULL) {
                if (strlen(buf) > 1024) {
                    fprintf(stderr,
                            "Error: There should be at most 1024 characters in "
                            "each line in the tfile\n");
                }
            }
            myPacket->pktID = pktOrder++;
            char *tmp;
            tmp = strtok(buf, " \n");
            myPacket->pktIntervalArrivalTIme = atof(tmp);
            tmp = strtok(NULL, " \n\t");
            myPacket->numToken = atoi(tmp);
            tmp = strtok(NULL, " \n\t");
            myPacket->serviceTime = atoi(tmp);

            sleepTime = myPacket->pktIntervalArrivalTIme * 1000;

            usleep(sleepTime);
            myPacket->arrivalTime = getInstantTime() - arrStart;
            pthread_mutex_lock(&mutex);

            if (myPacket->numToken <= B) {
                fprintf(stdout,
                        "%012.3fms: p%d arrives, needs %d tokens, "
                        "inter-arrival time = %.3fms\n",
                        myPacket->arrivalTime, myPacket->pktID,
                        myPacket->numToken,
                        myPacket->arrivalTime - prevPktArrTime);
                prevPktArrTime = myPacket->arrivalTime;
                My402ListAppend(&queue1, myPacket);
                myPacket->q1EnterTime = getInstantTime() - arrStart;
                fprintf(stdout, "%012.3fms: p%d enters Q1\n",
                        myPacket->q1EnterTime, myPacket->pktID);

            } else {  // packeted needed token is greater than B
                fprintf(stdout,
                        "%012.3fms: p%d arrives, needs %d tokens, "
                        "inter-arrival time = %.3fms, dropped\n",
                        myPacket->arrivalTime, myPacket->pktID,
                        myPacket->numToken,
                        myPacket->arrivalTime - prevPktArrTime);
                prevPktArrTime = myPacket->arrivalTime;
                tokenDrop++;
            }
        } else {  // Deterministic mode
            myPacket->pktID = pktOrder++;
            myPacket->numToken = P;
            myPacket->pktIntervalArrivalTIme = min(10000, (1000.0 / lambda));
            myPacket->serviceTime = min(10000, (1000.0 / mu));

            sleepTime = myPacket->pktIntervalArrivalTIme * 1000.0;
            usleep(sleepTime);

            pthread_mutex_lock(&mutex);

            if (myPacket->numToken <= B) {
                myPacket->arrivalTime = getInstantTime() - arrStart;
                fprintf(stdout,
                        "%012.3fms: p%d arrives, needs %d tokens, "
                        "inter-arrival time = %.3fms\n",
                        myPacket->arrivalTime, myPacket->pktID,
                        myPacket->numToken,
                        myPacket->arrivalTime - prevPktArrTime);
                prevPktArrTime = myPacket->arrivalTime;
                My402ListAppend(&queue1, myPacket);
                myPacket->q1EnterTime = getInstantTime() - arrStart;
                fprintf(stdout, "%012.3fms: p%d enters Q1\n",
                        myPacket->q1EnterTime, myPacket->pktID);
            } else {  // packeted needed token is greater than B
                fprintf(stdout,
                        "%012.3fms: p%d arrives, needs %d tokens, "
                        "inter-arrival time = %.3fms, dropped\n",
                        myPacket->arrivalTime, myPacket->pktID,
                        myPacket->numToken,
                        myPacket->arrivalTime - prevPktArrTime);
                prevPktArrTime = myPacket->arrivalTime;
                tokenDrop++;
            }
        }

        pthread_cond_broadcast(&queue_cv);
        pthread_mutex_unlock(&mutex);
    }
    if (emulationMode == 'T') {
        fclose(fp);
    }
    arrivalThreadWorking = FALSE;
    pthread_exit(NULL);
}

// need to generate tokens in the specified way, then add a token to a token
// bucekt (int) if thread is moving a packet into Q2, it needs to wake up a
// sleeping server thread -> call pthread_cond_broadcase()
void *tokenThread(void *id) {
    // printf("I'm in tokThread!\n");
    double tokenArrTime = 1000.0 * min(10, (1 / r));
    while (!My402ListEmpty(&queue1) || arrivalThreadWorking) {
        int sleepTime = 1000 * tokenArrTime;
        usleep(sleepTime);
        pthread_mutex_lock(&mutex);
        tokenCount++;
        if (tokenInBucket < B) {
            tokenInBucket++;
            fprintf(stdout,
                    "%012.3fms: token t%d arrives, token bucket now has %d "
                    "tokens\n",
                    getInstantTime() - tokStart, tokenCount, tokenInBucket);
        } else {
            fprintf(stdout, "%012.3fms: token t%d arrives, dropped\n",
                    getInstantTime() - tokStart, tokenCount);
        }
        // if queue1 is not empty && token is enough, move packet from q1 to q2
        //
        Packet *myPacket = NULL;
        if (!My402ListEmpty(&queue1)) {
            My402ListElem *elem = My402ListFirst(&queue1);
            myPacket = elem->obj;
            if (tokenInBucket >= myPacket->numToken) {
                tokenInBucket -= myPacket->numToken;
                My402ListUnlink(&queue1, elem);
                myPacket->q1LeaveTime = getInstantTime() - tokStart;
                fprintf(stdout,
                        "%012.3fms: p%d leaves Q1, time in Q1 = %.3fms, token "
                        "bucket now has %d token\n",
                        myPacket->q1LeaveTime, myPacket->pktID,
                        myPacket->q1LeaveTime - myPacket->q1EnterTime,
                        tokenInBucket);
                My402ListAppend(&queue2, myPacket);
                myPacket->q2EnterTime = getInstantTime() - tokStart;
                fprintf(stdout, "%012.3fms: p%d enters Q2\n",
                        myPacket->q2EnterTime, myPacket->pktID);
                pthread_cond_broadcast(&queue_cv);
            }
        }

        pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&queue_cv);
    tokenThreadWorking = FALSE;
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

// checks if Q2 is empty or not. if Q2 is empty, it goes to sleep (in a CV
// queue).

// server 1
void *serverThread(void *id) {
    int serID = (int)id;
    int sleepTime = 0;
    while (!My402ListEmpty(&queue2) || tokenThreadWorking) {
        Packet *myPacket = NULL;
        pthread_mutex_lock(&mutex);
        while (!time_to_quit && My402ListEmpty(&queue2) && tokenThreadWorking) {
            pthread_cond_wait(&queue_cv, &mutex);
        }
        if (!My402ListEmpty(&queue2)) {
            My402ListElem *elem = NULL;
            elem = My402ListFirst(&queue2);
            myPacket = (Packet *)elem->obj;
            My402ListUnlink(&queue2, elem);
            myPacket->q2LeaveTime = getInstantTime() - tokStart;
            fprintf(stdout, "%012.3fms: p%d leave Q2, time in Q2 = %.3fms\n",
                    myPacket->q2LeaveTime, myPacket->pktID,
                    myPacket->q2LeaveTime - myPacket->q2EnterTime);
            myPacket->startServerTime = getInstantTime() - tokStart;
            fprintf(stdout,
                    "%012.3fms: p%d begins services at S%d, requesting %dms of "
                    "service\n",
                    myPacket->startServerTime, myPacket->pktID, serID,
                    myPacket->serviceTime);
        } else {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);
        sleepTime = myPacket->serviceTime * 1000;
        usleep(sleepTime);
        myPacket->quitTime = getInstantTime() - tokStart;
        fprintf(stdout,
                "%012.3fms: p%d departs from S%d, service time = %.3fms, time "
                "in system = %.3fms\n",
                myPacket->quitTime, myPacket->pktID, serID,
                myPacket->quitTime - myPacket->startServerTime,
                myPacket->quitTime - myPacket->arrivalTime);
    }
    time_to_quit = 1;
    pthread_exit(NULL);
}

void printEmulationPara() {
    fprintf(stdout, "Emulation Parameters:\n");
    fprintf(stdout, "\tnumber to arrive = %d\n", num_packets);
    if (emulationMode == 'D') {  // Deterministic mode
        fprintf(stdout, "\tlambda = %.6g\n", lambda);
        fprintf(stdout, "\tmu = %.6g\n", mu);
        fprintf(stdout, "\tr = %.6g\n", r);
        fprintf(stdout, "\tB = %d\n", B);
        fprintf(stdout, "\tP = %d\n", P);
    } else {  // Trace-driven mode
        fprintf(stdout, "\tr = %.6g\n", r);
        fprintf(stdout, "\tB = %d\n", B);
        fprintf(stdout, "\ttsfile = %s\n", tsfile);
    }
}

void checkPara() {
    if (lambda < 0.1) {
        lambda = 0.1;
    }

    if (mu < 0.1) {
        mu = 0.1;
    }
}

int main(int argc, char *argv[]) {
    // setting time parameters to default values
    lambda = 1;
    mu = 0.35;
    r = 1.5;
    B = 10;
    P = 3;
    num_packets = 20;
    ////////////////////////////////////////////////
    readCommandLine(argc, argv);
    if (emulationMode == 'T') {
        readFile(tsfile);
    }
    if (emulationMode == 'D') {
        checkPara();
    }
    double startTime = getInstantTime();
    arrStart = tokStart = getInstantTime();
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&queue_cv, NULL);
    My402ListInit(&queue1);
    My402ListInit(&queue1);
    printEmulationPara();
    fprintf(stdout, "%012.3fms: emulation begins\n", 0.0);
    pthread_create(&arrThread, NULL, pktArrivalThread, NULL);
    pthread_create(&tokThread, NULL, tokenThread, NULL);
    pthread_create(&ser1Thread, NULL, serverThread, (void *)1);
    pthread_create(&ser2Thread, NULL, serverThread, (void *)2);
    pthread_join(tokThread, NULL);
    pthread_join(arrThread, NULL);
    pthread_join(ser1Thread, NULL);
    pthread_join(ser2Thread, NULL);
    fprintf(stdout, "%012.3fms: emulation ends\n",
            getInstantTime() - startTime);

    return 0;
}