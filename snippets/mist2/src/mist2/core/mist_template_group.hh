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
#include <list>

#include "mist_param_set_slice.hh"


/* 'Block' for build template group */
class MistTemplateGroupBlock
{
public:
    MistTemplateGroupBlock(): refs(1) {}

    /* Refcount support */
    MistTemplateGroupBlock* ref() {++refs; return this;}
    void unref() {if(--refs == 0) delete this;}
    
    /* Context for template group evaluation. */
    class Context;
    
    /* 
     * Evaluate group according to slice of the parameters set.
     * 
     * Result is written into the stream.
     */
    virtual std::ostream& evaluate(Context& groupContext,
        std::ostream& os) const = 0;

    /* Mask of parameters which should be set for evaluation */
    virtual MistParamMask getParamMask() const = 0;

    /* 
     * Whether result of evaluation is empty.
     * 
     * Default implementation evaluate group into temporary string stream
     * and check emptiness of resulted string.
     */
    virtual bool isEmpty(Context& groupContext) const;
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


/* Context for instantiate template group into stream. */
class MistTemplateGroupBlock::Context
{
public:
    Context(const Mist::ParamSet& paramSet, const MistParamMask& maskAll);
    ~Context();

    const ParamSetSlice& currentSlice(void) const;
    
    void beginJoinScope(const MistParamNameAbs& context,
        const MistParamMask& submask);
    void endJoinScope(void);

    bool advanceScope(void);
    /* 
     * Return index of iteration.
     * depth give depth index of joinScope, that is 
     * 0 corresponds to current joinScope,
     * 1 corresponds to previous one and so on.
     */
    int iterationIndex(int depth) const;
private:
    struct JoinContext
    {
        //Full parameters slice
        ParamSetSlice slice;
        //Base for joining. Note: this member is needed only when copy/assign objects.
        MistParamNameAbs paramName;
        // Cached subslice for iterate
        ParamSetSlice* subslice_p;
        // Index of current iteration(from 0).
        int index;
        
        // Create first(main) context.
        JoinContext(const Mist::ParamSet& paramSet, const MistParamMask& maskAll);
        // Create new join context according to previous one
        JoinContext(const JoinContext& prev, const MistParamNameAbs& paramName,
            const MistParamMask& submask);
        // Copy constructor
        JoinContext(const JoinContext& context);
        // Assignment
        JoinContext& operator=(const JoinContext& context);
        
        bool advance(void);
    };
    
    /* 
     * NB: Do not use vector<> because in that case expression
     *      .push_back(.back(), ...) 
     * become incorrect when vector's elements are reallocated.
     */
    std::list<JoinContext> joinContextStack;
};

/* Implementation for the template group class. */
class Mist::TemplateGroup::Impl
{
public:
    Impl(MistTemplateGroupBlock* templateGroupBlock);
    
    MistTemplateGroupBlockRef templateGroupBlockRef;
    MistParamMask paramMask;
};

/**********************************************************************/
// Real template group blocks

/******************** Empty template  *********************************/
class MistEmptyGroup: public MistTemplateGroupBlock
{
    std::ostream& evaluate(Context&, std::ostream& os) const
    {return os;}

    bool isEmpty(Context&) const
    {return true;}

    MistParamMask getParamMask() const {return MistParamMask();}
};

/********************** Sequence **************************************/
class MistTemplateSequenceGroup: public MistTemplateGroupBlock
{
public:
    std::vector<MistTemplateGroupBlockRef> subtemplates;
    
    void addTemplate(MistTemplateGroupBlock* subtemplate);
    
    std::ostream& evaluate(Context& groupContext, std::ostream& os) const;

    bool isEmpty(Context& groupContext) const;

    MistParamMask getParamMask() const;
};

/******************* Text *********************************************/
class MistTextGroup: public MistTemplateGroupBlock
{
public:
    const std::string text;
    
    MistTextGroup(const std::string& text): text(text) {}
    
    std::ostream& evaluate(Context&, std::ostream& os) const
    { return os << text; }

    bool isEmpty(Context& groupContext) const
    {return text.empty();}

    MistParamMask getParamMask() const {return MistParamMask();}
};

/********************** Reference to parameter ************************/
class MistTemplateParamRefGroup: public MistTemplateGroupBlock
{
public:
    const MistParamNameAbs name;
                    
    MistTemplateParamRefGroup(const MistParamNameAbs& name): name(name) {}
    
    
    std::ostream& evaluate(Context& groupContext, std::ostream& os) const
    {
        const ParamSetSlice& paramSlice = groupContext.currentSlice().getSubslice(name);
        
        os << paramSlice.getValue();
        return os;
    }

    bool isEmpty(Context& groupContext) const
    {
        const ParamSetSlice& paramSlice = groupContext.currentSlice().getSubslice(name);
        
        return paramSlice.getValue().empty();
    }

    MistParamMask getParamMask() const {return MistParamMask(name);}
};

/******************** "If" sentence ***********************************/
class MistIfGroup: public MistTemplateGroupBlock
{
public:
    MistTemplateGroupBlockRef conditionBlock;
    MistTemplateGroupBlockRef ifBlock;
    MistTemplateGroupBlockRef elseBlock;

    
    MistIfGroup(MistTemplateGroupBlock* conditionBlock,
        MistTemplateGroupBlock* ifBlock,
        MistTemplateGroupBlock* elseBlock):
            conditionBlock(conditionBlock),
            ifBlock(ifBlock), elseBlock(elseBlock) {}
    
    std::ostream& evaluate(Context& groupContext, std::ostream& os) const;

    bool isEmpty(Context& groupContext) const;

    MistParamMask getParamMask() const;
};

/********************* "Join" sentence ********************************/
class MistJoinGroup: public MistTemplateGroupBlock
{
public:
    MistTemplateGroupBlockRef block;
    MistParamNameAbs context;
    std::string textBetween;
    MistParamMask mask;
    MistParamMask submask;
    
    MistJoinGroup(MistTemplateGroupBlock* block,
        const MistParamNameAbs& context,
        const std::string& textBetween);
    
    std::ostream& evaluate(Context& groupContext, std::ostream& os) const;

    bool isEmpty(Context& groupContext) const;

    MistParamMask getParamMask() const;
};

/* Reverse join is very similar to normal join. */
class MistRJoinGroup: public MistJoinGroup
{
public:
    MistRJoinGroup(MistTemplateGroupBlock* block,
        const MistParamNameAbs& context,
        const std::string& textBetween):
        MistJoinGroup(block, context, textBetween) {}
    /* The only method differs from one in join. */
    std::ostream& evaluate(Context& groupContext, std::ostream& os) const;
};

/************************ Index of join iteration *********************/
class MistIndexGroup: public MistTemplateGroupBlock
{
    int depth;
public:
    MistIndexGroup(int depth) : depth(depth) {}
    
    /* Way for interpret and print index. */
    virtual std::ostream& printFormatted(int index, std::ostream& os) const = 0;
    
    std::ostream& evaluate(Context& groupContext, std::ostream& os) const;
    
    bool isEmpty(Context& groupContext) const {return false;}
    
    MistParamMask getParamMask() const {return MistParamMask();}
};

/********************* Indent functionality ***************************/
class MistIndentGroup: public MistTemplateGroupBlock
{
public:
    MistTemplateGroupBlockRef block;
    std::string indent;
    
    MistIndentGroup(MistTemplateGroupBlock* block,
        const std::string& indent): block(block), indent(indent) {}
    
    std::ostream& evaluate(Context& groupContext, std::ostream& os) const;

    bool isEmpty(Context& groupContext) const;

    MistParamMask getParamMask() const;
};

#endif /* MIST_TEMPLATE_GROUP_HH */