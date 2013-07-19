#! /usr/bin/python

import jinja2
import yaml
import sys, os

def debug(message):
    pass #print >> sys.stderr, message


def error(message):
    print >> sys.stderr, "Error: " + message
    exit(1)

usage = """
Usage: jinja2-yaml <yaml-document> <templates-dir> [<temlate-name>]

Where:
    <yaml-document> - file containing YAML document
    <templates-dir> - directory with Jinja2 templates
    <template-name> - name of the template used to instantiate data.
                      If not given, 'document' is used.
"""

if len(sys.argv) < 3 or len(sys.argv) > 4:
    print usage
    exit(1)

yaml_document = sys.argv[1]
templates_dir = sys.argv[2]

template_name= 'document'
if len(sys.argv) == 4:
    template_name = sys.argv[3]

class TemplateLoader(jinja2.BaseLoader):
    """ Template loader wich locates templates in the 'path/%name%.tpl'.
    """
    def __init__(self, path, debug=False):
        self.path = path
        self.debug = debug
    
    def get_source(self, environment, template):
        filename = os.path.join(self.path, template + '.tpl')
        if not os.path.exists(filename):
            if self.debug:
                source = '{TODO:' + template + '}' 
                return source, None, lambda: True
            else:
                raise TemplateNotFound(template)
        mtime = os.path.getmtime(filename)
        with file(filename) as f:
            source = f.read().decode('utf-8')
        return source, filename, lambda: mtime == os.path.getmtime(filename)

loader = TemplateLoader(templates_dir)

env = jinja2.Environment(loader=loader,
    block_start_string='<$',
    block_end_string='$>',
    comment_start_string='<#',
    comment_end_string='#>',
    trim_blocks=True
# For some reasons, jinja 2.6 lack of support for 'lstrip_block' parameter
# (but support {%- %} in templates )
# jinja 2.6 lack of support for 'keep_trailing_newline' parameter
    )

template = env.get_template(template_name)

documents = yaml.safe_load_all(file(yaml_document, 'r'))

# Dictionary of top-level variables
topvars = {}

# Combine top-level variables from all documents
for document in documents:
    debug("Begin new document")
    for topvar in document.keys():
        debug("Find toplevel variable '" + topvar + "' in document")
        if topvar not in topvars:
            topvars[topvar] = document[topvar]
        else:
            if(not isinstance(topvars[topvar], list) or not isinstance(document[topvar], list)):
                error("Several YAML documents have '" + topvar + "' toplevel variable, but type of that variable is not a 'list'.")
            debug("Combine values for toplevel variable '" + topvar + "'")
            topvars[topvar] = topvars[topvar] + (document[topvar])

# Use traling ',' for not output additional newline symbol.
print template.render(dict([(topvar, topvars[topvar]) for topvar in topvars.keys()])),
