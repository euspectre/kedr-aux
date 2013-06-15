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

#include "mist_param_set_slice.hh"

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
    virtual std::ostream& evaluate(const ParamSetSlice& slice,
        std::ostream& os) const = 0;

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
    ostream& evaluate(const ParamSetSlice&, ostream& os) const
    {return os;}

    bool isEmpty(const ParamSetSlice&) const
    {return true;}

    MistParamMask getParamMask() const {return MistParamMask();}
    MistParamMask getParamMaskAll() const {return MistParamMask();}
};

/********************** Sequence **************************************/
class MistTemplateSequenceGroup: public MistTemplateGroupBlock
{
public:
    vector<MistTemplateGroupBlockRef> subtemplates;
    
    void addTemplate(MistTemplateGroupBlock* subtemplate);
    
    ostream& evaluate(const ParamSetSlice& slice, ostream& os) const;

    bool isEmpty(const ParamSetSlice& slice) const;

    MistParamMask getParamMask() const;
    MistParamMask getParamMaskAll() const;
};

/******************* Text *********************************************/
class MistTextGroup: public MistTemplateGroupBlock
{
public:
    const string text;
    
    MistTextGroup(const string& text): text(text) {}
    
    ostream& evaluate(const ParamSetSlice&, ostream& os) const
    { return os << text; }

    bool isEmpty(const ParamSetSlice&) const
    {return text.empty();}

    MistParamMask getParamMask() const {return MistParamMask();}
    MistParamMask getParamMaskAll() const {return MistParamMask();}
};

/********************** Reference to parameter ************************/
class MistTemplateParamRefGroup: public MistTemplateGroupBlock
{
public:
    const MistParamNameAbs name;
                    
    MistTemplateParamRefGroup(const MistParamNameAbs& name): name(name) {}
    
    
    ostream& evaluate(const ParamSetSlice& slice, ostream& os) const
    {
        const ParamSetSlice& paramSlice = slice.getSubslice(name);
        
        os << paramSlice.getValue();
        return os;
    }

    bool isEmpty(const ParamSetSlice& slice) const
    {
        const ParamSetSlice& paramSlice = slice.getSubslice(name);
        
        return paramSlice.getValue().empty();
    }

    MistParamMask getParamMask() const {return MistParamMask(name);}
    MistParamMask getParamMaskAll() const {return MistParamMask(name);}
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
    
    ostream& evaluate(const ParamSetSlice& slice, ostream& os) const;

    bool isEmpty(const ParamSetSlice& slice) const;

    MistParamMask getParamMask() const;
    MistParamMask getParamMaskAll() const;
};

/********************* "Join" sentence ********************************/
class MistJoinGroup: public MistTemplateGroupBlock
{
public:
    MistTemplateGroupBlockRef block;
    MistParamNameAbs context;
    string textBetween;
    MistParamMask mask;
    MistParamMask submask;
    
    MistJoinGroup(MistTemplateGroupBlock* block,
        const MistParamNameAbs& context,
        const string& textBetween);
    
    ostream& evaluate(const ParamSetSlice& slice, ostream& os) const;

    bool isEmpty(const ParamSetSlice& slice) const;

    MistParamMask getParamMask() const;
    MistParamMask getParamMaskAll() const;
};

/* Reverse join is very similar to normal join. */
class MistRJoinGroup: public MistJoinGroup
{
public:
    MistRJoinGroup(MistTemplateGroupBlock* block,
        const MistParamNameAbs& context,
        const string& textBetween):
        MistJoinGroup(block, context, textBetween) {}
    /* The only method differs from one in join. */
    ostream& evaluate(const ParamSetSlice& slice, ostream& os) const;
};

/********************* Indent functionality ***************************/
class MistIndentGroup: public MistTemplateGroupBlock
{
public:
    MistTemplateGroupBlockRef block;
    string indent;
    
    MistIndentGroup(MistTemplateGroupBlock* block,
        const string& indent): block(block), indent(indent) {}
    
    ostream& evaluate(const ParamSetSlice& slice, ostream& os) const;

    bool isEmpty(const ParamSetSlice& slice) const;

    MistParamMask getParamMask() const;
    MistParamMask getParamMaskAll() const;
};

#endif /* MIST_TEMPLATE_GROUP_HH */