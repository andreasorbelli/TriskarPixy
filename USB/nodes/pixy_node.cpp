
#include "ch.h"
#include "hal.h"
#include "r2p/Middleware.hpp"
#include <r2p/msg/pixy.hpp>
#include <r2p/msg/pixyBuffer.hpp>
#include <r2p/msg/proximity.hpp>

#define PIXY_ARRAYSIZE              100
#define PIXY_START_WORD             0xaa55
#define PIXY_START_WORD_CC          0xaa56
#define PIXY_START_WORDX            0x55aa

typedef enum
{
	NORMAL_BLOCK,
	CC_BLOCK // color code block
} BlockType;

static BlockType g_blockType; // use this to remember the next object block type between function calls
static int g_skipStart = 0;

//get 32-bit word from SD3
uint16_t getWord(void){
	// this routine assumes little endian
	uint16_t w;
	uint8_t c;

	//get chars ( 16 bit)
	c = sdGetTimeout(&SD3, MS2ST(100));
	w = sdGetTimeout(&SD3, MS2ST(100));

	//convert chars and convert in 32 bit word
	w <<= 8;
	w |= c;
	return w;
}

//return 1 when pixy is in syncro
int getStart(void)
{
	uint16_t w, lastw;

	lastw = 0xffff; // some inconsequential initial value

	while(1){
		w = getWord();
		if (w==0 && lastw==0)
			return 0; // in I2C and SPI modes this means no data, so return immediately
		else if (w==PIXY_START_WORD && lastw==PIXY_START_WORD)
		{
			g_blockType = NORMAL_BLOCK; // remember block type
			return 1; // code found!
		}
		else if (w==PIXY_START_WORD_CC && lastw==PIXY_START_WORD)
		{
			g_blockType = CC_BLOCK; // found color code block
			return 1;
		}
		else if (w==PIXY_START_WORDX) // this is important, we might be juxtaposed
			sdGetTimeout(&SD3, MS2ST(100)); // we're out of sync! (backwards)
		lastw = w; // save
	}
}
//main node pixy
msg_t pixy_node(void *arg) {

	//standard routine for nodes
	r2p::Publisher<r2p::PixyMsg> pixy_pub;
	r2p::Node node("pixy");
	r2p::PixyMsg * msgp;
	sdStart(&SD3, NULL);
	node.advertise(pixy_pub,"pixy");


	int curr=0;
	for (;;) {
		curr = getStart();// if is 1 is in syncro
		if (curr) // two start codes means start of new frame
		{
			bool found=false;
			int checksum = getWord();
			if (checksum==PIXY_START_WORD) // we've reached the beginning of the next frame
			{
				g_skipStart = 1;
				g_blockType = NORMAL_BLOCK;
				found=true;
			}
			else if (checksum==PIXY_START_WORD_CC)
			{
				g_skipStart = 1;
				g_blockType = CC_BLOCK;
				found=true;
			}
			else if (checksum==0)
				found=true;

			if(!found){
				uint16_t w,sum=0;
				w = getWord();
				int sig = w;
				sum = sum + sig;
				w = getWord();
				int x=w;
				sum = sum + x;
				w = getWord();
				int y=w;
				sum = sum + y;
				w = getWord();
				int width=w;
				sum = sum + width;
				w = getWord();
				int height=w;
				sum = sum + height;
				if(checksum==sum)
				{
					if (pixy_pub.alloc(msgp)) {
						msgp->checksum =checksum;
						msgp->signature=sig;
						msgp->x = x;
						msgp->y = y;
						msgp->width = width;
						msgp->height = height;
						pixy_pub.publish(*msgp);
					}
				}
			}
		}
	}
	return CH_SUCCESS;
}





