#include <mist2/mist.hh>

#include "mist_test_common.hh"

#include <iostream>
#include <sstream>
#include <cassert>
using namespace std;

class TestTemplateCollection: public Mist::TemplateCollection
{
public:
    Mist::Template* findTemplate(const string& name)
    {
        if(name == "main")
        {
            istringstream ss("<$subtemplate: join \"\\n\"$>");
            return new Mist::Template(ss, "main");
        }
        if(name == "subtemplate")
        {
            istringstream ss("<$with param$><$with .subparam$><$.value : join \",\"$><$endwith$><$endwith$>");
            return new Mist::Template(ss, "subtemplate");
        }

        else return NULL;
    }
};

int main(void)
{
    Mist::ParamSet paramSet;
    
    Mist::ParamSet* subset = paramSet.addSubset("param")->addSubset("subparam");
    subset->addParameter("value", "value11");
    
    subset = paramSet.addSubset("param")->addSubset("subparam");
    subset->addParameter("value", "value21");
    subset->addParameter("value", "value22");

    subset = paramSet.addSubset("param")->addSubset("subparam");
    subset->addParameter("value", "value31");
    
    TestTemplateCollection templateCollection;
    
    Mist::TemplateGroup templateGroup(templateCollection, "main");
    
    string result = templateGroup.instantiate(paramSet);
    
    assert_instantiation(result, "value11\nvalue21,value22\nvalue31");
    
    return 0;
}