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

#define BUFFERSIZE 512
#define PROTOPORT 27015

void ErrorHandler(char *errorMessage) {
    printf("%s\n", errorMessage);
}

void ClearWinSock() {
#if defined _WIN32
    WSACleanup();
#endif
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

    // CREAZIONE DELLA SOCKET
    int Csocket;
    Csocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Csocket < 0) {
        ErrorHandler("socket creation failed.\n");
        ClearWinSock();
        return -1;
    }

    // COSTRUZIONE DELL'INDIRIZZO DEL SERVER
    struct sockaddr_in sad;
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = inet_addr("127.0.0.1"); // IP localhost
    sad.sin_port = htons(PROTOPORT);

    // CONNESSIONE AL SERVER
    if (connect(Csocket, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
        ErrorHandler("Failed to connect.\n");
        closesocket(Csocket);
        ClearWinSock();
        return -1;
    }

    // 1. INVIA MESSAGGIO INIZIALE "Hello"
    char *helloMsg = "Hello";
    int helloLen = strlen(helloMsg);
    
    if (send(Csocket, helloMsg, helloLen, 0) != helloLen) {
        ErrorHandler("send() sent a different number of bytes than expected");
        closesocket(Csocket);
        ClearWinSock();
        return -1;
    }
    printf("Messaggio 'Hello' inviato al server.\n");

    // 2. LETTURA STRINGA DA TASTIERA
    char inputString[BUFFERSIZE];
    printf("Inserisci la stringa da inviare: ");
    fgets(inputString, BUFFERSIZE, stdin);

    // Rimuove eventuale "a capo" finale
    size_t len = strlen(inputString);
    if (len > 0 && inputString[len-1] == '\n') {
        inputString[len-1] = '\0';
    }

    int stringLen = strlen(inputString);

    // 3. INVIARE DATI AL SERVER
    if (send(Csocket, inputString, stringLen, 0) != stringLen) {
        ErrorHandler("send() failed");
        closesocket(Csocket);
        ClearWinSock();
        return -1;
    }

    // 4. RICEVERE RISPOSTA
    char buf[BUFFERSIZE];
    int bytesRcvd;
    
    printf("In attesa di risposta...\n");
    
    if ((bytesRcvd = recv(Csocket, buf, BUFFERSIZE - 1, 0)) <= 0) {
        ErrorHandler("recv() failed or connection closed prematurely");
        closesocket(Csocket);
        ClearWinSock();
        return -1;
    }

    buf[bytesRcvd] = '\0'; // Terminatore stringa
    printf("Risposta dal server (No Vocali): %s\n", buf);

    // CHIUSURA
    closesocket(Csocket);
    ClearWinSock();
    printf("\n");
    system("pause");
    return 0;
}