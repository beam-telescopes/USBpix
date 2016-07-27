#ifndef _READOUT_H
#define _READOUT_H

#include <thread>
#include <atomic>
#include <future>
#include <condition_variable>
#include <functional>

#include "dllexport.h"

class DllExport Readout {
	public:
		Readout(std::function<void(void)> _readout_function, unsigned _readout_interval) : readout_function(_readout_function), readout_interval(_readout_interval) , readout_thread(&Readout::readoutThread, this) {} // workaround for MSVC12
		~Readout(void);

		void startReadout(void);
		void pauseReadout(void);
		void stopReadout(void);

	protected:
		enum State {
			PAUSE,
			RUN,
			STOP,
			ERROR
		};

		void setState(State s);

		void readoutThread(void);
		void readoutLoop(void);

		std::function<void(void)> readout_function;
		const unsigned readout_interval;

		std::promise<void> promise;
		std::future<void> future = promise.get_future();

		std::condition_variable change_state;
		std::condition_variable acknowledge_state;

		std::atomic<State> state = {PAUSE};
		std::atomic<bool> change = {false};

		std::mutex mutex;
		std::thread readout_thread;// = std::thread(&Readout::readoutThread, this); //has to be last to guarantee proper initialization
};

#endif
