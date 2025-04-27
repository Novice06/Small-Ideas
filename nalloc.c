#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ordered_array.h"

void free_nalloc(void* block);
void* renalloc(void* block, size_t size);
void* canalloc(size_t num, size_t size);
void* nalloc(size_t size);

typedef struct Nalloc_header Nalloc_header;

struct Nalloc_header{
    size_t size;
    bool isFree;
    Nalloc_header *next;
    Nalloc_header *back;
};

Nalloc_header *head, *tail;
ordered_array array;

Nalloc_header* search_freeBlock(size_t size)
{
    Nalloc_header *header;

    for(int i = 0; i < array.size; i++) // search a free block from the free block array
    {
        header = (Nalloc_header*)array.array[i];
        if(header->size >= size && header->isFree)
            return header;
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
        header->isFree = false;
        remove_ordered_array(getIndex_ordered_array(header, &array), &array); // remove from free block list

        if((header->size - size) >= (sizeof(Nalloc_header) + 16)) // we split the block
        {
            Nalloc_header* newHeader = (Nalloc_header*)((void*)header + size + sizeof(Nalloc_header));

            //filling new header information
            newHeader->size = header->size - size - sizeof(Nalloc_header);
            newHeader->isFree = true;
            newHeader->back = header;
            newHeader->next = NULL;

            if(header->next != NULL)
            {
                newHeader->next = header->next;
                header->next->back = newHeader;
            }

            insert_ordered_array(newHeader, &array); // add a new free block !

            header->next = newHeader;
            header->size = size;

            if(header == tail)
                tail = newHeader;
        }

        return ((void*)header + sizeof(Nalloc_header));
    }

    block = sbrk(totalSize);    // requesting memory from the heap
    if(block == (void*) -1)
        return NULL;

    //filling header information
    header = (Nalloc_header*)block;
    header->size = size;
    header->isFree = false;
    header->next = NULL;
    header->back = NULL;

    // now our linked list of allocated block
    if(!head)
        head = header;
    
    if(tail)
    {
        tail->next = header;
        header->back = tail;
    }

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
    Nalloc_header* header = (Nalloc_header*)(block - sizeof(Nalloc_header));
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
    Nalloc_header *left_block, *right_block;
    Nalloc_header *header = (Nalloc_header*)(block - sizeof(Nalloc_header));
    size_t totalSize = 0;

    header->isFree = true;
    insert_ordered_array((type_t)header, &array);

    // merging left block if it's free
    left_block = header->back;
    if(left_block != NULL && left_block->isFree)
    {
        remove_ordered_array(getIndex_ordered_array(header, &array), &array);

        left_block->size += header->size + sizeof(Nalloc_header);
        left_block->next = header->next;
        
        if(header->next != NULL)
            header->next->back = left_block;

        if(header == tail)      // if we are merging the last block
            tail = left_block; // the last block become the left one

        header = left_block;    // the actual header now to keep the right merge and the resto of the function consistent
    }

    //merging right block if it's free
    right_block = header->next;
    if(right_block != NULL && right_block->isFree)
    {
        remove_ordered_array(getIndex_ordered_array(right_block, &array), &array);

        header->size += right_block->size + sizeof(Nalloc_header);
        header->next = right_block->next;
        
        if(right_block->next != NULL)
            right_block->next->back = header;

        if(right_block == tail) // if we are merging the last block
            tail = header;      // the last block become the left one in this case the header
    }

    totalSize = sizeof(Nalloc_header) + header->size;

    // if it's the last block we need to release the memory to the OS
    if(header == tail)      //(block + header->size) == sbrk(0)
    {
        if(tail == head)    // if it's the first element in the linked list
        {
            tail = NULL;    // erase the actuel linked list
            head = NULL;
            sbrk(-1 * totalSize);
        }
        else
        {
            tail = header->back;
            tail->next = NULL;

            sbrk(-1 * totalSize);
        }

        remove_ordered_array(getIndex_ordered_array(header, &array), &array); // in all case we need to remove it from the free list array
    }
}

bool criteria_function(type_t a, type_t b)
{
    return ((Nalloc_header*)a)->size < ((Nalloc_header*)b)->size;   // ordered by size in ascending order
}

void nallocTest()
{
    array = create_dynamic_array(100, criteria_function);

    int* test = nalloc(sizeof(int) * 5);
    int* test0 = nalloc(sizeof(int) * 10);
    int* test1 = nalloc(sizeof(int) * 15);

    printf("initial array size %d\n", array.size);

    Nalloc_header *temp = head;

    while(temp != NULL)
    {
        printf("size: %ld, isfree: %d\n",temp->size, temp->isFree);
        temp = temp->next;
    }

    printf("freeing a block in the middle:\n");
    free_nalloc(test0); // freeing a block in the middle
    printf("array size %d\n", array.size);

    temp = head;

    while(temp != NULL)
    {
        printf("size: %ld, isfree: %d\n",temp->size, temp->isFree);
        temp = temp->next;
    }

    printf("add a block in the middle using renalloc:\n");
    test = renalloc(test, sizeof(int) * 7);   // add a block in the middle using renalloc
    printf("array size %d\n", array.size);

    temp = head;

    while(temp != NULL)
    {
        printf("size: %ld, isfree: %d\n",temp->size, temp->isFree);
        temp = temp->next;
    }

    printf("freeing the last block:\n");
    free_nalloc(test1);     // freeing the last block
    printf("array size %d\n", array.size);

    temp = head;

    while(temp != NULL)
    {
        printf("size: %ld, isfree: %d\n",temp->size, temp->isFree);
        temp = temp->next;
    }

    printf("using canalloc:\n");
    test1 = canalloc(15, sizeof(int)); // using canalloc
    printf("array size %d\n", array.size);

    temp = head;

    while(temp != NULL)
    {
        printf("size: %ld, isfree: %d\n",temp->size, temp->isFree);
        temp = temp->next;
    }

    printf("freeing another block in the middle:\n");
    free_nalloc(test);     // freeing another block in the middle
    printf("array size %d\n", array.size);

    temp = head;

    while(temp != NULL)
    {
        printf("size: %ld, isfree: %d\n",temp->size, temp->isFree);
        temp = temp->next;
    }

    printf("allocating after a merge another block:\n");
    test0 = nalloc(sizeof(int) * 10);    // allocating after a merge another block
    printf("array size %d\n", array.size);

    temp = head;

    while(temp != NULL)
    {
        printf("size: %ld, isfree: %d\n",temp->size, temp->isFree);
        temp = temp->next;
    }

    printf("freeing the last block again:\n");
    free_nalloc(test1);     // freeing the last block again
    printf("array size %d\n", array.size);

    temp = head;

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