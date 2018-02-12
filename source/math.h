#ifndef MATH_3D
#define MATH_3D

typedef short fix16;
/*fix16s are shorts with 6 fractional bits and 9 integer bits and 1 sign bit
basically the value of a fix 16 is the value of the underlying short / 64
they use the same native addition and subtraction functions as shorts,
but for multiplication you have to multiply them into a 32 bit result, and
then shift that right 12 bits (6 for each fix16)
 
for division you have to shift the  divident left 6 bits, then divide as normal
*/

#define PUTIWRAM __attribute__((section(".iwram")))

#define FIX16MULTIPLY(x,y)  ((((int)(x))*(y))>>6)

#define MULT(x,y)  FIX16MULTIPLY(x,y)

#define FIX16DIVIDE(x,y)  (((int)(x))<<6)/(y)

#define INTTOFIX16(x)  ((x)<<6)

#define FIX16(x) ((x)<<6)

#define FIX16TOINT(x)  ((x)>>6)

#define FLOATTOFIX16(x)  ((short)((x)*64))

fix16 sinFix16(fix16 theta);
fix16 cosFix16(fix16 theta);
fix16 sqrtFix16(fix16 x);


typedef struct quaternion {
	fix16 x;
	fix16 y;
	fix16 z;
	fix16 w;
} quaternion;

typedef struct vector {
	fix16 x;
	fix16 y;
	fix16 z;
	fix16 w;
} vector;

quaternion identityQuat(void);

quaternion multiplyQuat(quaternion * a, quaternion * b);

quaternion quatFromAngles(fix16 x, fix16 y, fix16 z, fix16 theta);

void normalizeQuaternion(quaternion * a);

void setCurrentRotation(quaternion * a);
void translateMatrix(fix16 x, fix16 y, fix16 z);
void rotateMatrix(quaternion rot);

vector transformVector(vector in);

void transformVectors(vector * in, vector * out, u16 count);

void eulerRotationMatrix(fix16 x, fix16 y, fix16 z);

void perspectiveMatrix(void);

void translateTo(fix16 x, fix16 y, fix16 z);
#endif

