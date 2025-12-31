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
#include <ctype.h>

#define ECHOMAX 255
#define PORT 9999 

// Funzione helper per errori
void ErrorHandler(char *errorMessage) {
    printf("%s\n", errorMessage);
}

// Funzione helper per cleanup
void ClearWinSock() {
#if defined WIN32
    WSACleanup();
#endif
}

// Funzione logica per l'esercizio (Rimozione Vocali)
void rimuoviVocali(char *str) {
    int i = 0, j = 0;
    while (str[i] != '\0') {
        char c = tolower(str[i]);
        // Se NON è una vocale e NON è un invio, la teniamo
        if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u' && c != '\n') {
            str[j++] = str[i];
        }
        i++;
    }
    str[j] = '\0';
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
    struct sockaddr_in echoClntAddr;
    int cliAddrLen;
    char echoBuffer[ECHOMAX];
    int recvMsgSize;
    struct hostent *client_info;

    // CREAZIONE DELLA SOCKET
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        ErrorHandler("socket() failed");

    // COSTRUZIONE DELL'INDIRIZZO DEL SERVER
    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_port = htons(PORT);
    echoServAddr.sin_addr.s_addr = INADDR_ANY;

    // BIND DELLA SOCKET
    if ((bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr))) < 0)
        ErrorHandler("bind() failed");

    printf("SERVER UDP avviato sulla porta %d. In attesa...\n", PORT);

    while(1) {
        cliAddrLen = sizeof(echoClntAddr);

        // --- 1. RICEZIONE "HELLO" ---
        memset(echoBuffer, 0, ECHOMAX);
        recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr*)&echoClntAddr, &cliAddrLen);
        
        // Identificazione Client (DNS Inverso)
        client_info = gethostbyaddr((const char*)&echoClntAddr.sin_addr, sizeof(echoClntAddr.sin_addr), AF_INET);
        char *client_name = (client_info != NULL) ? client_info->h_name : "Sconosciuto";

        printf("\n-------------------------------------------------\n");
        printf("Ricevuti dati dal client:\n");
        printf("Nome     : %s\n", client_name);
        printf("Indirizzo: %s\n", inet_ntoa(echoClntAddr.sin_addr));
        printf("Messaggio: %s\n", echoBuffer);
        printf("-------------------------------------------------\n");

        // --- 2. RICEZIONE STRINGA DA ELABORARE ---
        memset(echoBuffer, 0, ECHOMAX);
        recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr*)&echoClntAddr, &cliAddrLen);
        
        // MODIFICA: Stampa chiara della stringa ricevuta
        printf("Stringa ricevuta originale: %s\n", echoBuffer);

        // --- 3. ELABORAZIONE (NO VOCALI) ---
        rimuoviVocali(echoBuffer);

        // --- 4. INVIO RISPOSTA ---
        if (sendto(sock, echoBuffer, strlen(echoBuffer), 0, (struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr)) != strlen(echoBuffer))
            ErrorHandler("sendto() sent different number of bytes than expected");
        
        // MODIFICA: Stampa chiara della stringa elaborata
        printf("Stringa elaborata (%s) inviata al client.\n", echoBuffer);
    }
}