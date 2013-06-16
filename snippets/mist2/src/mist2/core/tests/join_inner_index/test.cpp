/* Check that index corretly works for inner join. */

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
            istringstream ss("<$with param$><$subtemplate: join \",\"$><$endwith: join \"\\n\"$>");
            return new Mist::Template(ss, "");
        }
        if(name == "subtemplate")
        {
            istringstream ss("<$param.param1$>(<$param.param1: i$>)");
            return new Mist::Template(ss, "");
        }


        else return NULL;
    }
};

int main(void)
{
    Mist::ParamSet paramSet;
    
    Mist::ParamSet* subset = paramSet.addSubset("param");
    subset->addParameter("param1", "value11");
    
    subset = paramSet.addSubset("param");
    subset->addParameter("param1", "value21");
    subset->addParameter("param1", "value22");

    subset = paramSet.addSubset("param");
    subset->addParameter("param1", "value31");
    
    TestTemplateCollection templateCollection;
    
    Mist::TemplateGroup templateGroup(templateCollection, "main");
    
    string result = templateGroup.instantiate(paramSet);
    
    assert_instantiation(result, "value11(1)\nvalue21(1),value22(2)\nvalue31(1)");
    
    return 0;
}