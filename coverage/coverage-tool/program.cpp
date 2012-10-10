#include "trace.hh"

#include "trace_modifier.hh"

#include <iostream>
#include <fstream>

#include <unistd.h> /* getopt */
#include <cstring> /* strcmp */

#include <vector>



/*
 * Usage:
 * 
 * coverage_tool <command> args...
 * 
 * where 'command' and its argument may be:
 * 
 * ---  diff <trace-file> <sub-trace-file> [-o <out-file>]
 * 
 *       Substract counters of trace, contained in 'sub-trace-file',
 *     from corresponded counters of trace in 'trace-file'.
 *     Resulted trace is written into stdout or 'out-file'.
 *
 * 
 * ---  add <trace-file> ... [-o <out-file>]
 * 
 *       Add corresponded counters in all traces.
 *     Resulted trace is written into stdout or 'out-file'.
 *
 * 
 * ---  new-coverage <trace-file> <trace-file-prev> [-o <out-file>]
 * 
 *       Determine
 * All positive counters in trace, contained which also positive in
 *          'sub-trace-file',
 *     from corresponded counters of trace in 'trace-file'.
 *     Resulted trace is written into stdout or 'out-file'.
 */

using namespace std;

static void usage(const char* commandName)
{
    cerr << "Usage:" << endl
        << "\t" << commandName << "diff <trace> <trace-sub> [-o <out-file>]" << endl
        << "\t" << commandName << "add <trace> ... [-o <out-file>]" << endl
        << "\t" << commandName << "new-coverage <trace> <trace-prev> [-o <out-file>]" << endl
        << endl
        << "'diff' command compute per-counter difference between <trace> and <trace-sub>" << endl
        << "\tThat is, set every counter in <trace>, which is positive and for which" << endl
        << "\tcorrespondent counter is positive to the difference between these counters." << endl
        << "\tIf resulter counter become negative, it is set to 0." << endl
        << endl
        << "'add' command compute per-counter sum for all given traces." << endl
        << endl
        << "'new-coverage' command reset(set to 0) all positive counters in <trace>" << endl
        << "\nfor which corresponded counters in <trace-prev> are also positive." << endl;
}

/* Base class for commands parameters and parsing process. */
struct ParamsBase
{
    const char* outFile;
    
    ParamsBase();
    virtual ~ParamsBase() {}

    int parseParams(int argc, char** argv);

    /* Called when no-option argument is encountered while parse params. */
    virtual int onNoOpt(const char* arg) = 0;
    /* Called after all parameters has been parsed. */
    virtual int checkState(void) = 0;
    
    /* Output trace into corresponded file (or stdout) */
    int outTrace(Trace& trace);
};

/* Program execution for 'diff' command. */
struct DiffProcessor: public ParamsBase
{
    const char* traceFile;
    const char* subTraceFile;

    DiffProcessor(void);

    int onNoOpt(const char* arg);

    int checkState(void);
    
    int exec();
};

/* Program execution for 'add' command. */
struct AddProcessor: public ParamsBase
{
    vector<const char*> traceFiles;
    
    int onNoOpt(const char* arg);

    int checkState(void);
    
    int exec();
};


/* Program execution for 'new-coverage' command. */
struct NewCoverageProcessor: public ParamsBase
{
    const char* traceFile;
    const char* prevTraceFile;

    NewCoverageProcessor(void);

    int onNoOpt(const char* arg);

    int checkState(void);
    
    int exec();
};


int main(int argc, char** argv)
{
    if(argc < 1)
    {
        cerr << argv[0] << ": Command argument is required." << endl;
        usage(argv[0]);
        return 1;
    }
    
    if(strcmp(argv[1], "diff") == 0)
    {
        DiffProcessor diffProcessor;
        if(diffProcessor.parseParams(argc - 1, argv + 1)) return 1;
        
        return diffProcessor.exec();
    }
    else if(strcmp(argv[1], "add") == 0)
    {
        AddProcessor addProcessor;
        if(addProcessor.parseParams(argc - 1, argv + 1)) return 1;
        
        return addProcessor.exec();
    }
    else if(strcmp(argv[1], "new-coverage") == 0)
    {
        NewCoverageProcessor newCoverageProcessor;
        if(newCoverageProcessor.parseParams(argc - 1, argv + 1)) return 1;
        
        return newCoverageProcessor.exec();
    }
    else
    {
        cerr << "Invalid command: " << argv[1] << endl;
        usage(argv[0]);
    }
    
    return 0;
}

/************************* Helpers ************************************/


/* 
 * Read trace from given file.
 * 
 * Return 0 on success and negative error on fail.
 */
static int traceReadFromFile(Trace& trace, const char* filename)
{
    ifstream is(filename);
    if(!is)
    {
        cerr << "Failed to open file '"  << filename << "' for read trace." << endl;
        return 1;
    }
    
    trace.read(is, filename);
    
    return 0;
}
/* Print warnings about difference in elements in the traces */
class EventNotifierWarnNew: public TraceModifierEventNotifier
{
public:
    void onFileGroupEndNew()
    {
        cerr << "Traces has different file groups." << endl;
    }

    void onSourceEndNew()
    {
        cerr << "Traces has different sources." << endl;
    }
    
    void newFunc()
    {
        cerr << "Traces has different functions." << endl;
    }

    void newBranch()
    {
        cerr << "Traces has different branches." << endl;
    }

    void newLine()
    {
        cerr << "Traces has different lines." << endl;
    }
};

/* Base parameters class */
ParamsBase::ParamsBase(): outFile(NULL) {}

int ParamsBase::parseParams(int argc, char** argv)
{
    static const char options[] = "-o:";
    int result;
    
    for(int opt = getopt(argc, argv, options);
        opt != -1;
        opt = getopt(argc, argv, options))
    {
        switch(opt)
        {
        case '?':
            //error in options
            return -1;
        case 'o':
            if(outFile)
            {
                cerr << "Double '-o' option" << endl;
                return -1;
            }
            outFile = optarg;
            break;
        case 1:
            result = onNoOpt(optarg);
            if(result) return result;
            break;
        default:
            return -1;
        }
    }
    return checkState();
}

int ParamsBase::outTrace(Trace& trace)
{
    if(outFile)
    {
        ofstream os(outFile);
        if(!os)
        {
            cerr << "Failed to open file '"  << outFile << "' for write trace." << endl;
            return 1;
        }
        trace.write(os);
    }
    else
    {
        trace.write(cout);
    }

    return 0;
}

/********************* Difference implementation **********************/
/* Params */
DiffProcessor::DiffProcessor(): traceFile(NULL), subTraceFile(NULL) {}

int DiffProcessor::onNoOpt(const char* arg)
{
    if(!traceFile)
    {
        traceFile = optarg;
    }
    else if(!subTraceFile)
    {
        subTraceFile = optarg;
    }
    else
    {
        cerr << "Exceeded command-line argument: " << optarg << endl;
        return -1;
    }
    
    return 0;
}

int DiffProcessor::checkState(void)
{
    if(!traceFile)
    {
        cerr << "Trace file is missed." << endl;
        return -1;
    }
    else if(!subTraceFile)
    {
        cerr << "File for substracted trace is missed." << endl;
        return -1;
    }
    
    return 0;
}

/* Trace */
class EventNotifierDiff: public EventNotifierWarnNew
{
public:
    bool onFunction(const map<string, Trace::FuncInfo>::value_type& func)
    {
        /* Needn't process function if its counter non-positive */
        return func.second.counter <= 0;
    }
    void modifyFuncCounter(counter_t& counter,
        counter_t counterAnother)
    {
        diffCounter(counter, counterAnother);
    }

    bool onBranch(const map<Trace::BranchID, counter_t>::value_type& branch)
    {
        return branch.second <= 0;
    }

    void modifyBranchCounter(counter_t& counter, counter_t counterAnother)
    {
        diffCounter(counter, counterAnother);
    }

    bool onLine(const map<int, counter_t>::value_type& line)
    {
        return line.second <= 0;
    }

    void modifyLineCounter(counter_t& counter, counter_t counterAnother)
    {
        diffCounter(counter, counterAnother);
    }
private:
    void diffCounter(counter_t& counter, counter_t counterAnother)
    {
        if(counter > 0)
        {
            if(counterAnother >= counter)
            {
                counter = 0;
            }
            else
            {
                counter -= counterAnother;
            }
        }
    }
};

int DiffProcessor::exec()
{
    int result;
    
    Trace trace;
    result = traceReadFromFile(trace, traceFile);
    if(result) return result;
    
    Trace traceSub;
    result = traceReadFromFile(traceSub, subTraceFile);
    if(result) return result;

    
    EventNotifierDiff notifier;
    modifyTrace(trace, traceSub, notifier);

    return outTrace(trace);
}
/************************* Add implementation *************************/
/* Params */

int AddProcessor::onNoOpt(const char* arg)
{
    traceFiles.push_back(optarg);
    return 0;
}

int AddProcessor::checkState()
{
    if(traceFiles.empty())
    {
        cerr << "At least one trace should be given for 'add' command." << endl;
        return -1;
    }
    
    return 0;
}

/* Trace */
class EventNotifierAdd: public EventNotifierWarnNew
{
public:
    bool onFunction(const map<string, Trace::FuncInfo>::value_type& func)
    {
        /* Needn't process function if its counter non-positive */
        return func.second.counter <= 0;
    }
    void modifyFuncCounter(counter_t& counter,
        counter_t counterAnother)
    {
        addCounter(counter, counterAnother);
    }

    bool onBranch(const map<Trace::BranchID, counter_t>::value_type& branch)
    {
        return branch.second <= 0;
    }

    void modifyBranchCounter(counter_t& counter, counter_t counterAnother)
    {
        addCounter(counter, counterAnother);
    }

    bool onLine(const map<int, counter_t>::value_type& line)
    {
        return line.second <= 0;
    }

    void modifyLineCounter(counter_t& counter, counter_t counterAnother)
    {
        addCounter(counter, counterAnother);
    }
private:
    void addCounter(counter_t& counter, counter_t counterAnother)
    {
        if(counter > 0)
        {
            counter+= counterAnother;
        }
        else
        {
            counter = counterAnother;
        }
    }
};

int AddProcessor::exec()
{
    int result;

    Trace trace;
    result = traceReadFromFile(trace, traceFiles[0]);
    if(result) return result;

    EventNotifierAdd notifier;
    
    for(int i = 1; i < traceFiles.size(); i++)
    {
        Trace traceAdd;
        result = traceReadFromFile(traceAdd, traceFiles[i]);
        if(result) return result;

        modifyTrace(trace, traceAdd, notifier);
    }

    return outTrace(trace);
}

/************************ New coverage implementation *****************/
/* Params */
NewCoverageProcessor::NewCoverageProcessor()
    : traceFile(NULL), prevTraceFile(NULL) {}

int NewCoverageProcessor::onNoOpt(const char* arg)
{
    if(!traceFile)
    {
        traceFile = optarg;
    }
    else if(!prevTraceFile)
    {
        prevTraceFile = optarg;
    }
    else
    {
        cerr << "Exceeded command-line argument: " << optarg << endl;
        return -1;
    }

    return 0;
}

int NewCoverageProcessor::checkState()
{
    if(!traceFile)
    {
        cerr << "Trace file is missed." << endl;
        return -1;
    }
    else if(!prevTraceFile)
    {
        cerr << "File for base(previous) trace is missed." << endl;
        return -1;
    }
    
    return 0;
}

/* Trace */
class EventNotifierNewCoverage: public EventNotifierWarnNew
{
public:
    bool onFunction(const map<string, Trace::FuncInfo>::value_type& func)
    {
        /* Needn't process function if its counter non-positive */
        return func.second.counter <= 0;
    }
    void modifyFuncCounter(counter_t& counter,
        counter_t counterAnother)
    {
        newCounter(counter, counterAnother);
    }

    bool onBranch(const map<Trace::BranchID, counter_t>::value_type& branch)
    {
        return branch.second <= 0;
    }

    void modifyBranchCounter(counter_t& counter, counter_t counterAnother)
    {
        newCounter(counter, counterAnother);
    }

    bool onLine(const map<int, counter_t>::value_type& line)
    {
        return line.second <= 0;
    }

    void modifyLineCounter(counter_t& counter, counter_t counterAnother)
    {
        newCounter(counter, counterAnother);
    }
private:
    void newCounter(counter_t& counter, counter_t counterAnother)
    {
        if(counterAnother > 0)
        {
            counter = 0;
        }
    }
};

int NewCoverageProcessor::exec()
{
    int result;
    
    Trace trace;
    result = traceReadFromFile(trace, traceFile);
    if(result) return result;

    Trace tracePrev;
    result = traceReadFromFile(tracePrev, prevTraceFile);
    if(result) return result;

    EventNotifierNewCoverage notifier;
    modifyTrace(trace, tracePrev, notifier);

    return outTrace(trace);
}
