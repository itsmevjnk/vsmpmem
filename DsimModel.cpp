#define _CRT_SECURE_NO_WARNINGS // fopen_s is not available in older Windows versions

#include "DsimModel.h"
#include <stdio.h>
#include <string.h>

INT DsimModel::isdigital(CHAR* pinname) {
	return TRUE;											// Indicates all the pins are digital
}


VOID DsimModel::setup(IINSTANCE* instance, IDSIMCKT* dsimckt) {
	_inst = instance;
	_ckt = dsimckt;

	/* connect control pins */
	_ce_pin = _inst->getdsimpin((char*)"$CE$", true); _ce_pin->setstate(FLT);
	_we_pin = _inst->getdsimpin((char*)"$WE$", false); if (_we_pin) _we_pin->setstate(FLT);
	_oe_pin = _inst->getdsimpin((char*)"$OE$", false);
	if (!_oe_pin) _oe_pin = _inst->getdsimpin((char*)"$OE$/VPP", true); // OE is often multiplexed with VPP (eg. 27C32)
	_oe_pin->setstate(FLT);

	char pin_name[4];
	/* connect data pins */
	pin_name[0] = 'D'; pin_name[2] = '\0'; // we only need to change pin_name[1] to '0' through '7'
	for (size_t i = 0; i < 8; i++) {
		pin_name[1] = (char)i + '0';
		_data_pins[i] = _inst->getdsimpin(pin_name, true);
		_data_pins[i]->setstate(FLT);
	}
	/* connect address pins */
	pin_name[0] = 'A';
	for (size_t i = 0; ; i++) {
		snprintf(&pin_name[1], 3, "%d", (int)i);
		IDSIMPIN* pin = _inst->getdsimpin(pin_name, false); // will return NULL if the pin isn't found
		if (!pin) break; // no more pins to discover
		pin->setstate(FLT);
		_addr_pins.push_back(pin);
	}
	
	size_t size = 1ULL << _addr_pins.size();
	if (size == 1) {
		_inst->fatal((char*)"No address pins found!"); // 1 << 0
		return;
	}
	
	_memory = new uint8_t[size]; // allocate memory buffer
	memset(_memory, _inst->getinitval((char*)"INITVAL", 0xFF), size);

	_inst->message((char*)"Simulating a %lu byte parallel %s.", size, (_we_pin) ? "RAM" : "ROM");

	/* read initial memory dump if applicable */
	const char* mem_file_name = _inst->getstrval((char*)"FILE");
	if (mem_file_name && mem_file_name[0]) {
		/* file specified */
		FILE* mem_file = fopen(mem_file_name, "rb");
		if (!mem_file) {
			_inst->fatal((char*)"Cannot open file \"%s\" for reading.", mem_file_name);
			return; // NOTE: probably not actually needed, but we put it here so VS won't complain
		}

		fseek(mem_file, _inst->getinitval((char*)"BASE"), SEEK_SET); // seek to base address
		size_t inc = 1ULL << _inst->getinitval((char*)"SHIFT", 0);; // get address shift and convert to increment

		size_t read = 0; // number of bytes read
		if (inc == 1) read = fread(_memory, size, 1, mem_file); // linear read - issue a single fread
		else {
			/* non-linear read */
			for (; read < size; read++) {
				if (!fread(&_memory[read], 1, 1, mem_file)) break; // end of file reached
				fseek(mem_file, (long)inc - 1, SEEK_CUR); // skip bytes
			}
		}
		_inst->log((char*)"Read %lu bytes from \"%s\".", read, mem_file_name);

		fclose(mem_file);
	}
}

DsimModel::~DsimModel() {
	if (_memory) delete _memory; // deallocate memory
}

VOID DsimModel::runctrl(RUNMODES mode) {
	
}

VOID DsimModel::actuate(REALTIME time, ACTIVESTATE newstate) {

}

BOOL DsimModel::indicate(REALTIME time, ACTIVEDATA* data) {
	return FALSE;
}

/* fixed isdefined (for SUD and whatnot) */
inline BOOL isundefined(STATE s) {
	return (s & SP_MASK) == SP_UNDEFINED;
}

VOID DsimModel::simulate(ABSTIME time, DSIMMODES mode) {
	// if (mode == DSIMSETTLE) return; // ignore settling phase
	//if (mode == DSIMBOOT) {
	//	float_data(time);
	//	return;
	//}

	STATE ce = _ce_pin->istate();
	STATE oe = _oe_pin->istate();
	STATE we = (_we_pin) ? _we_pin->istate() : SHI; // default to high (no writing) if WE pin doesn't exist (ie. ROM)

	uintptr_t addr = get_address();
	if (isundefined(ce) || isundefined(oe) || isundefined(we) || isfloating(ce) || ishigh(ce) || isfloating(oe) || isfloating(we) || ishigh(oe) == ishigh(we)/* || addr == UINTPTR_MAX*/) {
		/* chip not selected for reading/writing or is in undefined state - go Hi-Z and ignore */
		float_data(time);
		return;
	}

	if (!ishigh(we)) {
		/* write */
		size_t data = get_data();
		/*if (data != SIZE_MAX)*/ _memory[addr] = (uint8_t)data;
	}
	else {
		/* read */
		set_data(time, _memory[addr]);
	}
}

VOID DsimModel::callback(ABSTIME time, EVENTID eventid) {

}

uintptr_t DsimModel::get_address() {
	uintptr_t result = 0;
	size_t bits = _addr_pins.size();
	for (size_t i = 0; i < bits; i++) {
		STATE state = _addr_pins[i]->istate();
		//if (isfloating(state)) return UINTPTR_MAX;

		if (ishigh(state)) result |= (1ULL << i);
	}
	return result;
}

size_t DsimModel::get_data() {
	size_t result = 0;
	for (size_t i = 0; i < 8; i++) {
		STATE state = _data_pins[i]->istate();
		//if (isfloating(state)) return SIZE_MAX;

		if (ishigh(state)) result |= (1ULL << i);
	}
	return result;
}

void DsimModel::set_data(ABSTIME time, uint8_t data) {
	for (size_t i = 0; i < 8; i++, data >>= 1) _data_pins[i]->setstate(time, 1, (data & 1) ? SHI : SLO);
}

void DsimModel::float_data(ABSTIME time) {
	for (int i = 0; i < 8; i++) _data_pins[i]->setstate(time, 1, FLT);
}