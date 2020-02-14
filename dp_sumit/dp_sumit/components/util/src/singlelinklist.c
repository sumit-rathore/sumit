///////////////////////////////////////////////////////////////////////////////
///
///   Icron Technology Corporation - Copyright 2019
///
///
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
// Add an entry to a singly linked list
//
// Parameters:  node      = place to add an entry after
//              nodeToAdd = pointer to the node to add
//              list      = collection of the head and tail of the list
// Return:
// Assumptions: 
//
// #################################################################################################
void  UTIL_SglListAddNodePtr(struct singleListPtrInfo *list,
    singleListNodePtr *node,
    singleListNodePtr *nodeToAdd)

{
    if (list->head == NULL) // the list has not been initialized
    {
        list->head = nodeToAdd;
        list->tail = nodeToAdd;
        nodeToAdd->next = NULL;
    }
    else if (node == NULL) // simply add it to the end of the list
    {
        list->tail->next = nodeToAdd;
        nodeToAdd->next = NULL;
        list->tail = nodeToAdd;
    }
    else // add the nodeToAdd before node
    {
        singleListNodePtr *currentNode = list->head;

        while (currentNode != NULL)
        {
            if (node == currentNode->next)
            {
                currentNode->next = nodeToAdd;
                nodeToAdd->next = node;
                break;
            }
            currentNode = currentNode->next;
        }
    }
}

// #################################################################################################
// Delete an entry from a singly linked list
//
// Parameters:  nodeToDelete = pointer to the node to add
//              list      = collection of the head and tail of the list
// Return:
// Assumptions: 
//
// #################################################################################################
void  UTIL_SglListDelNodePtr(struct singleListPtrInfo *list,
    singleListNodePtr *nodeToDelete)
{

    //	iassert_UTIL_COMPONENT_1(listHead != NULL, UTIL_EMPTY_LIST, __LINE__);

    if (list->head == nodeToDelete)
    {
        if (list->tail == nodeToDelete) // the list has not been initialized
        {
            list->head = NULL;
            list->tail = NULL;
        }
        else
        {
            list->head = list->head->next;
        }
    }
    else // simply delete the node when found
    {
        singleListNodePtr  *currentNode = list->head;
        singleListNodePtr  *previousNode = list->head;

        while (currentNode != NULL)
        {
            if (currentNode == nodeToDelete)
            {
                if (currentNode == previousNode) // delete the head
                {
                    list->head = currentNode->next;
                }
                previousNode->next = currentNode->next;
                return;
            }
            previousNode = currentNode;
            currentNode = currentNode->next;
        }
    }
}
