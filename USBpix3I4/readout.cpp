#include <chrono>

#include "readout.h"
#include "fifo.h"

void Readout::setState(State s) {
	std::unique_lock<std::mutex> lock(mutex);

	if(state == ERROR) {
		future.get();
	} else {
		state = s;
		change = true;

		change_state.notify_one();
		acknowledge_state.wait(lock, [&] {return !change.load();});
	}
}

void Readout::startReadout(void) {
	setState(RUN);
}

void Readout::pauseReadout(void) {
	setState(PAUSE);
}

void Readout::stopReadout(void) {
	setState(STOP);
}

Readout::~Readout(void) {
	if(state != STOP && state != ERROR) {
		setState(STOP);
	}
	readout_thread.join();
}

void Readout::readoutThread(void) {
	try {
		State current_state = {PAUSE};

		for(;;) {
			if(change) {
				std::lock_guard<std::mutex> lock(mutex);
				change = false;
				current_state = state.load();
				acknowledge_state.notify_one();

				if(current_state == STOP) {
					return;
				}
			}

			switch(current_state) {
				case RUN:
					readoutLoop();
					break;

				case PAUSE: {
					std::unique_lock<std::mutex> lock(mutex);
					change_state.wait(lock, [&] {return change.load();});
					break;
				}

				default: //fixme
					return;

			}
		}
	} catch(...) {
		std::unique_lock<std::mutex> lock(mutex);
		state = ERROR;
		promise.set_exception(std::current_exception());
	}
}

void Readout::readoutLoop(void) {
	readout_function();
	std::this_thread::sleep_for(std::chrono::milliseconds(readout_interval));
}
