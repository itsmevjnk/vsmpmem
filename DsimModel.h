#pragma once

#include "sdk/vsm.hpp"

#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <vector>

class DsimModel : public IDSIMMODEL
{
public:
	~DsimModel();

	/* IDSIMMODEL methods */
	INT isdigital(CHAR* pinname);
	VOID setup(IINSTANCE* inst, IDSIMCKT* dsim);
	VOID runctrl(RUNMODES mode);
	VOID actuate(REALTIME time, ACTIVESTATE newstate);
	BOOL indicate(REALTIME time, ACTIVEDATA* data);
	VOID simulate(ABSTIME time, DSIMMODES mode);
	VOID callback(ABSTIME time, EVENTID eventid);
private:
	IINSTANCE* _inst;
	IDSIMCKT* _ckt;

	/* pins to be connected */
	std::vector<IDSIMPIN*> _addr_pins;
	IDSIMPIN* _data_pins[8];
	IDSIMPIN* _ce_pin;
	IDSIMPIN* _oe_pin;
	IDSIMPIN* _we_pin; // if null then it's a ROM

	uint8_t* _memory = nullptr; // memory buffer

	uintptr_t get_address(); // get address from pins (return UINTPTR_MAX if address line is floating)
	size_t get_data(); // get data from pins (return SIZE_MAX if data line is floating)

	void set_data(ABSTIME time, uint8_t data); // set data to pins
	void float_data(ABSTIME time); // float the data bus
};
