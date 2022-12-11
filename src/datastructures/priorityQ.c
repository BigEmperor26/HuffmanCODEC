#include "./priorityQ.h"

Node* createNode(int value, ull priority, Node* left, Node* right) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->left = left;
    node->right = right;
    node->value = value;
    node->priority = priority;
    return node;
}

void freeNode(Node* node){
    if (node->left != NULL){
        freeNode(node->left);
        node->left = NULL;
    }

    if (node->right != NULL){
        freeNode(node->right);
        node->right = NULL;
    }

    free(node);
}

PriorityQ* createPriorityQ() {
    PriorityQ* pq = (PriorityQ*)malloc(sizeof(PriorityQ));
    pq->minHeap = (Node**)malloc(sizeof(Node*) * MAX_HEAP_SIZE);
    pq->size = 0;
    pq->capacity = MAX_HEAP_SIZE;
    // pq->minHeap is an array of Node* stored in the stack
    return pq;
}

void freePriorityQ(PriorityQ* pq) {
    for (int i = 0; i < pq->size; i++) {
        free(pq->minHeap[i]);
        pq->minHeap[i] = NULL;
    }
    free(pq->minHeap);
    free(pq);
    return;
}

void printPriorityQ(PriorityQ* pq) {
    printf("Value\tPriority\n");
    for (int i = 0; i < pq->size; i++) {
        printf("%d\t%llu\n", pq->minHeap[i]->value, pq->minHeap[i]->priority);
    }
    printf("\nSize: %d\n", pq->size);
    printf("Capacity %d\n", pq->capacity);
    return;
}

bool isEmpty(PriorityQ* pq) {
    return pq->size == 0;
}

bool isFull(PriorityQ* pq) {
    return pq->size == pq->capacity;
}

bool swapMinHeapElements(Node** minHeap, int index_a, int index_b) {
    Node* temp = *(minHeap + index_a);
    *(minHeap + index_a) = *(minHeap + index_b);
    *(minHeap + index_b) = temp;
    return true;
}

bool pushPriorityQ(PriorityQ* pq, Node* node) {
    if (pq->size == pq->capacity) {
        return false;
    }
    pq->minHeap[pq->size] = node;
    pq->size++;
    int i = pq->size - 1;
    while (i > 0 && pq->minHeap[i]->priority < pq->minHeap[(i - 1) / 2]->priority) {
        swapMinHeapElements(pq->minHeap, i, (i - 1) / 2);
        i = (i - 1) / 2;
    }
    return true;
}

bool popPriorityQ(PriorityQ* pq, Node** node) {
    if (pq->size == 0) {
        return false;
    }
    // pop the first element, which is the minimum
    *node = pq->minHeap[0];
    pq->minHeap[0] = pq->minHeap[pq->size - 1];
    pq->size--;
    int i = 0;
    while (i < pq->size) {
        Node* current = pq->minHeap[i];
        int leftChildIndex = 2 * i + 1;
        int rightChildIndex = 2 * i + 2;
        // if all children
        if (leftChildIndex < pq->size && leftChildIndex < pq->size) {
            Node* leftChild = pq->minHeap[leftChildIndex];
            Node* rightChild = pq->minHeap[rightChildIndex];
            if (current->priority > leftChild->priority || current->priority > rightChild->priority) {
                if (leftChild->priority < rightChild->priority) {
                    swapMinHeapElements(pq->minHeap, i, leftChildIndex);
                    i = leftChildIndex;
                }
                else {
                    swapMinHeapElements(pq->minHeap, i, rightChildIndex);
                    i = rightChildIndex;
                }
            }
            else {
                break;
            }
        }
        // if no right child
        else if (2 * i + 1 < pq->size) {
            Node* leftChild = pq->minHeap[leftChildIndex];
            if (current->priority < leftChild->priority) {
                swapMinHeapElements(pq->minHeap, i, leftChildIndex);
                i = leftChildIndex;
            }
            else {
                break;
            }
        }
        // if no children
        else {
            break;
        }
    }
    return true;
}

bool topPriorityQ(PriorityQ* pq, Node** node) {
    if (isEmpty(pq)) {
        return false;
    }

    // the first element in the min heap is the minimum
    *node = pq->minHeap[0];
    return true;
}
