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
            istringstream ss("<$param_def: join \"\\n\"$>");
            return new Mist::Template(ss, "");
        }
        if(name == "param_def")
        {
            istringstream ss("<$with param.subparam$><$subparam_def: join \", \"$><$endwith$>");
            return new Mist::Template(ss, "");
        }
        if(name == "subparam_def")
        {
            istringstream ss("<$param.name$>_<$param.subparam.name$> = <$param.subparam.value$>");
            return new Mist::Template(ss, "");
        }



        else return NULL;
    }
};

int main(void)
{
    Mist::ParamSet paramSet;
    Mist::ParamSet* param;
    Mist::ParamSet* subparam;
    
    param = paramSet.addSubset("param");
    param->addParameter("name", "aaa");
    subparam = param->addSubset("subparam");
    
    subparam->addParameter("name", "a1");
    subparam->addParameter("value", "value1");
    
    subparam->addParameter("name", "a2");
    subparam->addParameter("value", "value2");

    param = paramSet.addSubset("param");
    param->addParameter("name", "bbb");
    subparam = param->addSubset("subparam");
    
    subparam->addParameter("name", "b1");
    subparam->addParameter("value", "value1");

    TestTemplateCollection templateCollection;
    
    Mist::TemplateGroup templateGroup(templateCollection, "main");
    
    string result = templateGroup.instantiate(paramSet);
    
    assert_instantiation(result, "aaa_a1 = value1, aaa_a2 = value2\n"
                                 "bbb_b1 = value1");
    
    return 0;
}