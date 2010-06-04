#ifndef BSMAILER_H_1353_INCLUDED
#define BSMAILER_H_1353_INCLUDED

#include <stdexcept>
#include <string>
#include <vector>

#include "Common.h"

// Loads parameters from the specified .conf file.
// Throws exceptions in case of errors.
void
loadConf(std::string confFile);

// Prepares and sends the message using the loaded configuration.
// Throws exceptions in case of errors.
void 
sendMessage();

#endif // BSMAILER_H_1353_INCLUDED
