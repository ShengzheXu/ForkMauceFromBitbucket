#!/usr/bin/env python

from __future__ import print_function
import sys
import os
import errno
import shutil
import subprocess

# This script is used in order to create a single PNG file containing the plot
# of the specified methods.
#
# The data used to create the plots must already exist, and be in a folder hierarchy
# that this script understands. To sum it up:
#
# - This file will look in the directory [folder/experiment]
# - For each method to plot, it will look into [folder/experiment/method]. If
#   no methods are specified, all folders found are used.
# - Inside each method folder, it will look for all files which contain options
#   which match the ones specified by the user. If no options are specified, all
#   files will match.
#
# This script makes use of the makeGnuplotScript.sh file, which you may need to
# modify in order to change GnuPlot behaviour.

# NOTE: ADDITIONAL PARAMETERS
#
# To avoid complicating the script parsing too much, since it's already pretty
# bad, we offer a couple of options here in forms of variables.
use_title = 0                   # Whether to add a title to the plot.
title_name = ""                 # Title name (if unspecified, the experiment)
use_subtitle_parameters = 0     # Whether to add a subtitle containing all common flags for all experiments
use_line_parameters = 0         # Whether to add line-specific parameters to the legend

if len(sys.argv) < 2:
    print("usage: " + sys.argv[0] + " [--output=file] folder experiment [--name=value ...] [method [--name=value ...] [method [--name=value ...] ...]]")
    sys.exit(0)

def mkdirMinusP(dirName):
    """ This function creates the folder specified, even if nested. """
    try:
        os.makedirs(dirName)
    except OSError as exc:  # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(dirName):
            pass
        else:
            raise

def readName(i, label):
    if i >= len(sys.argv):
        print("Couln't read " + label)
        sys.exit(1)

    name = sys.argv[i]
    if name.startswith("-"):
        print("String passed for " + label + " is invalid.")
        sys.exit(1)

    return name, i+1

def readFlags(i):
    flags = {}
    while i < len(sys.argv) and sys.argv[i].startswith("--"):
        flag = sys.argv[i][2:]
        flag_kv = flag.split("=")
        if (len(flag_kv) != 2 or
            len(flag_kv[0]) < 1 or len(flag_kv[1]) < 1):
            print("Option " + flag + " is invalid.")
            sys.exit(1)
        flags[flag_kv[0]] = flag_kv[1]
        i += 1
    return flags, i

i = 1
output, i = readFlags(i)
if len(output) > 1 or (len(output) == 1 and not output.keys()[0] == "output"):
    print("Only output option is allowed at the beginning")
    sys.exit(1)
if len(output) == 1:
    output = output["output"]
else:
    output = None

folder, i = readName(i, "folder")
experiment, i = readName(i, "experiment")
experiment_flags, i = readFlags(i)
experiment_folder = os.path.join(folder, experiment)

# Read the methods to plot, and the flags to use for each one.
methods_order = []
methods_flags = {}
while i < len(sys.argv):
    method, i = readName(i, "method")
    method_flags, i = readFlags(i)

    methods_flags[method] = method_flags
    methods_order.append(method)

if len(methods_flags) == 0:
    methods_flags = {m: {} for m in os.listdir(experiment_folder) if os.path.isdir(os.path.join(experiment_folder, m))}
    methods_order = methods_flags.keys()

if len(methods_flags) == 0:
    print("Nothing to plot")
    sys.exit(1)

files_to_plot = []
for m in methods_order:
    param = methods_flags[m]
    data_folder = os.path.join(experiment_folder, m)
    options = param.copy()
    options.update(experiment_flags)
    options_list = []
    for kv in options.items():
        options_list.append([kv[0] + "=" + v for v in kv[1].split(",")])

    method_files = [f for f in os.listdir(data_folder) if os.path.isfile(os.path.join(data_folder, f))]
    for f in method_files:
        file_options = f.split("_")
        if all(any(value in file_options for value in options) for options in options_list):
            files_to_plot.append((f, data_folder, m))

if len(files_to_plot) == 0:
    print("No files match the specified options")
    sys.exit(0)

plotTmpDir = "/tmp/plotDir"
mkdirMinusP(plotTmpDir);

common_flags = set(files_to_plot[0][0].split("_"))
for f in files_to_plot:
    common_flags = common_flags.intersection(set(f[0].split("_")))

common_flags_list = list(common_flags)
common_flags_list.sort()

fileLabels = []
for f in files_to_plot:
    params = f[0].split("_")
    new_params = [p for p in params if p not in common_flags]

    newFilename = f[2] + " " + ",".join(new_params)
    fileLabels.append(newFilename)
    if use_line_parameters:
        fileLabels.append(newFilename)
    else:
        fileLabels.append(f[2])

    shutil.copyfile(os.path.join(f[1], f[0]), os.path.join(plotTmpDir, newFilename))

title = ""
if use_title:
    title = experiment
    if title_name:
        title = title_name

    # Add common flags to the title.
    if use_subtitle_parameters and len(common_flags_list) > 0:
        title += '\\\\n{/*0.75' + ",".join(common_flags_list) + "}"

execall = ["./makeGnuplotScript.sh"]
if output is not None:
    cwd = os.getcwd()
    output_filename = os.path.join(cwd, output)
    execall.append(output_filename)
execall.append(title)
execall.extend(fileLabels)
# Create plot script
with open(os.path.join(plotTmpDir, "plot"), "w+") as plotFile:
    subprocess.call(execall, stdout=plotFile)

with open(os.path.join(plotTmpDir, "plot"), "r") as plotFile:
    print(plotFile.read())
# Plot
subprocess.call(["gnuplot","-p","plot"], cwd=plotTmpDir)

shutil.rmtree(plotTmpDir)
