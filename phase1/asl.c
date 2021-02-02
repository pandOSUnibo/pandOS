#include "asl.h"

HIDDEN semd_t semd_table[MAXPROC];
HIDDEN semd_t *semdFree_h;
HIDDEN semd_t *semd_h;

int insertBlocked(int *semAdd, pcb_t *p) {

}

pcb_t* removeBlocked(int *semAdd) {

}

pcb_t* outBlocked(pcb_t *p) {

}

pcb_t* headBlocked(int *semAdd) {

}

void initASL() {
    semdFree_h = &(semd_table[0]);
    for (int i = 0; i < MAXPROC - 1; i++) {
        semd_table[i].s_next = &(semd_table[i + 1]);
    }
    semd_table[MAXPROC - 1].s_next = NULL;

    semd_h = NULL;
}