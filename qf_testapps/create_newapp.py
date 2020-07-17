# This script can be used to clone any app into a new app folder
# The original source app folder will not be modified.
# The .json file used can have options to change the macro values
# from header files specified.
#
# Example Usage:
#   python create_newapp.py --source <source_folder> --dest <dest_folder>
# 
# where 
#   <source_folder> is the application project to be cloned
#   <dest_folder> is name of new application and destination folder
#
# See CreateNewProject.txt in this folder for some more info
#

import os, subprocess
import sys
import time
import argparse
import shutil
import json
import re, os, subprocess, shutil, fileinput, glob

mswindows = (sys.platform == 'win32')
script_summary = ''
sys.path.insert( 0, os.path.basename( __file__ ) )

def summary_add(report_str,sep='',end='\n',flush=False):
    global script_summary
    print(report_str, sep=sep, end=end, flush=flush)
    script_summary = script_summary + report_str + end
    
def update_header_file(path, macroname_dict):
    fw = fileinput.input(files=path, inplace=True)
    for line in fw:
      line_orig = line
      #for macroname_dict in macroname_dict_array:
      for macro_name, macro_value in macroname_dict.items():
          line = re.sub(r"^#define(\s)"+macro_name+r"(\s+).*$", 
                      r"#define "+macro_name+" "+macro_value, line)
      if (line_orig == line):
        pass
      else:
        print("Original: {}Replaced: {}".format(line_orig, line), file=sys.stderr)
      print(line.rstrip())
    fw.close()
    return

def update_projfile(path, old, new):
    summary_add("Updating project file: {} ( {} ==> {} )".format(path, old, new))
    fw = fileinput.input(files=path, inplace=True)
    for line in fw:
      line_orig = line
      line = re.sub(old, new, line) 
      if (line_orig == line):
        pass
      else:
        print("Original: {}Replaced: {}".format(line_orig, line), file=sys.stderr)
      print(line.rstrip())
    fw.close()
    return

def main(cmdline):
    parser = argparse.ArgumentParser(description='Make a new application from an existing application')

    # read release package spec from json file
    parser.add_argument( "--source", dest='source_app_folder', default=None,
                         help='source application folder to be cloned')
    # read release package spec from json file
    parser.add_argument( "--dest", dest='dest_app_folder', default=None,
                         help='destination folder and name of cloned application')

    # read release package spec from json file
    parser.add_argument( "--app_spec", dest='app_spec', default=None,
                         help='json file describing application package')

    input_args = parser.parse_args(cmdline)
    
    if ( (input_args.source_app_folder) and
         (input_args.dest_app_folder) ):
        src_folder = input_args.source_app_folder
        length = len(src_folder)
        if src_folder[length-1] == '/':
            src_folder = src_folder[0:length-1]
        js = { "source_app_folder": src_folder,
               "dest_app_folder": input_args.dest_app_folder,
               "hfile": {}
             }
        summary_add("Creating a new App {} --> {}\n\n"
           .format(input_args.source_app_folder, input_args.dest_app_folder))
    else:
        js_package = input_args.app_spec
        summary_add("Creating a new App {}\n\n".format(js_package))
        
        try:
          with open(input_args.app_spec) as fp_js_package:
            # Read the package specification from js_package file 
            js = json.load(fp_js_package)
        except ValueError as err:
          summary_add("Error parsing: {} document {}".format(js_package, err))
          raise

    # Copy source files specified in package spec to output folder
    source_folder = js.get("source_app_folder", "")
    dest_folder = js.get("dest_app_folder", "")
    summary_add("Copying from {} to {}".format(source_folder, dest_folder))

    shutil.copytree( source_folder, dest_folder )
    
    for hname, hdict in js["hfile"].items():
        hfile_path = os.path.join(dest_folder, hname)
        summary_add('Updating file: {}'.format(hfile_path))
        summary_add('   Change spec: {}'.format(hdict))
        update_header_file(hfile_path, hdict)

    if (0):
      # IAR Project modifications
      summary_add("IAR Project modifications : \n")
       
      # Remove Debug and settings folders
      shutil.rmtree ( os.path.join(dest_folder, 'IAR_Project', 'Debug'), ignore_errors=True)
      shutil.rmtree ( os.path.join(dest_folder, 'IAR_Project', 'settings'), ignore_errors=True )
  
      # Rename project files
      files  = glob.glob( os.path.join(dest_folder, 'IAR_Project', '*.ew[dptw]') )
      files += glob.glob( os.path.join(dest_folder, 'IAR_Project', '*.icf') )
      files += glob.glob( os.path.join(dest_folder, 'IAR_Project', '*.dep') )
      dest_basename = os.path.basename(dest_folder)
      src_basename = os.path.basename(source_folder)
  
      for file in files:
        file_dirname = os.path.dirname(file)
        file_basename = os.path.basename(file)
        file_head, file_ext = os.path.splitext(file_basename)
        new_filename = os.path.join(file_dirname, '{}{}'.format(dest_basename, file_ext))
        if (file_ext.lower() == '.ewp'):
           src_ewp = file
           dst_ewp = new_filename
        elif (file_ext.lower() == '.eww'):
           src_eww = file
           dst_eww = new_filename
        elif (file_ext.lower() == '.dep'):
           os.remove(file)
           continue
        summary_add("Moving {} --> {}".format(file, new_filename))
        shutil.move( file, new_filename )
      update_projfile( dst_eww, src_basename, dest_basename )
      update_projfile( dst_ewp, src_basename, dest_basename )
      update_projfile( os.path.join(dest_folder, 'IAR_Project', 'cmd_line_build.bat'), src_basename, dest_basename )

    # GCC Project modifications
    summary_add("GCC Project modifications : \n")
    
    # Remove Debug and settings folders
    shutil.rmtree ( os.path.join(dest_folder, 'GCC_Project', 'output'), ignore_errors=True)

    # Rename project files
    files  = glob.glob( os.path.join(dest_folder, 'GCC_Project', '*.ld') )
    src_makefile = os.path.join(dest_folder, 'GCC_Project', 'config.mk')
    dest_basename = os.path.basename(dest_folder)
    src_basename = os.path.basename(source_folder)
    print("source folder = {}, source basename= {} ".format( source_folder,  src_basename))
    print("dest   folder = {}, dest   basename= {} ".format( dest_folder,   dest_basename))

    for file in files:
      file_dirname = os.path.dirname(file)
      file_basename = os.path.basename(file)
      file_head, file_ext = os.path.splitext(file_basename)
      new_filename = os.path.join(file_dirname, '{}{}'.format(dest_basename, file_ext))
      summary_add("Moving {} --> {}".format(file, new_filename))
      shutil.move( file, new_filename )

    update_projfile( src_makefile, "PROJ_NAME={}".format(src_basename), "PROJ_NAME={}".format(dest_basename))

if __name__ == '__main__':
    args = sys.argv[1:]
    summary_add('********************************')
    summary_add('Create New App Script Execution summary : ', end='')
    summary_add(" {}".format(os.path.basename(__file__)), end='')
    summary_add(" ".join(args))
    #args = ["--visible-word",'--out_dir=./testoutput/' ]
    main(args)
