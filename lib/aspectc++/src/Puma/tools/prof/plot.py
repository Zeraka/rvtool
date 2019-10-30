# -*- coding: utf-8 -*-

# Argument: csv-file containing profiling data
# Output on stdout: names of all created pdf files

import sys
import re
import subprocess
import os

# Regular expression to match the lines of the csv-file
# group ; grouptime ; function ; number of executions ; total time (clock cycles)

regex_csv = re.compile('^((?P<group>\S+))?\s+;\s*(?P<grouptime>[^;]+);\s*(?P<func>[^;]+);\s*(?P<execs>[0-9]+)\s+;\s+(?P<time>[0-9]+)\s+$')

regex_args = re.compile('\(.*\)')
regex_scope = re.compile('\S*::')

def short_sig(sig):
    argsmatch = regex_args.search (sig)
    scopematch = regex_scope.search (sig)
    if scopematch:
        scope = scopematch.group()
    else:
        scope = ""

    # substitute any arguments and trailing tokens by an empty string
    s = sig[:argsmatch.start()]

    arguments = sig[argsmatch.start()+1:argsmatch.end()-1]
    if arguments == "":
        arg_list = []
    else:
        arg_list = arguments.split(",")
    s = s.split("::")[-1]
    return (s.split()[-1], scope, arg_list)



def create_unique_signature(short, siglist):
    new_signatures = {}

    if len(siglist) == 1:
        (longsig,scope,args) = siglist[0]
        if len(args) == 0:
            new_signatures[longsig] = short+"()"
        else:
            new_signatures[longsig] = short+"(...)"
        return new_signatures

    # init with short signature
    for siginfo in siglist:
        (longsig, scope, args) = siginfo
        new_signatures[longsig] = short

    diff_signatures = set(new_signatures.values())

    # are there different scopes?
    scopes = set()
    for siginfo in siglist:
        (longsig, scope, args) = siginfo
        scopes.add(scope)
    if len(scopes) > 1:
        # add scope
        for siginfo in siglist:
            (longsig, scope, args) = siginfo
            newsig = new_signatures[longsig]
            new_signatures[longsig] = scope+newsig
            diff_signatures = set(new_signatures.values())

    index = 0
    while len(diff_signatures) < len(new_signatures):
        # not yet unique, add arguments
        for siginfo in siglist:
            (longsig, scope, args) = siginfo
            newsig = new_signatures[longsig]
            if index == 0:
                newsig = newsig + "("
            if index < len(args):
                newsig = newsig + args[index]
            if index+1 < len(args):
                newsig = newsig + ","
            if index+1 == len(args):
                newsig = newsig +")"
            new_signatures[longsig] = newsig
        diff_signatures = set(new_signatures.values())
        index+= 1

    # add parentheses
    for longsig in new_signatures:
        new_sig = new_signatures[longsig]
        if str.find (new_sig,'(') == -1:
            new_sig += "("
        if str.find (new_sig,')') == -1:
            new_sig += "...)"
        new_signatures[longsig] = new_sig
    return new_signatures



class Function:
    func = ""
    short_func = ""
    execs = 0;
    time = 0
    def __init__(self, func, execs, time):
        self.short_name = short_sig(func)
        self.func = func
        self.execs = execs
        self.time = time
        return

class Group:
    def __init__(self, name, is_function, grptime):
        self.name = name
        self.is_function = is_function
        self.grptime = grptime
        self.maxtime = 0
        self.functions = []
        return

    def add_data(self,func, execs, time):
        f = Function(func,execs,time)
        self.functions.append(f)
        if self.maxtime < time:
            self.maxtime = time
        return
    

class ProfilingData:
    csv_file = ""
    datafile = ""
    commandfile = ""
    maxtime = 0
    groups = {}

    def __init__(self, filename):
        self.csv_file = filename
        # create names for temporary files containing the gnuplot commands and data
        (basename, extension) = os.path.splitext(self.csv_file)
        self.commandfile = basename + ".gp"
        self.datafile = basename + ".data"
        self.maxtime = 0
        self.groups = {}
        try:
            fd = open (self.csv_file, 'r')
            data = fd.readlines()
            fd.close()
        except:
            print "Error when reading '" + self.csv_file + "'"
            sys.exit (2)

        first = True
        for line in data:
            if first:
                first = False
                continue
            # Is it a valid line?
            match = re.match(regex_csv, line)
            if match:
                group = match.group('group')
                grouptime = match.group('grouptime')
                func = match.group('func').strip()
                execs = long(match.group('execs'))
                time = long(match.group('time'))
                if not group:
                    is_function = True
                    group = func
                    grouptime = time
                else:
                    is_function = False
                    grouptime = long(grouptime)
                if self.maxtime < grouptime:
                    self.maxtime = grouptime
                if group in self.groups:
                    groupInfo = self.groups[group]
                    groupInfo.add_data(func,execs,time)
                else:
                    groupInfo = Group(group,is_function,grouptime)
                    self.groups[group] = groupInfo
                    groupInfo.add_data(func,execs,time)
            else:
                print "nomatch", line
        return


    def print_data (self):
        print "Data read from ", self.csv_file
        for name in self.groups:
            groupInfo = self.groups[name]
            print groupInfo.name, groupInfo.grptime


    def assemble_commands (self, x, name, time):
        if x == 1:
            command = "plot "
            data = ""
        else:
            command = ", "
            data = ";"
        command += "newhistogram at "+str(x)+" '', "
        command += "'"+self.datafile+"' "
        command += "using "+str(2*x)+":xtic("+str(2*x-1)+"), "
        command += "'' using 0:"+str(2*x)+":"+str(2*x)+" with labels left "
        command += "offset first "+str(x)+",graph 0.05 notitle"
        data += name+";"+str(time)

        return (command,data)


    def print_gnuplot_commands(self,group,maxval):
        title = self.csv_file
        if group != None:
            title += ", "+ group
        
        commandhdr = ""
        commandhdr += "#gnuplot commands for profiling data of "
        commandhdr += title
        commandhdr += "\n\n"
        commandhdr += "set style fill solid 1.00 border -1\n"
        commandhdr += "set style histogram\n"
        commandhdr += "set style data histograms\n"
        commandhdr += "set key off\n"
        commandhdr += "set ylabel 'time in clock ticks'\n"
        commandhdr += "set xtics rotate by -45\n"
        commandhdr += "set datafile separator ';'\n"
        commandhdr += "set title 'Weaving Performance for "+title+"'\n"
        commandhdr += "set format y \"%.0f\"\n"
        command = ""
        data = ""
        xtics = 0;
        
        if group == None:
            # create and count short signatures
            short_signatures = {}
            for name in self.groups:
                groupInfo = self.groups[name]
                if groupInfo.is_function:
                    sig = groupInfo.name
                    (short, scope, args) = short_sig(sig)
                    if short in short_signatures:
                        sig_list = short_signatures[short]
                        sig_list.append ((sig,scope,args))
                    else:
                        short_signatures[short] = [(sig,scope,args)]

            # Create a histogram with infos about all groups and functions
            unique_signatures = {}
            xtics = len (self.groups)
            maxy = maxval.maxtime
            x = 0
            for name in self.groups:
                x += 1
                groupInfo = self.groups[name]
                if len(groupInfo.functions) == 1:
                    execStr = " ["+str(groupInfo.functions[0].execs)+"]"
                else:
                    execStr = ""

                sig = groupInfo.name
                if groupInfo.is_function:
                    if not unique_signatures.has_key(sig):
                        (short, scope, args) = short_sig(sig)
                        sig_list = short_signatures[short]
                        unique_signatures.update(create_unique_signature(short, sig_list))
                    sig = unique_signatures[sig]
                (com, dat) = self.assemble_commands (x, sig+execStr,
                                   groupInfo.grptime)
                command += com
                data += dat
                
        else:
            # create and count short signatures
            short_signatures = {}
            groupInfo = self.groups[group]
            for func in groupInfo.functions:
                sig = func.func
                (short, scope, args) = short_sig(sig)
                if short in short_signatures:
                    sig_info = short_signatures[short]
                    sig_info.append ((sig,scope,args))
                else:
                    short_signatures[short] = [(sig,scope,args)]

            # Create a histogram with infos about the given group
            maxy = maxval.grouptime[group]
            x = 0
            xtics = len(groupInfo.functions)
            if xtics == 1:
                return (None, None)

            unique_signatures = {}
            for func in groupInfo.functions:
                x += 1
                sig = func.func
                if unique_signatures.has_key(sig):
                    sig = unique_signatures[sig]
                else:
                    (short, scope, args) = short_sig(sig)
                    sig_info = short_signatures[short]
                    unique_signatures.update(create_unique_signature(short, sig_info))
                    sig = unique_signatures[sig]
                (com, dat) = self.assemble_commands (x, sig+" ["+str(func.execs)+"]",
                                        func.time)
                command += com
                data += dat

        commandhdr += "set yrange [0:"+str(maxy*1.1)+"]\n"
        commandhdr += "set terminal postscript eps noenhanced "
        commandhdr += "color solid rounded "
        if xtics < 30:
             commandhdr += "'Helvetica' 12\n"
        else:
            commandhdr += "'Helvetica' 8\n"

        return (commandhdr+command, data)
                            

    def create_pdf (self, command, data, name=None):
        if command == None:
            return

        # save the histograms as several eps and pdf files
        (basename, extension) = os.path.splitext(self.csv_file)
        if name != None:
            epsfile = basename + "-"+name+".eps"
        else:
            epsfile = basename + ".eps"

        try:
            dat = open(self.datafile, "w")
            sys.stdout = dat
            print data
        except:
            print "Cannot create temporary file '" + self.datafile + "'"
        finally:
            sys.stdout = sys.__stdout__
            dat.close()

        try:
            gp = open(self.commandfile, "w")
            sys.stdout = gp
            print command
        except:
            print "Cannot create temporary file '" + self.commandfile + "'"
        finally:
            sys.stdout = sys.__stdout__
            gp.close()

        # execute "gnuplot <datafile>"
        try:
            eps = open(epsfile, "w")
            gnuplot = subprocess.call(["gnuplot", self.commandfile], stdout=eps)
        except Exception, e:
            print "Error executing gnuplot: %s" % e
            sys.exit (2)
        finally:
            eps.close()

        # execute "epstopdf <epsfile>"
        try:
            epstopdf = os.system("epstopdf " + epsfile)
        except Exception, e:
            print "Error executing epstopdf: %s" % e
            sys.exit (2)

        # delete temporary gnuplot command and data files
        os.remove (self.commandfile)
        os.remove (self.datafile)

        # return the name of the pdf file
        (basename, extension) = os.path.splitext(epsfile)
        pdffile = basename+".pdf"
        return pdffile


    def create_histograms (self,maxval):
        # Create an overview histogram
        (command, data) = self.print_gnuplot_commands (None,maxval)
        pdffile = self.create_pdf (command, data)
        if pdffile != None:
            print pdffile,

        # create a histogram for each group
        for name in self.groups:
            (command, data) = self.print_gnuplot_commands (name,maxval)
            pdffile = self.create_pdf (command, data, name)
            if pdffile != None:
                print pdffile,

class MaxVal:
    maxtime = 0
    grouptime = {}

    def update (self, that):
        for group in that.groups:
            thatval = that.groups[group].maxtime
            if group in self.grouptime:
                if self.grouptime[group] < thatval:
                    self.grouptime[group] = thatval
            else:
                self.grouptime[group] = thatval
        if self.maxtime < that.maxtime:
            self.maxtime = that.maxtime
        return


def main(argv=None):
    if not argv: argv = sys.argv
    if len(argv) < 2:
        print "usage: python plot.py csv-files"
        sys.exit(1)

    # read the profiling data of all csv files
    profiling_data = []
    maxval = MaxVal()
    for csv_file in argv[1:]:
        data = ProfilingData(csv_file)
        profiling_data.append(data)
        maxval.update(data)

    # create histograms
    for data in profiling_data:
        data.create_histograms(maxval)

if __name__ == "__main__":
    sys.exit(main())

