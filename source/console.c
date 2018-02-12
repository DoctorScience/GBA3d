
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <stdio.h>
#include "pal.h"



typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;

#include "math.h"

#define VRAM 0x6000000
#define VRAM2 0x600a000
#define BG_PAL  0x5000000
#define VIDC_BASE 0x4000000


volatile u16 * curBuf;
u8 activeFrameBuffer;



u8 drawColor;
u8 collumn;
u8 row;
u8 minCollumn;
u8 maxCollumn;
u8 minRow;
u8 maxRow;

typedef struct triangle{
	u8 color;
	fix16 x1;
	fix16 x2;
	fix16 x3;
	fix16 y1;
	fix16 y2;
	fix16 y3;
	fix16 z;
} tri;


tri trique [512];
tri * currentTri;
u16 triCount;
u16 orderQue [512];

vector vectorBuffer [256];
vector lighting;
vector normalBuffer [256];
void resetQue(void){
	/*for(u16 i=0;i!=512;i++){  
		orderQue[i] = i;
	}*/
	triCount = 0;
	currentTri = &trique[0];
}

typedef struct model{
	vector * vertices;
	vector * normals;
	u8 vertexCount;
	u8 * indices;
	u8 * colors;
	u16 triCount;
} model;

const vector cubeVerts [] = {
	{-128,-128,-128,64},
	{-128,-128,128,64},
	{128,-128,128,64},
	{128,-128,-128,64},
	{0,128,0,64}
};



u8 cubeIndex [] = {
	0,1,2,//bottom face
	2,3,0,
	
	0,1,4,//left face
	
	1,2,4,//back face
	
	2,3,4,//right face
	
	3,0,4//front face
};

u8 cubeColors [] = {
	1,1,
	2,
	3,
	2,
	3
};

void initGFX(void){
	u16 writeShort;
	
	*((unsigned int *)(VIDC_BASE)) = 4|0x400;
	
	for(u8 i=0;i!=240;i++){
		((u16 *)BG_PAL)[i] = pallette[i]; 
	}
	for(u16 i2 =0;i2!=(38400/2);i2++){
		((u16 *)VRAM)[i2] = 0;
	} 
	minCollumn = 20;
	minRow = 0;
	maxCollumn = 30;
	maxRow = 20;
	
	row = minRow;
	collumn = minCollumn;
	
	//drawX = 0;
	curBuf = VRAM;
	activeFrameBuffer = 0;
	
	for(u16 i=0;i!=512;i++){
		orderQue[i] = i;
	}
}
	
PUTIWRAM static inline void set(s16 x, s16 y){
	if(x>=240||y>=160||x<0||y<0) return;
	
	u16 writeShort = curBuf[(x>>1)+y*120];
	if(x&1){
		
		writeShort = ((writeShort&255))|(drawColor<<8);
	}else{
		writeShort = (writeShort&(255<<8))|drawColor;
	}
	curBuf[(x>>1)+y*120] = writeShort;
}

static inline void setLine(s16 x1, s16 x2, s16 y);
	
void setRect(u8 color, u16 x1, u16 x2, u16 y1, u16 y2){
	u16 writeShort = color|color<<8;
	u16 writeY;
	drawColor = color;
	if(x1&1){
		for(u16 y=y1;y!=y2;y++){
			set(x1,y);
		}
	}
	
	if((x2-1)&1){
		for(u16 y=y1;y!=y2;y++){
			set((x2-1),y);
		}
	}
	writeY = y1*120;
	for(u16 y=y1;y!=y2;y++){
		setLine(x1,x2,y);
	}
}	

PUTIWRAM void clearScreen(){
	
	u32 * screenBuf = ((u32*)curBuf);
	for(u32 i=0;i!=(240*40);i++){
		(*screenBuf) = 0;
		screenBuf++;
	}
	
	
}
s16 tempy;
PUTIWRAM inline void setLine(s16 x1, s16 x2,s16 y){
	
	if(y>=160||y<0) return;
	//x1++;
	if(x1>x2){
		tempy = x1;
		x1 = x2;
		x2 = tempy;
	}
	
	if(x2<0)return;
	if(x1>239) return;
	if(x1<0) x1 = 0;
	
	//x2+=2;
	if(x2>239) x2 = 239;
	
	u16 writeShort = drawColor|(drawColor<<8);
	u16 writeY;
	u32 writeInt = writeShort|(writeShort<<16);
	
	
	/*if(x1&3){
		set(color,x1,y);
		x1++;
		
		if(x1==x2) return;
		
		
		if((x2-1)&3){
			set(color,x2,y);
			x2--;
			if(x1==x2) return;
		}
	}
	
	while((x2-1)&3){
			set(color,x2,y);
			x2--;
			if(x1==x2) return;
	}*/
	
	/*
	while(x2&3){
		set(color,x2,y);
		x2--;
	}*/
	
	
	if( (x1-1)&1){
		set(x1-1,y);
	}
	
//	x2++;
	if(!((x2-1)&1)){
		set((x2-1),y);
	}
	
	x1 = x1>>1;
	x2 = x2>>1;
	
	
	writeY = y*120;
	
	curBuf[x1+writeY] = writeShort;
	curBuf[x2-1+writeY] = writeShort;
	x1++;
	x1 = x1>>1;
	x2 = x2>>1;
	writeY = writeY>>1;
	
	if(x1>=x2) return;
	
	for(u16 x=x1;x!=x2;x++){
		((u32 *)curBuf)[x+writeY] = writeInt;
	}
}

PUTIWRAM void transformModel(model * m){
	/*for(u16 i=0;i!=m->vertexCount;i++){
		vectorBuffer[i] = transformVector(m->vertices[i]);
		vectorBuffer[i].x = FIX16DIVIDE(vectorBuffer[i].x,vectorBuffer[i].z)+120;
		vectorBuffer[i].y = FIX16DIVIDE(vectorBuffer[i].y,vectorBuffer[i].z)+80;
	}*/
	transformVectors(m->vertices,vectorBuffer,m->vertexCount);
	transformVectors(m->normals,normalBuffer,m->triCount);
}

PUTIWRAM void registerModel(model * m){
	transformModel(m);
	u16 triInd = 0;
	fix16 z;
	s16 bright;
	for(u16 i=0;i!=m->triCount;i++){
		z = (vectorBuffer[m->indices[triInd]].z);
		
		currentTri->x1 = vectorBuffer[m->indices[triInd]].x;
		currentTri->y1 = vectorBuffer[m->indices[triInd]].y;
		
		//bright = (normalBuffer[i].z/32)+2;
		
		if(normalBuffer[i].z<0){
		
		bright = ((MULT(normalBuffer[i].x,lighting.x)+MULT(normalBuffer[i].y,lighting.y)+MULT(normalBuffer[i].z,lighting.z)+64)/32);
		//bright = 2;
		
		if(bright <0) bright = 0;
		if(bright>3) bright = 3;
		//bright++;
		currentTri->color = (m->colors[i]&252)|bright;
		//currentTri->color = m->colors[i];
		
	
		triInd++;
		//if(z<vectorBuffer[m->indices[triInd]].z) z = vectorBuffer[m->indices[triInd]].z;
		z+=(vectorBuffer[m->indices[triInd]].z);
		
		currentTri->x2 = vectorBuffer[m->indices[triInd]].x;
		currentTri->y2 = vectorBuffer[m->indices[triInd]].y;
		
		triInd++;
		//if(z<vectorBuffer[m->indices[triInd]].z) z = vectorBuffer[m->indices[triInd]].z;
		z+=(vectorBuffer[m->indices[triInd]].z);
		
		currentTri->x3 = vectorBuffer[m->indices[triInd]].x;
		currentTri->y3 = vectorBuffer[m->indices[triInd]].y;
		
		triInd++;
		}else{
			triInd+=3;
			currentTri->x1 = 0;
			currentTri->x2 = 0;
			currentTri->x3 = 0;
			currentTri->y1 = 0;
			currentTri->y2 = 0;
			currentTri->y3 = 0;
			
		}
		if(z>0){
		currentTri->z = z;
		triCount++;
		currentTri = &trique[triCount];
		}
	}
}

u16 holder;

void drawTriangle(tri * in);
inline void drawTriangleNew(tri * in);

u8 currentBufNum;
void swapBuffers(void){
	currentBufNum = !currentBufNum;
	if(currentBufNum){
		curBuf = VRAM2;
		*((unsigned int *)(VIDC_BASE)) = 4|0x400;
	}else{
		curBuf = VRAM;
		*((unsigned int *)(VIDC_BASE)) = 4|0x400|0x10;
	}
}


PUTIWRAM void renderQue(void){
	u8 activity = 1;
	u16 max = 0;
	s16 min = 0;
	u16 pushEndHolder;


	
	
	for(u16 i = 0;i!=512;i++){
		if(orderQue[i]<triCount) max = i;
	}
	
	
	
	/*for(u16 i =0;i<max;i++){
			if(orderQue[i]>=triCount){
				pushEndHolder = orderQue[i];
				for(u16 j=i;j!=511;j++){
					orderQue[j] = orderQue[j+1];
				}
				orderQue[511] = pushEndHolder;
				max--;
			}
	}*/
	if(max>triCount){
	while(max>triCount){
		for(u16 i =0;i<max;i++){
			if(orderQue[i]>=triCount){
				pushEndHolder = orderQue[i];
				for(u16 j=i;j!=max;j++){
					orderQue[j] = orderQue[j+1];
				}
				orderQue[max] = pushEndHolder;
				max--;
				i--;
			}
		}
	}
	
	for(u16 i=triCount;i!=512;i++){
		orderQue[i] = i;
	}
	}
	
	
	min = 0;
	s16 newmin = -10;
	
	while(activity){
		activity = 0;
		newmin = -10;
		min++;
		for(u16 i=min;i!=triCount;i++){
			if(trique[orderQue[i]].z>trique[orderQue[i-1]].z){
				activity = 1;
				holder = orderQue[i];
				orderQue [i] = orderQue[i-1];
				orderQue[i-1] = holder;
				
				//if(i-2<min) min = i-2;
				
				if(newmin == -10) newmin = i-2;
				
				
			//	holder = trique[i];
			//	trique[i] = trique[i-1];
			//	trique[i-1] = holder;
			}
		}
		min = newmin;
		if(min<0) min = 0;
		
	}
	
	for(u16 i=0;i!=triCount;i++){
		drawTriangleNew(&trique[orderQue[i]]);
	}
	
	
	
}



void recursiveDrawTriangle(s16 x1, s16 x2, s16 y1, s16 x3, s16 y2);

s16 drawHolder;
fix16 xStepA;
fix16 xStepB;
PUTIWRAM inline void drawTriangleNew(tri * in){
	fix16 xStepA;
	fix16 xStepB;
	fix16 xa;
	fix16 xb;
	u16 yNow;
	u16 yGoal;
	
	
	
	drawColor = in->color;
	
	if(in->x1==0&&in->x2==0&&in->x3==0) return;
	
	//gotta sort that triangle
	if(in->y3>in->y2){
		drawHolder = in->y2;
		in->y2 = in->y3;
		in->y3 = drawHolder;
		drawHolder = in->x2;
		in->x2 = in->x3;
		in->x3 = drawHolder;
	}
	
	
	if(in->y2>in->y1){
		drawHolder = in->y2;
		in->y2 = in->y1;
		in->y1 = drawHolder;
		drawHolder = in->x2;
		in->x2 = in->x1;
		in->x1 = drawHolder;
	}
	
	if(in->y3>in->y2){
		drawHolder = in->y2;
		in->y2 = in->y3;
		in->y3 = drawHolder;
		drawHolder = in->x2;
		in->x2 = in->x3;
		in->x3 = drawHolder;
	}
	
	if(in->y3<0) in->y3 = 0;
	if(in->y2<0) in->y2 = 0;
	if(in->y1<0) in->y1 = 0;
	
	if(in->y3>239) in->y3 = 159;
	if(in->y2>239) in->y2 = 159;
	if(in->y1>239) in->y1 = 159;
	
	in->y1 = 159-in->y1;
	in->y2 = 159-in->y2;
	in->y3 = 159-in->y3;
	
	
	
	
	drawHolder = in->x1+(in->y2-in->y1)*(in->x3-in->x1)/(in->y3-in->y1);

	
	//updraw
	xa = FIX16(in->x2);
	xb = FIX16(drawHolder);
	
	yNow = in->y2;
	yGoal = in->y3;
	xStepA = FIX16DIVIDE((FIX16(in->x3-in->x2)),FIX16(yGoal-yNow));
	xStepB = FIX16DIVIDE((FIX16(in->x3-drawHolder)),FIX16(yGoal-yNow));

	while(yNow<yGoal){
		setLine(FIX16TOINT(xa),FIX16TOINT(xb),yNow);
		xa+=xStepA;
		xb+=xStepB;
		yNow++;
	}
	
	
		xa = FIX16(in->x2);
	xb = FIX16(drawHolder);
	
	yNow = in->y2;
	yGoal = in->y1;
	xStepA = FIX16DIVIDE((FIX16(in->x1-in->x2)),FIX16(yGoal-yNow));
	xStepB = FIX16DIVIDE((FIX16(in->x1-drawHolder)),FIX16(yGoal-yNow));

	yNow--;
	while(yNow>yGoal){
		setLine(FIX16TOINT(xa),FIX16TOINT(xb),yNow);
		xa-=xStepA;
		xb-=xStepB;
		yNow--;
	}
	
	
	
	
	
	
	

	
}

void drawTriangle(tri * in){
	drawColor = in->color;
	
	//gotta sort that triangle
	if(in->y3>in->y2){
		drawHolder = in->y2;
		in->y2 = in->y3;
		in->y3 = drawHolder;
		drawHolder = in->x2;
		in->x2 = in->x3;
		in->x3 = drawHolder;
	}
	
	
	if(in->y2>in->y1){
		drawHolder = in->y2;
		in->y2 = in->y1;
		in->y1 = drawHolder;
		drawHolder = in->x2;
		in->x2 = in->x1;
		in->x1 = drawHolder;
	}
	
	if(in->y3>in->y2){
		drawHolder = in->y2;
		in->y2 = in->y3;
		in->y3 = drawHolder;
		drawHolder = in->x2;
		in->x2 = in->x3;
		in->x3 = drawHolder;
	}
	
	if(in->y3<0) in->y3 = 0;
	if(in->y2<0) in->y2 = 0;
	if(in->y1<0) in->y1 = 0;
	
	if(in->y3>239) in->y3 = 159;
	if(in->y2>239) in->y2 = 159;
	if(in->y1>239) in->y1 = 159;
	
	in->y1 = 159-in->y1;
	in->y2 = 159-in->y2;
	in->y3 = 159-in->y3;
	
	
	if(in->y1==in->y2){
		setLine(in->x1,in->x2,in->y1);
		set(in->x3,in->y3);
		set(in->x3-1,in->y3);
		recursiveDrawTriangle(in->x1,in->x2,in->y1,in->x3,in->y3);
		return;
	}
	
	if(in->y3==in->y2){
		setLine(in->x2,in->x3,in->y2);
		set(in->x1,in->y1);
		set(in->x1-1,in->y1);
		recursiveDrawTriangle(in->x3,in->x2,in->y3,in->x1,in->y1);
		return;
	}
	
	
	drawHolder = in->x1+(in->y2-in->y1)*(in->x3-in->x1)/(in->y3-in->y1);
	setLine(in->x2,drawHolder,in->y2);
	/*set(drawColor,in->x1,in->y1);
	set(drawColor,in->x3,in->y3);
	set(drawColor,in->x1-1,in->y1);
	set(drawColor,in->x3-1,in->y3);*/
	
	recursiveDrawTriangle(in->x2,drawHolder,in->y2,in->x1,in->y1);
	recursiveDrawTriangle(in->x2,drawHolder,in->y2,in->x3,in->y3);
	
}


void recursiveDrawQuad(s16 x1,  s16 x2, s16 y1, s16 x3, s16 x4, s16 y2){
	u16 x5 = (x1+x3)>>1;
	u16 x6 = (x2+x4)>>1;
	u16 y3 = (y1+y2)>>1;
	
	if(y3==y1||y3==y2||y1==y2) return;
	
	setLine(x5,x6,y3);
	//set(drawColor,x5,y3);
	//set(drawColor,x6,y3);
	if(y3>0||y1>0){
	recursiveDrawQuad(x1,x2,y1,x5,x6,y3);
	}
	if(y2>0||y3>0){
	recursiveDrawQuad(x5,x6,y3,x3,x4,y2);
	}
}

void recursiveDrawTriangle(s16 x1, s16 x2, s16 y1, s16 x3, s16 y2){
	u16 x4 = (x1+x3)>>1;
	u16 x5 = (x2+x3)>>1;
	u16 y3 = (y1+y2)>>1;
	if(y3==y1||y3==y2||y1==y2) return;
	
	setLine(x4,x5,y3);
	//set(drawColor,x4,y3);
	//set(drawColor,x5,y3);
	
	
	
	if(y3>0||y2>0){
	recursiveDrawTriangle(x4,x5,y3,x3,y2);
	}
	if(y3>0||y1>0){
	recursiveDrawQuad(x1,x2,y1,x4,x5,y3);
	}
}

int meta;

volatile u32 frameCounter = 0;
void irqCustom(void){
	frameCounter++;
}

u32 start;
u32 frames;
u32 fps;

u32 lastFrame;

void drawTime(u16 y){
	u32 time = frameCounter-start;
	if(time<120){
		for(u16 i=0;i<time;i++){
			drawColor = 19;
			set(i<<1,150);
		}
	}
	start = frameCounter;  
	
}
 
#include "defualt.h"
u16 useless;

u32 frameTimes [16];

int main(void) {
	model pyramid;
	
	pyramid.vertices = defualtVerts;
	pyramid.vertexCount = defualtvertCount+1;
	pyramid.triCount = defualttriCount;
	//pyramid.triCount = 1;
	pyramid.colors = defualtColors;
	pyramid.indices = defualtTriangles;
	pyramid.normals = defualtNormals;
	
	quaternion q;
	quaternion q2;
	quaternion q3;

	
	irqInit();
	irqSet(IRQ_VBLANK,irqCustom);
	irqEnable(IRQ_VBLANK);
	
	
	
	
	initGFX();
	meta = FIX16(1);
	q = identityQuat();
	translateTo(0,0,5156);
	start = frameCounter;
	
	lighting.x = 0;
	lighting.y = 0;
	lighting.z = 0;
	lighting.w = 0;
	
	
	
	while (1) {  
		
		
		
		lastFrame = frameCounter;
		lighting.x = sinFix16(meta<<1);
		lighting.z = cosFix16(meta<<1);
		clearScreen(); 
		q = quatFromAngles(FIX16(1),FIX16(1),0,meta);
		q2 = quatFromAngles(FIX16(1),0,0,512);
		q3 = multiplyQuat(&q2,&q);
		
		
		setCurrentRotation(&q);
		resetQue();
		translateTo(0,0,856);
		registerModel(&pyramid);
		translateMatrix(1055,0,500);
		//if((meta&1023)>64){
		registerModel(&pyramid);
		translateTo(-2400,30,1826);
		setCurrentRotation(&q3);
		registerModel(&pyramid);
		
	//}
		renderQue();
		frames++;
		 
		
		for(u16 i =0 ;i!=15;i++){
			frameTimes[i] =frameTimes[i+1];
		}
		
		frameTimes[15] = frameCounter;
		
		 
		fps = ((60*16)/(frameTimes[15]-frameTimes[0]));
		if(fps<60){
			for(u16 i=0;i<fps;i++){
				drawColor = 19;
				set(i<<2,150);
			}
		}
		meta+=7;
		
		
		while(frameCounter-lastFrame<=2){
			VBlankIntrWait();
		}
		
		swapBuffers();
			
		
	}
}


