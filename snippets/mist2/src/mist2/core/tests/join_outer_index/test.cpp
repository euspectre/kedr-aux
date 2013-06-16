/* Check that index corretly address outer join. */

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
            istringstream ss("<$param.param1$>(<$param: i0$>)");
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
    
    assert_instantiation(result, "value11(0)\nvalue21(1),value22(1)\nvalue31(2)");
    
    return 0;
}