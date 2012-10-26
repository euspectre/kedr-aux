/*
 * Common algorithms for modify trace.
 */

#ifndef TRACE_MODIFIER_INCLUDED
#define TRACE_MODIFIER_INCLUDED

#include "trace.hh"

/* 
 * Abstract algorithm, which use one FileInfo object for modify another
 * FileInfo object.
 */
class Trace::FileInfo::Modifier
{
public:
	virtual ~Modifier() {}
    /* 
     * Called at the start of processing function in the trace.
     * 
     * By default, after that function corresponded function will be
     * searched in the 'trace'. If found, modifyFuncCounter() will be
     * called, otherwise newFunc() will be called.
     * 
     * If return true, processing of function will be canceled.
     */
    virtual bool onFunction(
        const std::map<std::string, Trace::FuncInfo>::value_type& /*func*/)
        {return false;}
    /* Modify function counter. */
    virtual void modifyFuncCounter(counter_t /*counter*/,
        counter_t& /*counterModified*/) {}
    /* 
     * Called when 'func', passed to onFunction, has no corresponded in
     * modified file.
     */
    virtual void newFunc(Trace::FileInfo& /*fileModified*/) {}

    /* 
     * Called at the start of branch processing.
     * 
     * By default, after that function corresponded branch will be
     * searched in the 'trace'. If found, modifyBranchCounter() will be
     * called, otherwise newBranch() will be called.
     * 
     * If return true, processing of branch will be canceled.
     */
    virtual bool onBranch(
        const std::map<Trace::BranchID, counter_t>::value_type& /*branch*/)
        {return false;}
    /* Modify branch counter. */
    virtual void modifyBranchCounter(counter_t /*counter*/,
        counter_t& /*counterModified*/) {}
    /* 
     * Called when 'branch', passed to onBranch, has no corresponded in
     * modified file.
     */
    virtual void newBranch(Trace::FileInfo& /*fileModified*/) {}

    /* 
     * Called at the start of line processing.
     * 
     * By default, after that function corresponded line will be
     * searched in the second trace. If found modifyBranchCounter() will
     * be called, otherwise newLine() will be called.
     * 
     * If return true, processing of line will be canceled.
     */
    virtual bool onLine(
        const std::map<int, counter_t>::value_type& /*line*/)
        {return false;}
    /* Modify line counter. */
    virtual void modifyLineCounter(counter_t /*counter*/,
        counter_t& /*counterModified*/) {}

    /* 
     * Called when 'line', passed to onLine, has no corresponded in
     * modified file.
     */
    virtual void newLine(Trace::FileInfo& /*fileModified*/) {}
};

/* 
 * Generic function for trace file modification.
 * 
 * Use 'traceFile' and modification algorithm 'modifier' for modify 
 * 'traceFileModified'.
 */
void modifyTraceFile(const Trace::FileInfo& traceFile,
	Trace::FileInfo::Modifier& modifier,
	Trace::FileInfo& traceFileModified);




/* 
 * Abstract algorithm, which use one Trace object for modify another
 * Trace object.
 */
class Trace::Modifier
{
public:
	virtual ~Modifier() {}

    /* 
     * Called when enter to file group in 'traceAnother'
     * for its processing.
     * 
     * By default, after that function corresponded group will be
     * searched in the 'trace'. If found, every file in it will be
     * processed(see below) and onFileGroupEnd() will be called at the
     * end of that processing. Otherwise onFileGroupEndNew() will be
     * called.
     * 
     * If return true, processing of file group will be canceled
     * (and onFileGroupEnd() will not be called!).
     */
    virtual bool onFileGroupStart(
        const std::map<Trace::FileGroupID, Trace::FileGroupInfo*>::value_type& /*group*/)
        {return false;}
    /* Called at the end of file group processing. */
    virtual void onFileGroupEnd(void) {}
    /* 
     * Called if corresponded file group was not found in the modified
     * trace.
     */
    virtual void onFileGroupEndNew(Trace& /*traceModified*/) {}
    
    /* 
     * Called when enter to source file in 'traceAnother'
     * for its processing.
     * 
     * By default, after that function corresponded file will be
     * searched in the 'trace'. If found, its functions, brahches and
     * lines will be processed using modifier object, returned by that
     * function, and onSourceEnd() will be called at the end of that
     * processing. Otherwise onSourceEndNew() will be called.
     * 
     * If return NULL, processing of file will be canceled
     * (and onSourceEnd() will not be called!).
     */

    virtual Trace::FileInfo::Modifier* onSourceStart(
        const std::map<std::string, Trace::FileInfo>::value_type& source);
    /* 
     * Called at the end of source file processing.
     */
    virtual void onSourceEnd(void) {}

    /* 
     * Called if corresponded source was not found in the modified group.
     */
    virtual void onSourceEndNew(Trace::FileGroupInfo& groupModified) {}
};

/* 
 * Generic function for trace modification.
 * 
 * Use 'trace' and modification algorithm 'modifier' for modify 
 * 'traceModified'.
 */
void modifyTrace(const Trace& trace,
	Trace::Modifier& modifier,
	Trace& traceModified);


#endif /* TRACE_MODIFIER_INCLUDED */
