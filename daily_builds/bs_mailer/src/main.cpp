#include <iostream>
#include <string>

#include <vmime/vmime.hpp>
#include <vmime/platforms/posix/posixHandler.hpp>

#include "ValueLoader.h"
#include "BSMailer.h"

using namespace std;

///////////////////////////////////////////////////////////////////////
// Common data
const string appName = "mist_gen";

///////////////////////////////////////////////////////////////////////
// Output information about the usage of the tool
static void
usage();

///////////////////////////////////////////////////////////////////////
int 
main(int argc, char* argv[])
{
    if (argc < 2) {
        usage();
        return EXIT_SUCCESS;
    }
    
    // Path to the configuration file.
    string confFile = argv[1];
    
    // Initialize platform-specific stuff for VMime.    
    vmime::platform::
        setHandler <vmime::platforms::posix::posixHandler>();
        
    try {
        // Do the work.
        loadConf(confFile);
        sendMessage();
    } 
    catch (CValueLoader::CLoadingError& e) {
        cerr << "Failed to load " << confFile << ": " << e.what() << endl;
        return EXIT_FAILURE;
    } 
    catch (vmime::exception& e) {
        cerr << "VMime exception: " << e.what() << endl;
        return EXIT_FAILURE;
    }
    catch (runtime_error& e) {
        cerr << "Error: " << e.what() << endl;
        return EXIT_FAILURE;
    } 
    catch (bad_alloc& e) {
        cerr << "Error: not enough memory" << endl;
        return EXIT_FAILURE;
    }
        
    //cout << "Hello, VMime! Conf. file: " << confFile << endl;
    return EXIT_SUCCESS;
}

///////////////////////////////////////////////////////////////////////
static void 
usage()
{
    cout << "Usage: " << appName << " "
         << "<configuration file>" << endl;
    return;
}
///////////////////////////////////////////////////////////////////////

