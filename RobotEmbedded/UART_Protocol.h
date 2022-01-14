#ifndef UART_PROTOCOL_H
#define	UART_PROTOCOL_H

#define LED 0x0020
#define SET_ROBOT_STATE 0x0051
#define SET_ROBOT_MANUAL_CONTROL 0x0052
#define POSITION_DATA 0x0061
#define FONCTION_ASSERVISSEMENT 0x0070
#define TEXTE 0x0080

unsigned char UartCalculateChecksum(int msgFunction, int msgPayloadLength, unsigned char* msgPayload);
void UartEncodeAndSendMessage(int msgFunction, int msgPayloadLength, unsigned char* msgPayload);
void UartDecodeMessage(unsigned char c);
void UartProcessDecodedMessage(unsigned char function, unsigned char payloadLength, unsigned char* payload);

#endif	/* UART_PROTOCOL_H */