/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

typedef struct request_info
{
    char method[8];
    char* path;
} request_info;

typedef struct Node
{
    struct request_info request_info;
    char* response;
    struct Node *prev;
    struct Node *next;
} node;

typedef struct DoublyLinkedList
{
    node* head;
    int size;
} DLL;

DLL *newDll();
void pushFront(DLL *dll, node *push_node);
void popFront(DLL *dll);
void popBack(DLL *dll);
node *search(DLL *dll, request_info request_info);
void moveFront(DLL *dll, node* move_node);
void printDll(DLL *dll);
void deleteList(DLL *dll);