#include "my402list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <sys/time.h>

#include "cs402.h"

int My402ListLength(My402List *myList) { return myList->num_members; }
int My402ListEmpty(My402List *myList) { return myList->num_members == 0; }

int My402ListAppend(My402List *myList, void *obj) {
    My402ListElem *newElem = (My402ListElem *)malloc(sizeof(My402ListElem));
    // initialize new Elem
    if (newElem == NULL) {
        printf("(Append)Error: newElem is not defined!");
        return FALSE;
    }
    newElem->prev = newElem->next = NULL;
    newElem->obj = NULL;
    /////////////////////////////////////////////////////
    My402ListElem *lastElem = My402ListLast(myList);
    newElem->obj = obj;

    if (My402ListEmpty(myList)) {
        newElem->prev = &(myList->anchor);
        newElem->next = &(myList->anchor);
        myList->anchor.prev = newElem;
        myList->anchor.next = newElem;
        myList->num_members++;
        return TRUE;
    } else {
        lastElem->next = newElem;
        newElem->prev = lastElem;
        newElem->next = &(myList->anchor);
        myList->anchor.prev = newElem;
        myList->num_members++;
        return TRUE;
    }
    return FALSE;
}
int My402ListPrepend(My402List *myList, void *obj) {
    My402ListElem *newElem = (My402ListElem *)malloc(sizeof(My402ListElem));
    // initialize new Elem
    if (newElem == NULL) {
        printf("(Prepend)Error: newElem is not defined!");
        exit(-1);
    }
    newElem->prev = newElem->next = NULL;
    newElem->obj = NULL;
    /////////////////////////////////////////////////////
    My402ListElem *firstElem = My402ListFirst(myList);
    newElem->obj = obj;

    if (My402ListEmpty(myList)) {
        newElem->prev = &(myList->anchor);
        newElem->next = &(myList->anchor);
        myList->anchor.prev = newElem;
        myList->anchor.next = newElem;
        myList->num_members++;
        return TRUE;
    } else {
        firstElem->prev = newElem;
        newElem->next = firstElem;
        newElem->prev = &(myList->anchor);
        myList->anchor.next = newElem;
        myList->num_members++;
        return TRUE;
    }
    return FALSE;
}
void My402ListUnlink(My402List *myList, My402ListElem *elem) {
    My402ListElem *prevElem = elem->prev;
    My402ListElem *nextElem = elem->next;
    prevElem->next = nextElem;
    nextElem->prev = prevElem;
    free(elem);
    myList->num_members--;
}
void My402ListUnlinkAll(My402List *myList) {
    My402ListElem *tmpElem = (My402ListElem *)malloc(sizeof(My402ListElem));
    tmpElem = My402ListFirst(myList);
    if (tmpElem == NULL) return;
    while (tmpElem != &(myList->anchor)) {
        My402ListElem *tmp = tmpElem;
        My402ListUnlink(myList, tmpElem);
        tmpElem = tmp->next;
    }
    myList->num_members = 0;
}

int My402ListInsertBefore(My402List *myList, void *obj, My402ListElem *elem) {
    if (elem == NULL) {
        return My402ListPrepend(myList, obj);
    } else {
        My402ListElem *newElem = (My402ListElem *)malloc(sizeof(My402ListElem));
        newElem->obj = obj;
        newElem->prev = elem->prev;
        newElem->next = elem;
        elem->prev->next = newElem;
        elem->prev = newElem;
        myList->num_members++;
        return TRUE;
    }
    return FALSE;
}

int My402ListInsertAfter(My402List *myList, void *obj, My402ListElem *elem) {
    if (elem == NULL) {
        return My402ListAppend(myList, obj);
    } else {
        My402ListElem *newElem = (My402ListElem *)malloc(sizeof(My402ListElem));
        newElem->obj = obj;
        newElem->prev = elem;
        newElem->next = elem->next;
        elem->next->prev = newElem;
        elem->next = newElem;
        myList->num_members++;
        return TRUE;
    }
    return FALSE;
}

My402ListElem *My402ListFirst(My402List *myList) {
    if (My402ListEmpty(myList)) {
        return NULL;
    } else {
        return myList->anchor.next;
    }
}
My402ListElem *My402ListLast(My402List *myList) {
    if (My402ListEmpty(myList)) {
        return NULL;
    } else {
        return myList->anchor.prev;
    }
}
My402ListElem *My402ListNext(My402List *myList, My402ListElem *elem) {
    if (elem == My402ListLast(myList)) {
        return NULL;
    } else {
        return elem->next;
    }
}
My402ListElem *My402ListPrev(My402List *myList, My402ListElem *elem) {
    if (elem == My402ListFirst(myList)) {
        return NULL;
    } else {
        return elem->prev;
    }
}

My402ListElem *My402ListFind(My402List *myList, void *obj) {
    My402ListElem *tmpElem = My402ListFirst(myList);
    while (tmpElem != &(myList->anchor)) {
        if (tmpElem->obj == obj) {
            return tmpElem;
        }
        tmpElem = tmpElem->next;
    }
    return NULL;
}

int My402ListInit(My402List *myList) {
    if (myList) {
        myList->num_members = 0;
        myList->anchor.obj = NULL;
        myList->anchor.prev = &(myList->anchor);
        myList->anchor.next = &(myList->anchor);
        return TRUE;
    } else {
        return FALSE;
    }
}
