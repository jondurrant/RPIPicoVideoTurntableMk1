/*
 * Stepper28BYJ.cpp
 *
 *  Stepper Motor driven by ULN2003 driver using FreeRTOS
 *
 *
 *  Created on: 24 Dec 2022
 *      Author: jondurrant
 */

#include "Stepper28BYJ.h"
#include <cstdio>

//Structure to use for queue
enum StepperAction {StepperStep, StepperTo, StepperReset, StepperCont};
struct StepperReq {
	StepperAction action;
	int16_t steps;
	float rpm;
	bool cw;
	uint32_t id;
};
typedef struct StepperReq StepperReqT;




/***
 * Constructor
 * @param gp1 - GPIO Pad
 * @param gp2 - GPIO Pad
 * @param gp3 - GPIO Pad
 * @param gp4 - GPIO Pad
 * @param slotGP - GPIO Pad for slot detector
 */
Stepper28BYJ::Stepper28BYJ(
		uint8_t gp1,
		uint8_t gp2,
		uint8_t gp3,
		uint8_t gp4,
		uint8_t hallGP ){
	pGPPAD[0] = gp1;
	pGPPAD[1] = gp2;
	pGPPAD[2] = gp3;
	pGPPAD[3] = gp4;

	xHallGP = hallGP;

	xCmdQ = xQueueCreate( STEPPER_QUEUE_LEN, sizeof(StepperReqT));
	if (xCmdQ == NULL){
		printf("ERROR: Unable to create Queue\n");
	}
	setDelay(14);
}


/***
 * Destructor
 */
Stepper28BYJ::~Stepper28BYJ() {
	if (xCmdQ != NULL){
		vQueueDelete(xCmdQ);
	}
}

/***
 * Get the static depth required in words
 * @return - words
 */
configSTACK_DEPTH_TYPE Stepper28BYJ::getMaxStackSize(){
	return 200;
}

/***
 * Initialise the GPIO
 */
void Stepper28BYJ::init(){
	for (uint8_t i=0; i < 4; i++){
		gpio_init(pGPPAD[i]);
		gpio_set_dir(pGPPAD[i], GPIO_OUT);
		gpio_put(pGPPAD[i], 0);
	}

	gpio_init(xHallGP);
	gpio_set_dir(xHallGP, GPIO_IN);

}

/***
 * Run loop for the agent.
 */
void Stepper28BYJ::run(){

	init();


	uint8_t s;
	uint32_t id = 0;

	//printf("Stepper started\n");

	for(;;) {
		bool b = gpio_get(xHallGP);

		if (!b) {
			if (xState == StepperStartHunt){
				xState = StepperCalibrateOne;
				step(100, 14);

				//printf("Stepper moved to Calibrate %d\n", xPos);
			} else if (xState == StepperCalibrateTwo){
				//printf("Number of steps is %d\n", xPos);
				xTotalSteps = xPos;
				xState = StepperCmd;
			}

			xPos = 0;

		}

		//Stop Processing
		if (xState == StepperStop){
			xTargetPos = xPos;
			if (pObserver != NULL){
				pObserver->actionComplete(id);
			}
			xState = StepperCmd;
			id = processQueue();
		}


		if (xPos == xTargetPos){
			if (id != 0){
				//printf("Target reached %d\n", xTargetPos);
				if (pObserver != NULL){
					pObserver->actionComplete(id);
				}
				if (xState == StepperCalibrateOne){
					xState = StepperCalibrateTwo;
					stepTo(0, 14, true);
				}
			}
			id = processQueue();
		}

		if ((id != 0) && (xPos != xTargetPos)){
			doStep();
		}

		if (xState == StepperStartHunt){
			//printf("StepperStartHunt %d\n", xPos);
			doStep();
		}

		//printf("Delay(%d) would be %d\n", xSeq, xDelay[xSeq] );
		vTaskDelay(xDelay[xSeq]);
	}
}


/***
 * Do a single step on the Stepper motor in required direction.
 */
void Stepper28BYJ::doStep(){
	//uint8_t sequence[STEPPER_SEQ_LEN] = {3, 6, 12, 9};
	uint8_t sequence[STEPPER_SEQ_LEN] = {9, 12, 6, 3};

	if (xClockwise){
		xPos = modPos(xPos + 1);
		xSeq = (xSeq + 1) % STEPPER_SEQ_LEN;
	} else {
		xPos = modPos(xPos - 1);
		if (xSeq == 0){
			xSeq = 3;
		} else {
			xSeq = xSeq - 1;
		}
	}
	//printf("Pos %d Target = %d\n", xPos, xTargetPos);
	uint8_t s = sequence[xSeq];
	for (uint8_t b = 0; b < STEPPER_SEQ_LEN; b++){
		uint8_t m = 1 << b;
		if ((m & s) > 0){
			gpio_put(pGPPAD[b], 1);
		} else {
			gpio_put(pGPPAD[b], 0);
		}
	}
	//printf("%d %d %d\n", xPos, xPos % 4, s);
}

/***
 * Step motor through x full steps
 * Possitive is clockwise, negative is counter clockwise
 * @param step - Step count >0 CW, < 0 CCW
 * @param rpm - Revolutions per mininute (under 14 to work)
 * RPM 0 is maximum speed
 */
uint32_t Stepper28BYJ::step(int16_t step, float rpm){
	StepperReqT req;

	req.action = StepperStep;
	req.steps = step;
	req.rpm = rpm;
	req.id = xNextId++;

	if (xCmdQ != NULL){
		BaseType_t res = xQueueSendToBack(xCmdQ, (void *)&req, 0);
		if (res != pdTRUE){
			printf("WARNING: Queue is full\n");
		}
	}
	return req.id;
}


/***
 * Process request from the queue
 */
uint32_t Stepper28BYJ::processQueue(){
	StepperReqT req;
	float delay;

	if (xCmdQ == NULL){
		return 0;
	}

	BaseType_t res = xQueueReceive(xCmdQ, (void *)&req, 0);
	if (res == pdTRUE){
		switch(req.action){
		case StepperStep:
			xTargetPos = modPos(xPos + req.steps);
			if (req.steps < 0){
				xClockwise = false;
			} else {
				xClockwise = true;
			}

			setDelay(req.rpm);

			break;
		case StepperTo:
			xTargetPos = req.steps;
			xClockwise = req.cw;
			setDelay(req.rpm);
			break;
		case StepperCont:
			xTargetPos = -1;
			xClockwise = req.cw;
			setDelay(req.rpm);
			break;
		default:
			break;
		}

		return req.id;
	}
	return 0;
}

/***
 * Calculate the pos of stepper based on modulus maths
 * @param pos
 * @return
 */
int16_t Stepper28BYJ::modPos(int16_t pos){

	if (xState == StepperCmd){
		int16_t p = pos % xTotalSteps;
		if (p < 0){
			p = xTotalSteps + p;
		}
		return p;
	} else {
		return pos;
	}
}

/***
 * Split RPM speed as a set of four delays over the four phases
 * @param rpm - 0 will set to maximum speed
 */
void Stepper28BYJ::setDelay(float rpm){
	float delay;

	if (rpm <= 0.0){
		for (uint8_t i=0; i < STEPPER_SEQ_LEN; i++){
			xDelay[i] = DELAY;
		}
		return;
	}

	delay = (60000.0 / rpm) / (float)xTotalSteps;

	for (uint8_t i=0; i < STEPPER_SEQ_LEN; i++){
		xDelay[i] = delay;
	}

	uint16_t e = (delay * (float)STEPPER_SEQ_LEN) - xDelay[0]*STEPPER_SEQ_LEN;
	switch(e){
	case 1:
		xDelay[1]++;
		break;
	case 2:
		xDelay[1]++;
		xDelay[3]++;
		break;
	case 3:
		xDelay[1]++;
		xDelay[2]++;
		xDelay[3]++;
		break;
	}

}



/***
 * Calibrate the disk and leave at possition zero
 */
void Stepper28BYJ::calibrate(){
	xState = StepperStartHunt;
}

/***
 * Step to a possition
 * @param pos = Possition between 0 and 4047
 * @param rpm - speed, 0 to 14. 0 is max speed
 * @param cw - clockwise
 */
uint32_t Stepper28BYJ::stepTo(int16_t pos, float rpm, bool cw){
	StepperReqT req;

	req.action = StepperTo;
	req.steps = pos;
	req.rpm = rpm;
	req.cw = cw;
	req.id = xNextId++;

	if (xCmdQ != NULL){
		BaseType_t res = xQueueSendToBack(xCmdQ, (void *)&req, 0);
		if (res != pdTRUE){
			printf("WARNING: Queue is full\n");
		}
	}

	return req.id;
}

/***
 * Rotate to Degrees possition (0 to 360)
 * @param deg - Float 0 to 360
 * @param rpm - Rotation speed, under 14
 * @param cw - Clockwise or CCW.
 * @return id of the instruction
 */
uint32_t Stepper28BYJ::stepToDeg(float deg, float rpm, bool cw){
	int16_t pos = (float)xTotalSteps * (deg / 360.0);
	return stepTo(pos, rpm, cw);
}




/***
 * Set Observer
 * @param obs
 */
void Stepper28BYJ::setObserver(StepperObserver *obs){
	pObserver = obs;
}

/***
 * Get state of stepper motor
 * @return StepperCmd if in normal mode - otherwise probably calibrating
 */
StepperState Stepper28BYJ::getState(){
	return xState;
}

/***
 * Stop the current step action.
 * If another action in queue it will then start that
 */
void Stepper28BYJ::stepStop(){
	xState = StepperStop;
}

/***
 * Rotate continuously at RPM rate
 * @param rpm - Revolutions per minute
 * @param cw - True if clockwise
 * @return
 */
uint32_t Stepper28BYJ::stepContinuous(float rpm, bool cw){
	StepperReqT req;

	req.action = StepperCont;
	req.steps = 0;
	req.rpm = rpm;
	req.cw = cw;
	req.id = xNextId++;

	if (xCmdQ != NULL){
		BaseType_t res = xQueueSendToBack(xCmdQ, (void *)&req, 0);
		if (res != pdTRUE){
			printf("WARNING: Queue is full\n");
		}
	}
	return req.id;
}


/***
 * Stop the stepper motor
 * Clear the queue of actions
 */
void Stepper28BYJ::stepHaltClear(){
	stepStop();
	xQueueReset(xCmdQ);
}

/***
 * Stop the stepper,
 * Clear the queue of actions
 * Return to zero possition
 * @return
 */
uint32_t Stepper28BYJ::stepReset(){
	stepHaltClear();
	bool cw = true;

	if (xPos < (xTotalSteps - xPos)){
		cw = false;
	}
	return stepToDeg(0.0, 3, cw);
}

/***
 * Get the current angle of the stepper motor
 * @return Degrees from Hall sensor
 */
float Stepper28BYJ::getDegrees(){
	return 360.0 * ((float)xPos / (float)xTotalSteps);
}


/***
 * Get Step possition. Hall sensor is possition 0
 * Counts clockwise
 * @return step possition
 */
uint16_t Stepper28BYJ::getPos(){
	return xPos;
}
