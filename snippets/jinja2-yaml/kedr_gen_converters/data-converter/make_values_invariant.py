#!/usr/bin/python

"""
    Change content of kedr_gen datafile, that its future conversion into
    YAML using convert-kedr-yaml.py preserves values for all parameters.
    
    Precisely, all multiline values become less indented by 'n',
    where n is an indentation of first value's line.
    Also, all multiline values become newline ended.
    
    Such conversion is useful when compare results of kedr_gen generation
    and jinja2-yaml one.
    
    Usage:
        python make_values_invariant.py
    
    Original datafile is read from STDIN.
    Resulted datafile is writen to STDOUT.
"""

import sys, re

multiline_param_begin = re.compile(r"[ \t]*[\w.]+[ \t]* =>>")
multiline_param_end = re.compile(r"<<")
indent_re = re.compile(r"[ ]*")
# Whether multiline parameter is parsed now
is_mparam = False
# Used when multiline parameter is parsed.
# In that case, value of variable is a indentation of the first line of
# the parameter's value.
# If the first line has not found yet, it is 'None'.
indent = None
# Used when multiline parameter is parsed.
# In that case, value of variable is True if last parsed line is empty.
last_newline = False

# Used when multiline parameter is parsed.
# In that case, value of variable is True if at least 2 lines are found.
really_multiline = False


for l in sys.stdin:
    if is_mparam:
        match = multiline_param_end.match(l)
        if match is not None:
            is_mparam = False
            indent = None
            # If parameter contains no newline symbols it can be converted
            # into YAML using short notation.
            if really_multiline:
                # Unconditionally print last newline symbol
                print ""
                really_multiline = False

            last_newline = False
        else:
            if indent is None:
                indent = indent_re.match(l).group()
            else
                really_multiline = True

            if l.startswith(indent): # Always true for the first line
                l = l[len(indent):]
            
            if last_newline:
                # Print previously stored newline symbol
                print ""
                last_newline = False
            if l == "\n":
                # Clear line, but store newline symbol as a flag.
                last_newline = True
                l = ""
    else:
        match = multiline_param_begin.match(l)
        if match is not None:
            is_mparam = True
    print l,
