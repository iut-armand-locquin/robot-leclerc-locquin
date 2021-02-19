#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include "ChipConfig.h"
#include "IO.h"
#include "timer.h"
#include "PWM.h"
#include "Robot.h"
#include "ADC.h"
#include "main.h"

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
        if (robotState.distanceTelemetreDroit > 20) {
            LED_ORANGE = 1;
        } else {
            LED_ORANGE = 0;
        }
        if (robotState.distanceTelemetreCentre > 15) {
            LED_BLEUE = 1;
        } else {
            LED_BLEUE = 0;
        }
        if (robotState.distanceTelemetreGauche > 20) {
            LED_BLANCHE = 1;
        } else {
            LED_BLANCHE = 0;
        }
    }
}

int alea = 0;
unsigned char stateRobot;

void OperatingSystemLoop(void) {
        switch (stateRobot) {
            case STATE_ATTENTE:
                timestamp = 0;
                PWMSetSpeedConsigne(0, MOTEUR_DROIT);
                PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
                stateRobot = STATE_ATTENTE_EN_COURS;

            case STATE_ATTENTE_EN_COURS:
                if (timestamp > 3000)
                    stateRobot = STATE_AVANCE;
                break;

            case STATE_AVANCE:
                alea++;
                PWMSetSpeedConsigne(20, MOTEUR_DROIT);
                PWMSetSpeedConsigne(20, MOTEUR_GAUCHE);
                stateRobot = STATE_AVANCE_EN_COURS;
                break;
            case STATE_AVANCE_EN_COURS:
                SetNextRobotStateInAutomaticMode();
                break;

            case STATE_TOURNE_PEU_GAUCHE:
                PWMSetSpeedConsigne(20, MOTEUR_DROIT);
                PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
                stateRobot = STATE_TOURNE_PEU_GAUCHE_EN_COURS;
                break;
            case STATE_TOURNE_PEU_GAUCHE_EN_COURS:
                SetNextRobotStateInAutomaticMode();
                break;

            case STATE_TOURNE_PEU_DROITE:
                PWMSetSpeedConsigne(0, MOTEUR_DROIT);
                PWMSetSpeedConsigne(20, MOTEUR_GAUCHE);
                stateRobot = STATE_TOURNE_PEU_DROITE_EN_COURS;
                break;
            case STATE_TOURNE_PEU_DROITE_EN_COURS:
                SetNextRobotStateInAutomaticMode();
                break;

            case STATE_TOURNE_GAUCHE:
                PWMSetSpeedConsigne(30, MOTEUR_DROIT);
                PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
                stateRobot = STATE_TOURNE_GAUCHE_EN_COURS;
                break;
            case STATE_TOURNE_GAUCHE_EN_COURS:
                SetNextRobotStateInAutomaticMode();
                break;

            case STATE_TOURNE_DROITE:
                PWMSetSpeedConsigne(0, MOTEUR_DROIT);
                PWMSetSpeedConsigne(30, MOTEUR_GAUCHE);
                stateRobot = STATE_TOURNE_DROITE_EN_COURS;
                break;
            case STATE_TOURNE_DROITE_EN_COURS:
                SetNextRobotStateInAutomaticMode();
                break;
                
            case STATE_TOURNE_BEAUCOUP_GAUCHE:
                PWMSetSpeedConsigne(35, MOTEUR_DROIT);
                PWMSetSpeedConsigne(0, MOTEUR_GAUCHE);
                stateRobot = STATE_TOURNE_BEAUCOUP_GAUCHE_EN_COURS;
                break;
            case STATE_TOURNE_BEAUCOUP_GAUCHE_EN_COURS:
                SetNextRobotStateInAutomaticMode();
                break;

            case STATE_TOURNE_BEAUCOUP_DROITE:
                PWMSetSpeedConsigne(0, MOTEUR_DROIT);
                PWMSetSpeedConsigne(35, MOTEUR_GAUCHE);
                stateRobot = STATE_TOURNE_BEAUCOUP_DROITE_EN_COURS;
                break;
            case STATE_TOURNE_BEAUCOUP_DROITE_EN_COURS:
                SetNextRobotStateInAutomaticMode();
                break;
            
            case STATE_TOURNE_SUR_PLACE_GAUCHE:
                PWMSetSpeedConsigne(15, MOTEUR_DROIT);
                PWMSetSpeedConsigne(-15, MOTEUR_GAUCHE);
                stateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS;
                break;
            case STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS:
                SetNextRobotStateInAutomaticMode();
                break;

            case STATE_TOURNE_SUR_PLACE_DROITE:
                PWMSetSpeedConsigne(-15, MOTEUR_DROIT);
                PWMSetSpeedConsigne(15, MOTEUR_GAUCHE);
                stateRobot = STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS;
                break;
            case STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS:
                SetNextRobotStateInAutomaticMode();
                break;

            case STATE_AVANCE_LENT:
                PWMSetSpeedConsigne(15, MOTEUR_DROIT);
                PWMSetSpeedConsigne(15, MOTEUR_GAUCHE);
                stateRobot = STATE_AVANCE_EN_COURS;
                break;
            case STATE_AVANCE_LENT_EN_COURS:
                SetNextRobotStateInAutomaticMode();
                break;
                
            case STATE_AVANCE_RAPIDE:
                PWMSetSpeedConsigne(25, MOTEUR_DROIT);
                PWMSetSpeedConsigne(25, MOTEUR_GAUCHE);
                stateRobot = STATE_AVANCE_EN_COURS;
                break;
            case STATE_AVANCE_RAPIDE_EN_COURS:
                SetNextRobotStateInAutomaticMode();
                break;

            case STATE_MBAPPE:
                PWMSetSpeedConsigne(27, MOTEUR_DROIT);
                PWMSetSpeedConsigne(27, MOTEUR_GAUCHE);
                stateRobot = STATE_MBAPPE_EN_COURS;
                break;
            case STATE_MBAPPE_EN_COURS:
                SetNextRobotStateInAutomaticMode();
                break;
            
            default:
                stateRobot = STATE_ATTENTE;
                break;
        }
    }

    unsigned char nextStateRobot = 0;

    void SetNextRobotStateInAutomaticMode(void) {
        unsigned char positionObstacle = PAS_D_OBSTACLE;

        //Détermination de la position des obstacles en fonction des télémètres
        
        if (robotState.distanceTelemetreDroit < 15 &&
                robotState.distanceTelemetreCentre > 20 &&
                robotState.distanceTelemetreGauche > 15)
            positionObstacle = OBSTACLE_A_DROITE_PROCHE;
        else if (robotState.distanceTelemetreDroit > 15 &&
                robotState.distanceTelemetreCentre > 20 &&
                robotState.distanceTelemetreGauche < 15)
            positionObstacle = OBSTACLE_A_GAUCHE_PROCHE;
        else if (robotState.distanceTelemetreDroit < 20 &&
                robotState.distanceTelemetreCentre > 30 &&
                robotState.distanceTelemetreGauche > 20) //Obstacle à droite
            positionObstacle = OBSTACLE_A_DROITE;
        else if (robotState.distanceTelemetreDroit > 20 &&
                robotState.distanceTelemetreCentre > 30 &&
                robotState.distanceTelemetreGauche < 20) //Obstacle à gauche
            positionObstacle = OBSTACLE_A_GAUCHE;
        else if (robotState.distanceTelemetreDroit < 30 &&
                robotState.distanceTelemetreCentre > 40 &&
                robotState.distanceTelemetreGauche > 30)
            positionObstacle = OBSTACLE_A_DROITE_LOIN;
        else if (robotState.distanceTelemetreDroit > 30 &&
                robotState.distanceTelemetreCentre > 40 &&
                robotState.distanceTelemetreGauche < 30)
            positionObstacle = OBSTACLE_A_GAUCHE_LOIN;
        else if (robotState.distanceTelemetreCentre < 20) //Obstacle en face
            positionObstacle = OBSTACLE_EN_FACE_PROCHE;
        else if (robotState.distanceTelemetreCentre < 40)
            positionObstacle = OBSTACLE_EN_FACE;
        else if (robotState.distanceTelemetreCentre < 60)
            positionObstacle = OBSTACLE_EN_FACE_LOIN;
        else if (robotState.distanceTelemetreDroit2 > robotState.distanceTelemetreGauche2)
                positionObstacle = OBSTACLE_A_DROITE2;
        else if (robotState.distanceTelemetreDroit2 < robotState.distanceTelemetreGauche2)
                positionObstacle = OBSTACLE_A_GAUCHE2;
        else if (robotState.distanceTelemetreDroit > 40 &&
                robotState.distanceTelemetreCentre > 60 &&
                robotState.distanceTelemetreGauche > 40) //pas d'obstacle
            positionObstacle = PAS_D_OBSTACLE;
        
        //Détermination de l'état à venir du robot
        if (positionObstacle == PAS_D_OBSTACLE)
            nextStateRobot = STATE_MBAPPE;
        else if (positionObstacle == OBSTACLE_A_DROITE_PROCHE)
            nextStateRobot = STATE_TOURNE_BEAUCOUP_DROITE;
        else if (positionObstacle == OBSTACLE_A_GAUCHE_PROCHE)
            nextStateRobot = STATE_TOURNE_BEAUCOUP_GAUCHE;
        else if (positionObstacle == OBSTACLE_A_DROITE)
            nextStateRobot = STATE_TOURNE_DROITE;
        else if (positionObstacle == OBSTACLE_A_GAUCHE)
            nextStateRobot = STATE_TOURNE_GAUCHE;
        else if (positionObstacle == OBSTACLE_A_DROITE_LOIN)
            nextStateRobot = STATE_TOURNE_PEU_DROITE;
        else if (positionObstacle == OBSTACLE_A_GAUCHE_LOIN)
            nextStateRobot = STATE_TOURNE_PEU_GAUCHE;
        else if (positionObstacle == OBSTACLE_EN_FACE_PROCHE)
            if (alea%2 == 0)
                nextStateRobot = STATE_TOURNE_SUR_PLACE_DROITE;
            else
                nextStateRobot = STATE_TOURNE_SUR_PLACE_GAUCHE;
        else if (positionObstacle == OBSTACLE_EN_FACE)
            nextStateRobot = STATE_AVANCE_LENT;
        else if (positionObstacle == OBSTACLE_EN_FACE_LOIN)
            nextStateRobot = STATE_AVANCE;
        else if (positionObstacle == OBSTACLE_A_DROITE2)
            nextStateRobot = STATE_TOURNE_PEU_GAUCHE;
        else if (positionObstacle == OBSTACLE_A_GAUCHE2)
            nextStateRobot = STATE_TOURNE_PEU_DROITE;

        //Si l'on n'est pas dans la transition de l'étape en cours
        if (nextStateRobot != stateRobot - 1)
            stateRobot = nextStateRobot;
    }