/** pagequeue.h
 * ===========================================================
 * Name: Dustin Mock, 23 Apr 2026
 * Section: CS483 / M4
 * Project: PEX3 - Page Replacement Simulator
 * Purpose: Doubly-linked list page queue ADT for LRU page
 *          replacement. Head = LRU end, Tail = MRU end.
 * =========================================================== */
#ifndef PAGEQUEUE_H
#define PAGEQUEUE_H

/* --- Type Definitions --- */

// Doubly-linked list node representing a single page in memory
typedef struct PqNode {
    unsigned long pageNum;
    struct PqNode *prev;
    struct PqNode *next;
} PqNode;

// Page queue with bounded capacity (head = oldest, tail = newest)
typedef struct PageQueue {
    PqNode *head;
    PqNode *tail;
    unsigned int size;
    unsigned int maxSize;
} PageQueue;

/* --- Function Prototypes --- */

/**
 * @brief Create and initialize a page queue with a given capacity
 * @param maxSize maximum number of pages the queue may hold
 * @return pointer to the newly allocated PageQueue
 */
PageQueue *pqInit(unsigned int maxSize);

/**
 * @brief Access a page in the queue (simulates a memory reference)
 * @param pq pointer to the PageQueue
 * @param pageNum the page number being accessed
 * @return depth from MRU end (0 = most recent) if page was found,
 *         or -1 if the page was not in the queue (page fault)
 * @note On a hit, the page is moved to the MRU (tail) position.
 *       New pages are always inserted at the tail. If inserting
 *       causes size to exceed maxSize, the head (LRU) page
 *       is evicted.
 */
long pqAccess(PageQueue *pq, unsigned long pageNum);

/**
 * @brief Free all nodes in the queue and reset it to empty
 * @param pq pointer to the PageQueue to free
 */
void pqFree(PageQueue *pq);

/**
 * @brief Print queue contents to stderr for debugging
 * @param pq pointer to the PageQueue to display
 */
void pqPrint(PageQueue *pq);

#endif  // PAGEQUEUE_H
