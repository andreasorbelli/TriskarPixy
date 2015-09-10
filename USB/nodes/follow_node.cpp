
#include "ch.h"
#include "hal.h"
#include "r2p/Middleware.hpp"
#include <r2p/msg/pixy.hpp>
#include <r2p/msg/motor.hpp>
#include <r2p/msg/follow.hpp>
#include <r2p/msg/proximity.hpp>

// Robot parameters
#define _L        0.160f    // Wheel distance [m]
#define _R        0.035f    // Wheel radius [m]
#define _MAX_DTH  52.0f     // Maximum wheel angular speed [rad/s]

#define _m1_R     (-1.0f / _R)
#define _mL_R     (-_L / _R)
#define _C60_R    (0.500000000f / _R)   // cos(60°) / R
#define _C30_R    (0.866025404f / _R)   // cos(30°) / R

#define _TICKS 64.0f
#define _RATIO 29.0f
#define _PI 3.14159265359f

#define R2T(r) (_TICKS * _RATIO)/(r * 2 * _PI)
#define T2R(t) (t / R2T)

#define M2T(m) (m * _TICKS * _RATIO)/(2 * _PI * _R)
#define T2M(t) (t / _M2TICK)

template<typename T> static inline T clamp(T min, T value, T max) {
	return (value < min) ? min : ((value > max) ? max : value);
}

static int follow=0;

msg_t follow_node(void *arg) {
	//Routine

	r2p::Node node("follow_node");
	r2p::Publisher<r2p::Speed3Msg> vel_pub;
	r2p::Subscriber<r2p::PixyMsg, 5> pixy_sub;
	r2p::Subscriber<r2p::FollowMsg, 5> shell_sub;
	r2p::Subscriber<r2p::ProximityMsg, 5> proximity_sub;

	node.advertise(vel_pub, "speed3", r2p::Time::INFINITE);
	node.subscribe(proximity_sub, "proximity");
	node.subscribe(pixy_sub, "pixy");
	node.subscribe(shell_sub, "follow");


	float x = 0;
	float y = 0;
	float w = 0;
	//int firsttime=0;
	bool goingAhead=false;
	bool stop = false;
	r2p::PixyMsg * pixy_msgp;
	r2p::Speed3Msg * speed_msgp;
	r2p::FollowMsg * follow_msgp;
	r2p::ProximityMsg * proximity_msgp;


	for(;;){

		if(shell_sub.fetch(follow_msgp))
		{
			follow = follow_msgp->follow;
			shell_sub.release(*follow_msgp);
		}

		if(follow){
			goingAhead=false;
			x = 0;
			y = 0;
			w = 0;
			int timeout = 0;

			//clean trash
			//			while(pixy_sub.fetch(pixy_msgp));

			while(!pixy_sub.fetch(pixy_msgp) && timeout <500)  {
				r2p::Thread::sleep(r2p::Time::ms(5));
				timeout++;
			}
			if(timeout<500){
				//temporary collision checking
				if(pixy_msgp->height<80){
					//no collision
					if(pixy_msgp->x>200){
						//Rotating Right
						x = -0.25;
						y = 0;
						w = 0.1;
					}else if(pixy_msgp->x<120){
						//Rotating Left
						x = -0.25;
						y = 0;
						w = -0.2;
					}else{
						x=-0.2;
						w = 0;
						//Going Ahead
						if(!goingAhead){
							x=-0.3;
							w = -0.1;
							goingAhead=true;
							stop=true;
						}
					y = 0;
					}
				}
				else {
					if(pixy_msgp->x>170){
						//CENTERING ROTATING RIGHT
						x = -0.2;
						y = 0;
						w = 0.1;
					}else if(pixy_msgp->x<150){
						//CENTERING ROTATING LEFT
						x = -0.2;
						y = 0;
						w = -0.2;
					}else{
						x=0;
						y = 0;
						w = 0;
					}
				}
				pixy_sub.release(*pixy_msgp);
			}
			else{
				//SEARCHING
				x=0;
				y = 0;
				w = 0.9;
				goingAhead=false;
				stop=false;
			}
			//discarding some messages to have a valid one
			for(int trash=0;trash<4;trash++){
				if(proximity_sub.fetch(proximity_msgp))
					proximity_sub.release(*proximity_msgp);
			}


			while(!proximity_sub.fetch(proximity_msgp))
			{
				r2p::Thread::sleep(r2p::Time::ms(5));
			}

			int value = proximity_msgp->value[0];
			if(value>2300){
				x=0;
				//	if(firsttime<1){
				//		x=0.5;
				//	}
				if(stop)
					x=0.3;
				y=0;
				w=0;
				//firsttime++;
				goingAhead=false;
				stop=false;
			}
			else{
				//	firsttime=0;
			}
			proximity_sub.release(*proximity_msgp);


			//moving
			// Wheel angular speeds
			const float dthz123 = _mL_R * w;
			const float dx12 = _C60_R * y;
			const float dy12 = _C30_R * x;

			float dth1 = dx12 - dy12 + dthz123;
			float dth2 = dx12 + dy12 + dthz123;
			float dth3 = _m1_R * y + dthz123;


			//ir collision checking
			// Motor setpoints
			if (vel_pub.alloc(speed_msgp)) {
				speed_msgp->value[0] = (int16_t) clamp(-_MAX_DTH, dth1, _MAX_DTH);
				speed_msgp->value[1] = (int16_t) clamp(-_MAX_DTH, dth2, _MAX_DTH);
				speed_msgp->value[2] = (int16_t) clamp(-_MAX_DTH, dth3, _MAX_DTH);
				vel_pub.publish(*speed_msgp);
			}

		}

		r2p::Thread::sleep(r2p::Time::ms(15));
	}
	return CH_SUCCESS;
}




