
#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <stdio.h>
#include "pal.h"



typedef unsigned char u8;
typedef unsigned short u16;

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
	
void set(u8 color,u16 x, u16 y){
	if(x>=240||y>=160) return;
	
	u16 writeShort = curBuf[(x>>1)+y*120];
	if(x&1){
		
		writeShort = ((writeShort&255))|(color<<8);
	}else{
		writeShort = (writeShort&(255<<8))|color;
	}
	curBuf[(x>>1)+y*120] = writeShort;
}

void setLine(u8 color, u16 x1, u16 x2, u16 y);
	
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
	
void setLine(u8 color, u16 x1, u16 x2,u16 y){
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





int meta;

int main(void) {
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
	
	
	
	quaternion q;

			

	
	irqInit();
	irqEnable(IRQ_VBLANK);
	initGFX();
	meta = FIX16(1);
	q = identityQuat();
	translateTo(0,0,128);
	while (1) {
		/*VBlankIntrWait();
		VBlankIntrWait();
		
		VBlankIntrWait();
		VBlankIntrWait();
		VBlankIntrWait();
		VBlankIntrWait();
		
		VBlankIntrWait();
		VBlankIntrWait();*/
		//setRect(0,0,240,0,160);
			q = quatFromAngles(0,FIX16(1),0,meta);
			setCurrentRotation(&q);
			//translateMatrix(0,0,128);
			
			for(int j=0;j!=4;j++){
				v2 = transformVector(v[j]);
				
				v2.x = FIX16DIVIDE(v2.x,v2.z);
				v2.y = FIX16DIVIDE(v2.y,v2.z);
				set(3,v2.x+120,v2.y+80);
			
			}
			
			
		meta++;
	}
}


