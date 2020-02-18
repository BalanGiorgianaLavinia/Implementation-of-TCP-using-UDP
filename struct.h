#define PAYLOAD_FRAME_SIZE (MSGSIZE - 2 * sizeof(int) - 3 * sizeof(unsigned char))
#include "link_emulator/lib.h"

typedef struct {
    int length;
    int index;
    unsigned char crc_length;
    unsigned char crc_index;
    unsigned char crc_payload;
    char payload[PAYLOAD_FRAME_SIZE];
}framework;


typedef struct List{
    void *info;
    struct List *next;
    int index;
}list;


//Functie care aduga un element in lista cu un index dat
void pushList(list **l, void *info, int index) {
    //inserare la inceputul listei
    if(*l == NULL) {
        *l = calloc(1, sizeof(list));
        if(*l == NULL) {
            printf("[pushList] Eroare la alocare celula \n"); 
            return;
        }

        (*l)->info = info;
        (*l)->index = index;
        (*l)->next = NULL;
        
        return;
    }

    list *cursor = *l;
    //ma mut pe ultima celula
    while(cursor->next) {
        cursor = cursor->next;
    }

    cursor->next = calloc(1, sizeof(list));
    
    if(cursor->next == NULL){
        printf("[pushList] Eroare la alocare celula \n");
        return;
    }
    cursor = cursor->next;

    cursor->index = index;
    cursor->info = info;
    cursor->next = NULL;
}


//Functie de stergere a unui element din lista
void removeList(list **l, int index) {
    if((*l) == NULL){
        printf("[removeList] Lista e vida\n"); 
        return;
    }

    //Elementul cautat este pe prima celula din lista
    if((*l)->index == index) {
        list *aux = *l;
        *l = (*l)->next;
        if(aux->info != NULL){
            free(aux->info);
        }

        free(aux);

        return;
    }

    list *ant = *l;
    list *cursor = (*l)->next;
    while(cursor) {
        if(cursor->index == index) {
            ant->next = cursor->next;
            if(cursor->info != NULL){
                free(cursor->info);
            }

            free(cursor);

            return;
        }

        ant = cursor;
        cursor = cursor->next;
    }

}



//Functie care returneaza un element din lista inlantuita
void *getFromList(list *l, int index) {

    while(l){
        if(l->index == index) {
            return l->info;
        }

        l = l->next;
    }

    return NULL;
}


//Functie care returneaza codul de verificare, utila pentru coruperi
unsigned char crc(void *info, int len){
    unsigned char *data = (unsigned char*)info;
    unsigned char crc = 0xff;
    int i;

    for (i = 0; i < len; i++) {
        crc ^= data[i];
    }
    return crc;
}


//Functie care  verifica daca mesajul este corupt
//Recalculeaza checksum-ul pentru fiecare camp din payload-ul mesajului
//si verifica daca se potriveste cu cel initial
int check_crc(framework *frame) {

    unsigned char check_length = crc((void *)&frame->length, sizeof(frame->length));
    if(check_length != frame->crc_length) return 0;

    unsigned char check_index = crc((void *)&frame->index, sizeof(frame->index));
    if(check_index != frame->crc_index) return 0;

    unsigned char check_payload = crc((void *)frame->payload, frame->length);
    if(check_payload != frame->crc_payload) return 0;

    return 1;
}