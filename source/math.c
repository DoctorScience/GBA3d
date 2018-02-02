#include <gba.h>
#include "math.h"

#include "sinTable.h"

fix16 currentMatrix [16];


const quaternion identityQuaternion = {
	0,
	0,
	0,
	INTTOFIX16(1)
};

fix16 sinFix16(fix16 theta){
	theta = theta&1023;
	if(theta<256){
		return sinTable[theta];
	}
	if(theta<512){
		return sinTable[255-(theta&255)];
	}
	if(theta<768){
		return -sinTable[theta&255];
	}
	return -sinTable[255-(theta&255)];
}

fix16 cosFix16(fix16 theta){
		theta = (theta+256)&1023;
	if(theta<256){
		return sinTable[theta];
	}
	if(theta<512){
		return sinTable[255-(theta&255)];
	}
	if(theta<768){
		return -sinTable[theta&255];
	}
	return -sinTable[255-(theta&255)];
}

fix16 sqrtFix16(fix16 x){
	fix16 sqrt;//this function needs to be cleaned up real bad
	fix16 guess;//sweet sweet newton's approximation, if the guess is improved then the function goes faster
	
	
	
	guess = 1;
	
	sqrt = FIX16DIVIDE(x,guess);
	guess = (guess+sqrt)>>1;
	sqrt = FIX16DIVIDE(x,guess);
	guess = (guess+sqrt)>>1;
	sqrt = FIX16DIVIDE(x,guess);
	guess = (guess+sqrt)>>1;
	sqrt = FIX16DIVIDE(x,guess);
	guess = (guess+sqrt)>>1;
	sqrt = FIX16DIVIDE(x,guess);
	guess = (guess+sqrt)>>1;
	sqrt = FIX16DIVIDE(x,guess);
	guess = (guess+sqrt)>>1;
	sqrt = FIX16DIVIDE(x,guess);
	guess = (guess+sqrt)>>1;
	sqrt = FIX16DIVIDE(x,guess);
	guess = (guess+sqrt)>>1;
	sqrt = FIX16DIVIDE(x,guess);
	guess = (guess+sqrt)>>1;
	sqrt = FIX16DIVIDE(x,guess);
	guess = (guess+sqrt)>>1;
	sqrt = FIX16DIVIDE(x,guess);
	guess = (guess+sqrt)>>1;
	sqrt = FIX16DIVIDE(x,guess);
	guess = (guess+sqrt)>>1;
	sqrt = FIX16DIVIDE(x,guess);
	guess = (guess+sqrt)>>1;
	sqrt = FIX16DIVIDE(x,guess);
	guess = (guess+sqrt)>>1;
	sqrt = FIX16DIVIDE(x,guess);
	guess = (guess+sqrt)>>1;
	sqrt = FIX16DIVIDE(x,guess);
	
	return sqrt;
}

quaternion identityQuat(void){
	return identityQuaternion;
}

quaternion multiplyQuat(quaternion * a, quaternion * b){
	quaternion q;
	q.w = MULT(a->w,b->w)-MULT(a->x,b->x)-MULT(a->y,b->y)-MULT(a->z,b->z);
	q.x = MULT(a->w,b->x)+MULT(a->x,b->w)+MULT(a->y,b->z)-MULT(a->z,b->y);
	q.y = MULT(a->w,b->y)-MULT(a->x,b->z)+MULT(a->y,b->w)+MULT(a->z,b->x);
	q.z = MULT(a->w,b->z)+MULT(a->x,b->y)-MULT(a->y,b->x)+MULT(a->z,b->w);
	
	normalizeQuaternion(&q);
	
	return q;
}

quaternion quatFromAngles(fix16 x, fix16 y, fix16 z,fix16 theta){
	quaternion q;
	
	fix16 total = sqrtFix16(MULT(x,x)+MULT(y,y)+MULT(z,z));
	
	x = FIX16DIVIDE(x,total);
	y = FIX16DIVIDE(y,total);
	z = FIX16DIVIDE(z,total);
	
	
	q.w = cosFix16(theta>>1);
	theta = sinFix16(theta>>1);
	
	
	
	q.x = MULT(theta,(x));
	q.y = MULT(theta,(y));
	q.z = MULT(theta,(z));
	
	
	
	normalizeQuaternion(&q);
	
	return q;
}

void normalizeQuaternion(quaternion * a){
	fix16 total = MULT(a->x,a->x)+MULT(a->y,a->y)+MULT(a->z,a->z)+MULT(a->w,a->w);
	if(total<63||total>65){
	total = sqrtFix16(total);
	a->x = FIX16DIVIDE(a->x,total);
	a->y = FIX16DIVIDE(a->y,total);
	a->z = FIX16DIVIDE(a->z,total);
	a->w = FIX16DIVIDE(a->w,total);
	}
}


void setCurrentRotation(quaternion * a){
	fix16 xy = MULT(a->x,a->y)<<1;
	fix16 xz = MULT(a->x,a->z)<<1;
	fix16 wx = MULT(a->w,a->x)<<1;
	fix16 wy = MULT(a->w,a->y)<<1;
	fix16 wz = MULT(a->w,a->z)<<1;
	fix16 yz = MULT(a->y,a->z)<<1;
	fix16 ww = MULT(a->w,a->w);
	fix16 xx = MULT(a->x,a->x);
	fix16 yy = MULT(a->y,a->y);
	fix16 zz = MULT(a->z,a->z);
	
	currentMatrix[0] = ww+xx-yy-zz;
	currentMatrix[1] = xy-wz;
	currentMatrix[2] = xz+wy;
	//currentMatrix[3] = 0;
	
	currentMatrix[4] = xy+wz;
	currentMatrix[5] = ww-xx+yy-zz;
	currentMatrix[6] = yz+wx;
	//currentMatrix[7] = 0;
	
	currentMatrix[8] = xz-wy;
	currentMatrix[9] = yz-wx;
	currentMatrix[10] = ww-xx-yy+zz;
	//currentMatrix[11] = 0;
	
	currentMatrix[12] = 0;
	currentMatrix[13] = 0;
	currentMatrix[14] = 0;
	currentMatrix[15] = FIX16(1);
}

void translateMatrix(fix16 x, fix16 y, fix16 z){
	currentMatrix[3] += x;
	currentMatrix[7] += y;
	currentMatrix[11] += z;
}

void translateTo(fix16 x, fix16 y, fix16 z){
	currentMatrix[3] = x;
	currentMatrix[7] = y;
	currentMatrix[11] = z;
}
void rotateMatrix(quaternion rot){

}

void multiplyMatrices(fix16 * a, fix16 * b, fix16 * c);


void eulerRotationMatrix(fix16 x, fix16 y, fix16 z){
	
	fix16 mat0 [16];
	fix16 mat1 [16];
	
	mat0[0] = FIX16(1);
	mat0[1] = 0;
	mat0[2] = 0;
	mat0[3] = 0;
	mat0[4] = 0;
	mat0[5] = cosFix16(x);
	mat0[6] = sinFix16(x);
	mat0[7] = 0;
	mat0[8] = 0;
	mat0[9] = -sinFix16(x);
	mat0[10] = cosFix16(x);
	mat0[11] = 0;
	mat0[12] = 0;
	mat0[13] = 0;
	mat0[14] = 0;
	mat0[15] = FIX16(1);
	
	mat1[0] = cosFix16(y);
	mat1[1] = 0;
	mat1[2] = -sinFix16(y);
	mat1[3] = 0;
	mat1[4] = 0;
	mat1[5] = FIX16(1);
	mat1[6] = 0;
	mat1[7] = 0;
	mat1[8] = sinFix16(y);
	mat1[9] = 0;
	mat1[10] = cosFix16(y);
	mat1[11] = 0;
	mat1[12] = 0;
	mat1[13] = 0;
	mat1[14] = 0;
	mat1[15] = FIX16(1);
	
	multiplyMatrices(mat0,mat1,currentMatrix);
	
	mat1[0] = cosFix16(z);
	mat1[1] = sinFix16(z);
	mat1[2] = 0;
	mat1[3] = 0;
	mat1[4] = -sinFix16(z);
	mat1[5] = cosFix16(z);
	mat1[6] = 0;
	mat1[7] = 0;
	mat1[8] = 0;
	mat1[9] = 0;
	mat1[10] = FIX16(1);
	mat1[11] = 0;
	mat1[12] = 0;
	mat1[13] = 0;
	mat1[14] = 0;
	mat1[15] = FIX16(1);
	
	multiplyMatrices(currentMatrix,mat1,mat0);
	for(int i=0;i!=16;i++){
	currentMatrix[i] = mat0[i];
	}
	
	
	
	
	
}

void multiplyMatrices(fix16 * a, fix16 * b, fix16 * c){
	c[0] = MULT(a[0],b[0])+MULT(a[1],b[4])+MULT(a[2],b[8])+MULT(a[3],b[12]);
	c[1] = MULT(a[0],b[1])+MULT(a[1],b[5])+MULT(a[2],b[9])+MULT(a[3],b[13]);
	c[2] = MULT(a[0],b[2])+MULT(a[1],b[6])+MULT(a[2],b[10])+MULT(a[3],b[14]);
	c[3] = MULT(a[0],b[3])+MULT(a[1],b[7])+MULT(a[2],b[11])+MULT(a[3],b[15]);

	c[4] = MULT(a[4],b[0])+MULT(a[5],b[4])+MULT(a[6],b[8])+MULT(a[7],b[12]);
	c[5] = MULT(a[4],b[1])+MULT(a[5],b[5])+MULT(a[6],b[9])+MULT(a[7],b[13]);
	c[6] = MULT(a[4],b[2])+MULT(a[5],b[6])+MULT(a[6],b[10])+MULT(a[7],b[14]);
	c[7] = MULT(a[4],b[3])+MULT(a[5],b[7])+MULT(a[6],b[11])+MULT(a[7],b[15]);
	
	c[8] = MULT(a[8],b[0])+MULT(a[9],b[4])+MULT(a[10],b[8])+MULT(a[11],b[12]);
	c[9] = MULT(a[8],b[1])+MULT(a[9],b[5])+MULT(a[10],b[9])+MULT(a[11],b[13]);
	c[10] = MULT(a[8],b[2])+MULT(a[9],b[6])+MULT(a[10],b[10])+MULT(a[11],b[14]);
	c[11] = MULT(a[8],b[3])+MULT(a[9],b[7])+MULT(a[10],b[11])+MULT(a[11],b[15]);
	
	c[12] = MULT(a[12],b[0])+MULT(a[13],b[4])+MULT(a[14],b[8])+MULT(a[15],b[12]);
	c[13] = MULT(a[12],b[1])+MULT(a[13],b[5])+MULT(a[14],b[9])+MULT(a[15],b[13]);
	c[14] = MULT(a[12],b[2])+MULT(a[13],b[6])+MULT(a[14],b[10])+MULT(a[15],b[14]);
	c[15] = MULT(a[12],b[3])+MULT(a[13],b[7])+MULT(a[14],b[11])+MULT(a[15],b[15]);
}


vector transformVector(vector in){
	vector out;
	out.x = MULT(in.x,currentMatrix[0])+MULT(in.y,currentMatrix[1])+MULT(in.z,currentMatrix[2])/*+currentMatrix[3]*/;
	out.y =  MULT(in.x,currentMatrix[4])+MULT(in.y,currentMatrix[5])+MULT(in.z,currentMatrix[6])/*+currentMatrix[7]*/;
	out.z =  MULT(in.x,currentMatrix[8])+MULT(in.y,currentMatrix[9])+MULT(in.z,currentMatrix[10])/*+currentMatrix[11]*/;
	out.w =  /*MULT(in.x,currentMatrix[12])+MULT(in.y,currentMatrix[13])+MULT(in.z,currentMatrix[14])+*/in.w;
	if(out.w){
		out.x+=currentMatrix[3];
		out.y+=currentMatrix[7];
		out.z+=currentMatrix[11];
	}
	return out;
}

void perspectiveMatrix(void){
	currentMatrix[14] = -FIX16(1);
	currentMatrix[15] = 0;
}

