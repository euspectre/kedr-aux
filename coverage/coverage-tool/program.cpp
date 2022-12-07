// program.cpp - main procedure

//      
//      Copyright (C) 2012, Institute for System Programming
//                          of the Russian Academy of Sciences (ISPRAS)
//      Author:
//          Andrey Tsyvarev <tsyvarev@ispras.ru>
//      
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.


#include "trace_simple.hh"

#include "test_set_optimizer.hh"
#include "do_trace_operation.hh"

#include <iostream>
#include <fstream>

#include <unistd.h> /* getopt */
#include <cstring> /* strcmp */

#include <vector>
#include <memory> /* auto_ptr */

#include <stdexcept> /* runtime_error */

#include <cassert>

#include <unistd.h> /* FILE, getline() */
#include <stdlib.h> /* strtod */

#include <dlfcn.h> /* dlopen() and others */

typedef TraceSimple::counter_t counter_t;

/*
 * Usage: See 'usage*' files.
 */

using namespace std;

// Declare print_<filename>() function, which prints content of shipped file.
#define DEFINE_FILE_PRINTER(filename) \
extern char _binary_##filename##_start[]; \
extern char _binary_##filename##_end[]; \
static void print_##filename(void) \
{ \
    cout.write(_binary_##filename##_start, _binary_##filename##_end - _binary_##filename##_start); \
}

DEFINE_FILE_PRINTER(usage)
static void usage(void)
{
    print_usage();
}


/* Base class for processor of the command. */
class CommandProcessor
{
public:
    CommandProcessor();
    virtual ~CommandProcessor();
    
    virtual int parseParams(int argc, char** argv) = 0;
    virtual int exec() = 0;
    virtual void usage(void) = 0;

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

struct OperationProcessor: public CommandProcessor
{
    OperationProcessor(void);
    OperationProcessor(const char* opName);
    
    ~OperationProcessor();
    
    int parseParams(int argc, char** argv);
    int exec();
    
    void usage(void);

    class TraceOperationFactory
    {
    public:
        virtual ~TraceOperationFactory() {}
        
        virtual TraceOperation* getOperation(int n, const map<string, string>& params) = 0;
        virtual void putOperation(TraceOperation* op) = 0;
    };

private:    
    TraceOperationFactory* opFactory;
    TraceOperation* operation;
    
    vector<const char*> traceFiles;
    
    void reset();
    
    static TraceOperationFactory* getOperationFactory(const char* opName);
};

/* Program execution for 'stat' command */
struct StatProcessor: public CommandProcessor
{
    const char* traceFile;
    const char* format;
    const char* prefix;
    
    StatProcessor(void);

    int parseParams(int argc, char** argv);
    int exec();
    
    void usage(void);
private:
    static const char* defaultFormat;
};

/* Program execution for 'optimize-tests' */
struct OptimizeTestsProcessor: public CommandProcessor
{
    const char* testsFile;
    bool verbose;
    
    OptimizeTestsProcessor(void);

    int parseParams(int argc, char** argv);
    int exec();
    
    void usage(void);
private:
    /* Load tests from file. */
    void loadTests(vector<TestCoverageDesc>& tests);
};

static CommandProcessor* selectCommand(const char* cmd)
{
#define isCommand(command) (strcmp(cmd, command) == 0)
    if(isCommand("operation"))
    {
         return new OperationProcessor();
    }
    else if(isCommand("diff"))
    {
        return new OperationProcessor("diff");
    }
    else if(isCommand("add"))
    {
        return new OperationProcessor("add");
    }
    else if(isCommand("new-coverage"))
    {
        return new OperationProcessor("new-coverage");
    }
    else if(isCommand("stat"))
    {
        return new StatProcessor();
    }
    else if(isCommand("optimize-tests"))
    {
        return new OptimizeTestsProcessor();
    }
#undef isCommand
    else return NULL;
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        cerr << argv[0] << ": Command argument is required." << endl;
        usage();
        return 1;
    }

    unique_ptr<CommandProcessor> commandProcessor(selectCommand(argv[1]));

    if(commandProcessor.get())
    {
        if(commandProcessor->parseParams(argc - 1, argv + 1)) return 1;
        return commandProcessor->exec();
    }
    else if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
    {
        if(argc > 2)
        {
            commandProcessor.reset(selectCommand(argv[2]));
            if(commandProcessor.get())
            {
                commandProcessor->usage();
                return 0;
            }
        }
        usage();
        return 0;
    }
    else
    {
        cerr << "Invalid command: " << argv[1] << endl;
        cerr << "Use " << argv[0] << " -h for help" << endl;
        return 1;
    }
}

/************************* Helpers ************************************/
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

/******************* OperationProcessor implementation ****************/
void OperationProcessor::reset(void)
{
    if(operation)
    {
        opFactory->putOperation(operation);
        operation = NULL;
    }
    
    if(opFactory)
    {
        delete opFactory;
        opFactory = NULL;
    }
    
    traceFiles.clear();
}

OperationProcessor::~OperationProcessor()
{
    reset();
}

OperationProcessor::OperationProcessor(void):
    opFactory(NULL),
    operation(NULL)
{
}

OperationProcessor::OperationProcessor(const char* opName):
    opFactory(getOperationFactory(opName)),
    operation(NULL)
{
}

static void readParameters(const char* arg, map<string, string>& params)
{
    string paramName, paramValue;
    const char* paramEnd;
    
    for(; *arg; arg = *paramEnd ? paramEnd + 1 : paramEnd)
    {
        paramEnd = strchrnul(arg, ',');
        
        const char* equalPos = strchrnul(arg, '=');
        if(equalPos < paramEnd)
        {
            paramName = string(arg, equalPos - arg);
            paramValue = string(equalPos + 1, paramEnd - equalPos - 1);
        }
        else
        {
            paramName = string(arg, paramEnd - arg);
            paramValue = "";
        }
        
        params[paramName] = paramValue;
    }
}

//debug
//#include <algorithm>
//static void printParam(const map<string,string>::value_type& value)
//{
    //cerr << value.first;
    //if(value.second.size() > 0)
    //{
        //cerr << ":" << value.second;
    //}
    
    //cerr << endl;
//}

int OperationProcessor::parseParams(int argc, char** argv)
{
    if(!opFactory)
    {
        if(argc < 2)
        {
            cerr << "Error: operation name is required" << endl;
            return 1;
        }
        
        try
        {
            opFactory = getOperationFactory(argv[1]);
        }
        catch(exception& ex)
        {
            cerr << "Error: " << ex.what() << endl;
            return 1;
        }
        
        --argc;
        ++argv;
    }
    
    static const char options[] = "+o:p:";
    
    map<string,string> params;
    
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
        case 'p':
            readParameters(optarg, params);
            break;
        default:
            return 1;
        }
    }
    
    //debug
    //cerr << "Params are:" << endl;
    //for_each(params.begin(), params.end(), printParam);
    
    argc -= optind;
    argv += optind;
    
    if(argc == 0)
    {
        cerr << "Error: At least one trace should be given for operation." << endl;
        return 1;
    }
    
    int n = argc;
    
    traceFiles.resize(n);

    for(int i = 0; i < n; i++)
    {
        traceFiles[i] = argv[i];
    }
    
    try
    {
        operation = opFactory->getOperation(n, params);
    }
    catch(exception& ex)
    {
        cerr << "Error: " << ex.what() << endl;
        return 1;
    }
    if(!operation) return 1;
    
    return 0;
}

int OperationProcessor::exec(void)
{
    int n = traceFiles.size();
    
    vector<TraceSimple> operands(n);
    
    for(int i = 0; i < n; i++)
        operands[i].read(traceFiles[i]);
    
    TraceSimple trace;
    
    doTraceOperation(*operation, operands, trace);
    
    ostream& outStream = getOutStream();
    trace.write(outStream);
    if(!outStream)
    {
        cerr << "Errors occure while write trace." << endl;
        return 1;
    }
    
    return 0;
}

DEFINE_FILE_PRINTER(usage_operation)

void OperationProcessor::usage(void)
{
    //TODO: If operation is given, print its own usage.
    print_usage_operation();
}

/*******************Container for internal trace operation ************/
class TraceOperationFactoryInternal:
    public OperationProcessor::TraceOperationFactory
{
protected:
    void putOperation(TraceOperation* op) {delete op;}
};
/********************* Difference implementation **********************/
/* Trace operation*/
class TraceOperationDiff: public TraceOperationUnified
{
public:
    counter_t counterOperation(const vector<counter_t>& operands)
    {
        counter_t op1 = operands[0], op2 = operands[1];
        if(op1 > 0)
        {
            if(op2 >= op1)
            {
                return 0;
            }
            else
            {
                return op1 - op2;
            }
        }
        else
        {
            return op1;
        }
    }
};

/* Trace operation factory. */
class TraceOperationFactoryDiff: public TraceOperationFactoryInternal
{
protected:
    TraceOperation* getOperation(int n, const map<string,string>&)
    {
        if(n != 2)
        {
            throw logic_error("Diff operation accepts precisely 2 trace arguments");
        }
        
        return new TraceOperationDiff();
    }
};

/************************* Add implementation *************************/
/* Trace operation*/
class TraceOperationAdd: public TraceOperationUnified
{
public:
    TraceOperationAdd(int n): n(n) {}

    counter_t counterOperation(const vector<counter_t>& operands)
    {
        counter_t result = operands[0];
        
        for(int i = 1; i < n; i++)
        {
            counter_t op = operands[i];
            if(op >= 0)
            {
                if(result < 0) result = 0;
                result += op;
            }
        }
        
        return result;
    }
private:
    int n;
};

/* Trace operation factory. */
class TraceOperationFactoryAdd: public TraceOperationFactoryInternal
{
protected:
    TraceOperation* getOperation(int n, const map<string,string>&)
    {
        return new TraceOperationAdd(n);
    }
};

/************************ New coverage implementation *****************/
/* Trace operation*/
class TraceOperationNewCoverage: public TraceOperationUnified
{
public:
    counter_t counterOperation(const vector<counter_t>& operands)
    {
        if(operands[1] > 0) return 0;
        else return operands[0];
    }
};

/* Trace operation container. */
class TraceOperationFactoryrNewCoverage: public TraceOperationFactoryInternal
{
protected:
    TraceOperation* getOperation(int n, const map<string,string>&)
    {
        if(n != 2)
        {
            throw logic_error("New coverage operation accepts precisely 2 trace arguments");
        }

        return new TraceOperationNewCoverage();
    }
};
/*************** User-defined operation, loaded from file *************/
class TraceOperationFactoryUser: public OperationProcessor::TraceOperationFactory
{
public:
    TraceOperationFactoryUser(const char* filename): filename(filename)
    {
        module = dlopen(filename, RTLD_NOW);
        if(!module)
        {
            cerr << dlerror() << endl;
            throw runtime_error("Failed to load module with user-defined operation");
        }
    }
    
    ~TraceOperationFactoryUser(void)
    {
        dlclose(module);
    }

    TraceOperation* getOperation(int n, const map<string,string>& params)
    {
        TraceOperation* (*getOperationF)(int n, const map<string,string>& params) =
            (typeof(getOperationF))dlsym(module, "getOperation");
        if(!getOperationF)
        {
            const char* err = dlerror();
            if(err)
                cerr << err << endl;
            else
                cerr << "In module " << filename << "getOperation symbol is NULL" << endl;
            
            throw runtime_error("Failed to create use-defined operation");
        }
        
        return getOperationF(n, params);
    }
    
    void putOperation(TraceOperation* op)
    {
        void (*putOperationF)(TraceOperation*) =
            (typeof(putOperationF))dlsym(module, "putOperation");
        if(!putOperationF)
        {
            const char* err = dlerror();
            if(err)
                cerr << err << endl;
        }
        else
        {
            putOperationF(op);
        }
    }

private:
    void* module;
    /* For error-reporting only */
    const char* filename;
};


/************************* Trace operation selector *******************/
OperationProcessor::TraceOperationFactory*
    OperationProcessor::getOperationFactory(const char* opName)
{
    if(strchr(opName, '/'))
    {
        return new TraceOperationFactoryUser(opName);
    }

#define isOperation(op) (!strcmp(opName, op))
    if(isOperation("add"))
        return new TraceOperationFactoryAdd();
    else if(isOperation("diff"))
        return new TraceOperationFactoryDiff();
    else if(isOperation("new-coverage"))
        return new TraceOperationFactoryrNewCoverage();
    else
        throw logic_error(string("Unknown trace operation: ") + opName);
#undef isOperation
}

/***************************** Stat implementation ********************/
/* Params */
const char* StatProcessor::defaultFormat =
    "Lines: %pl%% (%l of %L)\n"
    "Functions: %pf%% (%f of %F)\n"
    "Branches: %pb%% (%b of %B)\n";

StatProcessor::StatProcessor():
    traceFile(NULL), format(defaultFormat), prefix(NULL)
{
}

int StatProcessor::parseParams(int argc, char** argv)
{
    static const char options[] = "+o:f:p:";
    
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
        case 'p':
            prefix = optarg;
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
        else cerr << "Exceeded command-line argument: " << argv_rest[1] << endl;
        return -1;
    }
    
    traceFile = argv_rest[0];
    
    return 0;
}

/* Output */

class StatPrinter
{
public:
    StatPrinter(TraceSimple& trace, ostream& os);
    
    void print(const char* format);
private:
    TraceSimple& trace;
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

StatPrinter::StatPrinter(TraceSimple& trace, ostream& os)
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
        return;
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
    TraceSimple trace;
    trace.read(traceFile);

    trace.groupFiles();
    
    if(prefix)
    {
        string prefixStr = prefix;
        if(*prefix != '/')
            prefixStr = trace.commonSourcePrefix() + prefix;
        trace.filterSources(prefixStr);
    }
    
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

DEFINE_FILE_PRINTER(usage_stat)

void StatProcessor::usage(void)
{
    print_usage_stat();
}

/****************** Optimize-tests implementation *********************/
/* Params */
OptimizeTestsProcessor::OptimizeTestsProcessor()
    : testsFile(NULL), verbose(false) {}

int OptimizeTestsProcessor::parseParams(int argc, char** argv)
{
    static const char options[] = "+o:v";
    
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
        case 'v':
            verbose = true;
            break;
        default:
            return -1;
        }
    }
    
    char** argv_rest = argv + optind;
    int argc_rest = argc - optind;
    
    if(argc_rest != 1)
    {
        if(argc_rest == 0) cerr << "Tests file is missed." << endl;
        else cerr << "Exceeded command-line argument: " << argv_rest[1] << endl;
        return -1;
    }
    
    testsFile = argv_rest[0];
    
    return 0;
}

int OptimizeTestsProcessor::exec()
{
    vector<TestCoverageDesc> tests;
    loadTests(tests);
    
    TestSetOptimizer optimizer(tests);
    const vector<TestCoverageDesc>& optTests = optimizer.optimize(verbose);

    ostream& os = getOutStream();
    
    for(int i = 0; i < (int)optTests.size(); i++)
    {
        os << optTests[i].traceFile << endl;
    }
    return 0;
}

DEFINE_FILE_PRINTER(usage_optimize_tests)
void OptimizeTestsProcessor::usage(void)
{
    print_usage_optimize_tests();
}


void OptimizeTestsProcessor::loadTests(vector<TestCoverageDesc>& tests)
{
    FILE* f = fopen(testsFile, "r");
    if(f == NULL)
    {
        cerr << "Failed to open file with tests." << endl;
        throw runtime_error("Failed to open file");
    }
    
    char* line = NULL;
    size_t buffer_size;
    ssize_t len;
    while((len = getline(&line, &buffer_size, f)) != -1)
    {
        /* Drop delimiter if it is. */
        if((len > 0) && (line[len - 1] == '\n')) line[len - 1] = '\0';
        /* Ignore empty lines and lines started with '#' */
        if((line[0] == '\0') || (line[0] == '#')) continue;
        
        char* filename_start;
        
        double weight = strtod(line, &filename_start);
        
        while(isspace(*filename_start)) ++filename_start;
        
        tests.push_back(TestCoverageDesc(filename_start, weight));
    }
    
    free(line);
}
