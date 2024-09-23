#include <stdio.h>
#include <stdlib.h>
#include "cache.h"
#include "csapp.h"

DLL *newDll()
{
    DLL *new_dll = (DLL *)calloc(1, sizeof(DLL));
    node *head_node = (node *)calloc(1, sizeof(node));
    new_dll->head = head_node;
    head_node->next = head_node->prev = head_node;
    new_dll->size = 0;
    return new_dll;
}

void pushFront(DLL *dll, node *push_node)
{
    // 캐시가 가득 찼을 경우
    if (dll->size * sizeof(node) > MAX_CACHE_SIZE) {
        popBack(dll);
    }

    push_node->next = dll->head->next;
    dll->head->next->prev = push_node;
    dll->head->next = push_node;
    push_node->prev = dll->head;

    dll->size++;
    printf("push front : (%s, %s)\n", push_node->request_info.method, push_node->request_info.path);
    printDll(dll);
}


void popBack(DLL *dll)
{
    if (dll->size == 0){
        printf("List is empty, cannot pop.\n");
        return -1;
    }
    node *pop_node = dll->head->prev;

    dll->head->prev = pop_node->prev;
    pop_node->prev->next = dll->head;
    free(pop_node->request_info.path);
    free(pop_node);

    dll->size--;

    printf("pop back : (%s, %s)\n", pop_node->request_info.method, pop_node->request_info.path);
    printDll(dll);
}

node *search(DLL *dll, request_info request_info)
{
    node *find_node = dll->head->next;
    while (find_node != dll->head) {
        if (!strcmp(find_node->request_info.method, request_info.method) && !strcmp(find_node->request_info.path, request_info.path)) {
            printf("search (%s, %s) : find!\n", find_node->request_info.method, find_node->request_info.path);
            return find_node;
        }
        find_node = find_node->next;
    }
    printf("search (%s, %s) : not exsist!\n", find_node->request_info.method, find_node->request_info.path);
    return NULL;
}

void moveFront(DLL *dll, node *move_node)
{
    if (move_node == NULL) {
        printf("Cant Move");
        return;
    }
    move_node->prev->next = move_node->next;
    move_node->next->prev = move_node->prev;

    move_node->next = dll->head->next;
    dll->head->next->prev = move_node;
    dll->head->next = move_node;
    move_node->prev = dll->head;
    printDll(dll);
}

void printDll(DLL *dll)
{
    if (dll->size == 0)
    {
        printf("[]\n");
        return;
    }

    node *now = dll->head->next;
    while (now != dll->head->prev)
    {
        printf("%s ←→ ", now->request_info.path);
        now = now->next;
    }
    printf("%s ←→ ", now->request_info.path);
}

void deleteList(DLL *dll)
{
    node *delete_node = dll->head->next;
    while (delete_node != dll->head) {
        node *next_node = delete_node->next;
        free(delete_node);
        delete_node = next_node;
    }
    free(dll->head);
    free(dll);
}
