/* Convertors from template elements into template group elements */

#include <mist2/mist.hh>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "mist_template_group.hh"

using namespace Mist;
using namespace std;

bool MistParamNameAbs::operator<(const MistParamNameAbs& name) const
{
    int size = components.size();
    int name_size = name.components.size();
    
    int size_min = min(size, name_size);
    
    for(int i = 0; i < size_min; i++)
    {
        int comp = components[i].compare(name.components[i]);
        if(comp < 0) return true;
        else if(comp > 0) return false;
    }
    
    return size < name_size;
}

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

MistTemplateGroupBlock* MistTemplateEmpty::createGroup(
    Mist::TemplateGroup::Builder&) const
{
    return new MistEmptyGroup();
}

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

MistTemplateGroupBlock* MistTemplateSequence::createGroup(
    Mist::TemplateGroup::Builder& groupBuilder) const
{
    int size = subtemplates.size();
    /* Try to minimize count of elements. */
    if(size == 0)
        return new MistEmptyGroup();
    else if(size == 1)
        return subtemplates[0]->createGroup(groupBuilder);
    
    /* Two or more subtemplates */
    MistTemplateSequenceGroup* templateSequenceGroup =
        new MistTemplateSequenceGroup();
    
    for(int i = 0; i < size; i++)
        templateSequenceGroup->addTemplate(
            subtemplates[i]->createGroup(groupBuilder));

    return templateSequenceGroup;
}


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

MistTemplateGroupBlock* MistTemplateText::createGroup(
    Mist::TemplateGroup::Builder&) const
{
    return new MistTextGroup(text);
}


/****************** Reference to template or parameter*****************/
MistTemplateGroupBlock* MistTemplateRef::createGroup(
    Mist::TemplateGroup::Builder& groupBuilder) const
{
    if(!name.isRelative && (name.components.size() == 1))
        return groupBuilder.buildTemplateOrParamRef(name.components[0]);
    else
        return groupBuilder.buildParameterRef(name);
}

/**********************************************************************/
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


MistTemplateGroupBlock* MistTemplateIf::createGroup(
    Mist::TemplateGroup::Builder& groupBuilder) const
{
    return new MistIfGroup(conditionTemplate->createGroup(groupBuilder),
        ifTemplate->createGroup(groupBuilder),
        elseTemplate->createGroup(groupBuilder));
}

/**********************************************************************/
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
    
    while(!subslice.isSetLast())
    {
        if(!textBetween.empty()) return false;
        subslice.nextSet();
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

MistTemplateGroupBlock* MistTemplateJoin::createGroup(
    Mist::TemplateGroup::Builder& groupBuilder) const
{
    MistTemplateGroupBlock* blockInternal =
        templateInternal->createGroup(groupBuilder);
    return new MistJoinGroup(blockInternal,
        groupBuilder.getGroupParamName(),
        textBetween);
}

/**********************************************************************/
MistTemplateGroupBlock* MistTemplateWith::createGroup(
    Mist::TemplateGroup::Builder& groupBuilder) const
{
    groupBuilder.pushGroupParamName(context);
    MistTemplateGroupBlock* block = templateInternal->createGroup(groupBuilder);
    groupBuilder.popGroupParamName();
    
    return block;
}