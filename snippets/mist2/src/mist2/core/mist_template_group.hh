/*
 * Internals of template group object.
 * 
 * Used in template implementation.
 */

#ifndef MIST_TEMPLATE_GROUP_HH
#define MIST_TEMPLATE_GROUP_HH

#include <mist2/mist.hh>

#include <iostream>
#include <stdexcept>

#include <vector>

class MistTemplateGroupBlock;
#include "mist_template.hh"

#include "mist_param_set_slice.hh"

using namespace std;
using namespace Mist;



/* 'Block' for build template group */
class MistTemplateGroupBlock
{
public:
    MistTemplateGroupBlock(): refs(1) {}

    /* Refcount support */
    MistTemplateGroupBlock* ref() {++refs; return this;}
    void unref() {if(--refs == 0) delete this;}
    
    /* 
     * Evaluate group according to slice of the parameters set.
     * 
     * Result is written into the stream.
     */
    virtual ostream& evaluate(const ParamSetSlice& slice, ostream& os) const = 0;

    /* Whether result of evaluation is empty. */
    virtual bool isEmpty(const ParamSetSlice& slice) const = 0;
    /* Mask of parameters which should be set for evaluation */
    virtual MistParamMask getParamMask() const = 0;
    /* 
     * Mask of parameters which are used in the evaluation.
     * 
     * Normally, this mask is same as one returned by getParamMask().
     * Exception is for join() and concat() constructions.
     * 
     * May be used in the future optimizations.
     */
    virtual MistParamMask getParamMaskAll() const = 0;
protected:
    /* Destructor shouldn't be used directly, use unref() instead */
    virtual ~MistTemplateGroupBlock() {}
private:
    int refs;
};

/* 
 * Reference to the block. Similar to auto_ptr<>,
 * useful for delete object when it is not needed.
 */
class MistTemplateGroupBlockRef
{
public:
    MistTemplateGroupBlockRef(MistTemplateGroupBlock* block = NULL);
    MistTemplateGroupBlockRef(const MistTemplateGroupBlockRef& ref);
    ~MistTemplateGroupBlockRef();
    
    MistTemplateGroupBlockRef& operator=(
        const MistTemplateGroupBlockRef& ref);
    
    MistTemplateGroupBlock& operator*() const {return *block;}
    MistTemplateGroupBlock* operator->() const {return block;}
    
    MistTemplateGroupBlock* get(void) const {return block;}
    
    operator bool() {return block != NULL;}
private:
    MistTemplateGroupBlock* block;
};

/* Implementation for the template group class. */
class TemplateGroup::Impl
{
public:
    Impl(MistTemplateGroupBlock* templateGroupBlock);
    
    MistTemplateGroupBlockRef templateGroupBlockRef;
    MistParamMask paramMask;
};

/* 
 * Builder of template group from templates.
 */
class Mist::TemplateGroup::Builder
{
public:
    Builder(TemplateCollection& templateCollection);
    ~Builder();
    
    MistTemplateGroupBlock* build(const string& mainTemplate);

    /* Find template by name. Return NULL if not found. */
    Template* findTemplate(const string& name);
    
    /*
     * Build template group block for template or parameter with given
     * name.
     * 
     * Detect circular dependences between templates,
     * and throw exception in case of them.
     * 
     * Also cache blocks, created for templates and parameters.
     */
    MistTemplateGroupBlock* buildTemplateOrParamRef(const string& name);
    
    /* 
     * Build template group block for parameter reference. 
     * 
     * Name of the parameter may be relative or absolute.
     */
    MistTemplateGroupBlock* buildParameterRef(const MistTemplateName& name);

    /* 
     * Return parameter name which should be used in join and concat
     * statements.
     * 
     * NOTE: This method should be used only from template's createGroup()
     * method and only when it is needed.
     * 
     * For template which doesn't use this method its group instantiation
     * will used with any groupParam, otherwise for every groupParam
     * different instantiation will be created.
     */
    const MistParamNameAbs& getGroupParamName(void);
    /* 
     * Set parameter name which should be used in join and concat
     * statements.
     */
    void pushGroupParamName(const MistTemplateName& paramName);
    /* 
     * Restore parameter name which should be used in join and concat
     * statements.
     */
    void popGroupParamName(void);
private:
    TemplateCollection& templateCollection;

    /* Cache for parameter references */
    map<MistParamNameAbs, MistTemplateGroupBlockRef> paramCache;
    
    /* Cache for name, corresponded to template or parameter. */
    struct NamedBlockCached
    {
        /* 
         * Context-independed block if exist. Name which correspond to
         * parameter always has such block.
         * 
         * Template has such block only if it doesn't use context
         * in which it processed.
         */
        MistTemplateGroupBlockRef refGlobal;
        /* Block for every 'with' context */
        map<MistParamNameAbs, MistTemplateGroupBlockRef> contexts;
        /* 
         * If given name corresponds to template, this is reference to it.
         * Otherwise NULL.
         */
        Template* t;
        /* 
         * True if template is currently being instantiated.
         * 
         * Attempt to instantiate new instance while this flag is true
         * means cyclic dependencies between templates.
         */
        bool inProgress;
        
        NamedBlockCached(void): t(NULL), inProgress(false) {}
    };

    /* Cached named blocks */
    map<string, NamedBlockCached> namedCache;
    
    /* Stack of the 'with' contexts */
    vector <MistParamNameAbs> contextStack;

    /* Information about template which is currently instantiated */
    struct TemplateContextInfo
    {
        /* 
         * Whether currently instantiated template use outer context for
         * join/concat statements.
         * 
         * If at the end of instantiating template this parameter is true,
         * then resulted block will be stored in per-context map.
         * Otherwise it will be stored globally.
         */
        bool useContext;
        /* 
         * Index of context (in stack) with which template begins
         * instantiate.
         * 
         * The thing is that, template may itself setup context and use
         * it.
         * 
         * Such usage doesn't affect on global status of block created.
         */
        size_t contextIndex;

        TemplateContextInfo(size_t contextIndex): useContext(false),
            contextIndex(contextIndex) {}
    };
    
    /* Stack of the instantiated templates */
    vector<TemplateContextInfo> templateStack;
    /*
     * Return named cache for given name. Cache will be created if needed.
     */
    NamedBlockCached& getNamedCache(const string& name);
    
    MistTemplateGroupBlock* buildParameterRef(const MistParamNameAbs& name);
};


#endif /* MIST_TEMPLATE_GROUP_HH */