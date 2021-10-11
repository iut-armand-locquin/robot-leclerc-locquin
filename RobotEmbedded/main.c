#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <libpic30.h>
#include "ChipConfig.h"
#include "IO.h"
#include "timer.h"
#include "PWM.h"
#include "Robot.h"
#include "ADC.h"
#include "main.h"
#include "UART.h"
#include "CB_TX1.h"
#include "CB_RX1.h"
#include "UART_Protocol.h"

int main(void) {
    /***************************************************************************************************/
    //Initialisation de l'oscillateur
    /****************************************************************************************************/
    InitOscillator();

    /****************************************************************************************************/
    // Configuration des entrées sorties
    /****************************************************************************************************/
    InitIO();
    InitTimer1();
    InitTimer23();
    InitTimer4();
    InitPWM();
    InitADC1();
    InitUART();

    /****************************************************************************************************/
    // Boucle Principale
    /****************************************************************************************************/
    while (1) {
        if (ADCIsConversionFinished() == 1) {
            ADCClearConversionFinishedFlag();
            unsigned int * result = ADCGetResult();
            float volts = ((float) result [0]) * 3.3 / 4096 * 3.2;
            robotState.distanceTelemetreDroit2 = 34 / volts - 5;
            volts = ((float) result [1]) * 3.3 / 4096 * 3.2;
            robotState.distanceTelemetreDroit = 34 / volts - 5;
            volts = ((float) result [2]) * 3.3 / 4096 * 3.2;
            robotState.distanceTelemetreCentre = 34 / volts - 5;
            volts = ((float) result [4]) * 3.3 / 4096 * 3.2;
            robotState.distanceTelemetreGauche = 34 / volts - 5;
            volts = ((float) result [3]) * 3.3 / 4096 * 3.2;
            robotState.distanceTelemetreGauche2 = 34 / volts - 5;
        }

        unsigned char fonction = 0x0030;
        int payloadLength = 3;
        unsigned char payload[] = {robotState.distanceTelemetreGauche, robotState.distanceTelemetreCentre, robotState.distanceTelemetreDroit};
        UartEncodeAndSendMessage(fonction, payloadLength, payload);

        fonction = 0x0020;
        payloadLength = 2;

        if (robotState.distanceTelemetreDroit > 20) {
            LED_ORANGE = 1;
        } else {
            LED_ORANGE = 0;
        }

        payload[0] = 1;
        payload[1] = LED_ORANGE;
        UartEncodeAndSendMessage(fonction, payloadLength, payload);

        if (robotState.distanceTelemetreCentre > 15) {
            LED_BLEUE = 1;
        } else {
            LED_BLEUE = 0;
        }

        payload[0] = 2;
        payload[1] = LED_BLEUE;
        UartEncodeAndSendMessage(fonction, payloadLength, payload);

        if (robotState.distanceTelemetreGauche > 20) {
            LED_BLANCHE = 1;
        } else {
            LED_BLANCHE = 0;
        }

        payload[0] = 3;
        payload[1] = LED_BLANCHE;
        UartEncodeAndSendMessage(fonction, payloadLength, payload);

        fonction = 0x0040;
        payloadLength = 2;
        payload[0] = abs(robotState.vitesseGaucheConsigne);
        payload[1] = abs(robotState.vitesseDroiteConsigne);
        UartEncodeAndSendMessage(fonction, payloadLength, payload);

        //        unsigned char fonction = 0x0080;
        //        int payloadLength = 7;
        //        unsigned char payload[] = {'B', 'o', 'n', 'j', 'o', 'u', 'r'} ;
        //        UartEncodeAndSendMessage(fonction, payloadLength, payload);
        //        __delay32(40000000);


        //        SendMessage((unsigned char*) "Bonjour", 7);
        //        __delay32(4000000);

        int i;
        for (i = 0; i < CB_RX1_GetDataSize(); i++) {
            unsigned char c = CB_RX1_Get();
            //SendMessage(&c, 1);
            UartDecodeMessage(c);
        }
        //__delay32(10000);
    }
}

int alea = 0;
unsigned char stateRobot;

int modeAuto;

void SetRobotAutoControlState(unsigned char c) {
    modeAuto = c;
    if (modeAuto == 1) {
        stateRobot = STATE_AVANCE;
    } else {
        stateRobot = STATE_ARRET;
    }
}

void SetRobotState(unsigned char c) {
    stateRobot = c;
}

void OperatingSystemLoop(void) {
    switch (stateRobot) {
        case STATE_ATTENTE:
            timestamp = 0;
            PWMSetSpeedConsigne(0, MOTEUR_DROIT);
            PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
            stateRobot = STATE_ATTENTE_EN_COURS;

        case STATE_ATTENTE_EN_COURS:
            if (timestamp > 1000)
                stateRobot = STATE_AVANCE;
            break;

        case STATE_AVANCE:
            PWMSetSpeedConsigne(18, MOTEUR_DROIT);
            PWMSetSpeedConsigne(18, MOTEUR_GAUCHE);
            stateRobot = STATE_AVANCE_EN_COURS;
            break;
        case STATE_AVANCE_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

        case STATE_TOURNE_PEU_GAUCHE:
            PWMSetSpeedConsigne(16, MOTEUR_DROIT);
            PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_PEU_GAUCHE_EN_COURS;
            break;
        case STATE_TOURNE_PEU_GAUCHE_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

        case STATE_TOURNE_PEU_DROITE:
            PWMSetSpeedConsigne(0, MOTEUR_DROIT);
            PWMSetSpeedConsigne(16, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_PEU_DROITE_EN_COURS;
            break;
        case STATE_TOURNE_PEU_DROITE_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

        case STATE_TOURNE_GAUCHE:
            PWMSetSpeedConsigne(21, MOTEUR_DROIT);
            PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_GAUCHE_EN_COURS;
            break;
        case STATE_TOURNE_GAUCHE_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

        case STATE_TOURNE_DROITE:
            PWMSetSpeedConsigne(0, MOTEUR_DROIT);
            PWMSetSpeedConsigne(21, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_DROITE_EN_COURS;
            break;
        case STATE_TOURNE_DROITE_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

        case STATE_TOURNE_BEAUCOUP_GAUCHE:
            PWMSetSpeedConsigne(26, MOTEUR_DROIT);
            PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_BEAUCOUP_GAUCHE_EN_COURS;
            break;
        case STATE_TOURNE_BEAUCOUP_GAUCHE_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

        case STATE_TOURNE_BEAUCOUP_DROITE:
            PWMSetSpeedConsigne(0, MOTEUR_DROIT);
            PWMSetSpeedConsigne(26, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_BEAUCOUP_DROITE_EN_COURS;
            break;
        case STATE_TOURNE_BEAUCOUP_DROITE_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

        case STATE_TOURNE_SUR_PLACE_GAUCHE:
            PWMSetSpeedConsigne(16, MOTEUR_DROIT);
            PWMSetSpeedConsigne(-16, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS;
            break;
        case STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

        case STATE_TOURNE_SUR_PLACE_DROITE:
            PWMSetSpeedConsigne(-16, MOTEUR_DROIT);
            PWMSetSpeedConsigne(16, MOTEUR_GAUCHE);
            stateRobot = STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS;
            break;
        case STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

        case STATE_AVANCE_LENT:
            PWMSetSpeedConsigne(8, MOTEUR_DROIT);
            PWMSetSpeedConsigne(8, MOTEUR_GAUCHE);
            stateRobot = STATE_AVANCE_EN_COURS;
            break;
        case STATE_AVANCE_LENT_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

            //        case STATE_AVANCE_RAPIDE:
            //            PWMSetSpeedConsigne(22, MOTEUR_DROIT);
            //            PWMSetSpeedConsigne(22, MOTEUR_GAUCHE);
            //            stateRobot = STATE_AVANCE_EN_COURS;
            //            break;
            //        case STATE_AVANCE_RAPIDE_EN_COURS:
            //            SetNextRobotStateInAutomaticMode();
            //            break;

        case STATE_TURBO:
            alea++;
            PWMSetSpeedConsigne(24, MOTEUR_DROIT);
            PWMSetSpeedConsigne(24, MOTEUR_GAUCHE);
            stateRobot = STATE_TURBO_EN_COURS;
            break;
        case STATE_TURBO_EN_COURS:
            SetNextRobotStateInAutomaticMode();
            break;

        default:
            stateRobot = STATE_ATTENTE;
            break;
    }
}

unsigned char nextStateRobot = 0;

void SetNextRobotStateInAutomaticMode(void) {
    if (modeAuto == 1) {
        unsigned char positionObstacle = PAS_D_OBSTACLE;

        //Détermination de la position des obstacles en fonction des télémètres

        if (robotState.distanceTelemetreDroit < 18 &&
                robotState.distanceTelemetreCentre > 23 &&
                robotState.distanceTelemetreGauche > 18)
            positionObstacle = OBSTACLE_A_DROITE_PROCHE;
        else if (robotState.distanceTelemetreDroit > 18 &&
                robotState.distanceTelemetreCentre > 23 &&
                robotState.distanceTelemetreGauche < 18)
            positionObstacle = OBSTACLE_A_GAUCHE_PROCHE;
        else if (robotState.distanceTelemetreCentre < 22) //Obstacle en face
            positionObstacle = OBSTACLE_EN_FACE_PROCHE;
        else if (robotState.distanceTelemetreDroit < 22 &&
                robotState.distanceTelemetreCentre > 27 &&
                robotState.distanceTelemetreGauche > 22) //Obstacle à droite
            positionObstacle = OBSTACLE_A_DROITE;
        else if (robotState.distanceTelemetreDroit > 22 &&
                robotState.distanceTelemetreCentre > 27 &&
                robotState.distanceTelemetreGauche < 22) //Obstacle à gauche
            positionObstacle = OBSTACLE_A_GAUCHE;
        else if (robotState.distanceTelemetreDroit2 < 26 &&
                robotState.distanceTelemetreGauche2 > 26)
            positionObstacle = OBSTACLE_A_DROITE2;
        else if (robotState.distanceTelemetreDroit2 > 26 &&
                robotState.distanceTelemetreGauche2 < 26)
            positionObstacle = OBSTACLE_A_GAUCHE2;
        else if (robotState.distanceTelemetreDroit < 27 &&
                robotState.distanceTelemetreCentre > 37 &&
                robotState.distanceTelemetreGauche > 27)
            positionObstacle = OBSTACLE_A_DROITE_LOIN;
        else if (robotState.distanceTelemetreDroit > 27 &&
                robotState.distanceTelemetreCentre > 37 &&
                robotState.distanceTelemetreGauche < 27)
            positionObstacle = OBSTACLE_A_GAUCHE_LOIN;
            //        else if (robotState.distanceTelemetreDroit2 < robotState.distanceTelemetreGauche2 &&
            //                robotState.distanceTelemetreDroit2 < 20)
            //                positionObstacle = OBSTACLE_A_DROITE2;
            //        else if (robotState.distanceTelemetreGauche2 < robotState.distanceTelemetreDroit2 &&
            //                robotState.distanceTelemetreGauche2 < 20)
            //                positionObstacle = OBSTACLE_A_GAUCHE2;
        else if (robotState.distanceTelemetreCentre < 33)
            positionObstacle = OBSTACLE_EN_FACE;
        else if (robotState.distanceTelemetreDroit2 < 15 &&
                robotState.distanceTelemetreGauche2 > 30)
            positionObstacle = OBSTACLE_A_DROITE2_PROCHE;
        else if (robotState.distanceTelemetreDroit2 > 30 &&
                robotState.distanceTelemetreGauche2 < 15)
            positionObstacle = OBSTACLE_A_GAUCHE2_PROCHE;
        else if (robotState.distanceTelemetreCentre < 44)
            positionObstacle = OBSTACLE_EN_FACE_LOIN;
        else if (robotState.distanceTelemetreDroit > 40 &&
                robotState.distanceTelemetreCentre > 60 &&
                robotState.distanceTelemetreGauche > 40) //pas d'obstacle
            positionObstacle = PAS_D_OBSTACLE;

        //Détermination de l'état à venir du robot
        if (positionObstacle == PAS_D_OBSTACLE)
            nextStateRobot = STATE_TURBO;

        else if (positionObstacle == OBSTACLE_A_DROITE_PROCHE)
            nextStateRobot = STATE_TOURNE_BEAUCOUP_GAUCHE;

        else if (positionObstacle == OBSTACLE_A_GAUCHE_PROCHE)
            nextStateRobot = STATE_TOURNE_BEAUCOUP_DROITE;

        else if (positionObstacle == OBSTACLE_A_DROITE)
            nextStateRobot = STATE_TOURNE_GAUCHE;

        else if (positionObstacle == OBSTACLE_A_GAUCHE)
            nextStateRobot = STATE_TOURNE_DROITE;

        else if (positionObstacle == OBSTACLE_A_DROITE_LOIN)
            nextStateRobot = STATE_TOURNE_PEU_GAUCHE;

        else if (positionObstacle == OBSTACLE_A_GAUCHE_LOIN)
            nextStateRobot = STATE_TOURNE_PEU_DROITE;

        else if (positionObstacle == OBSTACLE_EN_FACE_PROCHE ||
                positionObstacle == OBSTACLE_A_DROITE2_PROCHE ||
                positionObstacle == OBSTACLE_A_GAUCHE2_PROCHE)
            if (alea % 2 == 0)
                nextStateRobot = STATE_TOURNE_SUR_PLACE_DROITE;
            else
                nextStateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE;

            //    else if (positionObstacle == OBSTACLE_A_DROITE2_PROCHE)
            //        nextStateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE;
            //    
            //    else if (positionObstacle == OBSTACLE_A_GAUCHE2_PROCHE)
            //        nextStateRobot = STATE_TOURNE_SUR_PLACE_DROITE;

        else if (positionObstacle == OBSTACLE_EN_FACE)
            nextStateRobot = STATE_AVANCE_LENT;

        else if (positionObstacle == OBSTACLE_EN_FACE_LOIN)
            nextStateRobot = STATE_AVANCE;

        else if (positionObstacle == OBSTACLE_A_DROITE2)
            nextStateRobot = STATE_TOURNE_PEU_GAUCHE;

        else if (positionObstacle == OBSTACLE_A_GAUCHE2)
            nextStateRobot = STATE_TOURNE_PEU_DROITE;

        //Si l'on n'est pas dans la transition de l'étape en cours
        if (nextStateRobot != stateRobot - 1) {
            stateRobot = nextStateRobot;

            //    if (nextStateRobot == STATE_TURBO) {
            //        LED_BLEUE = 1;
            //    } else {
            //        LED_BLEUE = 0;
            //    }
            //    
            //    if (nextStateRobot == STATE_AVANCE) {
            //        LED_BLANCHE = 1;
            //    } else {
            //        LED_BLANCHE = 0;
            //    }
            //    
            //    if (nextStateRobot == STATE_AVANCE_LENT) {
            //        LED_ORANGE = 1;
            //    } else {
            //        LED_ORANGE = 0;
            //    }


            unsigned char fonction = 0x0050;
            int payloadLength = 5;
            unsigned char payload[5];
            payload[0] = nextStateRobot;
            payload[1] = timestamp >> 24;
            payload[2] = timestamp >> 16;
            payload[3] = timestamp >> 8;
            payload[4] = timestamp >> 0;
            UartEncodeAndSendMessage(fonction, payloadLength, payload);
        }
    }
}