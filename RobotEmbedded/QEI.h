#ifndef QEI_H
#define	QEI_H

#define	FREQ_ECH_QEI 250
#define DISTROUES 0.2812
#define POINT_TO_METER 0.00001620
#define POSITION_DATA 0x0061

void InitQEI1();
void InitQEI2();
void QEIUpdateData();
void SendPositionData();

#endif	/* QEI_H */