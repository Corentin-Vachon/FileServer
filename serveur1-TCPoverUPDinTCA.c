#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>

#define RCVSIZE 1494
#define sizeWindow 35
#define sizeACK 6

int main (int argc, char *argv[]) {

    char bufferOfFileBuffer[sizeWindow][RCVSIZE + 6];
    int seg_counter = 1;
    char toSend[RCVSIZE + 6];
    size_t result;
    int indexOFBufferOfFileBuffer = 0;
    char string[7];
    char string2[8];
    char justeLeNombre[7];
    char buff[64];
    int windowCounter = sizeWindow;
    int lastOne = 0;
    int ack;

    int data_desc;
    int random_number;

    int tmp;
    int a;
    int msgSizeLink;

    int portPublic = atoi(argv[1]); // ce port correspond au port de control

    struct sockaddr_in adresse, client;
    socklen_t alen = sizeof(client);
    int valid = 1;
    int sndbuf = 900000;

    char fileBuffer[RCVSIZE]; // buffer correspondant au transfert de fichier
    char nameBuffer[RCVSIZE]; // buffer correspondant au nom du fichier
    char controlBuffer[RCVSIZE]; // buffer utilisé par le port de control

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;

    int control_desc = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(control_desc, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

    client.sin_family = AF_INET;
    client.sin_port = htons(portPublic);
    client.sin_addr.s_addr = INADDR_ANY;

    if (control_desc < 0) {
        perror("Cannot create socket\n");
        return -1;
    }

    if (bind(control_desc, (struct sockaddr *) &client, sizeof(client)) == -1) {
        perror("Bind failed\n");
        close(control_desc);
        return -1;
    }

    int n;

    int bytes_sent;
    char buffer[RCVSIZE];
    char synAckBuff[64];
    char synAckBuff2[7] = "SYN-ACK";

    while (1) {

        random_number = rand() % 4444 + 3333;

        data_desc = socket(AF_INET, SOCK_DGRAM, 0);
        if (setsockopt(data_desc, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout))){
            perror("Error");
        };
        // setsockopt(data_desc, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

        if (data_desc < 0) {
            perror("Cannot create socket\n");
            return -1;
        }

        adresse.sin_family = AF_INET;
        adresse.sin_addr.s_addr = INADDR_ANY;
        adresse.sin_port = htons(random_number);

        printf("Commencement de la procédure Hand Shake avec comme nouveau port : %i \n", random_number);

        snprintf(string2, 6, "%04d", random_number);
        memcpy(synAckBuff, synAckBuff2, 7);
        memcpy(synAckBuff + 7, string2, 7);

        int msgSizeHandShake = recvfrom(control_desc, buffer, RCVSIZE, 0, (struct sockaddr *) &client,&alen); // reçoit le nom du fichier que le client souhaite télécharger
        int accepted = 1;

        while (msgSizeHandShake > 0 && accepted) {
            if (strcmp(buffer, "SYN") == 0) {
                printf("--> Get SYN\n");
                accepted = 0;
            }
        }

        printf("Envoie de %s : \n", synAckBuff);

        bytes_sent = sendto(control_desc, synAckBuff, RCVSIZE, 0, (struct sockaddr *) &client, alen);

        msgSizeHandShake = recvfrom(control_desc, buffer, RCVSIZE, 0, (struct sockaddr *) &client,&alen); // reçoit le nom du fichier que le client souhaite télécharger
        accepted = 1;

        // send the first ACK
        while (msgSizeHandShake > 0 && accepted) {
            if (strcmp(buffer, "ACK") == 0) {
                printf("--> Get ACK\n");
                accepted = 0;
            }
        }

        if (bind(data_desc, (struct sockaddr *) &adresse, sizeof(adresse)) == -1) {
            perror("Bind faileddd\n");
            close(data_desc);
            return -1;
        }

        time_t start = time(NULL);

        printf("Communication ... \n");

        n = fork();

        if (n == 0){

            close (control_desc);

            FILE *file;

            msgSizeLink = recvfrom(data_desc, nameBuffer, RCVSIZE, 0, (struct sockaddr *) &adresse,&alen);

            if ((file = fopen(nameBuffer, "rb")) == NULL) {   // si le nom du fichier n'éxiste pas, le serveur se ferme
                printf("Mauvaus nom de fichier : %s\n", nameBuffer);
                exit(-1);
            } else {
                printf(" Bon fichier  : %s\n", nameBuffer);
            }

            a = 0;
            tmp = 0;
            while ((result = fread(fileBuffer, 1, RCVSIZE, file)) > 0) {

                //printf("On gagne : %i nouveau paquet in TCPoverUDPinTCA", windowCounter);
                if (windowCounter != 0)
                {
                printf("window : %i\n", windowCounter);
                snprintf(string, 7, "%06d", seg_counter);
                memcpy(toSend, string, 6);
                memcpy(toSend + 6, fileBuffer, result);
                sendto(data_desc, toSend, result+6, 0,(struct sockaddr*)&adresse, sizeof(adresse));
                memcpy(bufferOfFileBuffer[indexOFBufferOfFileBuffer], toSend, result+6);
                windowCounter -= 1;
                }
                if (windowCounter == 0) {
                    while (windowCounter <sizeWindow -1) {
                    ack = recvfrom(data_desc, buff, RCVSIZE, 0, (struct sockaddr*)&client, &alen);
                    memcpy(justeLeNombre, buff + 3, 7);
                    if (atoi(justeLeNombre) > lastOne)
                    {
                        windowCounter += (atoi(justeLeNombre) - lastOne);
                        lastOne = atoi(justeLeNombre);
                    }
                    sendto(data_desc, bufferOfFileBuffer[(lastOne + sizeWindow)%sizeWindow], result + 6 , 0,(struct sockaddr*)&adresse, sizeof(adresse));
                    }
                }

                // ack = recvfrom(data_desc, buff, RCVSIZE, 0, (struct sockaddr*)&client, &alen);
                // memcpy(bufferOfFileBuffer[indexOFBufferOfFileBuffer], toSend, result+6);

                // if (windowCounter > 0 ){
                //     sendto(data_desc, toSend, result+6, 0,(struct sockaddr*)&adresse, sizeof(adresse));
                //     sendto(data_desc, bufferOfFileBuffer[(lastOne + sizeWindow)%sizeWindow], result + 6 , 0,(struct sockaddr*)&adresse,sizeof(adresse));
                //     windowCounter = windowCounter - 1;
                // }

                // if (windowCounter == 0 ){
                //     sendto(data_desc, bufferOfFileBuffer[(lastOne + sizeWindow)%sizeWindow], result + 6 , 0,(struct sockaddr*)&adresse,sizeof(adresse));
                //     ack = recvfrom(data_desc, buff, RCVSIZE, 0, (struct sockaddr*)&client, &alen);
                //     memcpy(justeLeNombre, buff + 3, 7);
                //     a = atoi(justeLeNombre);
                //     if (a > lastOne)
                //     {
                //         windowCounter += (a - lastOne) -1;
                //         lastOne = a;

                //     }
                //     while (lastOne < seg_counter - sizeWindow + 1) {
                //         sendto(data_desc, bufferOfFileBuffer[(lastOne + sizeWindow)%sizeWindow], result + 6 , 0,(struct sockaddr*)&adresse, sizeof(adresse));
                //         ack = recvfrom(data_desc, buff, RCVSIZE, 0, (struct sockaddr*)&client, &alen);
                //         memcpy(justeLeNombre, buff + 3, 7);
                //         tmp = (atoi(justeLeNombre) + 1 + sizeWindow) % sizeWindow;
                //         if (atoi(justeLeNombre) > lastOne)
                //         {
                //             windowCounter += (atoi(justeLeNombre) - lastOne) - 1;
                //             lastOne = atoi(justeLeNombre);
                //         }
                //     }
                // }

                seg_counter += 1;
                indexOFBufferOfFileBuffer = ((indexOFBufferOfFileBuffer + 1 + sizeWindow) % sizeWindow);

            }
            while (lastOne != seg_counter - 1)
            {
                sendto(data_desc, bufferOfFileBuffer[(lastOne + sizeWindow)%sizeWindow],  sizeof(bufferOfFileBuffer[(lastOne + sizeWindow)%sizeWindow]) , 0,(struct sockaddr*)&adresse, sizeof(adresse));
                ack = recvfrom(data_desc, buff, RCVSIZE, 0, (struct sockaddr*)&client, &alen);
                memcpy(justeLeNombre, buff + 3, 7);
                if (atoi(justeLeNombre) > lastOne)
                {
                    windowCounter += (atoi(justeLeNombre) - lastOne) - 1;
                    lastOne = atoi(justeLeNombre);
                }
            }
            sendto(data_desc, "FIN", sizeof("FIN"), 0,(struct sockaddr*)&adresse, sizeof(adresse));
            // total_t = (float)start_t/ CLOCKS_PER_SEC;

            printf("Fin du transfert de : %s\nIl y a eu %i segments envoyés\n pour %i o", nameBuffer, seg_counter, seg_counter*RCVSIZE+RCVSIZE/2);

            fclose(file);
            exit(0);
        }
        else if (n > 0) {
            close (data_desc);
            data_desc = socket(AF_INET, SOCK_DGRAM, 0);
        }
    }

}
