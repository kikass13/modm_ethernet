/*
 * Copyright (c) 2020, Mike Wolfram
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#include <modm/board.hpp>
#include <modm/processing/rtos.hpp>

// overwrite freertos config with our local one
#include <FreeRTOSConfigLocal.h>
unsigned long ulHighFrequencyTimerTicks = 20000000;


#include <utils/ethernet.hpp>
#include <utils/random.hpp>

#include <tasks/http.hpp>
#include <tasks/scheduler_watchdog.hpp>
#include <tasks/led.hpp>


using namespace Board;


static void onEthUp(){
	static bool ethTasksCreated = false;
	if(!ethTasksCreated){
		// create tasks which depend on the eth interface now (not before)
		xTaskCreate(HttpServerListener::run, HttpServerListener::name, configMINIMAL_STACK_SIZE * 2, 0, configMAX_PRIORITIES + 1, 0);
		ethTasksCreated = true;
	}
}

static void onEthDown(){
}

int main()
{
	Board::initialize();
	Leds::setOutput();
	MODM_LOG_INFO << "\n\n============================\nHello world ... \n" << modm::endl;
	// create ethernet manager utility
	auto eth = EthernetUtils::EthernetManager::get();
	// register important callbacks
	eth.registerOnNetworkUpCallback(std::bind(onEthUp));
	eth.registerOnNetworkDownCallback(std::bind(onEthDown));
	// initialize ethernet interface
	MODM_LOG_INFO << "Initialzing ethernet interface ..." << modm::endl;
	eth.initialize();
	// initialize tasks we want to run later
	LedTask <Board::LedRed> p0;
	SchedulerUtils::SchedulerWatchdogTask p1;
    MODM_LOG_INFO << "Initializing scheduler" << modm::endl;
	modm::rtos::Scheduler::schedule();

	// we should never get here
	return 0;
}
