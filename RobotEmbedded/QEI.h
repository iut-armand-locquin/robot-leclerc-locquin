#ifndef QEI_H
#define	QEI_H

#define	FREQ_ECH_QEI 250
#define DISTROUES 0.22
#define POINT_TO_METER 0.00001765036644  //8192pts par tour , 1 tour = diametre_roue_codeuse (=0.046025) * 2 * pi = 

void InitQEI1();
void InitQEI2();
void QEIUpdateData();
void SendPositionData();

#endif	/* QEI_H */