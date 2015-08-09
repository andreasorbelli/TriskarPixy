
#include "ch.h"
#include "hal.h"
#include "r2p/Middleware.hpp"
#include <r2p/msg/pixy.hpp>
#include <r2p/msg/proximity.hpp>



static int pack_number = 0;

bool sync(char temp[]) {
	char tocmp1='U';
	char tocmp2='ª';
	if(temp[0]==tocmp1 && temp[1]==tocmp2 && temp[2]==tocmp1 && temp[3]==tocmp2)
	{
		return true;
	}

	return false;
}

int toInt(char temp[]) {
	int first;
	int second;

	first = (int)temp[0];
	first = first *16 *16;
	second  = (int)temp[1];

	return first + second;
}


msg_t pixy_node(void *arg) {
	r2p::Publisher<r2p::PixyMsg> pixy_pub;
	r2p::Node node("pixy");
	r2p::PixyMsg * msgp;
	sdStart(&SD3, NULL);
	node.advertise(pixy_pub,"pixy");

	char toCheck [4];
	toCheck[0] = sdGetTimeout(&SD3, MS2ST(100));
	toCheck[1] = sdGetTimeout(&SD3, MS2ST(100));
	toCheck[2] = sdGetTimeout(&SD3, MS2ST(100));

	for (;;) {
		//if (node.spin(r2p::Time::ms(300))) {
		toCheck[3] = sdGetTimeout(&SD3, MS2ST(100));
		if(sync(toCheck)){

			if (pixy_pub.alloc(msgp)) {
				char check_temp[2];
				check_temp[1]=sdGetTimeout(&SD3, MS2ST(100));
				check_temp[0]=sdGetTimeout(&SD3, MS2ST(100));

				char Sig_temp[2];
				Sig_temp[1]=sdGetTimeout(&SD3, MS2ST(100));
				Sig_temp[0]=sdGetTimeout(&SD3, MS2ST(100));

				char x_temp[2];
				x_temp[1]=sdGetTimeout(&SD3, MS2ST(100));
				x_temp[0]=sdGetTimeout(&SD3, MS2ST(100));

				char y_temp[2];
				y_temp[1]=sdGetTimeout(&SD3, MS2ST(100));
				y_temp[0]=sdGetTimeout(&SD3, MS2ST(100));

				char width_temp[2];
				width_temp[1]=sdGetTimeout(&SD3, MS2ST(100));
				width_temp[0]=sdGetTimeout(&SD3, MS2ST(100));

				char height_temp[2];
				height_temp[1]=sdGetTimeout(&SD3, MS2ST(100));
				height_temp[0]=sdGetTimeout(&SD3, MS2ST(100));

				int check = toInt(check_temp);
				int sig = toInt(Sig_temp);
				int x = toInt(x_temp);
				int y = toInt(y_temp);
				int width = toInt(width_temp);
				int height = toInt(height_temp);
				msgp->checksum=check;
				msgp->signature=sig;
				msgp->x_center = x;
				msgp->y_center = y;
				msgp->width = width;
				msgp->height = height;
				msgp->pack_number = pack_number;
				pixy_pub.publish(*msgp);
			}
			toCheck[0] = sdGetTimeout(&SD3, MS2ST(100));
			toCheck[1] = sdGetTimeout(&SD3, MS2ST(100));
			toCheck[2] = sdGetTimeout(&SD3, MS2ST(100));
		}else
		{
			for(int i=0;i<3;i++)
			{
				toCheck[i]=toCheck[i+1];
			}

		}
		pack_number++;
		r2p::Thread::sleep(r2p::Time::ms(500));
	}


	return CH_SUCCESS;
}





