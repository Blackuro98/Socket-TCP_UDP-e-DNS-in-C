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
#include <string.h>

#define ECHOMAX 255

void ErrorHandler(char *errorMessage) {
    printf("%s\n", errorMessage);
}

void ClearWinSock() {
#if defined WIN32
    WSACleanup();
#endif
}

int main() {
#if defined WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2 ,2), &wsaData);
    if (iResult != 0) {
        printf ("error at WSAStartup\n");
        return EXIT_FAILURE;
    }
#endif

    int sock;
    struct sockaddr_in echoServAddr;
    struct sockaddr_in fromAddr;
    int fromSize;
    char echoString[ECHOMAX]; // Buffer per la stringa input
    char echoBuffer[ECHOMAX]; // Buffer per la risposta
    char hostname[100];
    int port_input;
    struct hostent *server_info;   // Per DNS diretto
    struct hostent *sender_info;   // Per DNS inverso (risposta)

    // --- INPUT UTENTE (Punto 1 esercizio) ---
    printf("Inserisci nome host del server (es. localhost): ");
    scanf("%s", hostname);
    printf("Inserisci la porta del server (es. 9999): ");
    scanf("%d", &port_input);
    getchar(); // Consuma il "newline" rimasto nel buffer

    // RISOLUZIONE NOME HOST (DNS DIRETTO)
    server_info = gethostbyname(hostname);
    if (server_info == NULL) {
        ErrorHandler("Host non trovato");
        return EXIT_FAILURE;
    }

    // CREAZIONE DELLA SOCKET
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        ErrorHandler("socket() failed");

    // COSTRUZIONE DELL'INDIRIZZO DEL SERVER
    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_port = htons(port_input);
    // Copia l'indirizzo ottenuto dal DNS nella struttura
    memcpy(&echoServAddr.sin_addr, server_info->h_addr, server_info->h_length);

    // --- FASE 1: INVIO "Hello" ---
    char helloMsg[] = "Hello";
    if (sendto(sock, helloMsg, strlen(helloMsg), 0, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr)) != strlen(helloMsg))
        ErrorHandler("sendto() failed for Hello");

    // --- FASE 2: INPUT STRINGA ---
    printf("Inserisci la stringa da inviare: ");
    fgets(echoString, ECHOMAX, stdin);
    
    // Rimuove eventuale newline finale da fgets
    size_t len = strlen(echoString);
    if (len > 0 && echoString[len-1] == '\n') echoString[len-1] = '\0';

    // INVIO DELLA STRINGA
    if (sendto(sock, echoString, strlen(echoString), 0, (struct sockaddr*)&echoServAddr, sizeof(echoServAddr)) != strlen(echoString))
        ErrorHandler("sendto() sent different number of bytes than expected");

    // --- FASE 3: RICEZIONE RISPOSTA ---
    fromSize = sizeof(fromAddr);
    int respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr*)&fromAddr, &fromSize);

    if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
        fprintf(stderr, "Error: received a packet from unknown source.\n");
        exit(EXIT_FAILURE);
    }

    echoBuffer[respStringLen] = '\0'; // Terminatore stringa

    // --- FASE 4: INFO SERVER (DNS INVERSO) ---
    sender_info = gethostbyaddr((const char*)&fromAddr.sin_addr, sizeof(fromAddr.sin_addr), AF_INET);
    char *srv_name = (sender_info != NULL) ? sender_info->h_name : "Sconosciuto";

    printf("\nStringa '%s' ricevuta dal server\n", echoBuffer);
    printf("Nome server     : %s\n", srv_name);
    printf("Indirizzo server: %s\n", inet_ntoa(fromAddr.sin_addr));

    closesocket(sock);
    ClearWinSock();
    system("pause");
    return EXIT_SUCCESS;
}