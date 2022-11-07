/*
Francesco Bucci
1871502
bucci.1871502@studenti.uniroma1.it
*/

#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#define MAX_LINE_SIZE 80
#define BUFF_SIZE 256
#define PORT_NUM 5000

void error(char* msg) {
    perror(msg);
    exit(1);
}

typedef struct {
    char* ID;
    char** neighbors;
    int dimNeighbors;
    char** weights;
    int dimWeights;
} nodo;

char** controlloInput(char* input);
char* leggiFile();
nodo* creaGrafo(char* buf, int* dimGrafo);
void stampaGrafo(nodo* grafo, int dimGrafo);
char* percorsoMinimo(nodo* grafo, int dimGrafo, char* sorgente, char* destinazione);

int main() {
    int sockfd, newsockfd, portno, clilen;
    char buffer[BUFF_SIZE];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    bzero((char*)&serv_addr, sizeof(serv_addr));
    portno = PORT_NUM;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr*)&serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    printf("\nIn attesa di connessione del client...\n");
    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    while (1) {
        printf("Aspetto il messaggio del client\n");
        bzero(buffer, BUFF_SIZE);
        n = read(newsockfd, buffer, BUFF_SIZE - 1);
        if (n < 0) error("ERROR reading from socket");

        printf("Here is the message: %s\n", buffer);

        // controllo input
        char* sorgente = NULL;
        char* destinazione = NULL;
        char** inputCheck = controlloInput(buffer);
        sorgente = inputCheck[0];
        destinazione = inputCheck[1];
        if (strcmp(inputCheck[0], "-1") == 0) {
            close(newsockfd);
            return 0;
        } else if (strcmp(inputCheck[0], "-2") == 0) {
            n = write(newsockfd, "ERR - Formato non corretto\n", strlen("ERR - Formato non corretto\n"));
            if (n < 0) error("ERROR writing to socket");
            close(newsockfd);
            return 0;
        }

        // leggi file
        char* buf = leggiFile();

        // creazione grafo come array
        int dimGrafo = 0;
        nodo* grafo = creaGrafo(buf, &dimGrafo);

        // Ricerca percorso minimo
        char* ris = percorsoMinimo(grafo, dimGrafo, sorgente, destinazione);

        n = write(newsockfd, ris, strlen(ris));
        if (n < 0) error("ERROR writing to socket");
    }
    return 0;
}

char** controlloInput(char* input) {
    printf("\nControllo correttezza input...\n");

    char** v = malloc(sizeof(char*) * 2);
    bool flag = false;
    int j = 0, i;
    char* n1 = NULL;
    char* n2 = NULL;

    if (strncmp("min:", input, 4) == 0) {
        for (i = 0; i < strlen(input); i++) {
            if (input[i] == ',') {
                flag = true;
                n1[j] = '\0';
                j = 0;
            }
            if (!flag) {
                if (input[i] >= '0' && input[i] <= '9') {
                    n1 = (char*)realloc(n1, j++ * sizeof(char));
                    n1[j - 1] = input[i];
                }
            } else {
                if (input[i] >= '0' && input[i] <= '9') {
                    n2 = (char*)realloc(n2, j++ * sizeof(char));
                    n2[j - 1] = input[i];
                }
            }
        }
        n2[j] = '\0';

        printf("-Ricerca del percorso minimo tra %s e %s\n", n1, n2);
        v[0] = n1;
        v[1] = n2;
        return v;
    } else {
        if (strncmp("exit", input, 4) == 0) {
            printf("Ricevuto exit...\n");
            v[0] = "-1";
            return v;
        } else {
            v[0] = "-2";
            return v;
        }
    }
}

char* leggiFile() {
    printf("\nLettura del grafo nel file...\n");
    char* buf = calloc(10000, sizeof(char));
    char* tmp = malloc(sizeof(char) * MAX_LINE_SIZE);
    FILE* fp = fopen("rete.txt", "r");
    if (fp != NULL) {
        while (fscanf(fp, "%s", tmp) > 0) {
            sprintf(buf, "%s %s", buf, tmp);
        }
        fclose(fp);
    } else {
        printf("Errore nell'apertura del file...\n");
        printf("Interruzione programma...\n");
        return 0;
    }

    return buf;
}

nodo* creaGrafo(char* buf, int* dimGrafo) {
    printf("\nCreazione grafo...\n");

    nodo* grafo = NULL;
    bool flag = false;
    char* n1 = NULL;
    char* n2 = NULL;
    int j = 0, i;
    // cerco nel buffer i valori dei nodi
    for (i = 1; i <= strlen(buf); i++) {
        if (isdigit(buf[i])) {
            j += 1;
            n1 = (char*)realloc(n1, sizeof(char) * j);
            n1[j - 1] = buf[i];
        }

        if (buf[i] == ':') {
            n1[j] = '\0';
            j = 0;

            *dimGrafo += 1;
            grafo = (nodo*)realloc(grafo, sizeof(nodo) * *dimGrafo);

            grafo[*dimGrafo - 1].ID = malloc(sizeof(char) * strlen(n1));
            strcpy(grafo[*dimGrafo - 1].ID, n1);
            grafo[*dimGrafo - 1].dimNeighbors = 0;
            grafo[*dimGrafo - 1].dimWeights = 0;
            grafo[*dimGrafo - 1].neighbors = NULL;
            grafo[*dimGrafo - 1].weights = NULL;
            printf("-Creato un nodo! %s\n", n1);
            n1 = NULL;
        }
        if (buf[i] == ',') {
            n1[j] = '\0';
            j = 0;

            grafo[*dimGrafo - 1].dimNeighbors += 1;
            int tmp = grafo[*dimGrafo - 1].dimNeighbors - 1;
            grafo[*dimGrafo - 1].neighbors = realloc(grafo[*dimGrafo - 1].neighbors, (tmp + 1) * sizeof(char*));
            grafo[*dimGrafo - 1].neighbors[tmp] = malloc(strlen(n1) + 1);
            strcpy(grafo[*dimGrafo - 1].neighbors[tmp], n1);

            n1 = NULL;
        }
        if (buf[i] == ' ' || buf[i] == '\0') {
            n1[j] = '\0';
            j = 0;

            grafo[*dimGrafo - 1].dimWeights += 1;
            int tmp = grafo[*dimGrafo - 1].dimWeights - 1;
            grafo[*dimGrafo - 1].weights = realloc(grafo[*dimGrafo - 1].weights, (tmp + 1) * sizeof(char*));
            grafo[*dimGrafo - 1].weights[tmp] = malloc(strlen(n1) + 1);
            strcpy(grafo[*dimGrafo - 1].weights[tmp], n1);

            n1 = NULL;

            printf("--Trovato il vicino %s ", grafo[*dimGrafo - 1].neighbors[grafo[*dimGrafo - 1].dimNeighbors - 1]);
            printf("con peso %s\n", grafo[*dimGrafo - 1].weights[grafo[*dimGrafo - 1].dimWeights - 1]);
        }
    }
    printf("\n");
    return grafo;
}

int cercaNodo(char* src, nodo* grafo, int dimGrafo) {
    int i = 0;
    for (i = 0; i < dimGrafo; i++) {
        if (strcmp(src, grafo[i].ID) == 0) return i;
    }
    return -1;
}

bool isVuoto(int* v, int dimGrafo) {
    int i = 0;
    for (i = 0; i < dimGrafo; i++) {
        if (-1 != v[i]) return false;
    }
    return true;
}

int minimo(int* dist, int dimGrafo, int* Q) {
    int i = 0, min = 20000, index = 0;
    for (i = 1; i < dimGrafo; i++) {
        if (Q[i] != -1) {
            if (dist[i] < min) {
                min = dist[i];
                index = i;
            }
        }
    }
    return index;
}

char* stringaCompleta(int* v, int dimV, char* sorgente, char* destinazione, int peso) {
    char* buff = calloc(1000, sizeof(char));
    strcat(buff, "\nIl percorso a costo minimo tra ");
    strcat(buff, sorgente);
    strcat(buff, " e ");
    strcat(buff, destinazione);
    strcat(buff, " e' [");
    strcat(buff, sorgente);
    strcat(buff, ",");
    int i;
    for (i = 0; i < dimV; i++) {
        char tmp[6];
        sprintf(tmp, "%i", v[i]);
        strcat(buff, tmp);
        strcat(buff, ",");
    }
    strcat(buff, destinazione);
    strcat(buff, "] con costo ");
    char tmp[6];
    sprintf(tmp, "%d", peso);
    strcat(buff, tmp);
    strcat(buff, ".\n");

    return buff;
}

char* percorsoMinimo(nodo* grafo, int dimGrafo, char* sorgente, char* destinazione) {
    char* buf = calloc(10000, sizeof(char));
    if (cercaNodo(sorgente, grafo, dimGrafo) == -1 || cercaNodo(destinazione, grafo, dimGrafo) == -1) {
        strcat(buf, "\nUno o entrambi i nodi non esistono\n");
        return buf;
    }

    int MAX_INT = 20000000;
    int dist[dimGrafo], precedente[dimGrafo], Q[dimGrafo], i;
    for (i = 0; i < dimGrafo; i++) {
        dist[i] = MAX_INT;
        precedente[i] = MAX_INT;
        Q[i] = atoi(grafo[i].ID);
    }
    dist[cercaNodo(sorgente, grafo, dimGrafo)] = 0;
    while (!isVuoto(Q, dimGrafo)) {
        int u = minimo(dist, dimGrafo, Q);  // indice nodo distanza minima
        Q[u] = -1;
        if (u == cercaNodo(destinazione, grafo, dimGrafo)) break;
        if (dist[u] == MAX_INT) break;

        int j = 0, y = 0;
        for (j = 0; j < grafo[u].dimNeighbors; j++) {                   // cerca tra i vicini
            int v = cercaNodo(grafo[u].neighbors[j], grafo, dimGrafo);  // indice vicino nel grafo
            if (v == -1) printf("ERRORE\n");
            int alt = dist[u] + atoi(grafo[u].weights[j]);  // distanza tra sorgente e v passando per u
            if (alt < dist[v]) {
                dist[v] = alt;
                precedente[v] = atoi(grafo[u].ID);
                Q[v] = atoi(grafo[v].ID);
            }
        }
    }
    printf("\n");

    int peso = dist[cercaNodo(destinazione, grafo, dimGrafo)];

    int* s = NULL;
    int u = cercaNodo(destinazione, grafo, dimGrafo), dimS = 0;
    int j = 0;
    while (j < dimGrafo) {
        if (precedente[j] != MAX_INT && precedente[j] != atoi(sorgente) && precedente[j] != atoi(destinazione)) {
            int y = 0;
            for (y = 0; y < dimS; y++) {
                if (precedente[j] == s[y]) break;
            }
            if (y == dimS) {
                dimS++;
                s = (int*)realloc(s, dimS * sizeof(int));
                s[dimS - 1] = precedente[j];
                u = precedente[u];
            }
        }
        j++;
    }

    strcpy(buf, stringaCompleta(s, dimS, sorgente, destinazione, peso));
    return buf;
}
