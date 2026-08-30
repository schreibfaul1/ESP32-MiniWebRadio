#include "meteo.h"

METEO::METEO(){}

METEO::~METEO(){}

void METEO::begin(){}

bool METEO::set_coordinates(ps_ptr<char> latitude, ps_ptr<char> longitude){return false;}

bool METEO::send_request(ps_ptr<char> req){return false;}

bool METEO::parseHttpResponseHeader(){return false;}

void METEO::loop(){}