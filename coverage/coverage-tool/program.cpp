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


#include "trace.hh"

#include "trace_modifier.hh"

#include "test_set_optimizer.hh"

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

/* Program execution for 'optimize-tests' */
struct OptimizeTestsProcessor: public CommandProcessor
{
    const char* testsFile;
    bool verbose;
    
    OptimizeTestsProcessor(void);

    int parseParams(int argc, char** argv);
    int exec();
private:
    /* Load tests from file. */
    void loadTests(vector<TestCoverageDesc>& tests);
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
    else if(isCommand("optimize-tests"))
    {
        commandProcessor.reset(new OptimizeTestsProcessor());
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
class TraceModifierDiff: public Trace::Modifier
{
public:
    Trace::FileInfo::Modifier* onSourceStart(
        const std::map<std::string, Trace::FileInfo>::value_type& source)
    {
        return &fileModifier;
    }

private:
    class FileModifier: public Trace::FileInfo::Modifier
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

    }fileModifier;
};

int DiffProcessor::exec()
{
    Trace trace;
    trace.read(traceFile);
    
    Trace traceSub;
    traceSub.read(subTraceFile);
    
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
class TraceModifierAdd: public Trace::Modifier
{
public:
    bool onFileGroupStart(
        const map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& group)
    {
        currentGroup = &group;
        return false;
    }
    void onFileGroupEndNew(Trace& traceModified)
    {
        pair<map<Trace::FileGroupID, Trace::FileGroupInfo*>::iterator, bool>
            iterNew = traceModified.fileGroups.insert(*currentGroup);
        iterNew.first->second =
            new Trace::FileGroupInfo(*iterNew.first->second);
    }

    Trace::FileInfo::Modifier* onSourceStart(
        const std::map<std::string, Trace::FileInfo>::value_type& source)
    {
        currentFile = &source;
        return &fileModifier;
    }
    
    void onSourceEndNew(Trace::FileGroupInfo& groupModified)
    {
        groupModified.files.insert(*currentFile);
    }
private:
    class FileModifier: public Trace::FileInfo::Modifier
    {
    public:
        bool onFunction(const map<string, Trace::FuncInfo>::value_type& func)
        {
            current.func = &func;
            return false;
        }
        void modifyFuncCounter(counter_t counter,
            counter_t& counterModified)
        {
            addCounter(counter, counterModified);
        }
        void newFunc(Trace::FileInfo& fileModified)
        {
            fileModified.functions.insert(*current.func);
        }
        
        bool onBranch(const map<Trace::BranchID, counter_t>::value_type& branch)
        {
            current.branch = &branch;
            return false;
        }
        void modifyBranchCounter(counter_t counter, counter_t& counterModified)
        {
            addCounter(counter, counterModified);
        }
        void newBranch(Trace::FileInfo& fileModified)
        {
            fileModified.branches.insert(*current.branch);
        }

        bool onLine(const map<int, counter_t>::value_type& line)
        {
            current.line = &line;
            return false;
        }
        void modifyLineCounter(counter_t counter, counter_t& counterModified)
        {
            addCounter(counter, counterModified);
        }
        void newLine(Trace::FileInfo& fileModified)
        {
            fileModified.lines.insert(*current.line);
        }
    private:
        void addCounter(const counter_t counter, counter_t& counterModified)
        {
            if(counter > 0)
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
        }
        
        union currentType
        {
            const map<string, Trace::FuncInfo>::value_type* func;
            const map<Trace::BranchID, counter_t>::value_type* branch;
            const map<int, counter_t>::value_type* line;
        }current;
    } fileModifier;
    
    const map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type*
        currentGroup;
    const map<std::string, Trace::FileInfo>::value_type* currentFile;
};

int AddProcessor::exec()
{
    Trace trace;
    trace.read(traceFiles[0]);

    TraceModifierAdd modifier;
    
    for(int i = 1; i < (int)traceFiles.size(); i++)
    {
        Trace traceAdded;
        traceAdded.read(traceFiles[i]);

        modifyTrace(traceAdded, modifier, trace);
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
class TraceModifierNewCoverage: public Trace::Modifier
{
public:
    Trace::FileInfo::Modifier* onSourceStart(
        const map<std::string, Trace::FileInfo>::value_type& source)
    {
        return &fileModifier;
    }
private:
    class FileModifier: public Trace::FileInfo::Modifier
    {
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
    }fileModifier;
};

int NewCoverageProcessor::exec()
{
    Trace trace;
    trace.read(traceFile);

    Trace tracePrev;
    tracePrev.read(prevTraceFile);

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
    Trace trace;
    trace.read(traceFile);

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
        else cerr << "Exceeded command-line argument: " << optarg << endl;
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
