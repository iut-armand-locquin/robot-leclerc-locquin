#ifndef PI
#define PI 3.141592653589793

float Abs(float value);
float Max(float value, float value2);
float Min(float value, float value2);
float LimitToInterval(float value, float lowLimit, float highLimit);
float RadianToDegree(float value);
float DegreeToRadian(float value);


float getFloat(unsigned char *p, int index);
double getDouble(unsigned char *p, int index);

void getBytesFromFloat(unsigned char *p, int index, float f);
void getBytesFromDouble(unsigned char *p, int index, double d);
void getBytesFromInt32(unsigned char *p, int index, long in);


#endif	/* TOOLBOX_H */