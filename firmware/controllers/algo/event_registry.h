/**
 * @file	event_registry.h
 *
 * @date Nov 27, 2013
 * @author Andrey Belomutskiy, (c) 2012-2020
 */

#pragma once

#include "global.h"
#include "efi_gpio.h"
#include "scheduler.h"
#include "fl_stack.h"
#include "trigger_structure.h"
#include "accel_enrichment.h"

#define MAX_INJECTION_OUTPUT_COUNT INJECTION_PIN_COUNT
#define MAX_WIRES_COUNT 2

class Engine;

class InjectionEvent {
public:
	InjectionEvent();

	// Call this every decoded trigger tooth.  It will schedule any relevant events for this injector.
	void onTriggerTooth(size_t toothIndex, int rpm, efitick_t nowNt);

	/**
	 * This is a performance optimization for IM_SIMULTANEOUS fuel strategy.
	 * It's more efficient to handle all injectors together if that's the case
	 */
	bool isSimultanious = false;
	InjectorOutputPin *outputs[MAX_WIRES_COUNT];
	int ownIndex = 0;
	DECLARE_ENGINE_PTR;
	event_trigger_position_s injectionStart;

	scheduling_s signalTimerUp;
	scheduling_s endOfInjectionEvent;

	/**
	 * we need atomic flag so that we do not schedule a new pair of up/down before previous down was executed.
	 *
	 * That's because we want to be sure that no 'down' side callback would be ignored since we are counting to see
	 * overlaps so we need the end counter to always have zero.
	 * TODO: make watchdog decrement relevant counter
	 */
	bool isScheduled = false;

	WallFuel wallFuel;
};

/**
 * This class knows about when to inject fuel
 */
class FuelSchedule {
public:
	FuelSchedule();

	// Call this every trigger tooth.  It will schedule all required injector events.
	void onTriggerTooth(size_t toothIndex, int rpm, efitick_t nowNt DECLARE_ENGINE_PARAMETER_SUFFIX);

	/**
	 * this method schedules all fuel events for an engine cycle
	 */
	void addFuelEvents(DECLARE_ENGINE_PARAMETER_SIGNATURE);
	bool addFuelEventsForCylinder(int cylinderIndex DECLARE_ENGINE_PARAMETER_SUFFIX);

	void resetOverlapping();

	/**
	 * injection events, per cylinder
	 */
	InjectionEvent elements[MAX_INJECTION_OUTPUT_COUNT];
	bool isReady;

private:
	void clear();
};

class AngleBasedEvent {
public:
	scheduling_s scheduling;
	event_trigger_position_s position;
	action_s action;
	/**
	 * Trigger-based scheduler maintains a linked list of all pending tooth-based events.
	 */
	AngleBasedEvent *nextToothEvent = nullptr;
};

#define MAX_OUTPUTS_FOR_IGNITION 2

class IgnitionEvent {
public:
	IgnitionEvent();
	IgnitionOutputPin *outputs[MAX_OUTPUTS_FOR_IGNITION];
	scheduling_s dwellStartTimer;
	AngleBasedEvent sparkEvent;

	// How many additional sparks should we fire after the first one?
	// For single sparks, this should be zero.
	uint8_t sparksRemaining = 0;

	/**
	 * Desired timing advance
	 */
	angle_t sparkAngle = NAN;
	floatms_t sparkDwell = 0;
	/**
	 * this timestamp allows us to measure actual dwell time
	 */
	uint32_t actualStartOfDwellNt = 0;
	event_trigger_position_s dwellPosition{};
	/**
	 * Sequential number of currently processed spark event
	 * @see globalSparkIdCounter
	 */
	int sparkId = 0;
	/**
	 * [0, specs.cylindersCount)
	 */
	int cylinderIndex = 0;
	int8_t cylinderNumber = 0;
	char *name = nullptr;
	DECLARE_ENGINE_PTR;
	IgnitionOutputPin *getOutputForLoggins();
};

#define MAX_IGNITION_EVENT_COUNT IGNITION_PIN_COUNT

class IgnitionEventList {
public:
	/**
	 * ignition events, per cylinder
	 */
	IgnitionEvent elements[MAX_IGNITION_EVENT_COUNT];
	bool isReady = false;
};

class AuxActor {
public:
	int phaseIndex;
	int valveIndex;
	angle_t extra;

	AngleBasedEvent open;
	AngleBasedEvent close;
	DECLARE_ENGINE_PTR;
};
