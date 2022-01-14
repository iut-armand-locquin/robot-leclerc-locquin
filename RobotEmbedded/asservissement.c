#include <xc.h>
#include <libpic30.h>
#include "asservissement.h"
#include "QEI.h"
#include "Utilities.h"
#include "UART_Protocol.h"
#include "Robot.h"

void SetupPidAsservissement(volatile PidCorrector* PidCorr, double Kp, double Ki, double Kd, double proportionelleMax, double integralMax, double deriveeMax){   
    PidCorr->Kp = Kp;
    PidCorr->erreurProportionelleMax = proportionelleMax; //On limite la correction due au Kp
    PidCorr->Ki = Ki;
    PidCorr->erreurIntegraleMax = integralMax; //On limite la correction due au Ki
    PidCorr->Kd = Kd;
    PidCorr->erreurDeriveeMax = deriveeMax;
}

double Correcteur(volatile PidCorrector* PidCorr, double erreur){
    PidCorr->erreur = erreur;
    double erreurProportionnelle = LimitToInterval(erreur, -PidCorr->erreurProportionelleMax/PidCorr->Kp, PidCorr->erreurProportionelleMax/PidCorr->Kp);
    PidCorr->corrP = erreurProportionnelle * PidCorr->Kp;
    
    PidCorr->erreurIntegrale += erreur/FREQ_ECH_QEI;
    PidCorr->erreurIntegrale = LimitToInterval(PidCorr->erreurIntegrale, -PidCorr->erreurIntegraleMax/PidCorr->Ki, PidCorr->erreurIntegraleMax/PidCorr->Ki);
    PidCorr->corrI = PidCorr->erreurIntegrale * PidCorr->Ki;
    
    double erreurDerivee = (erreur - PidCorr->epsilon_1) * FREQ_ECH_QEI;
    double deriveeBornee = LimitToInterval(erreurDerivee, -PidCorr->erreurDeriveeMax/PidCorr->Kd, PidCorr->erreurDeriveeMax/PidCorr->Kd);
    PidCorr->epsilon_1 = erreur;
    PidCorr->corrD = PidCorr->Kd * deriveeBornee;
    
    return PidCorr->corrP + PidCorr->corrI + PidCorr->corrD;
}

unsigned char asservissementPayload[104];
//    double consigneX = 0.01;
//    double consigneTheta = 0.02;                    
//    double valueX = 0.03;
//    double valueTheta = 0.04;
//    double errorX = 0.05;
//    double errorTheta = 0.06;
//    double commandX = 0.07;
//    double commandTheta = 0.08;
//    
//    double corrPX = 0.09;
//    double corrPTheta = 0.10;
//    double corrIX = 0.11;
//    double corrITheta = 0.12;
//    double corrDX = 0.13;
//    double corrDTheta = 0.14;
//    
//    double KpX = 0.15;
//    double KpTheta = 0.16; 
//    double KiX = 0.17; 
//    double KiTheta = 0.18; 
//    double KdX = 0.19; 
//    double KdTheta = 0.20;
//    
//    double corrLimitPX = 0.21;
//    double corrLimitPTheta = 0.22;
//    double corrLimitIX = 0.23;
//    double corrLimitITheta = 0.24;
//    double corrLimitDX = 0.25;
//    double corrLimitDTheta = 0.26;

void AsservissementValeur(){
    //-------------------
    int nb_octet = 0;
    getBytesFromFloat(asservissementPayload, nb_octet, (float)(robotState.PidX.consigne));
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidTheta.consigne));
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidX.value)); 
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidTheta.value)); 
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidX.erreur));
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidTheta.erreur)); 
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.xCorrectionVitesseCommande));
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.thetaCorrectionVitesseCommande)); 
    //-------------------   
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidX.corrP));
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidTheta.corrP));
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidX.corrI));
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidTheta.corrI));
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidX.corrD));
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidTheta.corrD));
    //-------------------
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidX.Kp)); 
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidTheta.Kp));
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidX.Ki));
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidTheta.Ki));
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidX.Kd)); 
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidTheta.Kd));
    //-------------------
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidX.erreurProportionelleMax)); 
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidTheta.erreurProportionelleMax)); 
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidX.erreurIntegraleMax)); 
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidTheta.erreurIntegraleMax));
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidX.erreurDeriveeMax)); 
    getBytesFromFloat(asservissementPayload, nb_octet += 4, (float)(robotState.PidTheta.erreurDeriveeMax));

    UartEncodeAndSendMessage(FONCTION_ASSERVISSEMENT, 104, asservissementPayload);
}