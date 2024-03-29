#ifndef MAIN_H
#define	MAIN_H

// Configuration des paramètres du chip
#define FCY 40000000

#define STATE_ATTENTE 0
#define STATE_ATTENTE_EN_COURS 1
#define STATE_AVANCE 2
#define STATE_AVANCE_EN_COURS 3
#define STATE_TOURNE_PEU_GAUCHE 4
#define STATE_TOURNE_PEU_GAUCHE_EN_COURS 5
#define STATE_TOURNE_PEU_DROITE 6
#define STATE_TOURNE_PEU_DROITE_EN_COURS 7
#define STATE_TOURNE_GAUCHE 8
#define STATE_TOURNE_GAUCHE_EN_COURS 9
#define STATE_TOURNE_DROITE 10
#define STATE_TOURNE_DROITE_EN_COURS 11
#define STATE_TOURNE_BEAUCOUP_GAUCHE 12
#define STATE_TOURNE_BEAUCOUP_GAUCHE_EN_COURS 13
#define STATE_TOURNE_BEAUCOUP_DROITE 14
#define STATE_TOURNE_BEAUCOUP_DROITE_EN_COURS 15
#define STATE_TOURNE_SUR_PLACE_GAUCHE 16
#define STATE_TOURNE_SUR_PLACE_GAUCHE_EN_COURS 17
#define STATE_TOURNE_SUR_PLACE_DROITE 18
#define STATE_TOURNE_SUR_PLACE_DROITE_EN_COURS 19
#define STATE_ARRET 20
#define STATE_ARRET_EN_COURS 21
#define STATE_RECULE 22
#define STATE_RECULE_EN_COURS 23
#define STATE_AVANCE_LENT 24
#define STATE_AVANCE_LENT_EN_COURS 25
#define STATE_AVANCE_RAPIDE 26
#define STATE_AVANCE_RAPIDE_EN_COURS 27
#define STATE_TURBO 28
#define STATE_TURBO_EN_COURS 29

#define PAS_D_OBSTACLE 0
#define OBSTACLE_A_GAUCHE 1
#define OBSTACLE_A_DROITE 2
#define OBSTACLE_A_GAUCHE_PROCHE 3
#define OBSTACLE_A_DROITE_PROCHE 4
#define OBSTACLE_A_GAUCHE_LOIN 5
#define OBSTACLE_A_DROITE_LOIN 6
#define OBSTACLE_EN_FACE 7
#define OBSTACLE_EN_FACE_ET_A_GAUCHE 8
#define OBSTACLE_EN_FACE_ET_A_DROITE 9
#define OBSTACLE_EN_FACE_PROCHE 10
#define OBSTACLE_EN_FACE_LOIN 11
#define OBSTACLE_A_GAUCHE2 12
#define OBSTACLE_A_DROITE2 13
#define OBSTACLE_A_GAUCHE2_PROCHE 14
#define OBSTACLE_A_DROITE2_PROCHE 15

void SetRobotState(unsigned char c);
void SetRobotAutoControlState(unsigned char c);
void OperatingSystemLoop(void);
void SetNextRobotStateInAutomaticMode(void);

#endif	/* MAIN_H */