#include <mist2/mist.hh>

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <fstream>

using namespace Mist;
using namespace std;
using namespace YAML;

/* Add content of the document into parameters set. */
static void addDocument(ParamSet* paramSet, const Node& doc);

/* Collection of templates in given directory */
class DirectoryTemplateCollection: public TemplateCollection
{
public:
    /* 
     * Create collection of templates which corresponds to files
     * 
     * 'dirname' + <template-name> + 'ext'
     * 
     * Because template name cannot contain '/', such collection means:
     * "Templates for all files in given directory with given extension".
     */
    DirectoryTemplateCollection(const string& dirname,
        const string& ext);
    ~DirectoryTemplateCollection();
    
    Template* findTemplate(const std::string& name);
private:
    string dirname;
    string ext;
};

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        cerr << "Usage: " << argv[0] << " <templates-dir> <data-file>" << endl;
        return 1;
    }
    
    const char* templatesDir = argv[1];
    const char* filename = argv[2];
    
    vector<Node> documents = LoadAllFromFile(filename);

    ParamSet paramSet;

    for(vector<Node>::const_iterator iter = documents.begin();
        iter != documents.end();
        ++iter)
    {
        addDocument(&paramSet, *iter);
    }
    
    DirectoryTemplateCollection templateCollection(templatesDir, ".tpl");
    
    TemplateGroup templateGroup(templateCollection, "document");
    
    templateGroup.instantiate(cout, paramSet);
    
    return 0;
}

DirectoryTemplateCollection::DirectoryTemplateCollection(
    const string& dirname, const string& ext): dirname(dirname), ext(ext)
{
    if(dirname[dirname.size() - 1] != '/') this->dirname += '/';
}
DirectoryTemplateCollection::~DirectoryTemplateCollection() {}

Template* DirectoryTemplateCollection::findTemplate(const string& name)
{
    string filename = dirname + name + ext;
    ifstream ifs(filename.c_str());
    if(ifs)
    {
        return new Template(ifs, filename);
    }
    else
    {
        return NULL;
    }
}

/*
 * Add node as value for subset with given name. */
static void addMapValue(ParamSet* paramSet, const Node& value,
    const string& name);

/* 
 * Add sequence of values to given parameter set, using given subset
 * name.
 */
static void addSequence(ParamSet* paramSet, const Node& sequence,
    const string& name);

/*
 * Add map to the given parameter set.
 */
static void addMap(ParamSet* paramSet, const Node& map);

/* Error about converting YAML representation into parameters set. */
#define convert_error(what) \
cerr << what << endl; \
throw runtime_error("Incorrect YAML data")

void addDocument(ParamSet* paramSet, const Node& doc)
{
    assert(doc.IsDefined());
    
    if(doc.IsNull())
    {
        convert_error("Null at the top of the document is forbidden.");
    }
    else if(doc.IsScalar())
    {
        convert_error("Scalar at the top of the document is forbidden.");
    }
    else if(doc.IsSequence())
    {
        convert_error("Sequence at the top of the document is forbidden.");
    }
    else if(doc.IsMap())
    {
        addMap(paramSet, doc);
    }
}

void addMap(ParamSet* paramSet, const Node& map)
{
    assert(map.IsMap());
    
    YAML::const_iterator iter = map.begin(), iterEnd = map.end();
    for(; iter != iterEnd; ++iter)
    {
        const Node& key = iter->first;
        if(key.IsSequence() || key.IsMap())
        {
            convert_error("Complex keys are forbidden.");
        }
        else if(key.IsNull())
        {
            convert_error("Null key is forbidden.");
        }
        /* key is scalar */
        addMapValue(paramSet, iter->second, key.Scalar());
    }
}

void addMapValue(ParamSet* paramSet, const Node& value,
    const string& name)
{
    assert(value.IsDefined());

    if(value.IsSequence())
    {
        addSequence(paramSet, value, name);
    }
    else if(value.IsMap())
    {
        addMap(paramSet->addSubset(name), value);
    }
    else if(value.IsScalar())
    {
        paramSet->addParameter(name, value.Scalar());
    }
    else if(value.IsNull())
    {
        paramSet->addParameter(name);
    }
}

void addSequence(ParamSet* paramSet, const Node& sequence,
    const string& name)
{
    assert(sequence.IsSequence());
    
    YAML::const_iterator iter = sequence.begin(), iterEnd = sequence.end();
    for(; iter != iterEnd; ++iter)
    {
        addMapValue(paramSet, *iter, name);
    }
}