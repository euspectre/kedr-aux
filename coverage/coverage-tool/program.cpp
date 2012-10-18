#include "trace.hh"

#include "trace_modifier.hh"

#include <iostream>
#include <fstream>

#include <unistd.h> /* getopt */
#include <cstring> /* strcmp */

#include <vector>
#include <memory> /* auto_ptr */

#include <stdexcept> /* runtime_error */

#include <cassert>

/*
 * Usage: See 'usage' file.
 */

using namespace std;

extern char _binary_usage_start[];
extern char _binary_usage_end[];

static void usage(void)
{
    cout.write(_binary_usage_start, _binary_usage_end - _binary_usage_start);
}

/* Base class for processor of the command. */
class CommandProcessor
{
public:
    CommandProcessor();
    virtual ~CommandProcessor();
    
    virtual int parseParams(int argc, char** argv) = 0;
    virtual int exec() = 0;

protected:
    void setOutFile(const char* filename);
    const char* getOutFile(void) const;
    
    ostream& getOutStream(void);
private:
    const char* outFile;
    /* 
     * If outFile is not NULL, output stream will be created when
     * firstly requested. Otherwise this pointer will simply contain
     * cout.
     */
    ostream* outStream;
    
    void resetOutStream(void);
};

/* Program execution for 'diff' command. */
struct DiffProcessor: public CommandProcessor
{
    const char* traceFile;
    const char* subTraceFile;

    DiffProcessor(void);
    
    int parseParams(int argc, char** argv);
    int exec();
};

/* Program execution for 'add' command. */
struct AddProcessor: public CommandProcessor
{
    vector<const char*> traceFiles;
    
    int parseParams(int argc, char** argv);
    int exec();
};


/* Program execution for 'new-coverage' command. */
struct NewCoverageProcessor: public CommandProcessor
{
    const char* traceFile;
    const char* prevTraceFile;

    NewCoverageProcessor(void);

    int parseParams(int argc, char** argv);
    int exec();
};

/* Program execution for 'stat' command */
struct StatProcessor: public CommandProcessor
{
    const char* traceFile;
    const char* format;
    
    StatProcessor(void);

    int parseParams(int argc, char** argv);
    int exec();
private:
    static const char* defaultFormat;
};

int main(int argc, char** argv)
{
    if(argc < 1)
    {
        cerr << argv[0] << ": Command argument is required." << endl;
        usage();
        return 1;
    }


    auto_ptr<CommandProcessor> commandProcessor;
#define isCommand(command) (strcmp(argv[1], command) == 0)
    if(isCommand("diff"))
    {
        commandProcessor.reset(new DiffProcessor());
    }
    else if(isCommand("add"))
    {
        commandProcessor.reset(new AddProcessor());
    }
    else if(isCommand("new-coverage"))
    {
        commandProcessor.reset(new NewCoverageProcessor());
    }
    else if(isCommand("stat"))
    {
        commandProcessor.reset(new StatProcessor());
    }
    else if(isCommand("-h") || isCommand("--help"))
    {
        cerr << "Invalid command: " << argv[1] << endl;
        usage();
    }
    else
    {
        cerr << "Invalid command: " << argv[1] << endl;
        cerr << "Use " << argv[0] << " -h for help" << endl;
        return 1;
    }
#undef isCommand

    if(commandProcessor->parseParams(argc - 1, argv + 1)) return 1;
    
    return commandProcessor->exec();
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

/* 
 * Modifier class, which print warning messages when new elements
 * are found in traces.
 */
class TraceModifierWarnNew: public Trace::Modifier, public Trace::FileInfo::Modifier
{
public:
    void onFileGroupEndNew(Trace& /*traceModified*/)
    {
        cerr << "Traces has different file groups." << endl;
    }

    void onSourceEndNew(Trace::FileGroupID& /*groupModified*/)
    {
        cerr << "Traces has different sources." << endl;
    }

    Trace::FileInfo::Modifier* onSourceStart(
        const std::map<std::string, Trace::FileInfo>::value_type& /*source*/)
    {
        return this;
    }


    void newFunc(Trace::FileInfo& /*fileModified*/)
    {
        cerr << "Traces has different functions." << endl;
    }

    void newBranch(Trace::FileInfo& /*fileModified*/)
    {
        cerr << "Traces has different branches." << endl;
    }

    void newLine(Trace::FileInfo& /*fileModified*/)
    {
        cerr << "Traces has different lines." << endl;
    }
};


/* Base CommandProcessor class */
CommandProcessor::CommandProcessor(): outFile(NULL), outStream(NULL) {}

void CommandProcessor::resetOutStream(void)
{
    if(outStream)
    {
        if(outFile)
        {
            ofstream* realStream = static_cast<ofstream*>(outStream);
            realStream->close();
            delete realStream;
        }
        outStream = NULL;
    }
}

CommandProcessor::~CommandProcessor()
{
    resetOutStream();
}

void CommandProcessor::setOutFile(const char* filename)
{
    resetOutStream();
    outFile = filename;
}

const char* CommandProcessor::getOutFile(void) const
{
    return outFile;
}

ostream& CommandProcessor::getOutStream(void)
{
    if(!outStream)
    {
        if(outFile)
        {
            ofstream* realStream = new ofstream(outFile);
            if(!realStream)
            {
                cerr << "Failed to open file '" << outFile << "' for write." << endl;
                delete realStream;
                throw runtime_error("Failed to open file");
            }
            outStream = realStream;
        }
        else
        {
            outStream = &cout;
        }
    }
    return *outStream;
}


/********************* Difference implementation **********************/
/* Params */
DiffProcessor::DiffProcessor(): traceFile(NULL), subTraceFile(NULL) {}

int DiffProcessor::parseParams(int argc, char** argv)
{
    static const char options[] = "+o:";
    
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
            setOutFile(optarg);
            break;
        default:
            return -1;
        }
    }
    
    char** argv_rest = argv + optind;
    int argc_rest = argc - optind;
    
    if(argc_rest != 2)
    {
        if(argc_rest == 0) cerr << "Trace file is missed." << endl;
        if(argc_rest == 1) cerr << "File with substracted trace is missed." << endl;
        else cerr << "Exceeded command-line argument: " << optarg << endl;
        return -1;
    }
    
    traceFile = argv_rest[0];
    subTraceFile = argv_rest[1];
    
    return 0;
}

/* Trace modifier */
class TraceModifierDiff: public TraceModifierWarnNew
{
public:
    bool onFunction(const map<string, Trace::FuncInfo>::value_type& func)
    {
        /* Needn't process function if its counter non-positive */
        return func.second.counter <= 0;
    }
    void modifyFuncCounter(counter_t counter,
        counter_t& counterModified)
    {
        diffCounter(counter, counterModified);
    }

    bool onBranch(const map<Trace::BranchID, counter_t>::value_type& branch)
    {
        return branch.second <= 0;
    }

    void modifyBranchCounter(counter_t counter, counter_t& counterModified)
    {
        diffCounter(counter, counterModified);
    }

    bool onLine(const map<int, counter_t>::value_type& line)
    {
        return line.second <= 0;
    }

    void modifyLineCounter(counter_t counter, counter_t& counterModified)
    {
        diffCounter(counter, counterModified);
    }
private:
    void diffCounter(counter_t counter, counter_t& counterModified)
    {
        if(counterModified > 0)
        {
            if(counter >= counterModified)
            {
                counterModified = 0;
            }
            else
            {
                counterModified -= counter;
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

    
    TraceModifierDiff modifier;
    modifyTrace(traceSub, modifier, trace);

    ostream& outStream = getOutStream();
    trace.write(outStream);
    if(!outStream)
    {
        cerr << "Errors occure while write trace." << endl;
        return 1;
    }
    
    return 0;
}
/************************* Add implementation *************************/
/* Params */
int AddProcessor::parseParams(int argc, char** argv)
{
    static const char options[] = "+o:";
    
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
            setOutFile(optarg);
            break;
        default:
            return -1;
        }
    }
    
    char** argv_rest = argv + optind;
    int argc_rest = argc - optind;
    
    if(argc_rest < 1)
    {
        cerr << "At least one trace file should be specified for 'add' command." << endl;
        return -1;
    }
    
    traceFiles.insert(traceFiles.end(), argv_rest, argv_rest + argc_rest);
    
    return 0;
}

/* Trace modifier */
class TraceModifierAdd: public TraceModifierWarnNew
{
public:
    bool onFunction(const map<string, Trace::FuncInfo>::value_type& func)
    {
        /* Needn't process function if its counter non-positive */
        return func.second.counter <= 0;
    }
    void modifyFuncCounter(counter_t counter,
        counter_t& counterModified)
    {
        addCounter(counter, counterModified);
    }

    bool onBranch(const map<Trace::BranchID, counter_t>::value_type& branch)
    {
        return branch.second <= 0;
    }

    void modifyBranchCounter(counter_t counter, counter_t& counterModified)
    {
        addCounter(counter, counterModified);
    }

    bool onLine(const map<int, counter_t>::value_type& line)
    {
        return line.second <= 0;
    }

    void modifyLineCounter(counter_t counter, counter_t& counterModified)
    {
        addCounter(counter, counterModified);
    }
private:
    void addCounter(counter_t counter, counter_t& counterModified)
    {
        if(counterModified > 0)
        {
            counterModified += counter;
        }
        else
        {
            counterModified = counter;
        }
    }
};

int AddProcessor::exec()
{
    int result;

    Trace trace;
    result = traceReadFromFile(trace, traceFiles[0]);
    if(result) return result;

    TraceModifierAdd modifier;
    
    for(int i = 1; i < (int)traceFiles.size(); i++)
    {
        Trace traceAdd;
        result = traceReadFromFile(traceAdd, traceFiles[i]);
        if(result) return result;

        modifyTrace(traceAdd, modifier, trace);
    }

    ostream& outStream = getOutStream();
    trace.write(outStream);
    if(!outStream)
    {
        cerr << "Errors occure while write trace." << endl;
        return 1;
    }
    
    return 0;
}

/************************ New coverage implementation *****************/
/* Params */
NewCoverageProcessor::NewCoverageProcessor()
    : traceFile(NULL), prevTraceFile(NULL) {}

int NewCoverageProcessor::parseParams(int argc, char** argv)
{
    static const char options[] = "+o:";
    
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
            setOutFile(optarg);
            break;
        default:
            return -1;
        }
    }
    
    char** argv_rest = argv + optind;
    int argc_rest = argc - optind;
    
    if(argc_rest != 2)
    {
        if(argc_rest == 0) cerr << "Trace file is missed." << endl;
        if(argc_rest == 1) cerr << "File with substracted trace is missed." << endl;
        else cerr << "Exceeded command-line argument: " << optarg << endl;
        return -1;
    }
    
    traceFile = argv_rest[0];
    prevTraceFile = argv_rest[1];
    
    return 0;
}


/* Trace modifier */
class TraceModifierNewCoverage: public TraceModifierWarnNew
{
public:
    bool onFunction(const map<string, Trace::FuncInfo>::value_type& func)
    {
        /* Needn't process function if its counter non-positive */
        return func.second.counter <= 0;
    }
    void modifyFuncCounter(counter_t counter,
        counter_t& counterModified)
    {
        newCounter(counter, counterModified);
    }

    bool onBranch(const map<Trace::BranchID, counter_t>::value_type& branch)
    {
        return branch.second <= 0;
    }

    void modifyBranchCounter(counter_t counter, counter_t& counterModified)
    {
        newCounter(counter, counterModified);
    }

    bool onLine(const map<int, counter_t>::value_type& line)
    {
        return line.second <= 0;
    }

    void modifyLineCounter(counter_t counter, counter_t& counterModified)
    {
        newCounter(counter, counterModified);
    }
private:
    void newCounter(counter_t counter, counter_t& counterModified)
    {
        if(counter > 0)
        {
            counterModified = 0;
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

    TraceModifierNewCoverage modifier;
    modifyTrace(tracePrev, modifier, trace);

    ostream& outStream = getOutStream();
    trace.write(outStream);
    if(!outStream)
    {
        cerr << "Errors occure while write trace." << endl;
        return 1;
    }
    
    return 0;
}

/***************************** Stat implementation ********************/
/* Params */
const char* StatProcessor::defaultFormat =
    "Lines: %pl%% (%l of %L)\n"
    "Functions: %pf%% (%f of %F)\n"
    "Branches: %pb%% (%b of %B)\n";

StatProcessor::StatProcessor(): traceFile(NULL), format(defaultFormat) {}

int StatProcessor::parseParams(int argc, char** argv)
{
    static const char options[] = "+o:f:";
    
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
            setOutFile(optarg);
            break;
        case 'f':
            format = optarg;
            break;
        default:
            return -1;
        }
    }
    
    char** argv_rest = argv + optind;
    int argc_rest = argc - optind;
    
    if(argc_rest != 1)
    {
        if(argc_rest == 0) cerr << "Trace file is missed." << endl;
        else cerr << "Exceeded command-line argument: " << optarg << endl;
        return -1;
    }
    
    traceFile = argv_rest[0];
    
    return 0;
}

/* Output */

class StatPrinter
{
public:
    StatPrinter(Trace& trace, ostream& os);
    
    void print(const char* format);
private:
    Trace& trace;
    ostream& os;
    /* Print specificator. Return pointer to the end of specificator. */
    const char* printSpec(const char* specPointer);
    /* Print escape sequence. Return pointer to the end of the sequence. */
    const char* printEsc(const char* seqPointer);

    int linesTotal;
    int linesTotalHit;
    int functionsTotal;
    int functionsTotalHit;
    int branchesTotal;
    int branchesTotalHit;
    
    int getLinesTotal(void)
    {
        if(linesTotal == -1) linesTotal = trace.linesTotal();
        return linesTotal;
    }
    int getLinesTotalHit(void)
    {
        if(linesTotalHit == -1) linesTotalHit = trace.linesTotalHit();
        return linesTotalHit;
    }
    int getFunctionsTotal(void)
    {
        if(functionsTotal == -1) functionsTotal = trace.functionsTotal();
        return functionsTotal;
    }
    int getFunctionsTotalHit(void)
    {
        if(functionsTotalHit == -1) functionsTotalHit = trace.functionsTotalHit();
        return functionsTotalHit;
    }
    int getBranchesTotal(void)
    {
        if(branchesTotal == -1) branchesTotal = trace.branchesTotal();
        return branchesTotal;
    }
    int getBranchesTotalHit(void)
    {
        if(branchesTotalHit == -1) branchesTotalHit = trace.branchesTotalHit();
        return branchesTotalHit;
    }

    void printPercent(int a, int A);
};

StatPrinter::StatPrinter(Trace& trace, ostream& os)
    : trace(trace), os(os),
        linesTotal(-1), linesTotalHit(-1),
        functionsTotal(-1), functionsTotalHit(-1),
        branchesTotal(-1), branchesTotalHit(-1)
{}

void StatPrinter::print(const char* format)
{
    const char* p = format;
    while(*p) switch(*p)
    {
    case '%':
        p = printSpec(p + 1);
        break;
    case '\\':
        p = printEsc(p + 1);
        break;
    default:
        os << *p;
        p++;
        break;
    }
}

const char* StatPrinter::printSpec(const char* specPointer)
{
    int count;
    switch(*specPointer)
    {
    case 'l':
        count = getLinesTotalHit();
        cout << "Counter is " << count << "." << endl;
        os << count;
        break;
    case 'L':
        os << getLinesTotal();
        break;
    case 'f':
        os << getFunctionsTotalHit();
        break;
    case 'F':
        os << getFunctionsTotal();
        break;
    case 'b':
        os << getBranchesTotalHit();
        break;
    case 'B':
        os << getBranchesTotal();
        break;
    case 'p':
        switch(specPointer[1])
        {
        case 'l':
            printPercent(getLinesTotalHit(), getLinesTotal());
            break;
        case 'f':
            printPercent(getFunctionsTotalHit(), getFunctionsTotal());
            break;
        case 'b':
            printPercent(getBranchesTotalHit(), getBranchesTotal());
            break;
        case '\0':
            cerr << "WARINING: Unknown specificator '%p' at the end of stat format." << endl;
            return specPointer;
        default:
            cerr << "WARINING: Unknown specificator '%p" << specPointer[1] << "' in stat format." << endl;
            return specPointer;
        }
        return specPointer + 2;
    case '%':
        os << '%';
        break;
    case '\0':
        cerr << "WARINING: '%' at the end of stat format." << endl;
        return specPointer;
    default:
        cerr << "WARINING: Unknown specificator '%" << specPointer[0] << "' in stat format." << endl;
        return specPointer;
    }
    return specPointer + 1;
}

const char* StatPrinter::printEsc(const char* seqPointer)
{
    switch(*seqPointer)
    {
    case '\\':
        os << '\\';
        break;
    case 'n':
        os << '\n';
        break;
    case 't':
        os << '\t';
        break;
    case '\0':
        cerr << "WARINING: '\\' at the end of stat format." << endl;
        return seqPointer;
    default:
        cerr << "WARINING: Unknown escape sequence '\\" << seqPointer[0] << "' in stat format." << endl;
        return seqPointer;
    }
    return seqPointer + 1;
}

void StatPrinter::printPercent(int a, int A)
{
    assert(a <= A);
    if(A == 0)
    {
        os << '*';
    }
    int whole = (a * 100) / A;
    int frac = (a * 10000 / A - whole * 100);
    
    if((whole == 0) && (frac == 0))
    {
        if(a > 0) frac = 1;/* 0% only when a == 0 */
    }
    
    os << whole << ".";
    
    int oldWidth = os.width(2);
    os << frac;
    os.width(oldWidth);
}

/* Exec */
int StatProcessor::exec()
{
    int result;
    
    Trace trace;
    result = traceReadFromFile(trace, traceFile);
    if(result) return result;

    trace.groupFiles();
    
    ostream& outStream = getOutStream();
    StatPrinter printer(trace, outStream);
    printer.print(format);

    if(!outStream)
    {
        cerr << "Errors occure while write statistic." << endl;
        return 1;
    }
    
    return 0;
}
