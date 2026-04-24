/** main.c
 * ===========================================================
 * Name: Dustin Mock, 23 Apr 2026
 * Section: CS483 / M4
 * Project: PEX3 - Page Replacement Simulator
 * Purpose: Reads a BYU binary memory trace file and simulates
 *          LRU page replacement to measure fault rates across
 *          varying frame allocations.
 * Documentation: Used PEX slides, Google for minor library function lookups, and my DLL from CS220.
 * =========================================================== */
#include <stdio.h>
#include <stdlib.h>
#include "byutr.h"
#include "pagequeue.h"

#define PROGRESS_INTERVAL 100000  // print status every N accesses

int main(int argc, char **argv) {
    FILE *ifp = NULL;
    unsigned long numAccesses = 0;
    p2AddrTr traceRecord;

    // Validate command-line arguments: trace_file frame_size
    if (argc != 3) {
        fprintf(stderr, "usage: %s input_byutr_file frame_size\n", argv[0]);
        fprintf(stderr, "\nframe_size:\n\t1: 512 bytes\n\t2: 1KB\n\t3: 2KB\n\t4: 4KB\n");
        exit(1);
    }

    // Open the binary trace file
    ifp = fopen(argv[1], "rb");
    if (ifp == NULL) {
        fprintf(stderr, "cannot open %s for reading\n", argv[1]);
        exit(1);
    }

    // Parse and validate frame size selection
    int menuOption = atoi(argv[2]);
    if (menuOption < 1 || menuOption > 4) {
        fprintf(stderr, "invalid frame size option: %s (must be 1-4)\n", argv[2]);
        fclose(ifp);
        exit(1);
    }

    // Map menu option to page geometry
    int offsetBits = 0;
    int maxFrames = 0;
    switch (menuOption) {
        case 1:
            offsetBits = 9;   // 512-byte pages
            maxFrames = 8192;
            break;
        case 2:
            offsetBits = 10;  // 1KB pages
            maxFrames = 4096;
            break;
        case 3:
            offsetBits = 11;  // 2KB pages
            maxFrames = 2048;
            break;
        case 4:
            offsetBits = 12;  // 4KB pages
            maxFrames = 1024;
            break;
    }

    fprintf(stderr, "Frame size option %d: %d offset bits, %d max frames, algorithm=LRU\n",
            menuOption, offsetBits, maxFrames);

    // Allocate the page queue at the largest frame allocation we will sim
    PageQueue *pq = pqInit((unsigned int)maxFrames);

    // Difference array for range-update / point-query of faults.
    // diff[1..maxFrames] receives +1 / -1 events; a prefix sum at
    // output time produces faults[f] = total faults at f frames.
    // Size = maxFrames + 2 so diff[maxFrames + 1] is in bounds.
    long *diff = calloc((size_t)maxFrames + 2, sizeof(long));
    if (diff == NULL) {
        fprintf(stderr, "calloc failed for diff[%d]\n", maxFrames + 2);
        pqFree(pq);
        fclose(ifp);
        exit(1);
    }

    while (fread(&traceRecord, sizeof(p2AddrTr), 1, ifp) == 1) {
        // Extract page number by shifting off the offset bits
        unsigned long pageNum = traceRecord.addr >> offsetBits;
        numAccesses++;

        // Print progress indicator to stderr every PROGRESS_INTERVAL accesses
        // (also prints the last page number seen - useful for early debugging)
        if ((numAccesses % PROGRESS_INTERVAL) == 0) {
            fprintf(stderr, "%lu samples read, last page: %lu\r", numAccesses, pageNum);
        }

        long depth = pqAccess(pq, pageNum);
        if (depth == -1) {
            // MISS - every frame size 1..maxFrames would have faulted
            diff[1] += 1;
            diff[maxFrames + 1] -= 1;
        } else if (depth > 0) {
            // HIT at depth d > 0 - frame sizes 1..d would have faulted
            diff[1] += 1;
            diff[depth + 1] -= 1;
        }
        // depth == 0 (already-MRU hit): every frame size holds it; no fault
    }

    fprintf(stderr, "\n%lu total accesses processed\n", numAccesses);

    // Output CSV results to stdout (redirect with > to create a .csv file)
    printf("Total Accesses:,%lu\n", numAccesses);
    printf("Frames,Missees,Miss Rate\n");

    // Convert the difference array into running fault totals on the fly.
    // After processing N accesses with the rules above, prefix_sum(diff)[f]
    // equals the number of accesses that would have faulted with f frames.
    unsigned long running = 0;
    for (int f = 1; f <= maxFrames; f++) {
        running += (unsigned long)diff[f];
        printf("%d,%lu,%f\n", f, running,
            (double)running / (double)numAccesses);
    }
    // Cleanup: queue, difference array, file handle.
    pqFree(pq);
    free(diff);
    fclose(ifp);

    return 0;
}
