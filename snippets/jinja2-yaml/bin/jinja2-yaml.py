#! /usr/bin/python

import jinja2
import yaml
import sys, os
import getopt

def debug(message):
    pass #print >> sys.stderr, message


def error(message):
    print >> sys.stderr, "jinja2-yaml: " + message
    exit(1)

usage = """
Usage: jinja2-yaml [options] <templates-dir> <yaml-documents...>

Where:
    <yaml-documents> - file(s), containing YAML documents
    <templates-dir> - directory with Jinja2 templates

Acceptable options are:
    -o|--output=<file>
        File for output template instantiation. Default is stdout.
    -t|--template-name=<name>
        Set name of template to be instantiated. Default is 'document'.
    -d|--deps=<file>
        Store list of template files, which was really used during
        template instantiation, into given file. If given file already
        exists, it will be updated only when list of template files is
        changed.
    -f|--deps-format=<format>
        Format for file with dependencies. This is a jinja2 template,
        which is used for instantiate sorted list 'deps' of dependencies.
        Default is:
            <%for d in deps%>{{d}}
            <%endfor%>
    -F|--deps-format-file=<format-file>
        File, contained format for file with dependencies.
    -h|--help
        Print this usage and exit.
"""



# Arguments with default values
output_file = None
template_name = 'document'
deps_file = None
deps_file_format = """{%for d in deps%}{{d}}
{%endfor%}
"""

if len(sys.argv) == 1:
    print >> sys.stderr, "Usage: jinja2-yaml [options] <templates-dir> <yaml-documents...>"
    exit(1)


try:
    opts, args = getopt.gnu_getopt(sys.argv[1:], "-o:-t:-d:-f:-F:-h",
        "output:,template-name:,deps:,deps-format:,deps-format-file:,--help")
except getopt.GetoptError as err:
    error(str(err))

for o, v in opts:
    if o == "-o" or o == "--output":
        output_file = v
    if o == "-t" or o == "--template-name":
        template_name = v
    elif o == "-d" or o == "--deps":
        deps_file = v
    elif o == "-f" or o == "--deps-format":
        deps_file_format = v
    elif o == "-F" or o == "--deps-file-format":
        with open(v, 'r') as f:
            deps_file_format = f.read()
    elif o == "-h" or o == "--help":
        print usage
        exit(0)
        

if len(args) == 0:
    error("Template directory missed")
elif len(args) == 1:
    error("At least one YAML-file should be specified")

templates_dir = args[0]
yaml_files = args[1:]

class TemplateLoader(jinja2.BaseLoader):
    """ Template loader wich locates templates in the 'path/%name%.tpl'.
    """
    def __init__(self, path, debug=False, used=None):
        self.path = path
        self.debug = debug
        self.used = used
    
    def get_source(self, environment, template):
        filename = os.path.join(self.path, template + '.tpl')
        if not os.path.exists(filename):
            if self.debug:
                source = '{TODO:' + template + '}' 
                return source, None, lambda: True
            else:
                raise jinja2.TemplateNotFound(template)
        mtime = os.path.getmtime(filename)
        with file(filename) as f:
            source = f.read().decode('utf-8')
        # Update list of used templates.
        if self.used is not None:
            self.used.add(filename)
        return source, filename, lambda: mtime == os.path.getmtime(filename)

used_files = None
if(deps_file):
    used_files = set()

loader = TemplateLoader(templates_dir, used=used_files)

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

# Dictionary of top-level variables
topvars = {}

def add_yaml_document(yaml_document):
    for topvar in yaml_document.keys():
        debug("Find toplevel variable '" + topvar + "' in document")
        if topvar not in topvars:
            topvars[topvar] = yaml_document[topvar]
        else:
            if(not isinstance(topvars[topvar], list) or not isinstance(yaml_document[topvar], list)):
                error("Several YAML documents have '" + topvar + "' toplevel variable, but type of that variable is not a 'list'.")
            debug("Combine values for toplevel variable '" + topvar + "'")
            topvars[topvar] = topvars[topvar] + (yaml_document[topvar])
    

# Combine top-level variables from all documents from all files
for yaml_file in yaml_files:
    documents = yaml.safe_load_all(file(yaml_file, 'r'))
    for document in documents:
        add_yaml_document(document)

# Render template into stream...
stream = template.stream(topvars)

# ..and dump stream itself into file or stdout according to options.
if output_file:
    try:
        f = open(output_file, 'w')
    except Exception as e:
        error("Failed to open file for output: " + str(e))
    try:
        stream.dump(f)
    except Exception as e:
        f.close()
        # Remove resulted file on fail.
        #
        # Otherwise file will be assumed up-to-date when rerun build
        # process like 'make'.
        os.remove(output_file)
        error("Template instantiation failed: " + str(e))
    else:
        f.close()
else:
    stream.dump(sys.stdout)

# Generate deps-file if needed.
if(deps_file):
    deps = [dep for dep in sorted(used_files)]
    deps_content = jinja2.Template(deps_file_format).render(deps = deps)
    deps_content_old = ''
    if os.path.exists(deps_file):
        with open(deps_file, 'r') as f:
            deps_content_old = f.read()
    if(deps_content_old != deps_content):
        with open(deps_file, 'w') as f:
            f.write(deps_content)
