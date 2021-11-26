#ifndef QEI_H
#define	QEI_H

#define	FREQ_ECH_QEI 250

void InitQEI1();
void InitQEI2();
void QEIUpdateData();
long QeiDroitPosition;
long QeiGauchePosition;
long QeiDroitPosition_T_1;
long QeiGauchePosition_T_1;
long delta_g;
long delta_d;
long delta_theta;
long dx;
void SendPositionData();

#endif	/* QEI_H */