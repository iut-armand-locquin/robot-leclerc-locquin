#ifndef PWM_H
#define PWM_H

#define MOTEUR_DROIT 0
#define MOTEUR_GAUCHE 1

#define COEFF_VITESSE_LINEAIRE_PERCENT 90
#define COEFF_VITESSE_ANGULAIRE_PERCENT 77

void InitPWM(void);
//void PWMSetSpeed(float vitesseEnPourcents, int moteur);
void PWMUpdateSpeed();
void PWMSetSpeedConsigne(float vitesseEnPourcents, char moteur);
void UpdateAsservissement();
void PWMSetSpeedConsignePolaire(double xCorrectionVitessePourcent, double thetaCorrectionVitessePourcent);

#endif	/* PWM_H */
