/* Check that 'indent' function works. */

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
            istringstream ss("<$param: indent \"  \"$>");
            return new Mist::Template(ss, "");
        }
        else return NULL;
    }
};

int main(void)
{
    Mist::ParamSet paramSet;
    OneTemplate templateCollection;
    Mist::TemplateGroup templateGroup(templateCollection, "main");
    
    /* Empty value */
    string result = templateGroup.instantiate(paramSet);
    assert_instantiation(result, "  ");
    
    /* One-line value */
    paramSet.addParameter("param", "value");
    result = templateGroup.instantiate(paramSet);
    assert_instantiation(result, "  value");
    
    /* Multy-line value */
    Mist::ParamSet paramSetAnother;
    paramSetAnother.addParameter("param", "line1\nline2\nline3");
    result = templateGroup.instantiate(paramSetAnother);
    assert_instantiation(result, "  line1\n  line2\n  line3");
    
    return 0;
}