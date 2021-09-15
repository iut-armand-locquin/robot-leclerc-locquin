#include <xc.h>
#include <libpic30.h>
#include "UART_Protocol.h"
#include "CB_TX1.h"
#include "CB_RX1.h"

unsigned char UartCalculateChecksum(int msgFunction, int msgPayloadLength, unsigned char* msgPayload) {
    //Fonction prenant entrée la trame et sa longueur pour calculer le checksum
    unsigned char checksum = 0xFE;
    checksum ^= msgFunction >> 8;
    checksum ^= msgFunction >> 0;
    checksum ^= msgPayloadLength >> 8;
    checksum ^= msgPayloadLength >> 0;
    int i;
    for (i = 0; i < msgPayloadLength; i++) {
        checksum ^= msgPayload[i];
    }
    return checksum;
}

void UartEncodeAndSendMessage(int msgFunction, int msgPayloadLength, unsigned char* msgPayload) {
    //Fonction d?encodage et d?envoi d?un message
    unsigned char checksum = UartCalculateChecksum(msgFunction, msgPayloadLength, msgPayload);
    unsigned char msg[6 + msgPayloadLength];
    int pos = 0;
    msg[pos++] = 0xFE;
    msg[pos++] = msgFunction >> 8;
    msg[pos++] = msgFunction >> 0;
    msg[pos++] = msgPayloadLength >> 8;
    msg[pos++] = msgPayloadLength >> 0;
    int i;
    for (i = 0; i < msgPayloadLength; i++) {
        msg[pos++] = msgPayload[i];
    }
    msg[pos++] = checksum;
    SendMessage(msg, 6 + msgPayloadLength);
    __delay32(4000000);
}

int msgDecodedFunction = 0;
int msgDecodedPayloadLength = 0;
unsigned char msgDecodedPayload[128];
int msgDecodedPayloadIndex = 0;

void UartDecodeMessage(unsigned char c) {
    //Fonction prenant en entrée un octet et servant à reconstituer les trames

}

void UartProcessDecodedMessage(unsigned char function, unsigned char payloadLength, unsigned char* payload) {
    //Fonction appelée après le décodage pour exécuter l?action
    //correspondant au message reçu

}

//*************************************************************************/
//Fonctions correspondant aux messages
//*************************************************************************/