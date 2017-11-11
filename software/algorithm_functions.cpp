/*
 * algorithm_functions.c
 *
 *  Created on: 26 de ago de 2017
 *      Author: André
 */

#include "algorithm_functions.h"

int TimeCounter = 0, TemperatureSelect = DEFAULT_TEMPERATURE, TimeSelect = DEFAULT_TIME;
int ContentViewFlag = VIEW_TIME;
long StartRestoreTime = 0, EndRestoreTime = 0, RestoreTime = 0;
float ColletedData[LOG_DATA_SIZE];

float LogAvarageTemperature, LogMaximumTemperature, LogMinimumTemperature;
int LogTotalTime, LogHeatingTime, LogCoolingTime, LogInterruptionTime, LogInterruptionNumber;


/* Reset Microcontroller */ 

void(* resetFunc) (void) = 0;								// Declare reset function at address 0.

/* Reset internal system variables */

void ResetSystemVariables(void) {
	TimeCounter = 0;
	TemperatureSelect = DEFAULT_TEMPERATURE;
	TimeSelect = DEFAULT_TIME;
	ActuatorActivation(TURN_OFF, ACTUATOR_RELAY);
	ConfirmFlag = NOT_CONFIRMED;
}

/* Switch Interrupt */

void SwitchInterrupt(void) {
	while(ButtonVerification(SWITCH)) {						// Switch open.
		ActuatorActivation(TURN_OFF, ACTUATOR_RELAY);		// Actuator Relay off for security.
		DisplayTurnMode(TURN_OFF);							// Indicate that the switch is not open.
	}
	DisplayTurnMode(TURN_ON);
	if (SensorRoutine() < TemperatureSelect){
		ActuatorActivation(TURN_ON, ACTUATOR_RELAY);
	}
}

/* Menu Functions */

void MenuStart(void) {

	/* Initialization */
	int AuxiliaryCounter;
	for (AuxiliaryCounter = 0; AuxiliaryCounter < LOG_DATA_SIZE; AuxiliaryCounter++) {
		ColletedData[AuxiliaryCounter];
	}

	ResetSystemVariables();

	/* Menu: Initialization */
	while (!ButtonVerification(BUTTON_NEXT));				// Wait the button for start.

	DisplayPrint("Bem vindo!", NO_CONTENT, "P.A.L.O.M.A");
	ActuatorActivation(TURN_ON, ACTUATOR_INDUCTOR);
	delay(DELAY_PERIOD_WELCOME);
}

void MenuTemperatureSelect(void) {
	int DisplayFlag = VALUE_NOT_CHANGED, BlinkTurn = 0;

	/* Menu: Temperature text */
	DisplayPrint("Escolha a", NO_CONTENT, "temperatura!");
	delay(DELAY_PERIOD_SELECTION);	
	DisplayPrint("Temperatura(C):", TemperatureSelect, NO_MENU);

	/* Menu: Temperature Select */
	while (!ButtonVerification(BUTTON_NEXT)) {				// Loop to refresh the Display and verify the button state.
		if (ButtonVerification(BUTTON_PLUS)) {
			DisplayFlag = VALUE_CHANGED;
			if (TemperatureSelect < SAFETY_TEMPERATURE) {
				TemperatureSelect = TemperatureSelect + 5;
			}
			delay(DELAY_PERIOD_DEBOUNCER);
		}
		if (ButtonVerification(BUTTON_MINUS)) {
			DisplayFlag = VALUE_CHANGED;
			if (TemperatureSelect > 0) {
				TemperatureSelect = TemperatureSelect - 5;
			}
			delay(DELAY_PERIOD_DEBOUNCER);
		}

		if (DisplayFlag) {									// If value changed print in the display.
			DisplayPrint("Temperatura(C):", TemperatureSelect, NO_MENU);
			DisplayFlag = VALUE_NOT_CHANGED;
		}		

		delay(DELAY_PERIOD_DEBOUNCER);
	}
	for (BlinkTurn = 0; BlinkTurn < 3; BlinkTurn++) {		// Loop to indicate the chosen value.
		DisplayTurnMode(TURN_OFF);
		delay(DELAY_PERIOD_BLINK);
		DisplayTurnMode(TURN_ON);
		delay(DELAY_PERIOD_BLINK);
	}
	while (ButtonVerification(BUTTON_NEXT));				// Wait the button next.
}

void MenuTimeSelect(void) {									// Similar to the MenuTemperatureSelect().
	int DisplayFlag = VALUE_NOT_CHANGED, BlinkTurn = 0;

	/* Menu: Time text */
	DisplayPrint("Escolha o", NO_CONTENT, "tempo!");
	delay(DELAY_PERIOD_SELECTION);	
	DisplayPrint("Tempo(min):", TimeSelect, NO_MENU);

	/* Menu: Time Select */
	while (!ButtonVerification(BUTTON_NEXT)) {
		if (ButtonVerification(BUTTON_PLUS)) {
			DisplayFlag = VALUE_CHANGED;
			if (TimeSelect < SAFETY_TIME) {
				TimeSelect++;
			}
			delay(DELAY_PERIOD_DEBOUNCER);
		}
		if (ButtonVerification(BUTTON_MINUS)) {
			DisplayFlag = VALUE_CHANGED;
			if (TimeSelect > 1) {
				TimeSelect--;
			}
			delay(DELAY_PERIOD_DEBOUNCER);
		}
		if (DisplayFlag) {									// If value changed print in the display.
			DisplayPrint("Tempo(min):", TimeSelect, NO_MENU);
			DisplayFlag = VALUE_NOT_CHANGED;		
		}

		delay(DELAY_PERIOD_DEBOUNCER);
	}
	for (BlinkTurn = 0; BlinkTurn < 3; BlinkTurn++) {
		DisplayTurnMode(TURN_OFF);
		delay(DELAY_PERIOD_BLINK);
		DisplayTurnMode(TURN_ON);
		delay(DELAY_PERIOD_BLINK);
	}

	TimeSelect = TimeSelect * 60;							// Convertion to seconds.
	while (ButtonVerification(BUTTON_NEXT));				// Wait the button next.
}

void MenuConfirm(void) {
	/* Menu: Parameters confirmation */
	DisplayPrint("Confirmando...", NO_CONTENT, "Voltar | Iniciar");

	while (1) {	
		if (ButtonVerification(BUTTON_PLUS)) {
			while (ButtonVerification(BUTTON_PLUS));
			ConfirmFlag = CONFIRMED;
			break;
		}
		if (ButtonVerification(BUTTON_MINUS)) {
			while (ButtonVerification(BUTTON_MINUS));
			ResetSystemVariables();
			break;
		}
		delay(DELAY_PERIOD_DEBOUNCER);
	}
}

/* Control Functions */

void ControlStart(void) {
	float SensorTemperature;

	ActuatorActivation(TURN_ON, ACTUATOR_RELAY);			// Start the process.

	while (SensorRoutine() < TemperatureSelect) {
		SensorTemperature = SensorRoutine();
		DisplayPrint("Esquentando...", SensorTemperature, NO_MENU);
		LEDDebugBlink(7);									// Blink Debug 7 times per second
		//SwitchInterrupt();
	}

	DisplayPrint("Temperatura", NO_CONTENT, "certa!" );	
	delay(DELAY_PERIOD_CONTROL);
}

void ControlDisplayView(void) { 
	int PrintTimeRaw, PrintTimeSeconds, PrintTimeMinutes;
	float SensorTemperature;
	char PrintTimeString[5];

	if (ContentViewFlag == VIEW_TEMPERATURE) {					// Temperatura view mode.
		SensorTemperature = SensorRoutine();
		DisplayPrint("Temperatura(C):", SensorTemperature, NO_MENU);
	}
	if (ContentViewFlag == VIEW_TIME) {							// Time view mode.
		PrintTimeRaw = TimeSelect - TimeCounter;			// Countdown.
		PrintTimeMinutes = PrintTimeRaw / 60.0;
		PrintTimeSeconds = PrintTimeRaw % 60;
		if (PrintTimeSeconds < 10){
			sprintf(PrintTimeString, "%d:0%d", PrintTimeMinutes, PrintTimeSeconds);
		}
		else {
			sprintf(PrintTimeString, "%d:%d", PrintTimeMinutes, PrintTimeSeconds);
		}
		DisplayPrint("Tempo Restante:", NO_CONTENT, PrintTimeString);	
	}
}

void ControlProcessDynamicChange(void) {
	ResetSystemVariables();
	MenuTemperatureSelect();
	MenuTimeSelect();
	EndRestoreTime = millis();
	RestoreTime = RestoreTime + (EndRestoreTime - StartRestoreTime);
}

void ControlProcess(void) {
	LEDDebugBlink(5);										// Blink Debug 5 times per second

	if (SensorRoutine() >= (TemperatureSelect + RANGE_TEMPERATURE)) {
		ActuatorActivation(TURN_OFF, ACTUATOR_RELAY);
	}
	if (SensorRoutine() < TemperatureSelect)  {
		ActuatorActivation(TURN_ON, ACTUATOR_RELAY);
	}
}

void ControlSystemRun(void) {

	int ResetCounter = 0;
	long StartControlTime = 0, EndTurnTime = 0;

	ControlStart();

	StartControlTime = millis();

	while (TimeCounter <= TimeSelect) {						// Loop routine for control the actuator.
		/* Reset with continuos button minus pressed */
		if (ButtonVerification(BUTTON_MINUS)){				
			ActuatorActivation(TURN_OFF, ACTUATOR_RELAY);
			while(ButtonVerification(BUTTON_MINUS)) {
				ResetCounter++;
				delay(PERIOD);
			}
			if (ResetCounter >= RESET_PRESSED_TIME) {
				resetFunc();	
			}
		}

		/* Dynamic temperature change with button plus */
		if (ButtonVerification(BUTTON_PLUS)){
			StartRestoreTime = millis();
			ActuatorActivation(TURN_OFF, ACTUATOR_RELAY);
			while(ButtonVerification(BUTTON_PLUS));
			ControlProcessDynamicChange();
		}

		/* Show a different content with the button next */
		if (ButtonVerification(BUTTON_NEXT)) {					// Switch the view mode of the Display.
			if (ContentViewFlag == VIEW_TEMPERATURE) {
				ContentViewFlag = VIEW_TIME;
			}
			else {
				ContentViewFlag = VIEW_TEMPERATURE;
			}
		}
		ControlDisplayView();

		ControlProcess();
		//SwitchInterrupt();

		delay(PERIOD);

		EndTurnTime = millis();
		TimeCounter = ((EndTurnTime - StartControlTime) - RestoreTime) / 1000;	// Convertion to seconds, normaly RestoreTime = 0.
	}

	ActuatorActivation(TURN_OFF, ACTUATOR_RELAY);
	ActuatorActivation(TURN_OFF, ACTUATOR_INDUCTOR);

	DisplayPrint("Finalizado com", NO_CONTENT, "sucesso!");
	while (!ButtonVerification(BUTTON_NEXT));
}

/* Log Functions */

void LogOverview(void) {
	int LogOverviewMode = 0, LogMaxLength, DisplayFlag;
	char PrintData[13];

	LogMaxLength = LOG_DATA_SIZE + 8;

	DisplayPrint("Resultados:", NO_CONTENT, NO_MENU);
	delay(DELAY_PERIOD_LOG);

	while (!ButtonVerification(BUTTON_NEXT)) {
		if (ButtonVerification(BUTTON_PLUS)) {
			DisplayFlag = VALUE_CHANGED;
			if (LogOverviewMode < LogMaxLength) {
				LogOverviewMode++;
			}
			delay(DELAY_PERIOD_DEBOUNCER);
		}
		if (ButtonVerification(BUTTON_MINUS)) {
			DisplayFlag = VALUE_CHANGED;
			if (LogOverviewMode > 0) {
				LogOverviewMode--;
			}
			delay(DELAY_PERIOD_DEBOUNCER);
		}
		
		if (DisplayFlag) {									// If value changed print in the display.
			switch (LogOverviewMode) {
				case LOG_TEMPERATURE_AVERAGE:
					DisplayPrint("Temp. media(C):", LogAvarageTemperature, NO_MENU);
					break;
				case LOG_TEMPERATURE_MAXIMUM:
					DisplayPrint("Temp. max.(C):", LogMaximumTemperature, NO_MENU);
					break;
				case LOG_TEMPERATURE_MINIMUM:
					DisplayPrint("Temp. min.(C):", LogMinimumTemperature, NO_MENU);
					break;
				case LOG_TIME_TOTAL_ACTUAL:
					DisplayPrint("Tempo tot.(min):", LogTotalTime, NO_MENU);
					break;
				case LOG_TIME_HEATING:
					DisplayPrint("Tempo aqc.(min):", LogHeatingTime, NO_MENU);
					break;
				case LOG_TIME_COOLING:
					DisplayPrint("Tempo rsf.(min):", LogCoolingTime, NO_MENU);
					break;
				case LOG_INTERRUPTION_TOTAL_TIME:
					DisplayPrint("Tempo int.(min):", LogInterruptionTime, NO_MENU);
					break;
				case LOG_INTERRUPTION_NUMBER:
					DisplayPrint("interrup. numb.:", LogInterruptionNumber, NO_MENU);
					break;
				default: break;
			}

			if (LogOverviewMode >= LOG_COLLECTED_DATA) {
				sprintf(PrintData, "%f | %d", ColletedData[LogOverviewMode-8], TimeCounter);
				DisplayPrint("Temp.x Tempo(s):", NO_CONTENT, PrintData);
			}
			
			DisplayFlag = VALUE_NOT_CHANGED;		
		}
		delay(DELAY_PERIOD_DEBOUNCER);
	}

	DisplayPrint("Deseja iniciar", NO_CONTENT, "novamente?");
	while(!ButtonVerification(BUTTON_NEXT));
	resetFunc();
}


