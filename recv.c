#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"

#include "struct.h"


#define HOST "127.0.0.1"
#define PORT 10001


int main(int argc,char** argv){
  msg r, ack;
  init(HOST,PORT);

  char file_name[25] = "recv_";
  int COUNT = 0;
  int i;

  unsigned int loop = 0xffffffff;

  //Aloc dinamic vector pentru retinerea mesajelor primite
  //Util pentru reordonare
  msg *vec_messages = (msg*)calloc(10000, sizeof(msg));

  //Aloc un vector in care retin ce mesaje am primit, pe indexul corespunzator
  int *received = calloc(1005, sizeof(int));

  //In total_msg retin numarul curent de mesaje primite
  int total_msg = 0;




  while(total_msg < loop) {

    //Astept mesaje de la sender
    int ok = recv_message(&r);
    //Daca nu primesc niciun mesaj, mai astept
    if(ok < 0){
      printf("[RECV] Eroare la primire mesaj\n"); 
      continue;
    }

    //Daca am primit un mesaj
    framework *frame = (framework*)r.payload;
    int index = frame->index;

    //Il verific daca e corupt si daca da, astept altul
    if(!check_crc(frame)) {
      printf("[RECV] Mesajul e corupt\n"); 
      continue;
    }

    //Salvez mesajul primit in vector daca nu exista deja
    if(received[index] == 0) {
      memcpy((vec_messages + index), &r, sizeof(msg));

      //Il marchez ca primit
      received[index] = 1;

      total_msg++;

      //Trimit ACK pentru mesajul primit
      framework frame_ack;
      frame_ack.index = index;
      frame_ack.length = sizeof(framework);
      memcpy(ack.payload, &frame_ack, sizeof(framework));

      send_message(&ack);

    }else{
      printf("[RECV] Am deja mesajul %d, il arunc pe jos\n", index);
    }

    //Preiau numele fisierului si numarul de mesaje daca au ajuns
    if(index == 0) {
      strcat(file_name, frame->payload);
    }

    if(index == 1) {
      COUNT = atoi(frame->payload); 
      loop = COUNT;
    }

  }


  //deschid fisierul
  int fd = open(file_name, O_CREAT | O_WRONLY, 0600);
  if(fd < 0){
    printf("[RECV] Nu a reusit deschiderea fisieruli\n");
    free(vec_messages);
    free(received);
    return -5;
  }

  //Scriu mesajele in fisier
  for(i = 2; i < COUNT; i++){
    write(fd, 
          ((framework *)((vec_messages + i)->payload))->payload,
          ((framework *)((vec_messages + i)->payload))->length );
  }

  free(received);
  free(vec_messages);

  close(fd);


  //mesaj catre sender ca am primit toate mesajele
  ((framework*)ack.payload)->length = -8;
  send_message(&ack);

  return 0;
}
