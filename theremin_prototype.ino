/* Code for a prototype of a theremin that uses ultrasonic sensors for volume control and tone frequency control
Created by Richard Bailey as a proof of concept and prototype for potential kit production if there is interest.
*/
#include "NewPing/NewPing.h"	//ping library that handles multiple sensors and has more control
#include "toneAC/toneAC.h"		//tone library that provides software volume control and louder output.

//  set tone frequency range (frequency is in hertz)
#define freq_bottom 800
#define freq_top 3000

// NewPing global variable setup for multiple sensors (copied from 15 sensor example)

#define SONAR_NUM    2 // Number or sensors.
#define MIN_DISTANCE 3  // Minimum distance (in cm) to use. CANNOT be less than 2, limit of sensor
#define MAX_DISTANCE 30 // Maximum distance (in cm) to ping.
#define PING_INTERVAL 65 // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).
#define MIN_TIME MIN_DISTANCE*57  // calculate minimum time for ping for min distance
#define MAX_TIME MAX_DISTANCE*57 //calculate maximum time for ping for max distance

unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
unsigned long pingtime[SONAR_NUM];         // Where the ping times are stored.
uint8_t currentSensor = 0;          // Keeps track of which sensor is active.

// setup tone variables
long tone_freq=0;
long tone_volume=0;

// setup sensor array
NewPing sonar[SONAR_NUM] = {     // Sensor object array.
	NewPing(2, 2, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping. sonar[0] for tone frequency
	NewPing(4, 4, MAX_DISTANCE), // sonar[1] for volume control
};

void setup(){
	Serial.begin(112500);
	pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
	for (uint8_t i = 1; i < SONAR_NUM; i++) // Set the starting time for each sensor.
	pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;

}

void loop() {
	for (uint8_t i = 0; i < SONAR_NUM; i++) { // Loop through all the sensors.
		if (millis() >= pingTimer[i]) {         // Is it this sensor's time to ping?
			pingTimer[i] += PING_INTERVAL * SONAR_NUM;  // Set next time this sensor will be pinged.
			if (i == 0 && currentSensor == SONAR_NUM - 1) oneSensorCycle(); // Sensor ping cycle complete, do something with the results.
			sonar[currentSensor].timer_stop();          // Make sure previous timer is canceled before starting a new ping (insurance).
			currentSensor = i;                          // Sensor being accessed.
			pingtime[currentSensor] = 0;                      // Make distance zero in case there's no ping echo for this sensor.
			sonar[currentSensor].ping_timer(echoCheck); // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
		}
	}
	// The rest of your code would go here.
}

void echoCheck() { // If ping received, set the sensor distance to array.
	if (sonar[currentSensor].check_timer())
	pingtime[currentSensor] = sonar[currentSensor].ping_result;
}

void oneSensorCycle() { // Sensor ping cycle complete, do something with the results.
	for (uint8_t i = 0; i < SONAR_NUM; i++) {
		Serial.print(i);
		Serial.print("=");
		Serial.print(pingtime[i]);
		Serial.print("microseconds ");
	}
	Serial.println(); 
	if (pingtime[0]<MIN_TIME)
	{
		tone_freq = 0;
	}
	else
	{
		tone_freq = map(pingtime[0],1030,MAX_TIME, freq_top, freq_bottom);
	}
	if (pingtime[1]<MIN_TIME)
	{
		tone_volume = 0;
	}
	else
	{
		tone_volume = map(pingtime[1],1030,MAX_TIME,75,1);
		
	} 
	toneAC(tone_freq,tone_volume,0,true);
}
