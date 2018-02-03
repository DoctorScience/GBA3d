
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

void resetQue(void){
	for(u16 i=0;i!=512;i++){
		orderQue[i] = i;
	}
	triCount = 0;
	currentTri = &trique[0];
}

typedef struct model{
	vector * vertices;
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
}
	
void set(u8 color,s16 x, s16 y){
	if(x>=240||y>=160||x<0||y<0) return;
	
	u16 writeShort = curBuf[(x>>1)+y*120];
	if(x&1){
		
		writeShort = ((writeShort&255))|(color<<8);
	}else{
		writeShort = (writeShort&(255<<8))|color;
	}
	curBuf[(x>>1)+y*120] = writeShort;
}

void setLine(u8 color, s16 x1, s16 x2, s16 y);
	
void setRect(u8 color, u16 x1, u16 x2, u16 y1, u16 y2){
	u16 writeShort = color|color<<8;
	u16 writeY;
	if(x1&1){
		for(u16 y=y1;y!=y2;y++){
			set(color,x1,y);
		}
	}
	
	if((x2-1)&1){
		for(u16 y=y1;y!=y2;y++){
			set(color,(x2-1),y);
		}
	}
	writeY = y1*120;
	for(u16 y=y1;y!=y2;y++){
		setLine(color,x1,x2,y);
	}
}	
s16 tempy;
void setLine(u8 color, s16 x1, s16 x2,s16 y){
	
	if(y>=160||y<0) return;
	
	if(x1>x2){
		tempy = x1;
		x1 = x2;
		x2 = tempy;
	}
	
	if(x1<0) x1 = 0;
	if(x2<0)return;
	if(x1>239) return;
	x2++;
	if(x2>239) x2 = 239;
	
	u16 writeShort = color|(color<<8);
	u16 writeY;
	if(x1&1){
		set(color,x1,y);
	}
	
	if((x2-1)&1){
		set(color,(x2-1),y);
	}
	
	x1 = x1>>1;
	x2 = x2>>1;
	writeY = y*120;
	for(u16 x=x1;x!=x2;x++){
		curBuf[x+writeY] = writeShort;
	}
}

void transformModel(model * m){
	for(u16 i=0;i!=m->vertexCount;i++){
		vectorBuffer[i] = transformVector(m->vertices[i]);
		vectorBuffer[i].x = FIX16DIVIDE(vectorBuffer[i].x,vectorBuffer[i].z)+120;
		vectorBuffer[i].y = FIX16DIVIDE(vectorBuffer[i].y,vectorBuffer[i].z)+80;
	}
}

void registerModel(model * m){
	transformModel(m);
	u16 triInd = 0;
	fix16 z;
	
	for(u16 i=0;i!=m->triCount;i++){
		z = vectorBuffer[m->indices[triInd]].z;
		currentTri->x1 = vectorBuffer[m->indices[triInd]].x;
		currentTri->y1 = vectorBuffer[m->indices[triInd]].y;
		currentTri->color = m->colors[i];
		
	
		triInd++;
		if(z>vectorBuffer[m->indices[triInd]].z) z = vectorBuffer[m->indices[triInd]].z;
		
		currentTri->x2 = vectorBuffer[m->indices[triInd]].x;
		currentTri->y2 = vectorBuffer[m->indices[triInd]].y;
		
		triInd++;
		if(z>vectorBuffer[m->indices[triInd]].z) z = vectorBuffer[m->indices[triInd]].z;
		
		currentTri->x3 = vectorBuffer[m->indices[triInd]].x;
		currentTri->y3 = vectorBuffer[m->indices[triInd]].y;
		
		triInd++;
	
		if(z>0){
		currentTri->z = z;
		triCount++;
		currentTri = &trique[triCount];
		}
	}
}

u16 holder;

void drawTriangle(tri * in);

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

void renderQue(void){
	u8 activity = 1;
	while(activity){
		activity = 0;
		for(u16 i=1;i!=triCount;i++){
			if(trique[orderQue[i]].z>trique[orderQue[i-1]].z){
				activity = 1;
				holder = orderQue[i];
				orderQue [i] = orderQue[i-1];
				orderQue[i-1] = holder;
				
			//	holder = trique[i];
			//	trique[i] = trique[i-1];
			//	trique[i-1] = holder;
			}
		}
	}
	
	for(u16 i=0;i!=triCount;i++){
		drawTriangle(&trique[orderQue[i]]);
	}
	
	
}

u8 drawColor;

void recursiveDrawTriangle(s16 x1, s16 x2, s16 y1, s16 x3, s16 y2);

s16 drawHolder;
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
	
	//if(!(in->y1>in->y2&&in->y2>in->y3)) return;
	
	if(in->y3<0) in->y3 = 0;
	if(in->y2<0) in->y2 = 0;
	if(in->y1<0) in->y1 = 0;
	
	if(in->y3>239) in->y3 = 239;
	if(in->y2>239) in->y2 = 239;
	if(in->y1>239) in->y1 = 239;
	
	if(in->y1==in->y2){
		setLine(drawColor,in->x1,in->x2,in->y1);
		recursiveDrawTriangle(in->x1,in->x2,in->y1,in->x3,in->y3);
		return;
	}
	
	if(in->y3==in->y2){
		setLine(drawColor,in->x2,in->x3,in->y2);
		recursiveDrawTriangle(in->x3,in->x2,in->y3,in->x1,in->y1);
		return;
	}
	
	
	
	
	//drawHolder = FIX16(in->x1)+MULT(FIX16DIVIDE((FIX16(in->y2)-FIX16(in->y1)),FIX16(in->y3)-(FIX16(in->y1))),(FIX16(in->x3)-FIX16(in->x1)));
	//drawHolder = FIX16TOINT(drawHolder);
	
	
	
	drawHolder = in->x1+(in->y2-in->y1)*(in->x3-in->x1)/(in->y3-in->y1);
	setLine(drawColor,in->x2,drawHolder,in->y2);
	
	//if(in->y3<0) return;
	
	recursiveDrawTriangle(in->x2,drawHolder,in->y2,in->x1,in->y1);
	recursiveDrawTriangle(in->x2,drawHolder,in->y2,in->x3,in->y3);
	
	
	
}


void recursiveDrawQuad(s16 x1,  s16 x2, s16 y1, s16 x3, s16 x4, s16 y2){
	u16 x5 = (x1+x3)/2;
	u16 x6 = (x2+x4)/2;
	u16 y3 = (y1+y2)/2;
	
	if(y3==y1||y3==y2||y1==y2) return;
	
	setLine(drawColor,x5,x6,y3);
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
	u16 x4 = (x1+x3)/2;
	u16 x5 = (x2+x3)/2;
	u16 y3 = (y1+y2)/2;
	if(y3==y1||y3==y2||y1==y2) return;
	
	setLine(drawColor,x4,x5,y3);
	set(drawColor,x4,y3);
	set(drawColor,x5,y3);
	
	
	
	if(y3>0||y2>0){
	recursiveDrawTriangle(x4,x5,y3,x3,y2);
	}
	if(y3>0||y1>0){
	recursiveDrawQuad(x1,x2,y1,x4,x5,y3);
	}
}

int meta;

int main(void) {
	model pyramid;
	vector v [4];
	vector v2;
	v[0].x = -32;
	v[0].y = -32;
	v[0].z = 0;
	v[0].w = 64;
	
	v[1].x = -32;
	v[1].y = 32;
	v[1].z = 0;
	v[1].w = 64;
	
	v[2].x = 32;
	v[2].y = 32;
	v[2].z = 0;
	v[2].w = 64;
	
	v[3].x = 32;
	v[3].y = -32;
	v[3].z = 0;
	v[3].w = 64;
	
	pyramid.vertices = cubeVerts;
	pyramid.vertexCount = 5;
	pyramid.triCount = 6;
	pyramid.colors = cubeColors;
	pyramid.indices = &cubeIndex[0];
	
	
	quaternion q;
	quaternion q2;
	quaternion q3;

	
	irqInit();
	irqEnable(IRQ_VBLANK);
	initGFX();
	meta = FIX16(1);
	q = identityQuat();
	translateTo(0,0,256);
	while (1) {
		VBlankIntrWait();
		setRect(0,0,239,0,159);
		q = quatFromAngles(0,FIX16(1),0,meta);
		q2 = quatFromAngles(FIX16(1),0,0,512);
		
		q3 = multiplyQuat(&q2,&q);
		
		setCurrentRotation(&q);
			//translateMatrix(0,0,128);
		resetQue();
		translateTo(0,0,256);
		registerModel(&pyramid);
		
		setCurrentRotation(&q3);
		
		translateTo(300,32,380);
		registerModel(&pyramid);
		renderQue();
		swapBuffers();
			/*for(int j=0;j!=4;j++){
				v2 = transformVector(v[j]);
				
				v2.x = FIX16DIVIDE(v2.x,v2.z);
				v2.y = FIX16DIVIDE(v2.y,v2.z);
				set(3,v2.x+120,v2.y+80);
			
			}*/
			
			
		meta+=8;
	}
}


