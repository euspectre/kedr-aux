/* Check that index for text(without params) always return 0 for 'i0'. */

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
            istringstream ss("<$subtemplate: join \",\"$>");
            return new Mist::Template(ss, "");
        }
        if(name == "subtemplate")
        {
            istringstream ss("<$param$>(<$text: i0$>)");
            return new Mist::Template(ss, "");
        }
        if(name == "text")
        {
            istringstream ss("t");
            return new Mist::Template(ss, "");
        }

        else return NULL;
    }
};

int main(void)
{
    Mist::ParamSet paramSet;
    
    paramSet.addParameter("param", "value1");
    paramSet.addParameter("param", "value2");
    paramSet.addParameter("param", "value3");
    
    TestTemplateCollection templateCollection;
    
    Mist::TemplateGroup templateGroup(templateCollection, "main");
    
    string result = templateGroup.instantiate(paramSet);
    
    assert_instantiation(result, "value1(0),value2(0),value3(0)");
    
    return 0;
}