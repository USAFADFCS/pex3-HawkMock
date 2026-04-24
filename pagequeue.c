/** pagequeue.c
 * ===========================================================
 * Name: Dustin Mock, 23 Apr 2026
 * Section: CS483 / M4
 * Project: PEX3 - Page Replacement Simulator
 * Purpose: Implementation of the PageQueue ADT — a doubly-linked
 *          list for LRU page replacement.
 *          Head = LRU (eviction end), Tail = MRU end.
 * =========================================================== */
#include <stdio.h>
#include <stdlib.h>

#include "pagequeue.h"

/**
 * @brief Create and initialize a page queue with a given capacity
 */
PageQueue *pqInit(unsigned int maxSize) {
    PageQueue *pq = malloc(sizeof(PageQueue));
    if (pq == NULL) {
        fprintf(stderr, "pqInit: malloc failed for PageQueue\n");
        exit(1);
    }
    pq->head = NULL;
    pq->tail = NULL;
    pq->size = 0;
    pq->maxSize = maxSize;
    return pq;
    return NULL;
}

/**
 * @brief Access a page in the queue (simulates a memory reference)
 */
long pqAccess(PageQueue *pq, unsigned long pageNum) {
    /* Walk tail -> head so depth counts naturally from the MRU end. */
    long depth = 0;
    PqNode *cur = pq->tail;
    while (cur != NULL) {
        if (cur->pageNum == pageNum) {
            /* HIT - promote cur to the tail position unless it is
             * already there. */
            if (cur != pq->tail) {
                /* Unlink cur from its current position. cur->next is
                 * guaranteed non-NULL here because cur != tail. */
                if (cur->prev != NULL) {
                    cur->prev->next = cur->next;
                } else {
                    /* cur was the head - promote cur->next to head. */
                    pq->head = cur->next;
                }
                cur->next->prev = cur->prev;

                /* Re-insert cur at the tail. */
                cur->prev = pq->tail;
                cur->next = NULL;
                pq->tail->next = cur;
                pq->tail = cur;
            }
            return depth;
        }
        cur = cur->prev;
        depth++;
    }

    /* MISS - append a new node at the tail. */
    PqNode *node = malloc(sizeof(PqNode));
    if (node == NULL) {
        fprintf(stderr, "pqAccess: malloc failed for PqNode\n");
        exit(1);
    }
    node->pageNum = pageNum;
    node->prev = pq->tail;
    node->next = NULL;

    if (pq->tail == NULL) {
        /* Queue was empty - new node is both head and tail. */
        pq->head = node;
    } else {
        pq->tail->next = node;
    }
    pq->tail = node;
    pq->size++;

    /* Evict the LRU (head) if we have exceeded capacity. */
    if (pq->size > pq->maxSize) {
        PqNode *victim = pq->head;
        pq->head = victim->next;
        if (pq->head != NULL) {
            pq->head->prev = NULL;
        } else {
            pq->tail = NULL;
        }
        free(victim);
        pq->size--;
    }

    return -1;
}

/**
 * @brief Free all nodes in the queue and reset it to empty
 */
void pqFree(PageQueue *pq) {
    if (pq == NULL) {
        return;
    }
    PqNode *cur = pq->head;
    while (cur != NULL) {
        PqNode *next = cur->next;
        free(cur);
        cur = next;
    }
    free(pq);
}

/**
 * @brief Print queue contents to stderr for debugging
 */
void pqPrint(PageQueue *pq) {
    if (pq == NULL) {
        fprintf(stderr, "pqPrint: queue is NULL\n");
        return;
    }
    fprintf(stderr, "PageQueue (size=%u, maxSize=%u): ", pq->size, pq->maxSize);
    if (pq->head == NULL) {
        fprintf(stderr, "[empty]\n");
        return;
    }
    PqNode *cur = pq->head;
    fprintf(stderr, "[H]");
    while (cur != NULL) {
        fprintf(stderr, " %lu", cur->pageNum);
        if (cur == pq->tail) {
            fprintf(stderr, "[T]");
        }
        cur = cur->next;
    }
    fprintf(stderr, "\n");
}
