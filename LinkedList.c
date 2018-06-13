//
// Created by hamburger on 05/10/2017.
//

#include <stdlib.h>
#include "LinkedList.h"

ListNode *createNode() {
  ListNode *result = malloc(sizeof(ListNode));
  result->next = NULL;
  result->value = (Frame) {};
  return result;
}

int count(LinkedList *list) {
  int result = 0;
  ListNode *current = list->list;
  while (current->next) {
    result++;
    current = current->next;
  }
  return result;
}

void add(LinkedList *list, Frame value) {
  ListNode *current = list->list;
  while (current->next != NULL) {
    current = current->next;
  }
  current->next = createNode();
  current->next->value = value;
}

void removeNode(LinkedList *list, int index) {
  int i;
  ListNode *current = list->list;
  for (i = 0; i < index - 1; i++) {
    current = current->next;
  }
  ListNode *toFree = current->next;
  current->next = current->next->next;
  free(toFree);
}

Frame get(LinkedList *list, int index) {
  int i;
  ListNode *current = list->list;
  for (i = 0; i < index; i++) {
    current = current->next;
  }
  return current->next->value;
}

void insertAt(LinkedList *list, int index, Frame value) {
  int i;
  ListNode *current = list->list;
  for (i = 0; i < index - 1; i++) {
    current = current->next;
  }
  ListNode *next = current->next;
  current->next = createNode();
  current->next->value = value;
  current->next->next = next;
}

LinkedList *linkedList() {
  LinkedList *l = malloc(sizeof(LinkedList));
  l->add = add;
  l->count = count;
  l->get = get;
  l->insertAt = insertAt;
  l->removeNode = removeNode;
  l->list = createNode();

  return l;
}

void freeList(LinkedList *list) {
  int count = list->count(list);
  for (int i = 0; i < count; ++i) {
    list->removeNode(list, 0);
  }
  free(list);
}