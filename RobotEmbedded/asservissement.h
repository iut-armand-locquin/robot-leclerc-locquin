#ifndef ASSERVISSEMENT_H
#define	ASSERVISSEMENT_H

typedef struct _PidCorrector {
    double Kp;
    double Ki;
    double Kd;
    double erreurProportionelleMax;
    double erreurIntegraleMax;
    double erreurDeriveeMax;
    double erreurIntegrale;
    double epsilon_1;
    double erreur;
    
    double consigne;
    double value;
    double command;

    //For Debug only
    double corrP;
    double corrI;
    double corrD;
} PidCorrector;

void SetupPidAsservissement(volatile PidCorrector* PidCorr, double Kp, double Ki, double Kd, double proportionelleMax, double integralMax, double deriveeMax);
double Correcteur(volatile PidCorrector* PidCorr, double erreur);
void AsservissementValeur();

#endif	/* ASSERVISSEMENT_H */

