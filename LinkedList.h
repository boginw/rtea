//
// Created by hamburger on 05/10/2017.
//

#ifndef RTEA_DEFINITIONS_H
#include "definitions.h"

#endif

#ifndef RTEA_LINKEDLIST_H
#define RTEA_LINKEDLIST_H

#endif //RTEA_LINKEDLIST_H

typedef struct ListNode {
  struct ListNode *next;
  Frame value;
} ListNode;

typedef struct LinkedList {
  ListNode *list;
  int (*count)(struct LinkedList *list);
  void (*add)(struct LinkedList *list, Frame value);
  void (*removeNode)(struct LinkedList *list, int index);
  Frame (*get)(struct LinkedList *list, int index);
  void (*insertAt)(struct LinkedList *list, int index, Frame value);
} LinkedList;

LinkedList *linkedList();
void freeList(LinkedList *list);