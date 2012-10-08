#include <mist2/mist.hh>

#include "mist_test_common.hh"

#include <iostream>
#include <sstream>
#include <cassert>
using namespace std;

class OneTemplate: public Mist::TemplateCollection
{
public:
    Mist::Template* findTemplate(const string& name)
    {
        if(name == "main")
        {
            istringstream ss("<$param: join \" \"$>");
            return new Mist::Template(ss, "");
        }
        else return NULL;
    }
};

int main(void)
{
    Mist::ParamSet paramSet;
    paramSet.addParameter("param", "value");
    paramSet.addParameter("param", "value1");
    paramSet.addParameter("param", "value2");
    
    OneTemplate templateCollection;
    
    Mist::TemplateGroup templateGroup(templateCollection, "main");
    
    string result = templateGroup.instantiate(paramSet);
    
    assert_instantiation(result, "value value1 value2");
    
    return 0;
}