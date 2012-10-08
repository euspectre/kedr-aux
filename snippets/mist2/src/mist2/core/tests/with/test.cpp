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
            return new Mist::Template(ss, "");
        }
        if(name == "subtemplate")
        {
            istringstream ss("<$with param$><$param.param1: join \",\"$><$endwith$>");
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
    
    assert_instantiation(result, "value11\nvalue21,value22\nvalue31");
    
    return 0;
}