#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"

#include "struct.h"

#define HOST "127.0.0.1"
#define PORT 10000

#define MINIM(a, b) (((a) < (b)) ? (a) : (b))


int main(int argc,char** argv){
    init(HOST,PORT);
    msg ack;


    char *file_name = argv[1];
    int speed = atoi(argv[2]);
    int delay = atoi(argv[3]);

    int timeout = delay/3;


    //calculez dimensiunea fisierului
    int fd = open(file_name, O_RDONLY);
    if(fd < 0){
        printf("[SENDER] Eroare la deschiderea fisierului!\n");
        return -1;
    }
    lseek(fd, 0, SEEK_SET);
    int file_dim = lseek(fd, 0, SEEK_END);



    //calculez numarul total de pachete
    int COUNT = file_dim / PAYLOAD_FRAME_SIZE;
    if(COUNT * PAYLOAD_FRAME_SIZE < file_dim)
        COUNT += 1;
    //mai adaug si mesajele ce vor contine numele fisierului si COUNT-ul
    COUNT += 2;


    //repozitionez cursorul la inceputul fisierului
    lseek(fd, 0, SEEK_SET);

    //calculez fereastra
    int window = (speed * delay * 1000) / (sizeof(msg) * 8);
    window = MINIM(window, COUNT);

    //lista de mesaje
    list *l = NULL;  

    //pentru numele fisierului
    msg *m = calloc(1, sizeof(msg));
    if(m == NULL){
        printf("[SENDER] Nu s-a alocat spatiu pentru mesajul 0\n");
        return -2;
    }

    framework name;

    //Completez campurile structurii
    memset(&name, 0, sizeof(framework));

    name.index = 0;
    name.crc_index = crc((void*)&name.index, sizeof(name.index));

    name.length = strlen(file_name)+1;
    name.crc_length = crc((void*)&name.length, sizeof(name.length));

    memcpy(name.payload, file_name, name.length); 
    name.crc_payload = crc((void*)name.payload, name.length);

    memcpy(m->payload, &name, sizeof(framework));
    m->len = sizeof(framework);

    //Adaug in lista numele fisierului 
    pushList(&l, (void*)m, name.index);

    //Il trimit
    send_message(m);




    //pentru count
    msg *n = calloc(1, sizeof(msg));
    if(n == NULL){
        printf("[SENDER] Nu s-a alocat spatiu pentru mesajul 1\n");
        return -2;
    }

    framework count;

    //Completez campurile structurii
    memset(&count, 0, sizeof(framework));

    count.index = 1;
    count.crc_index = crc((void*)&count.index, sizeof(count.index));

    sprintf(count.payload, "%d", COUNT);
    count.crc_payload = crc((void*)count.payload, strlen(count.payload) + 1);

    count.length = strlen(count.payload) + 1;
    count.crc_length = crc((void*)&count.length, sizeof(count.length));

    memcpy(n->payload, &count, sizeof(framework));
    n->len = sizeof(framework);

    //Adaug in lista count-ul
    pushList(&l, (void*)n, count.index);

    //Il trimit
    send_message(n);



    int i;
    //Creez mesajele si le trimit
    for(i = 2; i < COUNT; i++) {
        //Daca s-a umplut fereastra incep si astept ACK-uri
        if(i >= window) {
            if(recv_message_timeout(&ack, timeout) >= 0) {
                framework *pack = (framework *)ack.payload;
                int index = pack->index;

                //Dupa ce am primit ACK sterg mesajul din lista
                removeList(&l, index);

            }else{
                printf("[SEND] TIMEOUT-ul a expirat\n");
            }
        }

        msg *data = calloc(1, sizeof(msg));
        if(data == NULL) {
            printf("[SENDER] Nu s-a alocat spatiu pentru mesajul %d\n", i);
            return -2;
        }

        framework frame;

        //Completez campurile structurii
        frame.length = read(fd, frame.payload, PAYLOAD_FRAME_SIZE);
        frame.crc_length = crc((void*)&frame.length, sizeof(frame.length));

        frame.crc_payload = crc((void*)frame.payload, frame.length);

        frame.index = i;
        frame.crc_index = crc((void*)&frame.index, sizeof(frame.index));

        memcpy(data->payload, &frame, sizeof(framework));  

        data->len = sizeof(framework);  

        //Adaug mesajul in lista
        pushList(&l, (void*)data, frame.index);

        //Indiferent ca primesc sau nu ACK trimit mai departe mesajul
        send_message(data);

    }

    //Folosesc un cursor pentru a ma plimba prin lista
    list *cursor = l;
    while(l) {
        if(recv_message_timeout(&ack, timeout) >= 0) {
            framework *pack = (framework *)ack.payload;
            int index = pack->index;

            //Daca primesc un ACK sterg mesajul corespunzator din lista
            removeList(&l, index);

        }else{
            printf("[SEND] TIMEOUT-ul a expirat\n");
        }

        if(l == NULL) break;

        //Resetez cursorul din lista
        if(cursor == NULL) 
            cursor = l;

        msg *next_message = cursor->info;

        //Trimit urmatorul mesaj care nu a fost trimis sau pentru care 
        //nu am primit ACK
        send_message(next_message);

        cursor = cursor->next;
    }

    //Astept semnalul de la recv pentru a stii ca s-a terminat
    recv_message(&ack);

    close(fd);

    return 0;
}
