#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

void free_nalloc(void* block);
void* renalloc(void* block, size_t size);
void* canalloc(size_t num, size_t size);
void* nalloc(size_t size);

typedef struct Nalloc_header Nalloc_header;

struct Nalloc_header{
    size_t size;
    uint8_t isFree;
    Nalloc_header *next;
};

Nalloc_header *head, *tail;

Nalloc_header* search_freeBlock(size_t size)
{
    Nalloc_header *header = head;
    while(header != NULL)
    {
        if(header->size >= size && header->isFree)
            return header;

        header = header->next;
    }

    return NULL;
}
/*
* +---------+-----------------+
* | header  | block allocated  |
* +---------+-----------------+
*/
void* nalloc(size_t size)
{
    void *block;
    Nalloc_header *header = NULL;
    size_t totalSize = sizeof(Nalloc_header) + size;

    if(!size)
        return NULL;

    header = search_freeBlock(size);
    if(header)
    {
        header->isFree = 0;
        return ((void*)header + sizeof(Nalloc_header));
    }

    block = sbrk(totalSize);    // requesting memory from the heap
    if(block == (void*) -1)
        return NULL;

    //filling header information
    header = (Nalloc_header*)block;
    header->size = size;
    header->isFree = 0;
    header->next = NULL;

    // now our linked list of allocated block
    if(!head)
        head = header;
    
    if(tail)
        tail->next = header;

    tail = header;

    block += sizeof(Nalloc_header);

    return block;
}

void* canalloc(size_t num, size_t size)
{
    size_t totalSize = num * size;

    if(!num || !size)
        return NULL;

    if(num != totalSize / size) // check mul overflow 
        return NULL;

    void* pointer = nalloc(totalSize);
    if(!pointer)
        return NULL;

    memset(pointer, 0, totalSize);
    
    return pointer;
}

void* renalloc(void* block, size_t size)
{
    Nalloc_header* header = block - sizeof(Nalloc_header);
    void* newBlock;

    if(header->size >= size)
        return block;

    newBlock = nalloc(size);
    if(!newBlock)
        return NULL;

    memcpy(newBlock, block, header->size);
    free_nalloc(block);

    return newBlock;
}

void free_nalloc(void* block)
{
    Nalloc_header *header = (Nalloc_header*)(block - sizeof(Nalloc_header));
    size_t totalSize = sizeof(Nalloc_header) + header->size;

    // if it's the last block we need to release the memory to the OS
    if(header == tail)      //(block + header->size) == sbrk(0)
    {
        if(tail == head)    // if it's the first element in the linked list
        {
            tail = NULL;
            head = NULL;
            sbrk(-1 * totalSize);
        }else{
            Nalloc_header *tempTail = head;
            while(tempTail->next != tail)
                tempTail = tempTail->next;

            tail = tempTail;
            tail->next = NULL;

            sbrk(-1 * totalSize);
        }
    }else{  // set the block to free
        header->isFree = 1;
    }
}

void nallocTest()
{
    int* test = nalloc(sizeof(int) * 5);
    int* test0 = nalloc(sizeof(int) * 10);
    int* test1 = nalloc(sizeof(int) * 15);

    Nalloc_header *temp = head;

    while(temp != NULL)
    {
        printf("size: %ld, isfree: %d\n",temp->size, temp->isFree);
        temp = temp->next;
    }

    free_nalloc(test0); // freeing a block in the middle

    temp = head;
    printf("\n\n");

    while(temp != NULL)
    {
        printf("size: %ld, isfree: %d\n",temp->size, temp->isFree);
        temp = temp->next;
    }

    test = renalloc(test, sizeof(int) * 7);   // add a block in the middle using renalloc

    temp = head;
    printf("\n\n");

    while(temp != NULL)
    {
        printf("size: %ld, isfree: %d\n",temp->size, temp->isFree);
        temp = temp->next;
    }

    free_nalloc(test1);     // freeing the last block

    temp = head;
    printf("\n\n");

    while(temp != NULL)
    {
        printf("size: %ld, isfree: %d\n",temp->size, temp->isFree);
        temp = temp->next;
    }

    test1 = canalloc(15, sizeof(int)); // using canalloc

    temp = head;
    printf("\n\n");

    while(temp != NULL)
    {
        printf("size: %ld, isfree: %d\n",temp->size, temp->isFree);
        temp = temp->next;
    }
}

int main()
{
    nallocTest();
}