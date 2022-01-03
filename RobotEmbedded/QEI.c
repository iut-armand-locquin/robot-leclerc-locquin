#include <xc.h>
#include <libpic30.h>
#include <math.h>
#include "QEI.h"
#include "Robot.h"
#include "timer.h"
#include "UART_Protocol.h"
#include "Utilities.h"

double QeiDroitPosition;
double QeiGauchePosition;

void InitQEI1()
{
    QEI1IOCbits.SWPAB = 1; //QEAx and QEBx are swapped
    QEI1GECL = 0xFFFF;
    QEI1GECH = 0xFFFF;
    QEI1CONbits.QEIEN = 1; //Enable QEI Module
}

void InitQEI2()
{
    QEI2IOCbits.SWPAB = 1; //QEAx and QEBx are not swapped
    QEI2GECL = 0xFFFF;
    QEI2GECH = 0xFFFF;
    QEI2CONbits.QEIEN = 1; //Enable QEI Module
}

void QEIUpdateData()
{
    //On sauvegarde les anciennes valeurs
    double QeiDroitPosition_T_1 = QeiDroitPosition;
    double QeiGauchePosition_T_1 = QeiGauchePosition;

    //On r�actualise les valeurs de positions
    long QEI1RawValue = POS1CNTL;
    QEI1RawValue += ((long) POS1HLD << 16);

    long QEI2RawValue = POS2CNTL;
    QEI2RawValue += ((long) POS2HLD << 16);

    //Conversion en mm (r�gl� pour la taille des roues codeuses)
    QeiDroitPosition = POINT_TO_METER * QEI1RawValue;
    QeiGauchePosition = -POINT_TO_METER * QEI2RawValue;

    //Calcul des deltas de position
    double delta_d = QeiDroitPosition - QeiDroitPosition_T_1;
    double delta_g = QeiGauchePosition - QeiGauchePosition_T_1;
    //delta_theta = atan((delta_d - delta_g) / DISTROUES);
    double delta_theta = (delta_d - delta_g) / DISTROUES;
    //double dx = (delta_d + delta_g) / 2;

    //Calcul des vitesses
    //attention � remultiplier par la fr�quence d'�chantillonnage
    robotState.vitesseDroitFromOdometry = delta_d*FREQ_ECH_QEI;
    robotState.vitesseGaucheFromOdometry = delta_g*FREQ_ECH_QEI;
    robotState.vitesseLineaireFromOdometry = (robotState.vitesseDroitFromOdometry + robotState.vitesseGaucheFromOdometry) / 2;
    robotState.vitesseAngulaireFromOdometry = delta_theta*FREQ_ECH_QEI;

    //Mise � jour du positionnement terrain � t-1
    robotState.xPosFromOdometry_1 = robotState.xPosFromOdometry;
    robotState.yPosFromOdometry_1 = robotState.yPosFromOdometry;
    robotState.angleRadianFromOdometry_1 = robotState.angleRadianFromOdometry;

    //Calcul des positions dans le r�f�rentiel du terrain
    robotState.xPosFromOdometry = robotState.xPosFromOdometry_1 + (robotState.vitesseLineaireFromOdometry / FREQ_ECH_QEI) * cos(robotState.angleRadianFromOdometry);
    robotState.yPosFromOdometry = robotState.yPosFromOdometry_1 + (robotState.vitesseLineaireFromOdometry / FREQ_ECH_QEI) * sin(robotState.angleRadianFromOdometry);
    robotState.angleRadianFromOdometry = robotState.angleRadianFromOdometry_1 + delta_theta;
    if (robotState.angleRadianFromOdometry > PI)
        robotState.angleRadianFromOdometry -= 2 * PI;
    if (robotState.angleRadianFromOdometry < -PI)
        robotState.angleRadianFromOdometry += 2 * PI;
}

void SendPositionData()
{
    unsigned char positionPayload[24];
    getBytesFromInt32(positionPayload, 0, timestamp);
    getBytesFromFloat(positionPayload, 4, (float) (robotState.xPosFromOdometry));
    getBytesFromFloat(positionPayload, 8, (float) (robotState.yPosFromOdometry));
    getBytesFromFloat(positionPayload, 12, (float) (robotState.angleRadianFromOdometry));
    getBytesFromFloat(positionPayload, 16, (float) (robotState.vitesseLineaireFromOdometry));
    getBytesFromFloat(positionPayload, 20, (float) (robotState.vitesseAngulaireFromOdometry));
    UartEncodeAndSendMessage(POSITION_DATA, 24, positionPayload);
}