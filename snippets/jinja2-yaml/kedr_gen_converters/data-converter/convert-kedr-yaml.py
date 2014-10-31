#!/usr/bin/python

import sys, os, re

def error(message):
    sys.stderr.write(message + "\n")

def debug(message):
    sys.stderr.write("### " + message + "\n")


class InputStream:
    def __init__(self, filename):
        self.filename = filename
        self.f = open(filename, "r")
        # Currently parsed line and its number. Used in error-reporting.
        self.line = None
        self.lineNo = 0
    
    def __del__(self):
        self.f.close()
    
    def readline(self):
        self.line = self.f.readline()
        if self.line:
            self.lineNo = self.lineNo + 1
        return self.line

    def __iter__(self):
        return self
    
    def next(self):
        self.readline()
        if not self.line:
            raise StopIteration
        return self.line
    
    def parse_error(self, message):
        if self.line:
            error("File " + self.filename + ", line " + str(self.lineNo))
            error("    " + self.line)
        else:
            error("File " + self.filename)
        error("Error: " + message)
        exit(1)

class DefFileParser:
    def __init__(self, sections):
        self.sections = sections
    
    def parse(self, filename):
        inputStream = InputStream(filename)
        section = None
        for line in inputStream:
            line = line.rstrip("\n")
            line = line.lstrip()
            if line == "" or line.startswith("#"):
                continue
            section_match = re.match(r"^\[([^\]]+)\]", line)
            if section_match is not None:
                section = section_match.group(1)
                if not section in self.sections:
                    inputStream.parse_error("Unknown section: " + section)
                continue
            param_match = re.match(r"^([\w._]+)[ \t]*(=[ \t]*(.*))?", line)
            if param_match is None:
                inputStream.parse_error("Syntax error")
            
            if section is None:
                inputStream.parse_error("Parameter definition without section")
            param_name = param_match.group(1)
            param_value = None
            if param_match.group(2) is not None:
                param_value = param_match.group(3)
            
            f = self.sections[section]
            try:
                f(param_name, param_value)
            except Exception as e:
                inputStream.parse_error(str(e))
            

# Definition of kedr_get parameters and how to convert them.
param_map_global = dict()
param_map_group = dict()
group_param = None
sequences = set()

def param_map_global_parse(param_name, param_value):
    global param_map_global
    if param_value is None:
        raise ValueError("Value should be given for parameter mapping")
    param_map_global[param_name] = param_value

def param_map_group_parse(param_name, param_value):
    global param_map_group
    if param_value is None:
        raise ValueError("Value should be given for parameter mapping")
    param_map_group[param_name] = param_value

def group_param_parse(param_name, param_value):
    global group_param
    if group_param is not None:
        raise ValueError("Group parameter is already defined before")
    group_param = param_name

def sequences_parse(param_name, param_value):
    global sequences
    global group_name
    if param_name in sequences:
        raise ValueError("Sequence '" + param_name + "' is already defined")
    sequences.add(param_name)

convertionSections = {
    "param_map_global": param_map_global_parse,
    "param_map_group": param_map_group_parse,
    "group_param": group_param_parse,
    "sequences": sequences_parse
}
##################################
if len(sys.argv) < 3:
    error("Usage: convert-kedr-yaml.py <def-file> <data-file>")
    exit(1)

def_filename = sys.argv[1]
input_file_name = sys.argv[2]

defParser = DefFileParser(convertionSections)
defParser.parse(def_filename)

if group_param is not None:
    # Add group prefix for group scope mapping.
    for p in param_map_group.keys():
        if param_map_group[p].startswith("."):
               param_map_group[p] = group_param + param_map_group[p]
 
input_file = open(input_file_name)
output_file = sys.stdout


input_file = InputStream(input_file_name)

def parse_error(message):
    input_file.parse_error(message)

# Possible types of tokens
tokenTypeEmpty, tokenTypeComment, tokenTypeGroup, tokenTypeParam = range(4)

# Tokens itself
class TokenEmpty:
    tokenType = tokenTypeEmpty

class TokenComment:
    tokenType = tokenTypeComment
    
    def __init__(self, comment):
        self.comment = comment

class TokenGroup:
    tokenType = tokenTypeGroup

class TokenParam:
    tokenType = tokenTypeParam
    
    def __init__(self, param_name, param_value):
        self.param_name = param_name
        self.param_value = param_value


class DataScanner:
    def __init__(self, inputStream):
        self.inputStream = inputStream
        self.lastToken = None
        self.tokenConsumed = True
        self.group_is_found = False

    def getToken(self):
        if not self.tokenConsumed:
            self.tokenConsumed = True
            return self.lastToken
        self.lastToken = self.doGetToken()
        return self.lastToken

    def ungetToken(self):
        assert self.tokenConsumed, "ungetToken() can only undo last getToken() operation."
        self.tokenConsumed = False    
    
    def __iter__(self):
        return self
    
    def next(self):
        token = self.getToken()
        if not token:
            raise StopIteration
        return token

    def doGetToken(self):
        line = self.inputStream.readline()
        if not line:
            return None
        line = line.rstrip("\n")
        line = line.lstrip()
        if not line:
            return TokenEmpty()
        elif line.startswith("#"):
            return TokenComment(line[1:])
        elif line == "[group]":
            if group_param is None:
                self.inputStream.parse_error("Data-file uses [group] section, " +
                    "but [group_param] section in conversion definitions file is missed(or empty).")
            self.group_is_found = True
            return TokenGroup()
        
        param_match = re.match(r"^([\w_.]+)[ \t]*(=>>|[ \t]=(.*))$", line)
        if param_match is None:
            parse_error("Failed to parse line")
            
        #print param_match.groups()
        param_name = param_match.group(1)
        
        if param_match.group(2) == "=>>":
            # Multiline param
            param_value = ""
            line = input_file.readline()
            while line:
                if line == "<<" or line == "<<\n":
                    break
                param_value += line
                line = input_file.readline()
            else:
                parse_error("Unexpected end of file file parse multiline value")
        else:
            param_value = param_match.group(3)
        
        if self.group_is_found:
            param_real_name = param_map_group.get(param_name, param_name)
        else:
            param_real_name = param_map_global.get(param_name, param_name)
        return TokenParam(param_real_name, param_value)


dataScanner = DataScanner(input_file)

def print_all():
    for token in dataScanner:
        if token.tokenType == tokenTypeEmpty:
            print "Empty line found"
        elif token.tokenType == tokenTypeComment:
            print "Comment found: " + token.comment
        elif token.tokenType == tokenTypeGroup:
            print "[group] found"
        elif token.tokenType == tokenTypeParam:
            print "Parameter definition found: " + token.param_name
            
        else:
            parse_error("Unknown line object")

#print_all()

# Auxiliary function. Print attribute's value.
#
# Should work with any possible parameter's values, suitable for kedr_gen.
def print_attribute_value(value, indent):
    if "\n" not in value:
        # One-line attribute value. TODO: process '"' in parameter value.
        output_file.write(" " + value + "\n")
        return
    # Multiline value
    strings = value.splitlines()
    value_indent = len(re.match(r"[ ]*", strings[0]).groups(0))
    if(value_indent):
        # Unindent first line. This identation will be lost anyway in YAML sence.
        #
        # Assuming that identation is global for all parameter lines,
        # unindent all lines on same value.
        indent_remove = re.compile("^[ ]{1," + str(value_indent) + "}")
        for i in range(len(strings) - 1):
            strings[i] = re.sub(indent_remove, "", strings[i])

    if strings[len(strings) - 1] == "":
        # Last newline symbol will be added in any case
        strings.pop()
    output_file.write(" |\n")
    for s in strings:
        output_file.write(indent + s + "\n")


# Process attribute and produce corresponded output.
#
# name - name of the attribute, if it is not global; otherwise None
# parent_prefix - common prefix of all attributes which belong to parent,
#       including '.' for non global attributes.
# indent - space indentation for subattributes
#
# Processing of global attribute starts at the very beginning of document.
# Processing of non-global attribute starts just after "name:" is printed
# on the same line
#
def process_attribute(name, parent_prefix, indent):
        
    if name is None:
        # Global section
        full_name = ""
        prefix = ""
    else:
        full_name = parent_prefix + name
        prefix = full_name + "."
    
    is_group = full_name == group_param
    # Group is a sequence also.
    is_sequence = is_group or full_name in sequences
    if is_sequence:
        # Prefix string before attribute name when it starts sequence's
        # element.
        # For group sequence element is started each [group] line.
        # For other sequences - when attribute set become non-empty.
        start_sequence_elem = indent[:-2] + "- "

    # Fast path for attribute which has only one string value.
    # That attribute definition ends immediately after that value is printed.
    elif name is not None:
        #debug ("Fast path")
        token = dataScanner.getToken()
        if (token is not None) and (token.tokenType == tokenTypeParam) and (token.param_name == full_name):
            print_attribute_value(token.param_value, indent + "    ")
            return
        #debug ("Fast path failed(token is " + str(token.tokenType) + ")")
        #if token.tokenType == tokenTypeParam:
        #    debug ("param name os " + token.param_name)
        dataScanner.ungetToken()
        # Otherwise print newline symbol and iterate tokens(including already parsed one)    

    if name is not None:
        output_file.write("\n")
        
    # Set of attributes found.
    #
    # Attribute may repeat previous one only when section is a sequence.
    # In that case, attributes set is reset and new sequence element is started.
    attributes = set()

    for token in dataScanner:
        if token.tokenType == tokenTypeEmpty:
            output_file.write("\n")
        elif token.tokenType == tokenTypeComment:
            output_file.write("#" + token.comment + "\n")
        elif token.tokenType == tokenTypeGroup:
            if name is None:
                # Start group attribute
                output_file.write(group_param + ":\n")
                token = process_attribute(group_param, prefix, "    ")
                continue
            elif is_group:
                # Restart group
                attributes = set()
            else:
                # Only top-level can start group attribute
                dataScanner.ungetToken()
                return
        elif token.tokenType == tokenTypeParam:
            if token.param_name == full_name:
                # Rare, but possible: current attribute is a sequence of values.
                if not is_sequence:
                    # This is only possible when parameter has both
                    # map and simple values; this is incorrect.
                    parse_error("Parameter " + full_name  + " has both simple and complex values. This is incorrect.")
                output_file.write(start_sequence_elem)
                print_attribute_value(token.param_value, indent + "    ")
            elif not token.param_name.startswith(prefix):
                # Parameter for another section is found
                if is_group:
                    parse_error("Once entered, group section should not be exited")
                dataScanner.ungetToken()
                return
            else:
                # Subparameter
                subparam_name = token.param_name[len(prefix):]
                
                # YAML- attribute definition
                
                attribute_name = re.match(r"[^.]+", subparam_name).group(0)
                
                # Check attribute name first.
                if attribute_name in attributes:
                    if is_group:
                        parse_error("New group sequence element is started only by [group] line")
                    elif is_sequence:
                        # Need to start new sequence element
                        attributes = set()
                    else:
                        print ("### Attributes: " + str(attributes))
                        error("Another attribute is defined between " + 
                          "several attribute '" + attribute_name + "' definitions")
                        parse_error("Cannot convert given data-file.")
                if is_sequence and not attributes:
                    # Starts new sequence element
                    output_file.write(start_sequence_elem)
                else:
                    output_file.write(indent)

                attributes.add(attribute_name)
                
                output_file.write(attribute_name + ":")
                dataScanner.ungetToken()
                process_attribute(attribute_name, prefix, indent + "    ")

# Debug wrapper of process_attribute. Just swap names of methods when need it.
def process_attribute1(name, parent_prefix, indent):
    if name is None:
        full_name = ""
    else:
        full_name = parent_prefix + name
    debug("Begin section '" + full_name + "'")
    process_attribute1(name, parent_prefix, indent)
    debug("End section '" + full_name + "'")

process_attribute(None, None, "")

