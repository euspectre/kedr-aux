#!/usr/bin/python

import sys, os, re, errno

# Print message into stderr. Automatically add newline symbol.
def msg(message):
    sys.stderr.write(message + "\n")

def debug(message):
    sys.stderr.write("### " + message + "\n")

# Print error-related message
def error(message):
    msg("Error: " + message)

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
            msg("File " + self.filename + ", line " + str(self.lineNo))
            msg("    " + self.line)
        else:
            msg("File " + self.filename)
        error(message)
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
group_param_map = None
sequences = dict()

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
    global group_param_map
    if group_param is not None:
        raise ValueError("Group parameter is already defined before")
    group_param = param_name
    if param_value is None:
        raise ValueError("Value should be given for group parameter name")
    
    group_param_map = param_value


def sequences_parse(param_name, param_value):
    global sequences
    if param_name in sequences:
        raise ValueError("Sequence '" + param_name + "' is already defined")
    if param_value is None:
        raise ValueError("Value should be given for sequence parameter name")

    sequences[param_name] = param_value

convertionSections = {
    "param_map_global": param_map_global_parse,
    "param_map_group": param_map_group_parse,
    "group_param": group_param_parse,
    "sequences": sequences_parse
}
##################################
if len(sys.argv) < 4:
    msg("Usage: convert-kedr-jinja2.py <def-file> <kedr-gen-templates> <jinja2-templates>")
    exit(1)

def_filename = sys.argv[1]
input_templates_dir = sys.argv[2]
output_templates_dir = sys.argv[3]

defParser = DefFileParser(convertionSections)
defParser.parse(def_filename)

if group_param is not None:
    # Add group prefix for group scope mapping.
    for p in param_map_group.keys():
        if param_map_group[p].startswith("."):
               param_map_group[p] = group_param + param_map_group[p]



# Position in the file
class Position:
    def __init__(self, filename, line = 1, column = 0):
        self.filename = filename
        self.line = line
        self.column = column
    
    def copy(self):
        return Position(self.filename, self.line, self.column)
    
    # Print position.
    # If @s is not None, also output @s as it is content of the file at that position.
    def pprint(self, s = None):
        msg("File " + self.filename + ", line " + str(self.line) + ", column " + str(self.column))
        if s is not None:
            if self.column > 0:
                prefix = "(...) "
            else:
                prefix = ""
            if s[len(s) - 1] != "\n":
                suffix = " (...)"
            else:
                suffix = ""
        
            msg(prefix + s + suffix)
            
    
    # Advance position after reading given string.
    def advance(self, s):
        nLines = s.count("\n")
        if nLines > 0:
            self.line = self.line + nLines
            pos = s.rfind("\n")
            assert(pos != -1)
            self.column = len(s) - pos
        else:
            self.column = self.column + len(s)



class Token:
    def __init__(self, match, position):
        self.match = match
        self.name = match.lastgroup
        self.value = match.group()
        
        self.position = position
        
    # Print position of the token and token's value
    def printOrigin(self):
        self.position.pprint(self.value)
        

# Generic scanner 
#
# It is based on example in
# https://docs.python.org/dev/library/re.html#writing-a-tokenizer
class Scanner:
    # @filename - file, which content should be scanned.
    # @rules - sequence of tuples [name, regex]
    def __init__(self, filename, rules):
        # Read whole file at once. This is needed for cross newlines while parsing
        self.content = open(filename, "r").read()
        self.regex = re.compile('|'.join("(?P<%s>%s)" % pair for pair in rules))
        
        # Position for the next read
        self.position = Position(filename)

        # Position in characters of last read
        self.pos = 0
        # Position in character for read next
        self.posNext = 0
        
    # Return next token.
    #
    # In token.match named groups, corresponded to ones in rule's regex,
    # may be extracted using standard accessors(match.group(name))
    # Position groups are shifted on match.groupindex(match.lastgroup).
    def token(self):
        self.pos = self.posNext
        if len(self.content) == self.pos:
            return None
        match = self.regex.match(self.content, self.pos)
        if match.lastgroup is None:
            end_line = self.content.find("\n", self.pos)
            if end_line != -1:
                s = self.content[self.pos: end_line]
            else:
                s = self.content[self.pos:]
            self.position.pprint(s)
            error("Syntax error")
            exit(1)
        
        t = Token(match, self.position.copy())
        
        self.posNext = match.end()
        self.position.advance(match.group())
        return t
    

# Return 'parent' parameter for given one.
#
# It input paremter is top-level one, return empty string ("").
def parent_param(paramName):
    assert(paramName != "")
    pos = paramName.rfind(".")
    if pos != -1:
        return paramName[:pos]
    else:
        return ""

# Check whether paramName is (indirect) subparameter of parent
# Parent may be "", so every parameter is a subparameter of it.
def is_subparam(paramName, parent):
    if parent == "":
        return True
    else:
        return paramName.startswith(parent + ".")

# Abstract element of template abstract tree.
class ASTElem:
    # Create element of given type.
    # Also assign position and value to it for 
    # print messages about this element.
    def __init__(self, t, position, value):
        self.t = t
        self.position = position
        self.value = value
    
    def printOrigin(self):
        self.position.pprint(self.value)

# Possible types of ASTElem'ent
ASTTypeText, ASTTypeRef, ASTTypeIf = range(3)

# Simple text as AST element.
class ASTText(ASTElem):
    def __init__(self, position, value, text):
        ASTElem.__init__(self, ASTTypeText, position, value)
        self.text = text

# Generic reference to the template or parameter.
# Like ASTElem, it has position and value for
# print messages about it.
class ElemRef:
    def __init__(self, isTemplate, position, value):
        self.isTemplate = isTemplate
        self.position = position
        self.value = value

    # Virtual method.
    # Return base context for template and parameter name for parameter.
    def getBaseContext(self):
        pass

    def printOrigin(self):
        self.position.pprint(self.value)
    
    def wrongReference(self, message):
        self.printOrigin()
        if self.isTemplate:
            error("Reference to the template %s [with base context %s] is semantically incorrect in the current template." %
                (self.template.templateName, self.template.baseContext))
        else:
            error("Reference to the paramter %s is semantically incorrect in the current template." % (self.paramName))
        
        error(message)
        exit(1)

# Reference to the parameter
class ParamRef(ElemRef):
    def __init__(self, paramName, position, value):
        ElemRef.__init__(self, False, position, value)
        self.paramName = paramName
    
    def getBaseContext(self):
        return self.paramName

# Reference to the 'TemplateAST' object.
class TemplateRef(ElemRef):
    def __init__(self, template, position, value):
        ElemRef.__init__(self, True, position, value)
        self.template = template
    
    def getBaseContext(self):
        return self.template.baseContext

# Reference to the template or parameter as ASTElem'ent
class ASTRef(ASTElem):
    def __init__(self, elemRef, isJoin, joinStr = None):
        ASTElem.__init__(self, ASTTypeRef, elemRef.position, elemRef.value)
        self.elemRef = elemRef
        self.isJoin = isJoin
        self.joinStr = joinStr

# Condition used in 'if'. Like ASTElem, it has position and value for
# print messages about it.
class IfCondition:
    def __init__(self, elemRef, isConcat):
        self.position = elemRef.position
        self.value = elemRef.value
        self.elemRef = elemRef
        self.isConcat = isConcat

    def printOrigin(self):
        self.position.pprint(self.value)

class ASTIf(ASTElem):
    def __init__(self, ifCondition):
        ASTElem.__init__(self, ASTTypeIf, ifCondition.position, ifCondition.value)
        self.ifCondition = ifCondition
        self.ifSequence = []


# AST-tree of the whole template
#
# Can be emitted into file.
class TemplateAST:
    # Abstract method.
    #
    # If paramName is a sequence, return name of iterator for it.
    # Otherwise return None.
    def getSequenceIterator(self, paramName):
        return None

    # Empty template with given name and join context
    def __init__(self, templateName, joinContext):
        self.templateName = templateName
        self.joinContext = joinContext
        # Initial value of baseContext
        self.baseContext = joinContext
        # Top-level sequence of ASTElem'ents
        self.sequence = []

    # Save template into file
    def store(self, filename):
        with open(filename, "w") as outputFile:
            # Output file should be accessed only vi emitBlock/emitVariable/emitText.
            self.outputFile = outputFile
            # Whether next newline symbol will be automatically trimmed
            # when jinja2 template will be parsed
            self.newlineTrimmed = False
            # Whether current last symbol is a newline.
            # It was was trimmed is it will be last in the file.
            self.lastNewLine = False
            self.emitSequence(self.sequence)
            if self.lastNewLine:
                # Append additional newline symbol to the end of file.
                # So it will be trimmed instead of valuable one.
                self.outputFile.write("\n")
            
            self.outputFile = None

    def emitSequence(self, sequence):
        for astElem in sequence:
            if astElem.t == ASTTypeText:
                self.emitText(astElem.text)
            elif astElem.t == ASTTypeRef:
                self.emitRef(astElem)
            elif astElem.t == ASTTypeIf:
                self.emitIf(astElem)
            else:
                assert(False)

    def emitRef(self, astRef):
        if astRef.isJoin:
            outerContexts = self.getContextPath(astRef.elemRef)
            if len(outerContexts) == 0:
                astRef.printOrigin()
                msg("Warning: Join is not needed")
        else:
            outerContexts = list()
        
        if len(outerContexts) == 0:
            self.emitRefSimple(astRef.elemRef)
            return
        
        # Join statement
        if astRef.elemRef.isTemplate:
            self.emitJoinGeneric(astRef.elemRef, outerContexts, astRef.joinStr)
            return
        
        if len(outerContexts) == 1:
            paramName = astRef.elemRef.paramName
            context = outerContexts[0]
            if context == paramName:
                self.emitParameterJoined(paramName, astRef.joinStr)
            else:
                self.emitParameterJoinedAttr(paramName, astRef.joinStr, context)
        else:
            self.emitJoinGeneric(astRef.elemRef, outerContexts, astRef.joinStr)

    def emitIf(self, astIf):
        conditionContent = self.getConditionStr(astIf.ifCondition)
        self.emitBlock("if " + conditionContent)
        
        if astIf.ifSequence is not None:
            self.emitSequence(astIf.ifSequence)
        
        
        if hasattr(astIf, 'elseSequence'):
            self.emitBlock("else")
            self.emitSequence(astIf.elseSequence)
        
        self.emitBlock("endif")
        
    # Emit simple reference to template of parametertemplate inclusion
    def emitRefSimple(self, elemRef):
        if elemRef.isTemplate:
            self.emitBlock("include '%s'" % (elemRef.template.templateName))
        else:
            self.emitVariable(self.relParamName(elemRef.paramName))
        
    
    # Emit join statement in generic way: via <$for ..$>
    def emitJoinGeneric(self, elemRef, outerContexts, joinStr):
        contexts_reversed = list(outerContexts)
        contexts_reversed.reverse()
        for n in range(len(contexts_reversed)):
            context = contexts_reversed[n]
            if len(contexts_reversed) != 1 and joinStr != "" and n != len(contexts_reversed) - 1:
                # Store loop variable before next loop.
                # This is needed for calculate "not first iteration"
                # condition when emit non-empty joined string.
                self.emitBlock("set loop%s = loop" % str(n))
            i = self.getSequenceIterator(context)
            relContext = self.relParamName(context)
            self.emitBlock("for %s in %s" % (i,  relContext))
        if joinStr != "":
            if len(contexts_reversed) != 1:
                cond = "not (" + ''.join("loop%s.first and" % str(m) for m in range(len(contexts_reversed) - 1)) + "loop.first)"
            else:
                cond = "not loop.first"
            self.emitBlock("if %s" % cond)
            self.emitText(joinStr)
            self.emitBlock("endif")
        self.emitRefSimple(elemRef)
        for context in outerContexts:
            self.emitBlock("endfor")

    # Emit join statement as join filtering
    def emitParameterJoined(self, paramName, joinStr):
        rel = self.relParamName(paramName)
        joinStrEscaped = joinStr.encode('string_escape')
        self.emitVariable('%s | join(d="%s")' % (rel, joinStrEscaped))
    # Emit join statement as join filtering on attributes
    def emitParameterJoinedAttr(self, paramName, joinStr, context):
        relContext = self.relParamName(context)
        attr = paramName[len(context) + 1:]
        joinStrEscaped = joinStr.encode('string_escape')
        self.emitVariable('%s | join(d="%s", attribute="%s")'
            % (relContext, joinStr, attr))


    # Return string representation of 'if' condition for jinja2 template.
    def getConditionStr(self, ifCondition):
        elemRef = ifCondition.elemRef
        
        if elemRef.isTemplate:
            self.convert_warning(ifCondition.position, ifCondition.value,
                "Cannot convert template as condition.")

            if ifCondition.isConcat:
                conditionContent = "STAB: concat(%s)" % (elemRef.templateName,)
            else:
                conditionContent = "STAB: %s" % (elemRef.templateName,)
        else:
            if ifCondition.isConcat:
                contexts = self.getContextPath(elemRef)
                if len(contexts) == 0:
                    elemRef.printOrigin()
                    msg("Warning: Concat is not needed.")
            else:
                contexts = list()

            # Simplify concat condition as it is possible
            # TODO: Should this be done before emitting.
            if len(contexts) > 0 and contexts[0] == elemRef.paramName:
                contexts = contexts[1:]

            if len(contexts) == 0:
                conditionContent = self.relParamName(elemRef.paramName)
            elif len(contexts) == 1:
                sequence = contexts[0]
                relSequence = self.relParamName(sequence)
                if sequence == elemRef.paramName:
                    conditionContent = relSequence
                else:
                    attr = elemRef.paramName[len(sequence) + 1:]
                    conditionContent = "%s | join(attribute='%s')" % (relSequence, attr)
            else:
                self.convert_warning(ifCondition.position, ifCondition.value,
                    "Concatenation of parameters as condition here too complex for automatical convertion.")
                conditionContent = "STAB: concat(%s)" % (elemRef.paramName,)
                
        return conditionContent


    # Emit text with suitable escaping and newline control.
    def emitText(self, text):
        if len(text) == 0: # Non-emptines will be used after.
            return
        # Escape '<$' and other special constructions
        def converter(match):
            return "{{ '" + match.group() + "' }}"

        s = re.sub(r"<$|$>|{{|}}|<#|#>", converter, text)
        if self.newlineTrimmed:
            if s[0] == "\n":
                self.outputFile.write("\n")
            self.newlineTrimmed = False
        self.outputFile.write(s)
        self.lastNewLine = s[len(s) - 1] == "\n"

    # Emit block with given content, that is <$..$>.
    def emitBlock(self, b):
        self.outputFile.write("<$");
        self.outputFile.write(b);
        self.outputFile.write("$>");
        self.newlineTrimmed = True
        self.lastNewLine = False
    
    # Emit variable block with given content, that is {{..}}.
    def emitVariable(self, v):
        self.outputFile.write("{{");
        self.outputFile.write(v);
        self.outputFile.write("}}");
        self.newlineTrimmed = False
        self.lastNewLine = False

    # Adjust self.baseContext for accept given parameter as single-valued.
    def adjustBaseContext(self, elemRef):
        context = elemRef.getBaseContext()
        #debug("Adjust context for parameter '%s'." % (paramName))
        while len(context) > 0:
            if self.getSequenceIterator(context) is not None:
                break
            context = parent_param(context)
            #debug("Next context for test: '%s" % (context))
        
        if len(context) == 0:
            return # Parameter is itself sinle-valued
        elif context == self.baseContext:
            return # Already adjusted
        if self.baseContext == self.joinContext:
            # It is allowed to change baseContext only once for fit
            # to parameter's context.
            if is_subparam(context, self.baseContext):
                self.baseContext = context
                return
        elif context == self.joinContext:
            return # Already adjusted

        # Print suitable error message
        if not is_subparam(context, self.joinContext):
            elemRef.wrongReference("It is incompatible with base template's join context '%s." % (self.joinContext))
        elif is_subparam(self.baseContext, context):
            elemRef.wrongReference("It requires additional adjustment of template's base context '%s', which already differs from join '%s'."
                    % (self.baseContext, self.joinContext))
        elif is_subparam(context, self.baseContext):
            elemRef.wrongReference("It requires context, differed from template's join one '%s', but some other reference in the template require stronger template's base context '%s'."
                    % (self.joinContext, self.baseContext))
        else:
            elemRef.wrongReference("It is incompatible with base template's context '%s'." % (self.baseContext))
        
        #debug("Resulted base context: '%s'" % (self.baseContext))
    
    # Return list of sequences, which should be iterated for accept given
    # parameter as single-valued.
    #
    # Sequences are ordered from the most inner to the most outer.
    #
    # Is used for join-like constructions.
    def getContextPath(self, elemRef):
        result = list()
        context = elemRef.getBaseContext()
        if len(context) == 0:
            return result
        
        while len(context) > len(self.joinContext):
            if self.getSequenceIterator(context) is not None:
                result.append(context)
            context = parent_param(context)

        if context != self.joinContext:
            elemRef.wrongReference("It is incompatible with base template's join context '%s." % (self.joinContext))

        return result
            
    # Return 'relative' name of parameter, corresponded to the most inner
    # sequence, contained it(or coinside with it)
    # If no such sequence, return paramName.
    def relParamName(self, paramName):
        assert(len(paramName) != 0)
        # Do not transform last component of the parameter.
        context = parent_param(paramName)
        
        while len(context) != 0:
            i = self.getSequenceIterator(context)
            if i is not None:
                return i + paramName[len(context):]
            context = parent_param(context)
        return paramName
    # Original template is correct, but convertion of token itself fails.
    #
    # 'STAB' will be used in the resulted template.
    def convert_warning(self, position, value, message):
        position.pprint(value)
        error("Note: " + message)
        error("Note: 'STAB' will be used in the resulted template")

        
# Convert 'str' in <$ paramName: join(str) $> construction into text
# it represents.
def join_str_to_text(s):
    def replacer(match):
        l = match.group()
        if l == "\\n":
            return "\n"
        elif l == "\\t":
            return "\t"
        elif l == "\\\\":
            return "\\"
        elif l == "\\":
            return "\\" # Trailing backslash
        else:
            assert(False)

    return re.sub(r"\\n|\\t|\\\\|\\$", replacer, s)
    
# Loader of 'TemplateAST' from kedr_gen template file.
class KEDRTemplateLoader(Scanner):
    # Abstract methods for convert names in KEDR template and for
    # extract referenced templates.

    # If 'refName' corresponds to template, return Template instance.
    # Otherwise return None.
    def get_template(self, refName):
        return None # Not a template
    
    # Map parameter name according to user's definitions.
    def map_param(self, refName):
        return refName # Identical conversion

    # Note, that we use different names groups for all regular expressions.
    # Them will be combined together, and python prohibit names collizions.
    KEDRTemplateRules = [
        ("ifSentence", r"<\$if[ \t]+((?P<concat>concat\((?P<name_if_c>[\w.]+)\))|(?P<name_if>[\w.]+))[ \t]*\$>"),
        ("elseSentence", r"<\$[ \t]*else[ \t]*\$>"),
        ("endifSentence", r"<\$[ \t]*endif[ \t]*\$>"),
        ("refSentence", r"<\$[ \t]*(?P<name_ref>[\w.]+)[ \t]*(?P<join>:[ \t]*join[ \t]*(\((?P<join_str>[^\)]*)\)[ \t]*)?)?\$>"),
        ("textSentence", r"[^<]+|<[^<]*")
    ]

    def __init__(self, inputFilename):
        Scanner.__init__(self, inputFilename, KEDRTemplateLoader.KEDRTemplateRules)
        # Last extracted token
        self.currentToken = None
        # Whether self.currentToken is assumed consumed, so next
        # getToken() call should read next token from the stream.
        self.isTokenConsumed = True

    
    # Wrapper over Scanner.token() which store last extracted token.
    #
    # Extracting of that token may be virtually cancelled with ungetToken(),
    # so next getToken() call return it again.
    def getToken(self):
        if not self.isTokenConsumed:
            self.isTokenConsumed = True
            return self.currentToken
        self.currentToken = self.token()
        return self.currentToken
    
    def ungetToken(self):
        assert(self.isTokenConsumed)
        self.isTokenConsumed = False
    
    def parse_error(self, message):
        if self.currentToken is not None:
            self.currentToken.printOrigin()
        else:
            # EOF in the file. Extract filename from 'Scanner'
            msg("In file " + self.position.filename)
        error(message)
        exit(1)

    # Load template into 'TemplateAST' object.
    def load(self, templateAST):
        self.templateAST = templateAST
        self.loadSequence(templateAST.sequence)
        self.templateAST = None

    def loadSequence(self, astSequence):
        while True:
            token = self.getToken()
            if token is None:
                return
            elif token.name == "textSentence":
                text = token.value
                # TODO: convert {{ and }} (and $>)
                astText = ASTText(token.position, token.value, text)
                astSequence.append(astText)
            elif token.name == "ifSentence":
                isConcat = token.match.group("concat") is not None
                if isConcat:
                    refName = token.match.group("name_if_c")
                else:
                    refName = token.match.group("name_if")
                elemRef = self.getElemRef(refName, token.position, token.value)
                if not isConcat:
                    self.templateAST.adjustBaseContext(elemRef)
                ifCondition = IfCondition(elemRef, isConcat)
                astIf = ASTIf(ifCondition)
                self.loadIf(astIf)
                astSequence.append(astIf)
            elif token.name == "elseSentence":
                self.ungetToken()
                return
            elif token.name == "endifSentence":
                self.ungetToken()
                return
            elif token.name == "refSentence":
                refName = token.match.group("name_ref")
                if token.match.group("join") is not None:
                    isJoin = True
                    if token.match.group("join_str") is not None:
                        joinStr = join_str_to_text(token.match.group("join_str"))
                    else:
                        joinStr = ""
                else:
                    isJoin = False
                    joinStr = None
                elemRef = self.getElemRef(refName, token.position, token.value)
                if not isJoin:
                    self.templateAST.adjustBaseContext(elemRef)
                astRef = ASTRef(elemRef, isJoin, joinStr)
                astSequence.append(astRef)
            else:
                assert(False)

    def getElemRef(self, refName, position, value):
        template = self.get_template(refName)
        if template is not None:
            return TemplateRef(template, position, value)
        else:
            paramName = self.map_param(refName)
            return ParamRef(paramName, position, value)
        
    def loadIf(self, astIf):
        self.loadSequence(astIf.ifSequence)
        
        token = self.getToken()
        
        if token.name == "elseSentence":
            astIf.elseSequence = []
            self.loadSequence(astIf.elseSequence)
        else:
            self.ungetToken()
        
        token = self.getToken()
        if token.name != "endifSentence":
            self.parse_error("Expected <$endif$>")


class TemplatesConverter:
    def __init__(self):
        self.documentTemplates = dict()
        self.blockTemplates = dict()
    
    def load(self, inputDir):
        self.inputDir = inputDir
        templateDocument = self.getTemplateDocument("document")
        if templateDocument is None:
            error("'document' template is absent")
            exit(1)

    def store(self, outputDir):
        templates_all = self.documentTemplates.values() + self.blockTemplates.values()
        for template in templates_all:
            output_filename = outputDir + "/" + template.templateName + ".tpl"
            template.store(output_filename)
            msg("Converted: " + template.input_filename + " to " + output_filename)

    def getTemplateDocument(self, templateName):
        templateDocument = self.documentTemplates.get(templateName)
        if templateDocument is not None:
            #debug("Found template '%s'" % (templateName))
            return templateDocument

        input_filename = self.inputDir + "/document/" + templateName + ".tpl"
        if not os.path.exists(input_filename):
            return None
        
        templateDocument = self.TemplateASTReal(templateName, "")
        
        templateLoader = self.KEDRTemplateLoaderDocument(self, input_filename)
        templateLoader.load(templateDocument)
        # Save input filename for future status messages.
        templateDocument.input_filename = input_filename
            
        self.documentTemplates[templateName] = templateDocument
        
        return templateDocument


    def getTemplateBlock(self, templateName):
        templateBlock = self.blockTemplates.get(templateName)
        if templateBlock is not None:
            #debug("Found template '%s'" % (templateName))
            return templateBlock

        input_filename = self.inputDir + "/block/" + templateName + ".tpl"
        if not os.path.exists(input_filename):
            return None

        # Modify template name for do not collide with document's templates names.
        templateBlock = self.TemplateASTReal("block_" + templateName, group_param)
        
        templateLoader = self.KEDRTemplateLoaderBlock(self, input_filename)
        templateLoader.load(templateBlock)
        # Save input filename for future status messages.
        templateBlock.input_filename = input_filename

        self.blockTemplates[templateName] = templateBlock
        
        return templateBlock


    class TemplateASTReal(TemplateAST):
        def __init__(self, templateName, joinContext):
            TemplateAST.__init__(self, templateName, joinContext)
            
        def getSequenceIterator(self, paramName):
            if paramName == group_param:
                return group_param_map
            else:
                return sequences.get(paramName)
            
    class KEDRTemplateLoaderDocument(KEDRTemplateLoader):
        def __init__(self, outer, templateName):
            KEDRTemplateLoader.__init__(self,  templateName)
            self.outer = outer
        
        def map_param(self, refName):
            paramName = param_map_global.get(refName)
            if paramName is not None:
                return paramName
            return param_map_group.get(refName, refName)
        
        def get_template(self, refName):
            if refName != "block":
                return self.outer.getTemplateDocument(refName)
            else:
                if group_param is None:
                    error("'block' template is referenced, " +
                     "but [group_param] section in conversion definitions file is missed(or empty).")
                    exit(1)
                template = self.outer.getTemplateBlock(refName)
                if template is None:
                    error("'block' template is referenced, but absent.")
                    exit(1)
                return template

    class KEDRTemplateLoaderBlock(KEDRTemplateLoader):
        def __init__(self, outer, templateName):
            KEDRTemplateLoader.__init__(self,  templateName)
            self.outer = outer
            
        def map_param(self, refName):
            return param_map_group.get(refName, refName)
        
        def get_template(self, refName):
            return self.outer.getTemplateBlock(refName)
            
# debug

def debug_output():
    templateScanner = Scanner(input_templates_dir, templateTokens)

    match = templateScanner.token()
    while match is not None:
        if match.lastgroup != "textSentence":
            print match.lastgroup + ":" + match.group()
        else:
            print match.lastgroup
        match = templateScanner.token()

# Emulate mkdir -p functionality
try:
    os.mkdir(output_templates_dir)
except OSError as e:
    if e.errno == errno.EEXIST and os.path.isdir(output_templates_dir):
        pass
    else:
        raise

converter = TemplatesConverter()

converter.load(input_templates_dir)
converter.store(output_templates_dir)
