#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "cs402.h"
#include "my402list.c"
#include "my402list.h"

#define MAX_LINE_LENGTH 1024
#define descFieldLength 24

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
            exit(0);
        } else {
            trans->type = start_ptr[0];
        }
        //////////////////////////////////////////////////////////////////////
        // 2nd Field (Time stamp)
        start_ptr = tab_ptr;
        tab_ptr = strchr(start_ptr, '\t');
        if (tab_ptr) {
            *tab_ptr++ = '\0';
        }
        time_t currentTime = time(NULL);
        time_t transTime = (int)atoi(start_ptr);
        // test time format
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
        //////////////////////////////////////////////////////////////////////
        // 3rd Field (amount)
        start_ptr = tab_ptr;
        tab_ptr = strchr(start_ptr, '\t');
        if (tab_ptr) {
            *tab_ptr++ = '\0';
        }
        trans->amount = atof(start_ptr) * 100;
        printf("amount: %s\n", start_ptr);
        printf("amount: %d\n", trans->amount);
        printf("amount: %d.%.2d\n", trans->amount / 100, trans->amount % 100);
        //////////////////////////////////////////////////////////////////////
        // 4th Field (description)
        start_ptr = tab_ptr;
        tab_ptr = strchr(start_ptr, '\t');
        if (tab_ptr) {
            *tab_ptr++ = '\0';
        }
        char *desc = start_ptr;
        trans->description = strdup(desc);
        // check empty -- might have bug
        if (strlen(trans->description) == 1 && trans->description[0] == '\n') {
            fprintf(stderr, "Transaction desciption cannot be empty.\n");
            exit(0);
        }
        printf("Description: %s\n", trans->description);
        // add in to myList
        My402ListAppend(myList, trans);
    }

    // printf("%d\n", My402ListLength(myList));

    // fgets(buf, sizeof(buf), fp);
    // fprintf(stdout, "%s\n", buf);
}

int amountDigit(int amount) {
    int count = 0;
    while (amount != 0) {
        amount /= 10;
        count++;
    }
    return count;
}

char *removeLeadingSpace(char *desc) {
    int i = 0;
    char *str;
    while (desc[i] == ' ') {
        i++;
    }
    str = desc + i;
    char *strFinal = strdup(str);
    return strFinal;
}

int lengthOfDesc(char *desc) {
    int i = 0;
    while (desc[i] != '\0') {
        i++;
    }
    return i;
}

void printList(My402List *myList) {
    My402ListElem *elem = (My402ListElem *)malloc(sizeof(My402ListElem));
    // TransField *trans = NULL;
    if (My402ListEmpty(myList)) {
        fprintf(stderr, "The list should not be empty! \n");
        exit(0);
    }

    int balance = 0;
    TransField *trans = (TransField *)malloc(sizeof(TransField));
    // char *desc = (char *)malloc(sizeof(char) * 1024);
    for (elem = My402ListFirst(myList); elem != NULL;
         elem = My402ListNext(myList, elem)) {
        // TransField *trans = (TransField *)malloc(sizeof(TransField));
        trans = (TransField *)elem->obj;
        // Time Field
        char timeStamp[16];
        time_t time = trans->time;
        convertTimeFormat(timeStamp, time);
        fprintf(stdout, "| %s | ", timeStamp);
        // Description Field
        char *tmpDesc = trans->description;
        char desc[25];
        tmpDesc = removeLeadingSpace(trans->description);
        desc[24] = '\0';
        strncpy(desc, tmpDesc, 24);
        // printf("[%ld]", strlen(desc));
        // printf("[%d]", lengthOfDesc(desc));
        if (strlen(desc) != 0)
            for (int i = strlen(desc) - 1; i < descFieldLength; i++) {
                desc[i] = ' ';
            }

        fprintf(stdout, "%s | ", desc);

        fprintf(stdout, "%d\n", trans->amount);

        // fprintf(stdout, "| %s", desc);
        // fprintf(stdout, "| %d | %d", trans->amount,
        // amountDigit(trans->amount)); printf("amount: %d.%.2d\n",
        // trans->amount / 100, trans->amount % 100); fprintf(stdout, "| %s",
        // trans->description);
        // free(trans);
    }
    free(elem);
    free(trans);
    // free(desc);
}

void printHeader() {
    fprintf(
        stdout,
        "+-----------------+--------------------------+----------------+-------"
        "---------+\n");
    fprintf(
        stdout,
        "|       Date      | Description              |         Amount |       "
        " Balance |\n");
    fprintf(
        stdout,
        "+-----------------+--------------------------+----------------+-------"
        "---------+\n");
}

void printFooter() {
    fprintf(
        stdout,
        "\n+-----------------+--------------------------+----------------+---"
        "-------------+\n");
}

/* ----------------------- main() ----------------------- */
int main(int argc, char *argv[]) {
    My402List *myList = (My402List *)malloc(sizeof(My402List));
    char *tok;
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        printf("Can't find %s\n", argv[1]);
        exit(0);
    } else {
        printf("reading %s...\n", argv[1]);
    }
    readFile(fp, myList);
    printHeader();
    printList(myList);
    printFooter();
    // printf("%s", My402ListLast(myList)->obj);
    fclose(fp);
    // ** time stamp format test //
    // char tmp[16];
    // time_t t = time(NULL);
    // convertTimeFormat(tmp, t);
    // printf("%s\n", tmp);
    free(myList);
    char *a = "      123 4\n";
    printf("%s", removeLeadingSpace(a));
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

// read file and store in the object
// sort by time stamp (need to check identical)
// if type == '-', need to add ( ) in the amount
// Date - buf[3] ~ buf[17]
// Desc - buf[21] ~ buf[44]
// Amount - buf[48] ~ buf[61]
// need to handle some
// digit <= 5, xxx.xx
//  5 < digit <= 8 && amount < 10 Million xxx,xxx.xx

// Balance - buf[65] ~ buf [78]
