#include <mist2/mist.hh>

#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cassert>
#include <algorithm>

#include "mist_template_group.hh"
#include "mist_template.hh"
#include "mist_param_set_slice.hh"

using namespace std;
using namespace Mist;


bool MistTemplateGroupBlock::isEmpty(Context& groupContext) const
{
    stringstream ss;
    
    evaluate(groupContext, ss);
    
    return ss.str().empty();
}

/**********************************************************************/
MistTemplateGroupBlockRef::MistTemplateGroupBlockRef(
    MistTemplateGroupBlock* block) : block(block) {}
MistTemplateGroupBlockRef::MistTemplateGroupBlockRef(
    const MistTemplateGroupBlockRef& ref) : block(ref.block)
{
    if(block) block->ref();
}
MistTemplateGroupBlockRef::~MistTemplateGroupBlockRef()
{
    if(block) block->unref();
}
    
MistTemplateGroupBlockRef& MistTemplateGroupBlockRef::operator=(
    const MistTemplateGroupBlockRef& ref)
{
    if(block != ref.block)
    {
        if(block) block->unref();
        block = ref.block;
        if(block) block->ref();
    }
    return *this;
}

/* Template group constructor */
Mist::TemplateGroup::TemplateGroup(TemplateCollection& templateCollection,
    const std::string& mainTemplateName): impl(NULL)
{
    Template::Impl::Context templateContext(templateCollection);
    
    Template* mainTemplate = templateContext.findTemplate(mainTemplateName);
    if(mainTemplate == NULL)
    {
        cerr << "Template collection has no main template with name "
            << mainTemplateName << "." << endl;
        throw std::logic_error("Main template is absent");
    }
    
    MistTemplateGroupBlock* block = templateContext.build(mainTemplateName);
    
    impl = new Impl(block);
}

Mist::TemplateGroup::~TemplateGroup()
{
    delete impl;
}

std::ostream& Mist::TemplateGroup::instantiate(std::ostream& os, const ParamSet& paramSet)
{
    MistTemplateGroupBlock::Context groupContext(paramSet, impl->paramMask);
    
    return impl->templateGroupBlockRef->evaluate(groupContext, os);
}

std::string Mist::TemplateGroup::instantiate(const ParamSet& paramSet)
{
    ostringstream os;
    
    instantiate(os, paramSet);
    return os.str();
}

/**********************************************************************/
TemplateGroup::Impl::Impl(MistTemplateGroupBlock* templateGroupBlock)
    : templateGroupBlockRef(templateGroupBlock),
    paramMask(templateGroupBlock->getParamMask())
{
}


/******************** Context for joining *****************************/
MistTemplateGroupBlock::Context::JoinContext::JoinContext(
    const ParamSet& paramSet, const MistParamMask& maskAll):
        slice(paramSet, maskAll),
        paramName(),
        subslice_p(&slice),
        index(0)
{
}

MistTemplateGroupBlock::Context::JoinContext::JoinContext(
    const JoinContext& prev, const MistParamNameAbs& paramName,
    const MistParamMask& submask):
        slice(prev.slice),
        paramName(paramName),
        //Note: recalculate subslice_p because slice is deed copied.
        subslice_p(&slice.getSubslice(paramName)),
        index(0)
{
    subslice_p->resetMask(submask);
}

MistTemplateGroupBlock::Context::JoinContext::JoinContext(
    const JoinContext& context):
        slice(context.slice),
        paramName(context.paramName),
        //Note: recalculate subslice_p because slice is deed copied.
        subslice_p(&slice.getSubslice(paramName)),
        index(context.index)
{
    
}
    
MistTemplateGroupBlock::Context::JoinContext&
MistTemplateGroupBlock::Context::JoinContext::operator=(
    const JoinContext& context)
{
    slice = context.slice;
    paramName = context.paramName;
    subslice_p = &slice.getSubslice(paramName);
    index = context.index;
    
    return *this;
}

bool MistTemplateGroupBlock::Context::JoinContext::advance(void)
{
    if(subslice_p->isSetLast()) return false;
    
    subslice_p->nextSet();
    index++;
    return true;
}

/*********** Context for instantiate template group into stream. ******/
MistTemplateGroupBlock::Context::Context(const ParamSet& paramSet,
    const MistParamMask& maskAll)
{
    joinContextStack.push_back(JoinContext(paramSet, maskAll));
    
    if(!joinContextStack.back().subslice_p->isSetLast())
    {
        cerr << "Main template is multivalued." << endl;
        throw logic_error("Main template is multivalued");
    }
}

MistTemplateGroupBlock::Context::~Context()
{
    assert(joinContextStack.size() == 1);
}

const ParamSetSlice& MistTemplateGroupBlock::Context::currentSlice(void) const
{
    assert(!joinContextStack.empty());
    return joinContextStack.back().slice;
}

void MistTemplateGroupBlock::Context::beginJoinScope(
    const MistParamNameAbs& paramName, const MistParamMask& submask)
{
    assert(!joinContextStack.empty());
    joinContextStack.push_back(JoinContext(joinContextStack.back(),
        paramName, submask));
}

void MistTemplateGroupBlock::Context::endJoinScope(void)
{
    joinContextStack.pop_back();
    //First slice should never been popped.
    assert(!joinContextStack.empty());
}

bool MistTemplateGroupBlock::Context::advanceScope(void)
{
    assert(!joinContextStack.empty());
    return joinContextStack.back().advance();
}

int MistTemplateGroupBlock::Context::iterationIndex(int depth) const
{
    //Navigate to selected join context.
    list<JoinContext>::const_reverse_iterator iter = joinContextStack.rbegin();
    for(; depth != 0; depth--, iter++)
    {
        assert(iter != joinContextStack.rend());
    }
    
    assert(iter != joinContextStack.rend());
    
    return iter->index;
}
/********************** Sequence **************************************/
void MistTemplateSequenceGroup::addTemplate(
    MistTemplateGroupBlock* subtemplate)
{
    subtemplates.push_back(subtemplate);
}


ostream& MistTemplateSequenceGroup::evaluate(
    Context& groupContext, ostream& os) const
{
    for(int i = 0; i < (int)subtemplates.size(); i++)
        subtemplates[i]->evaluate(groupContext, os);
    return os;
}

bool MistTemplateSequenceGroup::isEmpty(Context& groupContext) const
{
    for(int i = 0; i < (int)subtemplates.size(); i++)
    {
        if(!subtemplates[i]->isEmpty(groupContext)) return false;
    }
    
    return true;
}

MistParamMask MistTemplateSequenceGroup::getParamMask() const
{
    MistParamMask mask;
    
    for(int i = 0; i < (int)subtemplates.size(); i++)
        mask.append(subtemplates[i]->getParamMask());

    return mask;
}

MistTemplateSequence::~MistTemplateSequence()
{
    for(int i = 0; i < (int)subtemplates.size(); i++)
        delete subtemplates[i];
}

/******************** "If" sentence ***********************************/
ostream& MistIfGroup::evaluate(Context& groupContext, ostream& os) const
{
    if(!conditionBlock->isEmpty(groupContext))
        return ifBlock->evaluate(groupContext, os);
    else
        return elseBlock->evaluate(groupContext, os);
}

bool MistIfGroup::isEmpty(Context& groupContext) const
{
    if(!conditionBlock->isEmpty(groupContext))
        return ifBlock->isEmpty(groupContext);
    else
        return elseBlock->isEmpty(groupContext);
}

MistParamMask MistIfGroup::getParamMask() const
{
    MistParamMask mask(conditionBlock->getParamMask());
    mask.append(ifBlock->getParamMask());
    mask.append(elseBlock->getParamMask());
    
    return mask;
}

/********************* "Join" sentence ********************************/
MistJoinGroup::MistJoinGroup(MistTemplateGroupBlock* block,
    const MistParamNameAbs& context,
    const string& textBetween): block(block), context(context),
    textBetween(textBetween), mask(context)
{
    mask.append(block->getParamMask());

    /* 
     * Now 'mask' equal to maskAll. Need to split it into real mask and
     * submask(at 'context').
     */
    submask = mask.getSubmask(context);
    mask.cut(context);
}


ostream& MistJoinGroup::evaluate(Context& groupContext, ostream& os) const
{
    groupContext.beginJoinScope(context, submask);
    
    block->evaluate(groupContext, os);
    
    while(groupContext.advanceScope())
    {
        os << textBetween;
        block->evaluate(groupContext, os);
    };
    
    groupContext.endJoinScope();
    
    return os;
}

bool MistJoinGroup::isEmpty(Context& groupContext) const
{
    bool result = true;
    groupContext.beginJoinScope(context, submask);
    
    if(!block->isEmpty(groupContext))
    {
        result = false;
    }
    else if(!groupContext.advanceScope())
    {
        result = true;
    }
    else if(!textBetween.empty())
    {
        result = false;
    }
    else do
    {
        if(!block->isEmpty(groupContext))
        {
            result = false;
            break;
        }
        
    }while(groupContext.advanceScope());
    
    groupContext.endJoinScope();
    
    return result;
}

MistParamMask MistJoinGroup::getParamMask() const
{
    return mask;
}

/*********************** "RJoin" sentence *****************************/
ostream& MistRJoinGroup::evaluate(Context& groupContext, ostream& os) const
{
    groupContext.beginJoinScope(context, submask);

    /* Store evaluation results in string array. */
    vector<string> strings;
    
    ostringstream oss;
    
    block->evaluate(groupContext, oss);
    strings.push_back(oss.str());
    
    while(groupContext.advanceScope())
    {
        /* Before writing next string, clear string in temporary stream */
        oss.str("");

        block->evaluate(groupContext, oss);
        strings.push_back(oss.str());
    }
    
    /* Output stored strings in reverse order. */
    vector<string>::reverse_iterator iter = strings.rbegin(),
        iterEnd = strings.rend();
    
    os << *iter;
    for(++iter; iter != iterEnd; ++iter)
    {
        os << textBetween;
        os << *iter;
    }
    
    groupContext.endJoinScope();
    
    return os;
}

/************************ Index of join iteration *********************/
ostream& MistIndexGroup::evaluate(Context& groupContext, ostream& os) const
{
    int index = groupContext.iterationIndex(depth);
    
    return printFormatted(index, os);
}
    
/*********** Stream wrapper used for 'indent' functionality*************/
/* 
 * Writes to the controlled stream some text each time when stream is
 * created or '\n' is written into it.
 */

class IndentedStream: public ostream
{
public:
    IndentedStream(ostream& os, const string& indent);
private:
    class IndentedStreamBuffer: public streambuf
    {
    public:
        typedef streambuf::int_type int_type;
        typedef streambuf::traits_type traits_type;
    
        IndentedStreamBuffer(ostream& os, const string& indent);

        ostream& os;
        string indent;
    protected:
        /* No buffering here. */
        int_type overflow(int_type c);

    };
    IndentedStreamBuffer buffer;
};

IndentedStream::IndentedStream(ostream& os, const string& indent):
    ostream(&buffer), buffer(os, indent) {}

IndentedStream::IndentedStreamBuffer::IndentedStreamBuffer
    (ostream& os, const string& indent): os(os), indent(indent)
{
    os.write(indent.data(), indent.size());
}

IndentedStream::IndentedStreamBuffer::int_type
IndentedStream::IndentedStreamBuffer::IndentedStreamBuffer::overflow(
    int_type c)
{
    os.put(c);
    if(c == '\n')
    {
        os.write(indent.data(), indent.size());
    }
    
    if(!os)
    {
        return traits_type::eof();
    }
    else if(traits_type::eq_int_type(c ,traits_type::eof()))
    {
        return traits_type::not_eof(c);
    }
    else
    {
        return c;
    }
}

/********************* Indent functionality ***************************/
ostream& MistIndentGroup::evaluate(Context& groupContext, ostream& os) const
{
    IndentedStream indentedStream(os, indent);
    block->evaluate(groupContext, indentedStream);
    
    return os;
}

bool MistIndentGroup::isEmpty(Context& groupContext) const
{
    return !indent.empty() && block->isEmpty(groupContext);
}

MistParamMask MistIndentGroup::getParamMask() const
{
    return block->getParamMask();
}
