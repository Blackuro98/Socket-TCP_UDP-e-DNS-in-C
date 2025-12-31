#define _WINSOCK_DEPRECATED_NO_WARNINGS

#if defined _WIN32
#include <winsock2.h>
#else
#define closesocket close
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUFFERSIZE 512
#define PROTOPORT 27015 
#define QLEN 6          

void ErrorHandler(char *errorMessage) {
    printf("%s\n", errorMessage);
}

void ClearWinSock() {
#if defined _WIN32
    WSACleanup();
#endif
}

// Funzione logica per rimuovere le vocali
void rimuoviVocali(char *str) {
    int i = 0, j = 0;
    while (str[i] != '\0') {
        char c = tolower(str[i]);
        if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u') {
            str[j++] = str[i];
        }
        i++;
    }
    str[j] = '\0';
}

int main(void) {
#if defined _WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2 ,2), &wsaData);
    if (iResult != 0) {
        printf ("error at WSAStartup\n");
        return -1;
    }
#endif

    int serverSocket;
    int clientSocket;
    struct sockaddr_in sad; // Indirizzo Server
    struct sockaddr_in cad; // Indirizzo Client
    int alen;               // Lunghezza indirizzo client
    char buf[BUFFERSIZE];   // Buffer dati

    // CREAZIONE SOCKET
    serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket < 0) {
        ErrorHandler("socket creation failed.\n");
        ClearWinSock();
        return -1;
    }

    // CONFIGURAZIONE INDIRIZZO SERVER
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = INADDR_ANY; // Ascolta su tutte le interfacce
    sad.sin_port = htons(PROTOPORT);

    // BIND
    if (bind(serverSocket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        ErrorHandler("bind() failed.\n");
        closesocket(serverSocket);
        ClearWinSock();
        return -1;
    }

    // LISTEN
    if (listen(serverSocket, QLEN) < 0) {
        ErrorHandler("listen() failed.\n");
        closesocket(serverSocket);
        ClearWinSock();
        return -1;
    }

    printf("Server TCP Style in ascolto sulla porta %d...\n", PROTOPORT);

    // LOOP PRINCIPALE
    while (1) {
        alen = sizeof(cad);
        // ACCEPT
        clientSocket = accept(serverSocket, (struct sockaddr *)&cad, &alen);
        if (clientSocket < 0) {
            ErrorHandler("accept() failed.\n");
            continue; // Torna ad ascoltare
        }

        // 1. RICEZIONE "HELLO"
        memset(buf, 0, BUFFERSIZE);
        if (recv(clientSocket, buf, BUFFERSIZE, 0) <= 0) {
            ErrorHandler("recv() failed or connection closed prematurely");
            closesocket(clientSocket);
            continue;
        }

        // 2. STAMPA INFO CLIENT
        printf("\n--- NUOVO CLIENT ---\n");
        printf("Messaggio Iniziale: %s\n", buf);
        printf("Indirizzo IP Client : %s\n", inet_ntoa(cad.sin_addr));

        // 3. RICEZIONE STRINGA DA ELABORARE
        memset(buf, 0, BUFFERSIZE);
        if (recv(clientSocket, buf, BUFFERSIZE, 0) <= 0) {
            ErrorHandler("recv() failed reading string");
            closesocket(clientSocket);
            continue;
        }

        printf("Stringa ricevuta    : %s\n", buf);

        // 4. RIMOZIONE VOCALI
        rimuoviVocali(buf);

        // 5. INVIO RISPOSTA
        int stringLen = strlen(buf);
        if (send(clientSocket, buf, stringLen, 0) != stringLen) {
            ErrorHandler("send() sent a different number of bytes than expected");
        }

        printf("Risposta inviata    : %s\n", buf);
        printf("Chiusura connessione con questo client.\n");
        
        closesocket(clientSocket);
    }
}