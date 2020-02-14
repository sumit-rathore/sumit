###############################################################################
###
###   Icron Technology Corporation - Copyright 2016
###
###
###   This source file and the information contained in it are confidential and
###   proprietary to Icron Technology Corporation. The reproduction or disclosure,
###   in whole or in part, to anyone outside of Icron without the written approval
###   of a Icron officer under a Non-Disclosure Agreement, or to any employee of
###   Icron who has not previously obtained written authorization for access from
###   the individual responsible for the source code, will have a significant
###   detrimental effect on Icron and is expressly prohibited.
###
###############################################################################
##
##!   @file  - icron_to_json.py
##
##!   @brief - To read, translate ICRON files to JSON format, creat a JSON file and write the
##             translated data
##             sample basic case: icomponent file is written in format below:
##             components:
##             C:TOPLEVEL_COMPONENT
##             C:TEST_HARNESS_COMPONENT
##             ...
##             This script remove the string "component" and "C:" so the left over will be a list of
##             component names, for eg. "TOPLEVEL_COMPONENT", this list wil be translated into JSON
##             array and is published in a JSON file in the following steps:
##             1- open and read icomponent and save it into "in_file"
##             2- read the "in_file" line by line
##             3- search for strings after "C:" until the end of the line using regular expression
##                library.
##             4- once the specific string is found, add it to the proper (inner/outer) list or
##                dictioniary.
##             5- Lastly publish the "main_list" into a JSON file, (json.dump takes care of
##                translating python
##             list into JSON array)
##
##
##!   @note - Translation, creation and write of a json file must happen in a single command
##          - line so "main_list" or "main_dict" variable must be properly constructed before it
##            gets passed to json.
##          - name of the output file will be the same as input file with .Json extension
##
##          - Refer to "main_dict" or "main_list" for the final structure which will be
##            translated into json.
##
##          - name of the output file will be the same as input file with .json extension.
###############################################################################


import pprint
import json
from collections import OrderedDict
import re
import sys
import argparse

def icron_file_input():
    """
    Print out the usage of this script.
    """
    if len(sys.argv[1:]) == 0:
        print("\n Missing arguments, try [icron_to_json.py -h]")
    else:
        # the following part makes an objext containing all the required input arguments from user
        parser = argparse.ArgumentParser(usage="icron_to_json.py -h", description="This script \
                                                        converts icron files to json format.")
        parser.add_argument("--icomponent", help="path to icomponent", nargs=2, required = False)
        parser.add_argument("--icmd", help="path to icmd and icmdresp", nargs=3, required = False)
        parser.add_argument("--icron_header", help="path to icron_header", nargs=2, required = False)
        parser.add_argument("--ilog", help="path to ilog", nargs=2, required = False)
        parser.add_argument("--istatus", help="path to istatus", nargs=2, required = False)
        args = parser.parse_args()
        if args.icomponent:
            convert_icron_icomponent_to_json(args.icomponent[0], args.icomponent[1])
        if args.icmd:
            convert_icron_icmd_to_json(args.icmd[0], args.icmd[1], args.icmd[2])
        if args.icron_header:
            convert_icron_icron_header_to_json(args.icron_header[0], args.icron_header[1])
        if args.ilog:
            convert_icron_ilog_to_json(args.ilog[0], args.ilog[1])
        if args.istatus:
            convert_icron_istatus_to_json(args.istatus[0], args.istatus[1])

#function to map "icomponent" to json format
def convert_icron_icomponent_to_json(icomponent, output):
    """Convert icron "icomponent" file to JSON file
       final structure: ["component1", "component2", ...]"""
    main_list = [] #the finalized list with all the component names
    with open(icomponent, 'r') as in_file :
        for line in in_file:
            #Looking for the components and inserting them into the main list
            if re.search(r"^C:",line):
                main_list.append(re.search(r"^C:(?P<key>.*)$",line).group("key"))

        # Creat and Write the file out again
        # name of the output: icomponent.json
        with open("%s" %output, 'w') as out_file:
            json.dump(main_list, out_file, sort_keys = False, indent = 4)

#Function to map "icmd" file to JSON format
def convert_icron_icmd_to_json(icmd, icmdresp, output):
    """Convert icron "icmd" file to JSON file
       final structure: {component1:[{variable1}, {variable2}, ...], component2:[{}, {},...],..."""
    #the finalized dictionary with all the component names
    main_dict = OrderedDict()
    # to save in the response lables in order to compare them with function names in icmd
    response_matcher_dict = {}
    #some compnent will have aditional argument called response;
    #dictionary to keep the response strings
    response_dict = None
    compo_name = None
    #open and read icmd response file
    with open(icmdresp, 'r') as response_file:
        for response_line in response_file:
            if re.search(r"^.*C:(.*)L:.*$", response_line):
                response_dict = OrderedDict()
                response_dict["icmdresp_name"] = re.search(r"^.*R:(?P<response_string>.*)\sC:.*$",\
                                                           response_line).group("response_string")
                response_dict["ilog_name"] = re.search(r"^.*L:(?P<ilog_name>.*)\sA:.*$", \
                                                       response_line).group('ilog_name')

                if re.search(r"^.*A:\((?P<resp_arg>.*)$", response_line):
                    response_dict["icmdresp_argument_index_list"] = \
                            [int(arg) for arg in re.search(r"^.*A:\((?P<resp_arg>.*)\)$", \
                                response_line).group('resp_arg').split(",")]
                else:
                    response_dict["icmdresp_argument_index_list"] = \
                            [int(arg) for arg in re.search(r"^.*A:(?P<resp_arg>.*)$", \
                                response_line).group('resp_arg').split()]

                func_name = re.search(r"^.*C:(?P<func_name>.*)\sL:.*$", \
                                        response_line).group("func_name")
                func_name_dict[func_name] = response_dict
                response_matcher_dict[compo_name] = func_name_dict

            elif re.search(r"^.*C:(.*)$", response_line):
                        compo_name = re.search(r"^.*C:(?P<component>.*).*$",\
                        response_line).group("component")
                        func_name_dict={}
            else:
                print("Unknown response variable.")

    #open the icmd file and save it in in_file
    with open(icmd, 'r') as in_file: 
        for line in in_file:
            if re.search(r"^.*component:\S(.*)$",line):
                #components are list of dictionaries
                label = re.search(r"^.*component:(?P<component>.*)$",line).group('component')
                #"components" are lists that contain dictionaries
                function_dict = OrderedDict()
                function_inner_dict = None
                func_index = 0
            else:
                if re.search(r"^F:(.*)\sH:.*$",line):
                    #the components arguments dictionary
                    function_inner_dict = OrderedDict()
                    function_inner_dict["function_index"] = func_index
                    func_index += 1 

                    #note:do not combine the two following lines, each variables are used seperately
                    func_name = re.search(r"^F:(?P<func_name>.*)\sH:.*$",line).group('func_name')

                    #this list is used to compare the name of function with functinos found in
                    #icmd response file, if a function exist in both #files then add a response
                    #variable to the new icmd file.
                    if label in response_matcher_dict:
                        if func_name in response_matcher_dict[label]:
                            function_inner_dict["ICMDRESP"] = \
                                         (response_matcher_dict[label][func_name])

                    if re.search(r"^.*H:(.*)\sA:.*$",line):
                        function_inner_dict["icmd_help_string"] = \
                                        re.search(r'^.*H:"(?P<help>.*)"\sA:.*$',line).group('help')

                    if re.search(r"^.*A:(.*)$",line):
                       function_inner_dict["icmd_argument_type"] = \
                                re.search(r"^.*A:(?P<func_arg>.*)$",line).group('func_arg').split(", ")

                    #creats the dictionary that contains the component variables
                    function_dict[func_name] = function_inner_dict
                else: 
                    print("Function Not Found!")

            #constructs the outer and final structure
            main_dict[label] = function_dict
    # Create and Write the file out again
    # name of the output: icmd.json
    with open("%s" %output, 'w') as f:
        json.dump(main_dict, f, indent = 4, sort_keys = False)

#Function to map "icron_header" file to JSON format
def convert_icron_icron_header_to_json(icron_header, output):
    """Convert icron "icron_header" file to JSON file
       final structure:{"key1":"vlue1", "key2":"variable2", ...}"""
#Note: some lines in "icron_header" have two colons which makes search for string more complex
    main_dict = OrderedDict()#the finalized dictionary with icron_header names
    dict_for_buttons = OrderedDict() #the finalized dictionary for the hobbes buttons

    with open(icron_header, "r") as in_file: #open and save "icron_header"
        for line in in_file:
            #Get rid of extra white space
            no_space = re.sub(r'\s+','',line)
            #Looks for the key in using regex none greedy to find first colon
            key = re.search(r"^(?P<key>.*?):.*$",no_space).group('key')
            #the statment "hobbes_default_icmd_buttonX" is replaced with "hobbes_gui_buttonX"
            new_key = key.replace("default_icmd", "gui")
            #if this line has two collons then make another dictionary inside the main dictionary
            if re.search(r"^.*:\s(.*):\siCmds.*$",line):
            #Dictionary to save the inner object of button; contains bottun label and button number
                buttons_inner_dict = {}
                buttons_inner_dict[re.search(r"^.*:\s(?P<button>.*):.*$",line).group("button")] = \
                        re.search(r"^.*:\siCmds(?P<button_label>.*)$",line).group('button_label')
                dict_for_buttons[new_key] = OrderedDict(buttons_inner_dict)
            else:
                main_dict[new_key] = re.search(r"^.*:(?P<value>.*)$",no_space).group('value')

        # Write the file out again
    with open("%s" %output, 'w') as f:
        json.dump(main_dict, f, sort_keys = False, indent=4)
    with open("hobbes_buttons.json", 'w') as f:
        json.dump(dict_for_buttons, f, sort_keys = False, indent=4)

#The following code generates the ichannel_id.json
    main_dict = OrderedDict()
    channel_id_dict = {
            "goldenears":
            {
                "ilog_channel": 176,
                "icmd_channel": 177
            },
            "blackbird":
            {
                "ilog_channel": 128,
                "icmd_channel": 129
            }
    }

    with open("ichannel_id.json", 'w') as f:
        json.dump(channel_id_dict, f, sort_keys = False, indent=4)


#The following code generates the ilog_level.json

    ilog_level = ["ILOG_DEBUG",
                  "ILOG_MINOR_EVENT",
                  "ILOG_MAJOR_EVENT",
                  "ILOG_WARNING",
                  "ILOG_MINOR_ERROR",
                  "ILOG_MAJOR_ERROR", 
                  "ILOG_USER_LOG",
                  "ILOG_FATAL_ERROR",
                  "ILOG_DEBUG_PINK",     
                  "ILOG_DEBUG_GREEN",    
                  "ILOG_DEBUG_ORANGE"  
                  ]

    with open("ilog_level.json", 'w') as f:
        json.dump(ilog_level, f, sort_keys = False, indent=4)
        f.close()

def convert_icron_ilog_to_json(ilog, output):
    """Convert icron "ilog" file to JSON file
       Final structure:{"component1":[{"key1":"variable1", "key2":"variable2", ...}],
       "component2":[{},{},...],..."""
    #Note the output file must have a dictionary inside an array
    main_dict = OrderedDict()
    #open and read "ilog" file
    with open(ilog, 'r') as in_file:
        for line in in_file:
            #Look for components label, if exist:
            if re.search(r"^.*component:\S(.*)$",line):
                label = re.search(r"^.*component:(?P<component>.*)$",line).group('component')
                component_main_list = []
                #dictoinary to save the arguments and variables inside each component
                component_inner_dict = None
            else:
                component_inner_dict = OrderedDict()
                #Looks for the ilog label, if exist:
                if re.search(r"^L:(?P<ilog_label>\w+)\s+S:(?P<ilog_string>.*$)", line):
                    result = re.search(r"^L:(?P<ilog_label>\w+)\s+S:(?P<ilog_string>.*$)", line)
                    component_inner_dict['ilog_name'] = result.group('ilog_label')
                    component_inner_dict['ilog_string'] = \
                                            result.group('ilog_string').replace('\\n', '\n')
                    component_inner_dict['ilog_num_args'] = \
                                                        len(re.findall(r"(%\.*\d*[dix])", line))

            if component_inner_dict:
                #builds the inner dictionary
                component_main_list.append(component_inner_dict)
            main_dict[label] = list(component_main_list)#builds the main list(array)
                                #translate to jason, create and write the json file
    with open("%s" %output, 'w') as f:
        json.dump(main_dict, f, indent = 4, sort_keys = False)

def convert_icron_istatus_to_json(istatus_file, output_json_file):
    istatus_list = []
    with open(istatus_file, 'r') as f:
        for line in f:
            if re.search(r"^L:(?P<istatus_label>\w+)\s+S:(?P<istatus_string>.*$)", line):
                istatus_message = OrderedDict()
                result = re.search(r"^L:(?P<istatus_label>\w+)\s+S:(?P<istatus_string>.*$)", line)
                istatus_message['istatus_name'] = result.group('istatus_label')
                istatus_message['istatus_string'] = \
                                        result.group('istatus_string').replace('\\n', '\n')
                num_args_list = re.findall(r"(%\.*\d*[dix])",line)
                istatus_message['istatus_num_args'] = len(num_args_list)
                istatus_list.append(istatus_message)

    with open(output_json_file, 'w') as f:
        json.dump(istatus_list, f, indent = 4)

if __name__ == "__main__":
    icron_file_input()
