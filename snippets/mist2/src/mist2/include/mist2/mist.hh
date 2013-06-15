#ifndef MIST_H_INCLUDED
#define MIST_H_INCLUDED

#include <string>
#include <vector>
#include <map>

namespace Mist
{
    /* 
     * Parameters set.
     * 
     * Logically, each parameters set has a string value and for any name
     * has a non-empty sequence of subsets with given name(recursively).
     * 
     * That is, for any parameters set one can request value at, e.g.
     * 'name1.name2.name3', and get at least one instance of such value.
     * 
     * By default, value of parameter is "" (empty string), and its any
     * sequence of subsets consist only from one set (again, recursively).
     * 
     * 
     * When parameter set is being constructed, it is possible to add
     * set to any its subset's sequence. Subset which is firstly added
     * replace default one in the sequence, futher subsets are added to
     * the end.
     * 
     * Without explicitly added subsets, parameters set may be treated
     * as simple parameter, which only has a value.
     *
     * 
     * When parameter set is being read, for any name vector of subsets
     * is returned.
     */
    class ParamSet
    {
    public:
        /* 
         * Create default parameter set.
         */
        ParamSet();
        
        ~ParamSet();
        /* Set value for parameter set */
        void setValue(const std::string& value);

        /* 
         * Add default subset of parameters with given name.
         * 
         * Pointer to the added subset is returned and may be filled
         * after the call.
         */
        ParamSet* addSubset(const std::string& name);
        
        /* 
         * Add parameter with given name and string value.
         * 
         * This is shortcut for addSubset() when need to add named set
         * without futher subsets.
         */
        void addParameter(const std::string& name, const std::string& value)
        {
            addSubset(name)->setValue(value);
        }
        /*
         * Add parameter with given name and empty value.
         */
        void addParameter(const std::string& name)
        {
            addSubset(name);
        }
         
        
        /* 
         * Return value of the parameter set.
         * 
         * Usually, value is requested only for pure parameters,
         * that is parameters set without subsets.
         */
        const std::string& getValue(void) const;
        /*
         * Return vector of parameters sets, corresponded to given name.
         * 
         * Vector is non-empty with garantee.
         */
        const std::vector<ParamSet*>&
            getSubsets(const std::string& name) const;

        ParamSet(const ParamSet& paramSet);
        ParamSet& operator=(const ParamSet& paramSet);
    private:
        std::string value;
        std::map<std::string, std::vector<ParamSet*> > subsets;
    };
    

    /*
     * One template, loaded from file or from string.
     */
    class Template
    {
    public:
        /* 
         * Load template from stream. 
         * 
         * If not empty, 'filename' is used in error-reporting.
         */
        Template(std::istream& s, const std::string& filename);
        ~Template();

        class Impl;
    private:
        Impl* impl;

        Template(const Template& t); /* not implemented */
        Template& operator=(const Template& t); /* not implemented */
    };

    /* 
     * Abstract associative collection of templates.
     * 
     * Should be derived by the user for use in the constructor
     * of TemplateGroup class(see below).
     */
    class TemplateCollection
    {
    public:
        virtual ~TemplateCollection(){}
        /* 
         * Return template with given name. 
         * 
         * Return NULL if template with given name is not exist.
         * 
         * NOTE: Later situation is normal, because any identificator
         * may mean template or top-level parameter. If template with
         * name equal to identificator exists, it is assumed that
         * identificator refers to the template. Otherwise it is assumed
         * to be refered to the parameter.
         * 
         * Derived class is not responsible for storing and caching
         * results. Moreover, it may assume that any name may be
         * requested at most once while instantiate template group.
         */
        virtual Template* findTemplate(const std::string& name) = 0;
    };


    /*
     * Group of templates.
     * 
     * May be instantiated into stream using some parameters set.
     */
    class TemplateGroup
    {
    public:
        /*
         * Create template group from collection, using template with
         * given name as main.
         * That is, group contain given template and all templates
         * referenced by it.
         */
        TemplateGroup(TemplateCollection& templatesCollection,
            const std::string& mainTemplateName);
        ~TemplateGroup();
        
        /* 
         * Isntantiate templates group, using given parameters set,
         * into stream.
         */
        std::ostream& instantiate(std::ostream& os,
            const ParamSet& paramSet);
        /* Same but store result in the string instead of stream. */
        std::string instantiate(const ParamSet& paramSet);
    
        class Impl;
    private:
        TemplateGroup(const TemplateGroup& group); //not implemented
        TemplateGroup& operator=(const TemplateGroup& group); //not implemented

        Impl* impl;
    };

};

#endif /* MIST_H_INCLUDED */