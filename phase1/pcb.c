#include <pcb.h>

pcb_t *pcbFree_h;

void initPcbs(){
    return;
}

void freePcb(pcb_t *p){
    p->p_next = pcbFree_h;
    pcbFree_h = p;
    return;
}

pcb_t *allocPcb(){
    pcb_t *head = NULL;
    if (pcbFree_h != NULL){
        head = pcbFree_h;
        pcbFree_h = pcbFree_h->p_next;
        /*Clean the pcb*/
        head->p_child = NULL;
        head->p_next = NULL;
        head->p_next_sib = NULL;
        head->p_prev = NULL;
        head->p_prev_sib = NULL;
        head->p_prnt = NULL;
        head->p_semAdd = NULL;
        head->p_time = 0;
    }
    return head;
}

pcb_t *mkEmptyProcQ(){
    return NULL;
}

int emptyProcQ(pcb_t *tp){
    return 0;
}

void insertProcQ(pcb_t **tp, pcb_t *p){
    return;
}

pcb_t *headProcQ(pcb_t **tp){
    return NULL;
}

pcb_t *removeProcQ(pcb_t **tp){
    return NULL;
}

pcb_t *outProcQ(pcb_t **tp, pcb_t *p){
    return NULL;
}

int emptyChild(pcb_t *p){
    return 0;
}

void insertChild(pcb_t *prnt,pcb_t *p){
    return;
}

pcb_t* removeChild(pcb_t *p){
    return NULL;
}

pcb_t *outChild(pcb_t* p){
    return NULL;
}
