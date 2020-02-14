///////////////////////////////////////////////////////////////////////////////
//
///   Icron Technology Corporation - Copyright 2019
//
//
///   This source file and the information contained in it are confidential and
///   proprietary to Icron Technology Corporation. The reproduction or disclosure,
///   in whole or in part, to anyone outside of Icron without the written approval
///   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
///   Icron who has not previously obtained written authorization for access from
///   the individual responsible for the source code, will have a significant
///   detrimental effect on Icron and is expressly prohibited.
///
///////////////////////////////////////////////////////////////////////////////
//
//   @file  -  linkedlist.c
//
//   @brief -  Contains the code linked list management
//
//   @note  -
//
///////////////////////////////////////////////////////////////////////////////

// Includes #######################################################################################
#include <linkedlist.h>

// #################################################################################################
// Get the length of a doubly linked list
//
// Parameters:  head      = start of the list
// Return:      length of the list computed by summing up the next node count.
// Assumptions: should be a valid linked list
//
// #################################################################################################
uint16_t UTIL_DblListPtr_length(doubleListNodePtr *head)
{
    uint16_t              count = 0;

    while (head->next)
    {
        count++;
        head = head->next;
    }
    return count;
}


// #################################################################################################
// Add an entry to a doubly linked list
//
// Parameters:  node      = place to add an entry--before the node
//              nodeToAdd = pointer to the node to add
//              list      = collection of the head and tail of the list
// Return:
// Assumptions: The nodeToAdd is a new node and not an existing one...using a node that exists
//              will break the list
//
// #################################################################################################
void  UTIL_DblListAddNodePtr(struct doubleLinkPtrInfo *list,
    doubleListNodePtr *node,
    doubleListNodePtr *nodeToAdd)

{
    if (list->head == NULL) // the list has not been initialized
    {
        list->head = nodeToAdd;
        list->tail = nodeToAdd;
        nodeToAdd->next = NULL;
        nodeToAdd->prev = NULL;
    }
    else if (node == NULL) // Add it to the end of the list
    {
        list->tail->next = nodeToAdd; // Current tails next element is the nodeToAdd
        nodeToAdd->prev = list->tail; // The nodeToAdd previous is the old tail 
        list->tail = nodeToAdd;       // Change the tail to the nodeToAdd
        list->tail->next = NULL;      // make sure the tails next element shows the end of list

    }
    else // add the nodeToAdd before node
    {
        nodeToAdd->next = node; // insert the node
        nodeToAdd->prev = node->prev;

        if (node->prev == NULL) // At the head
        {
            list->head = nodeToAdd;    // Change the head
        }
        node->prev = nodeToAdd;
    }
}

// #################################################################################################
// Delete an entry from a doubly linked list
//
// Parameters:  nodeToDelete = pointer to the node to add
//              list      = collection of the head and tail of the list
// Return:
// Assumptions: The nodeToDelete is a valid list item
//
// #################################################################################################
void  UTIL_DblListDelNodePtr(struct doubleLinkPtrInfo *list,
    doubleListNodePtr *nodeToDelete)
{

    //	iassert_UTIL_COMPONENT_1(listHead != NULL, UTIL_EMPTY_LIST, __LINE__);
    if (list->head == nodeToDelete) // the list has not been initialized
    {
        if (list->tail == nodeToDelete)
        {
            list->head = NULL;
            list->tail = NULL;
        }
        else
        {
            list->head = nodeToDelete->next;
            nodeToDelete->next->prev = NULL;
        }
    }
    else if (list->tail == nodeToDelete) // simply add it to the end of the list
    {
        list->tail = nodeToDelete->prev;
        nodeToDelete->prev->next = NULL;
    }
    else // delete the node
    {
        nodeToDelete->next->prev = nodeToDelete->prev;
        nodeToDelete->prev->next = nodeToDelete->next;
    }
}

// #################################################################################################
// Add an entry to a doubly linked list at the head
//
// Parameters:  nodeToAdd = pointer to the node to add to the head
//              list      = collection of the head and tail of the list
// Return:
// Assumptions: The nodeToAdd is a new node and not an existing one...using a node that exists
//              will break the list
//
// #################################################################################################
void UTIL_DblListInsertHeadPtr(struct doubleLinkPtrInfo *list, doubleListNodePtr * nodeToAdd)
{
    doubleListNodePtr *head = list->head;
    UTIL_DblListAddNodePtr(list, head, nodeToAdd);
}

// #################################################################################################
// Add an entry to a doubly linked list at the tail
//
// Parameters:  nodeToAdd = pointer to the node to add
//              list      = collection of the head and tail of the list
// Return:
// Assumptions: The nodeToAdd is a new node and not an existing one...using a node that exists
//              will break the list
//
// #################################################################################################
void UTIL_DblListAddToTailPtr(struct doubleLinkPtrInfo *list, doubleListNodePtr *nodeToAdd)
{
    UTIL_DblListAddNodePtr(list, NULL, nodeToAdd);
}

// #################################################################################################
// Delete an entry from a doubly indexed linked list
//
// Parameters:  nodeToDelete = index into the list
//              list      = collection of the head and tail of the list
// Return:
// Assumptions:
//              upon initialization the head is always 0 as is the tail
//              you can prune the list by simply setting the next to LINKEDLIST_EMPTY_IDX_LIST
//
//
//
//
//
//
//
// #################################################################################################
void UTIL_DblListDeleteNodeIdx(struct doubleListIdxInfo *listInfo, uint8_t nodeToDelete, struct doubleListNodeIdx *list)
{

    struct doubleListNodeIdx *tmp = &list[nodeToDelete];
    struct doubleListNodeIdx *rst = &list[nodeToDelete];

    //	iassert_UTIL_COMPONENT_1(listHead != NULL, UTIL_EMPTY_LIST, __LINE__);
    if (listInfo->head == nodeToDelete) // We want to delete the head
    {
        if (listInfo->tail == nodeToDelete) // If the head == the tail the list becomes empty
        {
            listInfo->head = LINKEDLIST_EMPTY_IDX_LIST;
            listInfo->tail = LINKEDLIST_EMPTY_IDX_LIST;
        }
        else                               // just delete the head the new head is the [head.next] element
        {                                  // the [head.next].prev element is now invalid
            listInfo->head = tmp->next;
            tmp = &list[listInfo->head];
            tmp->prev = LINKEDLIST_EMPTY_IDX_LIST;
        }
    }
    else if (listInfo->tail == nodeToDelete) // simply delete it from the end of the list
    {
        listInfo->tail = tmp->prev;
        tmp = &list[listInfo->tail];
        tmp->next = LINKEDLIST_EMPTY_IDX_LIST;
    }
    else // delete the node somewhere in the middle of the list
    {
        struct doubleListNodeIdx *next = &list[tmp->next];
        struct doubleListNodeIdx *prev = &list[tmp->prev];
        next->prev = tmp->prev;
        prev->next = tmp->next;
    }
    rst->next = rst->prev = LINKEDLIST_EMPTY_IDX_LIST;
}

// #################################################################################################
// Add an entry into a doubly indexed linked list
//
// Parameters:  node      = index of the node to add before
//              nodeToAdd = Which index you want to insert
//              list      = collection of the head and tail of the list
// Return:
// Assumptions: 
//              upon initialization the head is always 0 as is the tail
//              should fail if node to add is 255, as this is an invalid node index.
//
//
//
//
//
//
//
// #################################################################################################
void UTIL_DblListAddNodeIdx(struct doubleListIdxInfo *listInfo, uint8_t node, uint8_t nodeToAdd, struct doubleListNodeIdx *list)
{
    struct doubleListNodeIdx *tmp = &list[nodeToAdd];

    if (listInfo->head == LINKEDLIST_EMPTY_IDX_LIST) // the list has not been initialized
    {
        tmp->next = LINKEDLIST_EMPTY_IDX_LIST;
        tmp->prev = LINKEDLIST_EMPTY_IDX_LIST;

        listInfo->head = nodeToAdd;
        listInfo->tail = nodeToAdd;

    }
    else if (node == LINKEDLIST_EMPTY_IDX_LIST) // Add it to the end of the list
    {
        struct doubleListNodeIdx *oldEndOfList = &list[listInfo->tail];
        struct doubleListNodeIdx *newEndOfList = &list[nodeToAdd];

        newEndOfList->prev = listInfo->tail;
        listInfo->tail = nodeToAdd;
        oldEndOfList->next = nodeToAdd;
        newEndOfList->next = LINKEDLIST_EMPTY_IDX_LIST;
    }
    else // add the nodeToAdd before node
    {
        struct doubleListNodeIdx *insertBeforeNode = &list[node];
        struct doubleListNodeIdx *nodeToInsert = &list[nodeToAdd];

        if (node == listInfo->head)
        {
            listInfo->head = nodeToAdd;
            nodeToInsert->prev = LINKEDLIST_EMPTY_IDX_LIST;
        }
        insertBeforeNode->prev = nodeToAdd;
        nodeToInsert->next = node;
    }
}

