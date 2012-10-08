/* Build template from AST. */

#include <mist2/mist.hh>
#include <iostream>
#include <stdexcept>

#include "mist_ast.hh"
#include "mist_parser.hh"

#include "mist_template.hh"

using namespace Mist;
using namespace std;

class Mist::Template::Builder
{
public:
    Template::Impl* build(const MistAST& ast);
private:
    Template::Impl* buildSequence(const MistASTTemplateSequence& astSequence);
    Template::Impl* buildTemplate(const MistASTTemplate& astTemplate);

    class InternalTemplateBuilder: public MistASTTemplate::Visitor
    {
    public:
        InternalTemplateBuilder(Builder& builder): builder(builder) {}
        
        Template::Impl* build(const MistASTTemplate& astTemplate);
    
        void visitRef(const MistASTTemplateRef& astRef);
        void visitText(const MistASTText& astText);
        void visitIf(const MistASTIf& astIf);
        void visitJoin(const MistASTJoin& astJoin);
        void visitWith(const MistASTWith& astWith);
        void visitSequence(const MistASTTemplateSequence& astSequence);
    private:
        Builder& builder;
        Template::Impl* result;
    };
};

Template::Impl* Mist::Template::Builder::build(const MistAST& ast)
{
    return buildSequence(*ast.astSequence);
}

Template::Impl* Mist::Template::Builder::buildSequence(
    const MistASTTemplateSequence& astSequence)
{
    MistTemplateSequence* templateSequence = new MistTemplateSequence();
    
    MistASTTemplateSequence::iterator iter = astSequence.begin(),
        iter_end = astSequence.end();
    for(;iter != iter_end; ++iter)
    {
        templateSequence->addTemplate(buildTemplate(**iter));
    }
    
    return templateSequence;
}

Template::Impl* Mist::Template::Builder::buildTemplate(
    const MistASTTemplate& astTemplate)
{
    InternalTemplateBuilder templateBuilder(*this);
    
    return templateBuilder.build(astTemplate);
}

Template::Impl* Mist::Template::Builder::InternalTemplateBuilder::build(
    const MistASTTemplate& astTemplate)
{
    result = NULL;
    visit(astTemplate);
    return result;
}

void Mist::Template::Builder::InternalTemplateBuilder::visitIf(
    const MistASTIf& astIf)
{
    const MistASTTemplate& condition = *astIf.condition;
    Template::Impl* conditionTemplate = builder.buildTemplate(condition);
    
    Template::Impl* ifTemplate;
    Template::Impl* elseTemplate;
    
    ifTemplate = builder.buildSequence(*astIf.positivePart);

    if(astIf.negativePart.get())
        elseTemplate = builder.buildSequence(*astIf.negativePart);
    else
        elseTemplate = new MistTemplateEmpty();

    result = new MistTemplateIf(conditionTemplate,
        ifTemplate, elseTemplate);
}


void Mist::Template::Builder::InternalTemplateBuilder::visitRef(
    const MistASTTemplateRef& astRef)
{
    result = new MistTemplateRef(MistTemplateName(
        astRef.name->components, astRef.name->isRelative));
}

void Mist::Template::Builder::InternalTemplateBuilder::visitText(
    const MistASTText& astText)
{
    result = new MistTemplateText(*astText.text);
}

void Mist::Template::Builder::InternalTemplateBuilder::visitJoin(
    const MistASTJoin& astJoin)
{
    string textBetween;
    if(astJoin.textBetween.get())
        textBetween = *astJoin.textBetween;

    result = new MistTemplateJoin(
        builder.buildTemplate(*astJoin.joinedPart), textBetween);
}

void Mist::Template::Builder::InternalTemplateBuilder::visitWith(
    const MistASTWith& astWith)
{
    result = new MistTemplateWith(
        builder.buildSequence(*astWith.templateInternal),
        MistTemplateName(astWith.paramName->components,
            astWith.paramName->isRelative));
}

Template::Template(istream& s, const string& filename)
{
    MistAST ast;
    
    MistParser parser;
    
    parser.parse(s, ast, filename);
    
    Builder builder;
    
    impl = builder.build(ast);
}

Template::~Template()
{
    delete impl;
}