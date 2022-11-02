#include "priorityQ.h"
// simple implementation of priority queue using binary max heap

// implemented using a vector because it's more efficient and easier to send using MPI

struct vectorTree *createVectorTree(int capacity){
    struct vectorTree *v = malloc(sizeof(struct vectorTree));
    v->data = malloc(sizeof(void*) * capacity);
    v->priority = malloc(sizeof(int) * capacity);
    v->size = 0;
    v->capacity = capacity;
    return v;
}

void freeVectorTree(struct vectorTree *v){
    free(v->data);
    free(v->priority);
    free(v);
}

void printVectorTree(struct vectorTree *v){
    printf("Data: \t");
    for(int i = 0; i < v->size; i++){
        printf("%X ",  * (unsigned int*) (v->data[i]));

    }
    printf("\n");
    printf("Priority: \t");
    for(int i = 0; i < v->size; i++){
        printf("%d ",v->priority[i]);
        
    }
    printf("\n");
}
void printHeight(struct vectorTree *v,int height){
    int start = intpow2(height) - 1;
    int end = intpow2(height + 1) - 1;
    for(int i = start; i < end; i++){
        printf("(%X,%d)",* (unsigned int*) v->data[i], v->priority[i]);
    }
}
void printTree(struct vectorTree *v){
    printf("Tree: \n");
    int height = (int) intlog2(v->size);
    for(int i = 0; i < height; i++){
        printHeight(v, i);
        printf("\n");
    }
    int start = intpow2(height) - 1;
    int end = v->size;
    for(int i = start; i < end; i++){
        printf("(%X,%d)", * (unsigned int*) v->data[i], v->priority[i]);
    }
    printf("\n");
        
}


int empty(struct vectorTree *v){
    return v->size == 0;
}
int full(struct vectorTree *v){
    return v->size == v->capacity;
}
int size(struct vectorTree *v){
    return v->size;
}

int intlog2(int x){
    int i = 0;
    while(x > 1){
        x = x / 2;
        i++;
    }
    return i;
}
int intpow2(int x){
    int i = 1;
    while(x > 0){
        i = i * 2;
        x--;
    }
    return i;
}

int swap(void **value, int *priority, int index_a, int index_b){
    void  *temp = *(value+index_a);
    *(value+index_a) = *(value+index_b);
    *(value+index_b) = temp;
    int tmp = *(priority+index_a);
    *(priority+index_a) = *(priority+index_b);
    *(priority+index_b) = tmp;
    return 1;
}

int push(struct vectorTree *v, void * value,int priority){
    if(v->size == v->capacity){
        return 0;
    }
    v->data[v->size] = value;
    v->priority[v->size] = priority;
    v->size++;
    int i = v->size - 1;
    while(i > 0 && v->priority[i] > v->priority[(i - 1) / 2]){
        swap(v->data, v->priority, i, (i - 1) / 2);
        i = (i - 1) / 2;
    }
    return 1;
}
int top(struct vectorTree *v,void ** data, int *priority){
    if (empty(v)){
        return 0;
    }
    *data = v->data[0];
    *priority = v->priority[0];
    return 1;
}
// remove max priority
int pop(struct vectorTree *v,void ** data, int *priority){
    if(v->size == 0){
        return 0;
    }
    *data = v->data[0];
    *priority = v->priority[0];
    v->data[0] = v->data[v->size - 1];
    v->priority[0] = v->priority[v->size - 1];
    v->size--;
    int i = 0;
    while(i < v->size ){
        // if all children
        if(2 * i + 1 < v->size && 2 * i + 2 < v->size){
            if(v->priority[i] < v->priority[2 * i + 1] || v->priority[i] < v->priority[2 * i + 2]){
                if(v->priority[2 * i + 1] > v->priority[2 * i + 2]){
                    swap(v->data, v->priority, i, 2 * i + 1);
                    i = 2 * i + 1;
                }
                else{
                    swap(v->data, v->priority, i, 2 * i + 2);
                    i = 2 * i + 2;
                }
            }
            else{
                break;
            }
        }
        // if no right child
        else if(2 * i + 1 < v->size){
            if(v->priority[i] < v->priority[2 * i + 1]){
                swap(v->data, v->priority, i, 2 * i + 1);
                i = 2 * i + 1;
            }
            else{
                break;
            }
        }
        // if no children
        else{
            break;
        }
    }
    return 1;
}
int main(){
    struct vectorTree *v = createVectorTree(20);
    // v, value, priority
    int * a = malloc(sizeof(int));
    *a = 1;
    push(v, a,5);
    int * b = malloc(sizeof(int));
    *b = 2;
    push(v, b,4);
    // int c = 2;
    // push(v, &c,10);
    // int d = 3;
    // push(v, &d,6);
  
    // push(v, 9,9);
    // push(v, 10,8);
    // push(v, 1,3);
    // push(v, 2,2);
    // push(v, 3,1);
    // push(v, 4,7);
    // push(v, 7,4);
    // push(v, 8,10);

    // printVectorTree(v);
    printVectorTree(v);    
    printTree(v);

    int p = 0;
    int ** g = malloc(sizeof(int*));
    int res = pop(v,(void **)g, &p);
    printf("pop: %X, %d\n", **g, p);
    free(*g);
    free(g);
    //printf("pop: %X, %d\n", *g, p);
    // freeVectorTree(v);
    return 0;
}
