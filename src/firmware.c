
#include <stdio.h>
#include <libknp.h>

int main(int argc, const char *argv[]) {
	// TODO parse command line args
	printf("(exec) %s ", argv[0]);
	for (int i=1;i<argc;i++) {
		printf("%s ", argv[i]);
	}
	printf("\n");

	// init hardware
	hal_init();
	// setup dispatch callbacks
	cbString = dispatchString;
	cbSimple = dispatchSimple;
	cbSysex = dispatchSysex;
	//logWrite("Firmware.cpp, init OK\n");

	// loop
	uint16_t loop_counter = 0;
	uint16_t last_second = 0;
	uint16_t elapsed = 0;
	uint16_t utimeout = 999;
	while(1) {
		// start
		uint16_t ustart = micros();
		loop_counter++;
		utimeout = 999;

		// --- hardware tasks
		hal_run();

		// evaluate deadline
		uint16_t uend = micros();
		elapsed = 0;
		if (uend>=ustart) {
			elapsed = uend-ustart;
			//printf("start %d end %d elapsed %d (us)\n", ustart, uend, elapsed);
		} else {
			elapsed = ((999-ustart)+uend);
			//printf("start %d end %d elapsed %d (us) (ovf)\n", ustart, uend, elapsed);
		}
		if (elapsed > utimeout) return elapsed;
		else utimeout = utimeout - elapsed;

		// --- receive
		while (binConsole->available(binConsole)) {
			collect(0); // TODO
		}

		// evaluate deadline
		uend = micros();
		if (uend>=ustart) {
			elapsed = uend-ustart;
			//printf("start %d end %d elapsed %d (us)\n", ustart, uend, elapsed);
		} else {
			elapsed = ((999-ustart)+uend);
			//printf("start %d end %d elapsed %d (us) (ovf)\n", ustart, uend, elapsed);
		}
		if (elapsed > utimeout) return elapsed;
		else utimeout = utimeout - elapsed;

		// --- run tasks
		runTasks();

		// --- send
		// TODO

		// return elapsed time
		uend = micros();
		if (uend>=ustart) {
			elapsed = uend-ustart;
		} else {
			elapsed = ((999-ustart)+uend);
		}

		// evaluate spare time
		if (elapsed > 999)
			errWrite("LAG (+%d us)\n", elapsed-999);
		else {
			uint8_t txtData, binData, peerData;
			while(elapsed<=996) {
				// read
				if (txtConsole) if (txtConsole->available(txtConsole)) {
					txtConsole->read(txtConsole, &txtData, 1, 1);
					// TODO dispatch txtConsole
				}
				if (binConsole->available(binConsole)) {
					binConsole->read(binConsole, &binData, 1, 1);
					collect(binData);
				}
				if (peering) if (peering->available(peering)) {
					peering->read(peering, &peerData, 1, 1);
					// TODO dispatch peerConsole
				}
				// evaluate elapsed time
				uend = micros();
				if (uend>=ustart) {
					elapsed = uend-ustart;
				} else {
					elapsed = ((999-ustart)+uend);
				}
				//if (elapsed>995) printf("elapsed %dus\n", elapsed);
			}
			//printf("------------------------ TIMEOUT %dus\n", elapsed);
		}

		// log 1 tick every second
		if ((loop_counter / 1000) > last_second) {
			last_second = loop_counter / 1000;
			logWrite("TICK %9d | %5d:%4d | ---\n", loop_counter, last_second, millis());
		}
	}
	//logWrite("Firmware.cpp, loop OUT\n");
}

