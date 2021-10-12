#include <xc.h>
#include <libpic30.h>
#include "UART_Protocol.h"
#include "CB_TX1.h"
#include "CB_RX1.h"
#include "main.h"
#include "IO.h"

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
    //Fonction d'encodage et d'envoi d'un message
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
    __delay32(400000);
}

int msgDecodedFunction = 0;
int msgDecodedPayloadLength = 0;
unsigned char msgDecodedPayload[128];
int msgDecodedPayloadIndex = 0;

enum rcvState {
    Waiting,
    FunctionMSB,
    FunctionLSB,
    PayloadLengthMSB,
    PayloadLengthLSB,
    Payload,
    CheckSum
};

unsigned char rcvState = Waiting;
unsigned char calculatedChecksum = 0;

void UartDecodeMessage(unsigned char c) {
    //Fonction prenant en entrée un octet et servant à reconstituer les trames

    switch (rcvState) {
        case Waiting:
            if (c == 0xFE) {
                rcvState = FunctionMSB;
                msgDecodedFunction = 0;
            }
            break;

        case FunctionMSB:
            msgDecodedFunction = c << 8;
            rcvState = FunctionLSB;
            break;

        case FunctionLSB:
            msgDecodedFunction += c << 0;
            rcvState = PayloadLengthMSB;
            break;

        case PayloadLengthMSB:
            msgDecodedPayloadLength = c << 8;
            rcvState = PayloadLengthLSB;
            break;

        case PayloadLengthLSB:
            msgDecodedPayloadLength += c << 0;
            if (msgDecodedPayloadLength == 0) {
                rcvState = CheckSum;
            } else if (msgDecodedPayloadLength > 1024) {
                rcvState = Waiting;
            } else {
                rcvState = Payload;
                msgDecodedPayloadIndex = 0;
            }
            break;

        case Payload:
            msgDecodedPayload[msgDecodedPayloadIndex++] = c;
            if (msgDecodedPayloadIndex >= msgDecodedPayloadLength) {
                rcvState = CheckSum;
            }
            break;

        case CheckSum:
            calculatedChecksum = UartCalculateChecksum(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload);
            unsigned char receivedChecksum = c;
            if (calculatedChecksum == receivedChecksum) {
                UartProcessDecodedMessage(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload);
            } else {
                UartEncodeAndSendMessage(TEXTE, 6, "Erreur");
            }
            rcvState = Waiting;
            break;

        default:
            rcvState = Waiting;
            break;
    }
}

void UartProcessDecodedMessage(unsigned char function, unsigned char payloadLength, unsigned char* payload) {
    //Fonction appelée après le décodage pour exécuter l'action
    //Correspondant au message reçu
    int numLed, etatLed;

    switch (function) {
        case LED:
            numLed = payload[0];
            etatLed = payload[1];
            if (numLed == 1) {
                LED_ORANGE = 1;
            }
            if (numLed == 2) {
                LED_BLEUE = 1;
            }
            if (numLed == 3) {
                LED_BLANCHE = 1;
            }
            break;

        case TEXTE:
            UartEncodeAndSendMessage(TEXTE, payloadLength, payload);
            break;

        case SET_ROBOT_STATE:
            SetRobotState(payload[0]);
            break;

        case SET_ROBOT_MANUAL_CONTROL:
            SetRobotAutoControlState(payload[0]);
            break;

        default:
            break;
    }
}

//*************************************************************************/
//Fonctions correspondant aux messages
//*************************************************************************/