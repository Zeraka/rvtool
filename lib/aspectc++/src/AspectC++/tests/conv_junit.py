#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This script is reads "test-result.txt" and creates "test-result.xml".
# The latter provides the test results in JUnit format, so that it can
# be visualized by Jenkins.

import re
import xml.dom.minidom
import sys

# simply hard coded filename ...
input_file = "test-result.txt"
output_file = "test-result.xml"

re_test_start = re.compile('^\-\-\-START TEST (?P<no>[0-9]+): (?P<name>\S*):.*')
re_test_end   = re.compile('^\-\-\-END TEST (?P<no>[0-9]+): (?P<name>\S*): (?P<result>\w+).*')

test_results = []
test_number = None
test_name   = None
test_result = None
test_output = ""
test_summary = None

file = open(input_file, 'r')

for line in file:
    match_line_start = re.match(re_test_start, line)
    match_line_end   = re.match(re_test_end, line)
    if match_line_start:
        test_number = match_line_start.group('no')
        test_name   = match_line_start.group('name')
    elif match_line_end:
        test_result = match_line_end.group('result')
        test_results.append((test_number, test_name, test_result, test_output))
        test_number = None
        test_output = ""
    elif line[:-1] == "---START SUMMARY":
        test_number = None
        test_summary = ""
#        print "Start Summary"
    elif line[:-1] == "---END SUMMARY":
#        print 'End Summary:', test_summary
        test_summary = None
    else:
        if test_number:
            test_output += line
        elif test_summary != None:
#            print "Summary Line", line
            test_summary += line
#        else:
#            print "strange line"

file.close()

doc = xml.dom.minidom.Document()
    
suites = doc.createElement("testsuites")
doc.appendChild(suites)
suite = doc.createElement("testsuite")
suite.setAttribute("name", "AspectC++ regression tests")
suite.setAttribute("tests", str(len(test_results)))
suites.appendChild(suite)

for test in test_results:
    (number, name, result, output) = test
    elem = doc.createElement("testcase")
    elem.setAttribute("name", name)
    if result=="FAIL":
        elem_err = doc.createElement("error")
        elem.appendChild(elem_err)
    if output != "":
        elem_out = doc.createElement("system-out")
        elem_out_txt = doc.createTextNode(output)
        elem_out.appendChild(elem_out_txt)
        elem.appendChild(elem_out)
    suite.appendChild (elem)

try:
    file_object = open("test-result.xml", "w")
#    file_object.write(doc.toprettyxml(encoding="utf-8"))
    file_object.write(doc.toxml())
    file_object.close()
except:
    print "Unexpected error:", sys.exc_info()[0]
