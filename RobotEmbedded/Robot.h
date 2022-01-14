#ifndef ROBOT_H
#define ROBOT_H

#include "asservissement.h"

typedef struct robotStateBITS {

    union {

        struct {
            unsigned char taskEnCours;
            float vitesseGaucheConsigne;
            float vitesseGaucheCommandeCourante;
            float vitesseDroiteConsigne;
            float vitesseDroiteCommandeCourante;
            float distanceTelemetreDroit2;
            float distanceTelemetreDroit;
            float distanceTelemetreCentre;
            float distanceTelemetreGauche;
            float distanceTelemetreGauche2;
            
            double vitesseDroitFromOdometry;
            double vitesseGaucheFromOdometry;
            double vitesseLineaireFromOdometry;
            double vitesseAngulaireFromOdometry;
            double xPosFromOdometry;
            double yPosFromOdometry;
            double angleRadianFromOdometry;
            double xPosFromOdometry_1;
            double yPosFromOdometry_1;
            double angleRadianFromOdometry_1;
            
            PidCorrector PidX;
            PidCorrector PidTheta;
            
            double vitesseAngulairePourcent;
            double thetaCorrectionVitesseCommande;
            double vitesseLineairePourcent;
            double xCorrectionVitesseCommande;
            double vitesseLineaireConsigne;
            double vitesseAngulaireConsigne;
            double xCorrectionVitessePourcent;
            double thetaCorrectionVitessePourcent;
        }
        ;
    }
    ;
} ROBOT_STATE_BITS;

extern volatile ROBOT_STATE_BITS robotState;
#endif /* ROBOT_H */
