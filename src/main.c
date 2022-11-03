#include <stdio.h>
#include <stdlib.h>
#include "priorityQ.h"

int main() {
    PriorityQ* pq = createPriorityQ();

    Node* node3 = createNode(3, 30, NULL, NULL);
    pushPriorityQ(pq, node3);

    Node* node2 = createNode(2, 20, NULL, NULL);
    pushPriorityQ(pq, node2);

    Node* node1 = createNode(1, 10, NULL, NULL);
    pushPriorityQ(pq, node1);

    printPriorityQ(pq);

    Node* tmp;
    popPriorityQ(pq, tmp);

    printPriorityQ(pq);

    free(pq);

    return 0;
}
