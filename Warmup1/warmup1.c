#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "cs402.h"
#include "my402list.c"
#include "my402list.h"

int MAX_LINE_LENGTH = 1024;
typedef struct tagTransactionField {
    char type;
    time_t time;
    int amount;
    char *description;
} TransField;

/* ----------------------- functions ----------------------- */
void convertTimeFormat(char *tmp, time_t t) {
    char buf[26];
    strcpy(buf, ctime(&t));
    int j = 0;
    for (int i = 0; i < 16; i++) {
        if (i > 9) j = i + 9;
        tmp[i] = buf[j++];
    }
    tmp[15] = '\0';
}

void readFile(FILE *fp, My402List *myList) {
    char buf[2000];
    int initList = 0;
    // Initialize the list ////
    initList = My402ListInit(myList);
    if (!initList) {
        fprintf(stdout, "Error: Unable to initilize the list!\n");
        exit(0);
    }
    //////////////////////////
    // Read the tfile data line by line and store in tmp as a string
    while ((fgets(buf, sizeof(buf), fp) != NULL)) {
        TransField *trans = (TransField *)malloc(sizeof(TransField));
        // check if the length of the line is valid
        if (strlen(buf) > MAX_LINE_LENGTH) {
            fprintf(stderr,
                    "Error: tfile format is not valid, the maximum length of "
                    "characters in each line "
                    "should smaller than 1024!\n");
            exit(0);
        } else if (strlen(buf) == 0) {
            fprintf(stderr, "Error: The tfile is empty!\n");
            exit(0);
        }
        ///////////////////////////////////////////
        // check if the number of tabs equal to 3
        int tabCount = 0;
        for (int i = 0; i < strlen(buf); i++) {
            if (buf[i] == '\t') {
                tabCount++;
            }
        }
        if (tabCount != 3) {
            fprintf(stderr,
                    "Error: The input tfile format is not valid, the # of tabs "
                    "should equal to 3!\n");
            exit(0);
        }
        ////////////////////////////////////////////
        //
        // 1st Field (transaction type)
        char *start_ptr = buf;
        char *tab_ptr = strchr(start_ptr, '\t');
        if (tab_ptr) {
            *tab_ptr++ = '\0';
        }
        printf("type: %s\n", start_ptr);
        if (start_ptr[0] != '+' && start_ptr[0] != '-') {
            fprintf(stderr,
                    "Error: The transaction type should be \"+\" or \"-\" in "
                    "the tfile! \n");
        } else {
            trans->type = start_ptr[0];
        }
        // 2nd Field (Time stamp)
        start_ptr = tab_ptr;
        tab_ptr = strchr(start_ptr, '\t');
        if (tab_ptr) {
            *tab_ptr++ = '\0';
        }
        time_t currentTime = time(NULL);
        time_t transTime = (int)atoi(start_ptr);
        char tp[16];
        convertTimeFormat(tp, transTime);
        printf("Time: %s\n", start_ptr);
        printf("Time: %s\n", tp);
        if (transTime > currentTime) {
            fprintf(stderr,
                    "Error: The transaction time is later than the current "
                    "time!\n");
        } else if (transTime < 0) {
            fprintf(stderr,
                    "Error: The value of the transaction time could not be "
                    "negative!\n");
        } else {
            trans->time = transTime;
        }
        printf("Time: %ld\n", trans->time);
        // 3rd Field (amount)
        start_ptr = tab_ptr;
        tab_ptr = strchr(start_ptr, '\t');
        if (tab_ptr) {
            *tab_ptr++ = '\0';
        }
        trans->amount = atoi(start_ptr);
        printf("amount: %s\n", start_ptr);
        printf("amount: %d\n", trans->amount);
        // 4th Field (description)
        start_ptr = tab_ptr;
        tab_ptr = strchr(start_ptr, '\t');
        if (tab_ptr) {
            *tab_ptr++ = '\0';
        }
        trans->description = start_ptr;
        printf("Description: %s\n", trans->description);
    }

    // fgets(buf, sizeof(buf), fp);
    // fprintf(stdout, "%s\n", buf);
}

/* ----------------------- main() ----------------------- */
int main(int argc, char *argv[]) {
    My402List myList;
    char *tok;
    FILE *fp = fopen(argv[1], "r");
    if (!fp)
        printf("error file\n");
    else
        printf("reading %s!\n", argv[1]);
    readFile(fp, &myList);
    fclose(fp);

    // ** time stamp format test //
    // char tmp[16];
    // time_t t = time(NULL);
    // convertTimeFormat(tmp, t);
    // printf("%s\n", tmp);

    return 0;
}

// char *tmp[4];
// int tokenCount = 0;
// char *token = strtok(buf, "\t");
// while (token && tokenCount < 4) {
//     tmp[tokenCount++] = strdup(token);
//     // printf("%s\n", token);
//     token = strtok(NULL, "\t");
// }
// // for (int i = 0; i < 4; i++) {
// //     printf("%s\n", tmp[i]);
// // }
// tokenCount = 0;
