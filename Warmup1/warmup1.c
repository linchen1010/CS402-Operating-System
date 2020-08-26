#include <ctype.h>
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

int transCnt = 0;

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
        if (transTime > currentTime) {
            fprintf(stderr,
                    "Error: The transaction time is later than the current "
                    "time!\n");
            exit(0);
        } else if (transTime < 0) {
            fprintf(stderr,
                    "Error: The value of the transaction time could not be "
                    "negative!\n");
            exit(0);
        } else {
            trans->time = transTime;
        }
        //////////////////////////////////////////////////////////////////////
        // 3rd Field (amount)
        start_ptr = tab_ptr;
        tab_ptr = strchr(start_ptr, '\t');
        if (tab_ptr) {
            *tab_ptr++ = '\0';
        } else {
            fprintf(stderr, "Error: The amount can't not be empty!\n");
            exit(0);
        }
        char *amt = start_ptr;
        trans->amount = atof(start_ptr) * 100;
        if (start_ptr && strlen(start_ptr) > 10) {
            fprintf(stderr, "Error: Transaction amount is too large!\n");
            exit(0);
        }
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
        // append to myList
        My402ListAppend(myList, trans);
    }
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

void FormatDollarsWithOneCommas(int dollars, char buf[80]) {
    char dollars_rest[4];  // a period
    dollars_rest[3] = '\0';
    int dollars_first = dollars / 1000;
    int i = 2;
    while (dollars != 0 && i >= 0) {
        dollars_rest[i--] = dollars % 10 + '0';
        dollars /= 10;
    }
    snprintf(buf, 80, "%d,%s", dollars_first, dollars_rest);
}

void FormatDollarsWithTwoCommas(int dollars, char buf[80]) {
    int dollars_first = dollars / 1000000;
    char dollars_second[4];
    char dollars_rest[4];
    dollars_second[3] = dollars_rest[3] = '\0';
    int i = 2;
    while (dollars != 0 && i >= 0) {
        dollars_rest[i--] = dollars % 10 + '0';
        dollars /= 10;
    }
    i = 2;
    while (dollars != 0 && i >= 0) {
        dollars_second[i--] = dollars % 10 + '0';
        dollars /= 10;
    }
    snprintf(buf, 80, "%d,%s,%s", dollars_first, dollars_second, dollars_rest);
}
/* cited from Warmup1 FAQ */
void formatCents(int amt_in_cents, char buf[80]) {
    if (amt_in_cents >= 1000000000) {
        snprintf(buf, 80, "?,???,???.??");
        return;
    }
    int cents = amt_in_cents % 100;
    int dollars = amt_in_cents / 100;
    if (dollars >= 1000000) {
        char dollar_buf[80];
        FormatDollarsWithTwoCommas(dollars, dollar_buf);
        snprintf(buf, 80, "%s.%02d", dollar_buf, cents);
    } else if (dollars >= 1000) {
        char dollar_buf[80];
        FormatDollarsWithOneCommas(dollars, dollar_buf);
        snprintf(buf, 80, "%s.%02d", dollar_buf, cents);
    } else {
        snprintf(buf, 80, "%d.%02d", dollars, cents);
    }
}

void printList(My402List *myList) {
    My402ListElem *elem = (My402ListElem *)malloc(sizeof(My402ListElem));
    // TransField *trans = NULL;
    if (My402ListEmpty(myList)) {
        fprintf(stderr, "The list should not be empty! \n");
        exit(0);
    }

    int balance_in_cents = 0;
    TransField *trans = (TransField *)malloc(sizeof(TransField));
    // char *desc = (char *)malloc(sizeof(char) * 1024);
    for (elem = My402ListFirst(myList); elem != NULL;
         elem = My402ListNext(myList, elem)) {
        // TransField *trans = (TransField *)malloc(sizeof(TransField));
        trans = elem->obj;
        // Time Field
        char timeStamp[16];
        time_t time = trans->time;
        convertTimeFormat(timeStamp, time);
        fprintf(stdout, "| %s | ", timeStamp);
        ////////////////////////////////////////////////////////////////
        // Description Field
        char *tmpDesc = trans->description;
        char desc[25];
        tmpDesc = removeLeadingSpace(trans->description);
        desc[24] = '\0';
        strncpy(desc, tmpDesc, 24);
        // add ' ' to the end of the desc
        if (strlen(desc) < 24) {
            for (int i = strlen(desc) - 1; i < descFieldLength; i++) {
                desc[i] = ' ';
            }
        }
        fprintf(stdout, "%s | ", desc);
        ////////////////////////////////////////////////////////////////
        // amount Field
        int amt_in_cents = trans->amount;
        char amount_buf[80];
        formatCents(amt_in_cents, amount_buf);
        // printf("[%ld]", strlen(amount_buf));
        char amount[15];
        amount[14] = '\0';
        int amount_idx = 12;

        for (int i = strlen(amount_buf) - 1; i >= 0; i--) {
            amount[amount_idx--] = amount_buf[i];
        }
        while (amount_idx) {
            amount[amount_idx--] = ' ';
        }
        if (trans->type == '-') {
            amount[0] = '(';
            amount[13] = ')';
        } else {
            amount[0] = ' ';
            amount[13] = ' ';
        }
        fprintf(stdout, "%s | ", amount);
        ////////////////////////////////////////////////////////////////
        // balance Field
        if (trans->type == '+') {
            balance_in_cents += trans->amount;
        } else {
            balance_in_cents -= trans->amount;
        }
        char balance_buf[80];
        formatCents(balance_in_cents, balance_buf);
        // printf("[%ld]", strlen(amount_buf));
        char balance[15];
        balance[14] = '\0';
        int balance_idx = 12;

        for (int i = strlen(balance_buf) - 1; i >= 0; i--) {
            balance[balance_idx--] = balance_buf[i];
        }
        while (balance_idx) {
            balance[balance_idx--] = ' ';
        }
        balance[0] = ' ';
        balance[13] = ' ';
        fprintf(stdout, "%s |", balance);
        ////////////////////////////////////////////////////////////////
        transCnt++;
        if (transCnt != 4) fprintf(stdout, "\n");
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

void sortList(My402List *myList) {
    My402ListElem *elem = NULL;
    My402ListElem *nextElem = NULL;
    void *tmp;
    for (elem = My402ListFirst(myList); My402ListNext(myList, elem) != NULL;
         elem = My402ListNext(myList, elem)) {
        for (nextElem = My402ListNext(myList, elem); nextElem != NULL;
             nextElem = My402ListNext(myList, nextElem)) {
            if (((TransField *)elem->obj)->time >
                ((TransField *)nextElem->obj)->time) {
                tmp = elem->obj;
                elem->obj = nextElem->obj;
                nextElem->obj = tmp;
            } else if (((TransField *)elem->obj)->time ==
                       ((TransField *)nextElem->obj)->time) {
                fprintf(stderr, "Error: Two TimeStamp could not be same!\n");
                exit(0);
            }
        }
    }
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
    sortList(myList);
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
