//#################################################################################################
// Icron Technology Corporation - Copyright 2019
//
// This source file and the information contained in it are confidential and proprietary to Icron
// Technology Corporation. The reproduction or disclosure, in whole or in part, to anyone outside
// of Icron without the written approval of a Icron officer under a Non-Disclosure Agreement, or to
// any employee of Icron who has not previously obtained written authorization for access from the
// individual responsible for the source code, will have a significant detrimental effect on Icron
// and is expressly prohibited.
//#################################################################################################
#ifndef LINKEDLIST_H
#define LINKEDLIST_H

// Includes #######################################################################################
#include "ibase.h"

// Constants and Macros ###########################################################################

#define LINKEDLIST_EMPTY_IDX_LIST 0xFF

// Data Types #####################################################################################
struct doubleListNodePtr_t
{
    struct doubleListNodePtr_t *next;                     // the next item in the list, NULL if at start
    struct doubleListNodePtr_t *prev;                     // the previous item on the list, NULL if at the end
};

typedef struct doubleListNodePtr_t doubleListNodePtr;

struct doubleLinkPtrInfo
{
    doubleListNodePtr *head;         // indicating address of link header having first element's address
    doubleListNodePtr *tail;         // indicating address of link tail having last element's address
};

struct singleListNodePtr_t
{
    struct singleListNodePtr_t *next;                     // the next item in the list, NULL if at start
};

typedef struct singleListNodePtr_t singleListNodePtr;

struct singleListPtrInfo
{
    singleListNodePtr *head;         // indicating address of link header having first element's address
    singleListNodePtr *tail;         // indicating address of link header having last element's address
};

struct doubleListNodeIdx
{
    uint8_t next;                     // the next item in the list, 0xFF if at start
    uint8_t prev;                     // the previous item on the list, 0xFF if at the end
};

struct doubleListIdxInfo
{
    uint8_t head;                     // first element's index 0-254
    uint8_t tail;                     // last element's index 0-254
};

struct singleListNodeIdx
{
    uint8_t next;                     // the next item in the list(index 0-254), 0xFF if at end
    uint8_t prev;                     // the previous item on the list(index 0-254), 0xFF if at the start
};

struct singleListIdxInfo
{
    uint8_t head;         // indicating index of link header having first element's index
	uint8_t tail;         // indicating index of link header having first element's index
};


// Function Declarations ##########################################################################
//
//
//##########################################################################################
//  pointer linked list functions
//##########################################################################################
// Delete a node from the doubly linked list
void UTIL_DblListDelNodePtr(struct doubleLinkPtrInfo *,
	                        doubleListNodePtr *
	);

void UTIL_DblListAddNodePtr(struct doubleLinkPtrInfo *,
	                        doubleListNodePtr *,
	                        doubleListNodePtr *
	);

void UTIL_DblListInsertHeadPtr(struct doubleLinkPtrInfo *,
	doubleListNodePtr *
);

void UTIL_DblListAddToTailPtr(struct doubleLinkPtrInfo *,
	doubleListNodePtr *
);

uint16_t UTIL_DblListLengthPtr(doubleListNodePtr *);


// To be implemented
void UTIL_SglListDelNodePtr(struct singleListPtrInfo *,
	                         singleListNodePtr *);

void UTIL_SglListAddNodePtr(struct singleListPtrInfo *,
	                         singleListNodePtr *,
                             singleListNodePtr *);

 //##########################################################################################
//  Indexed linked list functions
//##########################################################################################
 void UTIL_DblListDeleteNodeIdx(struct doubleListIdxInfo *head, uint8_t nodeToDelete, struct doubleListNodeIdx *list);

 void UTIL_DblListAddNodeIdx(struct doubleListIdxInfo *head, uint8_t node, uint8_t nodeToAdd, struct doubleListNodeIdx *list);

#if 0
 // To be implemented
 void sglListIdx_deleteNode(uint8_t nodeToDelete, struct doubleListNodeIdx *list,
                       struct singleListIdxInfo *head);

 void sglListIdx_addNode(uint8_t nodeToAdd,
                    struct singleListIdxInfo *head);

#endif
#endif // LINKEDLIST_H
