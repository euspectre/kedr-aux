#include <mist2/mist.hh>

#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cassert>

#include "mist_template_group.hh"
#include "mist_template.hh"
#include "mist_param_set_slice.hh"

using namespace std;
using namespace Mist;


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
    ParamSetSlice mainSlice(paramSet, impl->paramMask);
    if(!mainSlice.isSetLast())
    {
        cerr << "Main template is multivalued." << endl;
        throw std::logic_error("Main template is multivalued");
    }
    
    return impl->templateGroupBlockRef->evaluate(mainSlice, os);
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

/********************** Sequence **************************************/
void MistTemplateSequenceGroup::addTemplate(
    MistTemplateGroupBlock* subtemplate)
{
    subtemplates.push_back(subtemplate);
}


ostream& MistTemplateSequenceGroup::evaluate(
    const ParamSetSlice& slice, ostream& os) const
{
    for(int i = 0; i < (int)subtemplates.size(); i++)
        subtemplates[i]->evaluate(slice, os);
    return os;
}

bool MistTemplateSequenceGroup::isEmpty(const ParamSetSlice& slice) const
{
    for(int i = 0; i < (int)subtemplates.size(); i++)
    {
        if(!subtemplates[i]->isEmpty(slice)) return false;
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

MistParamMask MistTemplateSequenceGroup::getParamMaskAll() const
{
    MistParamMask mask;
    
    for(int i = 0; i < (int)subtemplates.size(); i++)
        mask.append(subtemplates[i]->getParamMaskAll());

    return mask;
}

MistTemplateSequence::~MistTemplateSequence()
{
    for(int i = 0; i < (int)subtemplates.size(); i++)
        delete subtemplates[i];
}

/******************** "If" sentence ***********************************/
ostream& MistIfGroup::evaluate(const ParamSetSlice& slice, ostream& os) const
{
    if(!conditionBlock->isEmpty(slice))
        return ifBlock->evaluate(slice, os);
    else
        return elseBlock->evaluate(slice, os);
}

bool MistIfGroup::isEmpty(const ParamSetSlice& slice) const
{
    if(!conditionBlock->isEmpty(slice))
        return ifBlock->isEmpty(slice);
    else
        return elseBlock->isEmpty(slice);
}

MistParamMask MistIfGroup::getParamMask() const
{
    MistParamMask mask(conditionBlock->getParamMask());
    mask.append(ifBlock->getParamMask());
    mask.append(elseBlock->getParamMask());
    
    return mask;
}

MistParamMask MistIfGroup::getParamMaskAll() const
{
    MistParamMask mask(conditionBlock->getParamMaskAll());
    mask.append(ifBlock->getParamMaskAll());
    mask.append(elseBlock->getParamMaskAll());
    
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


ostream& MistJoinGroup::evaluate(const ParamSetSlice& slice, ostream& os) const
{
    ParamSetSlice joinSlice(slice);
    
    ParamSetSlice& subslice = joinSlice.getSubslice(context);
    subslice.resetMask(submask);
    
    block->evaluate(joinSlice, os);
    
    while(!subslice.isSetLast())
    {
        subslice.nextSet();
        os << textBetween;
        block->evaluate(joinSlice, os);
    }
    
    return os;
}

bool MistJoinGroup::isEmpty(const ParamSetSlice& slice) const
{
    ParamSetSlice joinSlice(slice);
    
    ParamSetSlice& subslice = joinSlice.getSubslice(context);
    subslice.resetMask(submask);
    
    if(!block->isEmpty(joinSlice)) return false;
    if(subslice.isSetLast()) return true;
    if(!textBetween.empty()) return false;
    
    for(subslice.nextSet(); !subslice.isSetLast(); subslice.nextSet())
    {
        if(!block->isEmpty(joinSlice)) return false;
    }
    
    return true;
}

MistParamMask MistJoinGroup::getParamMask() const
{
    return mask;
}

MistParamMask MistJoinGroup::getParamMaskAll() const
{
    MistParamMask result(mask);
    result.append(block->getParamMaskAll());
    
    return result;
}

/*********************** "RJoin" sentence *****************************/
ostream& MistRJoinGroup::evaluate(const ParamSetSlice& slice, ostream& os) const
{
    ParamSetSlice joinSlice(slice);
    
    ParamSetSlice& subslice = joinSlice.getSubslice(context);
    subslice.resetMask(submask);
    
    /* Store evaluation results in string array. */
    vector<string> strings;
    
    ostringstream oss;
    
    block->evaluate(joinSlice, oss);
    strings.push_back(oss.str());
    
    while(!subslice.isSetLast())
    {
        /* Before writing next string, clear string in temporary stream */
        oss.str("");

        subslice.nextSet();
        /*os << textBetween;*/
        block->evaluate(joinSlice, oss);
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
    
    return os;
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
ostream& MistIndentGroup::evaluate(const ParamSetSlice& slice, ostream& os) const
{
    IndentedStream indentedStream(os, indent);
    block->evaluate(slice, indentedStream);
    
    return os;
}

bool MistIndentGroup::isEmpty(const ParamSetSlice& slice) const
{
    return !indent.empty() && block->isEmpty(slice);
}

MistParamMask MistIndentGroup::getParamMask() const
{
    return block->getParamMask();
}

MistParamMask MistIndentGroup::getParamMaskAll() const
{
    return block->getParamMaskAll();
}
