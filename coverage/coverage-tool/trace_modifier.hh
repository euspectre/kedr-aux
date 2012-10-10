/*
 * Common algorithms for modify trace.
 */

#ifndef TRACE_MODIFIER_INCLUDED
#define TRACE_MODIFIER_INCLUDED

#include "trace.hh"

class TraceModifierEventNotifier;

/* 
 * Modification algorithm.
 * 
 * Modify trace 'trace' using 'traceAnother'
 * 
 * For every counter in 'traceAnother' call notifier callback with
 * this counter and corresponded counter in 'trace'.
 */
void modifyTrace(Trace& trace, const Trace& traceAnother,
    TraceModifierEventNotifier& notifier);


/* Implement concrete modification semantic */
class TraceModifierEventNotifier
{
public:
    /* 
     * Called when enter to file group in 'traceAnother'
     * for its processing.
     * 
     * By default, after that function corresponded group will be
     * searched in the 'trace'. If found, for every file in it will be
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
    /* Called if corresponded file group was not found in the 'trace'. */
    virtual void onFileGroupEndNew(void) {}
    
    /* 
     * Called when enter to source file in 'traceAnother'
     * for its processing.
     * 
     * By default, after that function corresponded file will be
     * searched in the 'trace'. If found, its functions, brahches and
     * lines will be processed(see below) and onSourceEnd() will be
     * called at the end of that processing. Otherwuse onSourceEndNew()
     * will be called.
     * 
     * If return true, processing of file will be canceled
     * (and onSourceEnd() will not be called!).
     */

    virtual bool onSourceStart(
        const std::map<std::string, Trace::FileInfo>::value_type& /*source*/)
        {return false;}
    /* Called at the end of source file processing. */
    virtual void onSourceEnd(void) {}
    /* Called if corresponded source was not found in the 'trace'. */
    virtual void onSourceEndNew(void) {}
    
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
    /* Modify counter in trace using counter in another trace. */
    virtual void modifyFuncCounter(counter_t& /*counter*/,
        counter_t /*counterAnother*/) {}
    /* 
     * Called when function in 'traceAnother' has no corresponded in
     * 'trace'.
     */
    virtual void newFunc() {}

    /* 
     * Called at the start of branch processing.
     * 
     * By default, after that function corresponded branch will be
     * searched in the 'trace'. If found, modifyBranchCounter() will be
     * called, otherwise newBranch() will be called.
     * 
     * If return true, processing of branch will be canceled.
     */
    virtual bool onBranchCounter(
        const std::map<Trace::BranchID, counter_t>::value_type& /*branch*/)
        {return false;}
    /* Modify counter in trace using counter in another trace. */
    virtual void modifyBranchCounter(counter_t& /*counter*/,
        counter_t /*counterAnother*/) {}
    /* 
     * Called when branch in 'traceAnother' has no corresponded in
     * 'trace'.
     */
    virtual void newBranch() {}

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
    /* Modify counter in trace using counter in another trace. */
    virtual void modifyLineCounter(counter_t& /*counter*/,
        counter_t /*counterAnother*/) {}

    /* 
     * Called when line in 'traceAnother' has no corresponded in
     * 'trace'.
     */
    virtual void newLine() {}
};


#endif /* TRACE_MODIFIER_INCLUDED */