#include "trace.hh"
#include "trace_parser.hh"

#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>
#include <cassert>

using namespace std;

/* 
 * Report about error in trace and throw exception.
 * 
 * 'msg' is used only for insert into stream.
 * (That is, it may contain '<<' operators).
 */
#define trace_error(msg) \
cerr << traceLine << ": " << msg << endl; \
throw runtime_error("Incorrect trace file");

/* Out file group identificator to the stream for error reporting. */
ostream& operator<<(ostream& os, const Trace::FileGroupID& groupId)
{
	os << "{";
	/*
	 * We doesn't want to print empty test name.
	 */
	if(!groupId.testName.empty())
		os << groupId.testName << ",";
	return os << groupId.filename << "}";
}

/* Whether filename correspond to source file(ends with ".c"). */
static bool isSource(const std::string& filename)
{
	size_t len = filename.length();
	return (len > 2) && (filename[len - 2] == '.') && (filename[len - 1] == 'c');
}

Trace::~Trace()
{
	map<FileGroupID, FileGroupInfo*>::iterator iter = fileGroups.begin(),
		iterEnd = fileGroups.end();
	for(;iter != iterEnd; ++iter)
	{
		delete iter->second;
	}
}

class Trace::TraceBuilder: private TraceEventProcessor
{
public:
	TraceBuilder(): currentGroupInfo(NULL) {}
	~TraceBuilder() {delete currentGroupInfo;}

	void build(Trace* trace, istream& is, TraceParser& parser,
		const char* filename)
	{
		/* Recover from possible previous errors */
		delete currentGroupInfo;
		currentGroupInfo = NULL;
		
		this->trace = trace;
		
		parser.parse(is, *this, filename);
	}


    /* TN: <testName> */
    void onTestStart(const std::string& testName, int traceLine)
    {
		currentGroupInfo = new FileGroupInfo();
		currentGroupID.testName = testName;
		currentGroupID.filename.clear();
    }
    void onTestEnd(int traceLine)
    {
		assert(currentGroupInfo);
		if(currentGroupID.filename.empty())
		{
			cerr << "WARNING: Ignore test '" << currentGroupID.testName
				<< "'without files." << endl;
			delete currentGroupInfo;
			currentGroupInfo = NULL;
			return;
		}
		
		pair<map<FileGroupID, FileGroupInfo*>::iterator, bool> iterNew =
			trace->fileGroups.insert(make_pair(currentGroupID, currentGroupInfo));
		if(!iterNew.second)
		{
			trace_error("Information for files group " << currentGroupID
				<< " is written twice.");
		}
		
		currentGroupInfo = NULL;
    }
    


    /* SF: <sourcePath> */
    void onSourceStart(const std::string& sourcePath, int traceLine)
    {
		pair<map<string, FileInfo>::iterator, bool> newIter = 
			currentGroupInfo->files.insert(make_pair(sourcePath, FileInfo()));
		if(!newIter.second)
		{
			trace_error("Attempt to add information about file '"
				<< sourcePath << "' to group"
				<< currentGroupID << " which already has it.");
		}
		
		currentFile = &newIter.first->second;
		
		if(currentGroupID.filename.empty())
		{
			/* First file in the group - its name may be name of group */
			currentGroupID.filename = sourcePath;
		}
		else if(isSource(sourcePath))
		{
			/* Non-first file in the group, but corresponds to source file. */
			if(isSource(currentGroupID.filename))
			{
				trace_error("Two source files in group " << currentGroupID
					<< ".");
			}
			currentGroupID.filename = sourcePath;
		}
    }
    
    /* FN: <funcLine>, <funcName> */
    void onFunction(const std::string& funcName, int funcLine, int traceLine)
    {
		pair<map<string, FuncInfo>::iterator, bool> newIter = 
			currentFile->functions.insert(make_pair(funcName, funcLine));
		if(!newIter.second)
		{
			trace_error("Attempt to add function '" + funcName
				+ "' for file which already has it.");
		}
    }
    
    /* FNDA: <counter>, <funcName> */
    void onFunctionCounter(const std::string& funcName,
        counter_t counter, int traceLine)
    {
		map<string, FuncInfo>::iterator iter = currentFile->functions.find(funcName);
		if(iter == currentFile->functions.end())
		{
			/* 
			 * Strange, but possible - see definition of 'lineStart'
			 * in 'FuncInfo' structure.
			 */
			
			pair<map<string, FuncInfo>::iterator, bool> newIter = 
				currentFile->functions.insert(make_pair(funcName, -1));
			iter = newIter.first;
		}
		
		iter->second.counter = counter;
    }
    
    /* 
     * BRDA: <branchLine>, <blockNumber>, <branchNumber>, <counter>
     */
    void onBranchCoverage(int branchLine,
        int blockNumber, int branchNumber, counter_t counter, int traceLine)
    {
		BranchID branchID(branchLine, blockNumber, branchNumber);
		
		pair<map<BranchID, counter_t>::iterator, bool> newIter = 
			currentFile->branches.insert(make_pair(branchID, counter));
		if(!newIter.second)
		{
			trace_error("Attempt to add counter for branch "
				"to file which already has it.");
		}
    }
    /*
     * BRDA: <branchLine>, <blockNumber>, <branchNumber>, -
     */
    void onBranchNotCovered(int branchLine,
        int blockNumber, int branchNumber, int traceLine)
    {
		BranchID branchID(branchLine, blockNumber, branchNumber);
		
		pair<map<BranchID, counter_t>::iterator, bool> newIter = 
			currentFile->branches.insert(make_pair(branchID, -1));
		if(!newIter.second)
		{
			trace_error("Attempt to add '-' taken for branch "
				"to file which already has it.");
		}
    }
    
    /* DA: <line>, <counter> */
    void onLineCounter(int line, counter_t counter, int traceLine)
    {
		pair<map<int, counter_t>::iterator, bool> newIter = 
			currentFile->lines.insert(make_pair(line, counter));
		if(!newIter.second)
		{
			trace_error("Attempt to add counter for line " << line <<
				" to file which already has it.");
		}
    }

private:
	/* Set when need to build trace. */
	Trace* trace;

	/* 
	 * Pointer to the allocated file group information.
	 * 
	 * When store this information to the trace pointer is nullified.
	 */
	FileGroupInfo* currentGroupInfo;
	/*
	 * Current information about group identificator.
	 */
	FileGroupID currentGroupID;
	
	/* Pointer to file under construction.
	 * 
	 * It points to the information which already added to the
	 * corresponded map.
	 */
	FileInfo* currentFile;
};

void Trace::read(std::istream& is, TraceParser& parser, const char* filename)
{
	TraceBuilder builder;
	
	builder.build(this, is, parser, filename);
}

void Trace::read(std::istream& is, const char* filename)
{
	TraceParser parser;
	read(is, parser, filename);
}


/* 
 * Write file information into stream.
 * 
 * Always start from current line.
 * Always append newline after last line.
 * Always non-empty.
 */
static void writeFileInfoToStream(const Trace::FileInfo& fileInfo, ostream& os)
{
	int functionsTotal = 0, functionsTotalHit = 0;

	map<string, Trace::FuncInfo>::const_iterator funcIter =
		fileInfo.functions.begin(), funcIterEnd = fileInfo.functions.end();
	
	/* Print functions lines */
	for(; funcIter != funcIterEnd; ++funcIter)
	{
		/* 
		 * See description of 'lineStart' field in 'FuncInfo' structure
		 * for special processing of its -1 value.
		 */
		if(funcIter->second.lineStart != -1)
		{
			os << "FN:" << funcIter->second.lineStart
				<< ',' << funcIter->first << endl;
		}
		++functionsTotal;
	}
	/* Print functions counters */
	funcIter = fileInfo.functions.begin();
	for(; funcIter != funcIterEnd; ++funcIter)
	{
		os << "FNDA:" << funcIter->second.counter << ',' << funcIter->first << endl;
		if(funcIter->second.counter > 0) ++functionsTotalHit;
	}
	/* Print functions statistic */
	os << "FNF:" << functionsTotal << endl;
	os << "FNH:" << functionsTotalHit << endl;
	
	/************/
	int branchesTotal = 0, branchesTotalHit = 0;
	
	map<Trace::BranchID, counter_t>::const_iterator branchIter =
		fileInfo.branches.begin(), branchIterEnd = fileInfo.branches.end();

	/* Print branches counters */

	for(; branchIter != branchIterEnd; ++branchIter)
	{
		os << "BRDA:"
			<< branchIter->first.line << ','
			<< branchIter->first.blockNumber << ','
			<< branchIter->first.branchNumber << ',';
		
		int counter = branchIter->second;

		if(counter != -1)
			os << counter;
		else
			os << '-';
		os << endl;

		++branchesTotal;
		// TODO: compare with 0 or with -1?
		if(counter > 0) ++branchesTotalHit;
	}
	/* Print branches statistic */
	os << "BRF:" << branchesTotal << endl;
	os << "BRH:" << branchesTotalHit << endl;

	/********************/
	int linesTotal = 0, linesTotalHit = 0;
	
	
	map<int, counter_t>::const_iterator lineIter = fileInfo.lines.begin(),
		lineIterEnd = fileInfo.lines.end();
	/* Print lines counters */
	for(; lineIter != lineIterEnd; ++lineIter)
	{
		os << "DA:" << lineIter->first << ',' << lineIter->second << endl;
		
		linesTotal++;
		if(lineIter->second > 0) ++linesTotalHit;
	}
	/* Print lines statistic */
	os << "LF:" << linesTotal << endl;
	os << "LH:" << linesTotalHit << endl;
}

/*
 * Write file group information into stream.
 * 
 * Always start from current line.
 * Never append newline at the end.
 * Only for group with files, in that case non-empty.
 */
static void writeFileGroupInfoToStream(
	const Trace::FileGroupInfo& groupInfo, ostream& os)
{
	map<string, Trace::FileInfo>::const_iterator
		iter = groupInfo.files.begin(),
		iterEnd = groupInfo.files.end();
	
	assert(iter != iterEnd);/* should be non-empty */
	
	os << "SF:" << iter->first << endl;
	writeFileInfoToStream(iter->second, os);
	os << "end_of_record";
	
	for(++iter; iter != iterEnd; ++iter)
	{
		os << endl; /* newline after previous record */

		os << "SF:" << iter->first << endl;
		writeFileInfoToStream(iter->second, os);
		os << "end_of_record";
	}
}

void Trace::write(std::ostream& os) const
{
	map<FileGroupID, FileGroupInfo*>::const_iterator iter
		= fileGroups.begin(), iterEnd = fileGroups.end();
	/* Found first non-empty group */
	for(; iter != iterEnd; iter++)
	{
		if(!iter->second->files.empty()) break;
	}
	
	if(iter == iterEnd)
	{
		/* Empty trace. TODO: is it possible? how to write it?*/
		os << "TN:";
		return;
	}
	/* Write first group. */
	os << "TN:" << iter->first.testName << endl;
	writeFileGroupInfoToStream(*iter->second, os);
	/* Write other non-empty groups. */
	for(++iter; iter != iterEnd; iter++)
	{
		if(iter->second->files.empty()) continue;
		
		os << endl;
		os << "TN:" << iter->first.testName << endl;
		writeFileGroupInfoToStream(*iter->second, os);
	}
}