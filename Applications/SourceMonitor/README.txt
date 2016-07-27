
		# DOCUMENTATION TO "online_monitor.exe" #
		# ..................................... #

by jordan daniel roth (2015).
any questions, please contact at "jordanroth2013@gmail.com".
no guarantees that i'll be there if it's been more than ten years or so since i wrote this, but probably!

table of contents (for ctrl+F):
  introduction
  output
    basic histograms
    2D histograms
    special 1D histograms
    bad pixels
    time diagnostics
  input
    "parameters" tab
    command line arguments
    buttons
  implimentation notes
  how to compile

	# INTRODUCTION #

this application is meant to provide an online (real-time) method of checking the results of a source scan on the FE-I4.
this way, a faulty scan can be terminated before it has completely finished.

it creates a window in which it displays a variety of histograms.
only one histogram may be displayed at a time.
it saves all of its displayed histograms to an output file.

press "start" to make it go, and push the other buttons to view different histograms.

it may be configured via command-line arguments or its "parameters" tab.

	# OUTPUT #

   # basic histograms

LVL1: local timing information. records which data header a hit is received in. should be between 1 and 16. see "LVL1.png".

BCID: global timing information. records which external trigger a hit is received after. see "LVL2.png".

ToT1: time over threshold. approximately proportional to the free charge created in the sensor by the hit. four-bit value. see "ToT1.png".

ToT2: the ToT in the adjascent pixel (col,row+1). not terribly useful for this application. four-bit value. suppressed by default. see "ToT2.png".

col: records the column index of each hit. ranges from 0 to 79. see "col.png".

row: records the row index of each hit. ranges from 0 to 336. see "row.png".

   # 2D histograms

hitmap: x-axis is column index, y-axis is row index, z-axis is number hits. see "hitmap.png".

ToT map: x- and y-axes are the same as for hitmap. z-axis is the mean ToT1 for hits in a given pixel. see "ToT_map.png".

bad pixel map: shows where bad pixels have been identified. the z-axis indicates the BCID time at which they were decided to be bad.  see "bad_pixel_map.png".

   # special 1D histograms

total hits: counts how many hits, globally, have been detected. x-axis is BCID time. see "hits_vs_time.png".

mean ToT: running average of the ToT1 of all hits detected. x-axis is BCID time. see "ToT_vs_time.png".

   # bad pixels

this detects bad (i.e. leaky) pixels with an upper limit on hits per pixel.
this upper limit is linear in BCID time.
any bad pixels will be whited-out from the 2D histograms, and no further data will be collected from them for any histogram.

this prints the bad pixels it receives to std::cout and a text file.
they're recorded in "(col,row) @ BCID" format.
the slope and intercept of the upper limit used to label them as bad is recorded at the start of the file.
the last BCID recorded by the application is recorded at the end of the file.

   # time diagnostics

this can print timing information to std::cout on exit.
this would mainly be useful for telling if your writing frequency is too high.
it is disabled by default, but can be enabled with the command line argument "--showtimediagnostics:1".

it prints:
the time the application spent reading (i.e. reading lines from input, parsing them, filling histograms, etc),
the time the application spent writing (i.e. writing histograms to output, painting them to the canvas, etc),
the time the application spent waiting (i.e. lacking input and not writing),
the total time the application had run for (during its main loop, so not counting initialization).

if time spent waiting is small compared to time spent writing, it means that your histograms might not actually be updating properly.
this can probably be fixed by increasing the write period and restarting the application.

after porting to the Qt GUI, no lagging issues appear to come up even if the writing period is made very small; hence, there will probably never be a need to use this feature.

	# INPUT #

   # "parameters" tab

it's at the top of the window, to the left. click it.
lists variables in "variable [value]" format, one per line.
an explaination of most of the variables within follows.
mouse over the entry-box on the GUI for a description of each variable.

input : the file that the FE-I4 is writing to, which this application reads. must be *.raw or *.txt.

output: the file that this writes its histograms to. must be *.root.

writing period: the time, in milliseconds, between histogram-writes / canvas-repaints.

max hits per pixel slope/intercept: used for detecting bad pixels.

name, title, low bin, high bin: histogram settings.

suppress: a suppressed histogram is not displayed or saved to the output file.

TH1 fill color: can't take constants like "kBlue"; it has to be a number.

patient latency: how long the application idles when it sees no input before checking for input again.

impatient latency: how many patient wait cycles the application goes through before printing a message to std::cout about how it's not getting input.

   # command line arguments

any variable from the "parameters" tab can be set from the command line via an argument of the form "--variable:value".
the variable name will usually not have any spaces in it.
for example, the input can be set like "--input:source_SOURCE_SCAN_16.raw".
command-line arguments override the options in the "parameters" tab.

   # buttons

on the main screen, the buttons on the left edge name histograms.
clicking them will display the indicated histogram.

on the bottom, the "start" button attempts to start the application (open the input file, start reading, etc).
"next" cycles the viewer to the next histogram (not in button-order).
"reset" is like "start", but you push it after the application has been running for a while.

	# IMPLIMENTATION NOTES #

if you want to use the "online::monitor" class separate from the application, you need to say "new TApplication (blah)" and "new QApplication (blah)" beforehand.
it's stupid and encapsulation-breaking, but whatever.

the class "online::external_input" uses weird variable-calling-up syntax, so i suggest that you read its header.
i gave it a one-character names and horribly abused operator overloading since i wanted referencing outside variables to be almost like referencing native variables; extra characters would have gotten in the way.

i used to call "BCID" as "LVL2", so if you see "LVL2" or "lvl2" anywhere, it means "BCID".

i have all the histogram names sorted into categories.
they're ALL in "all_histograms", which is a std::vector<std::string>, since order is hypothetically important for determining how they cycle in the viewer.
there's also "map_histograms", which is for 2D histograms, "time_histograms", which is for histograms whose x-axis corresponds to BCID, and "basic_histograms".
the sub-lists are std::set<std::string>s, since order don't matter, but membership does.
the TH1Ds and TH2Ds themselves are stored in separate lists, which kinda sucks, since mostly it'd be okay to just access their shared methods, but sometimes not, so ...
if you wanna try to store them all in a heterogenous list, that'd be cool, but i didn't feel up to it.

if you add a new histogram, be sure to add it to "all_histograms" and some sub-list.
if you need to create a new sub-list, you may need to modify "initialize_histograms_and_data_containers()" as well as "define_histogram_filters()".
you'll also need to give it "name" and "title" parameters in "parameters.txt", and possibly "low bin" and "high bin".
you WILL need to give it a "suppress" option.

for "online::monitor", the constructor definition is a pretty good place to go to get an idea of what's going on.
REDACTED when i tried pushing this into a GUI, the constructor got all screwed up and now it's covered in Qt code and no good to read.
it may actually be better to go back to a previous, command-line-based version and look at the constructor of online::monitor there.
for the other classes, read the header. i tried to comment 'em up real purtty.

	# HOW TO COMPILE #

on Unix, it's worked pretty well for me so far; Windows has been a bit more finicky.

on both platforms, you start out by opening a command prompt and navigating to the source directory containing "main.cpp".
there, you type "qmake -project", push <enter>, and type "qmake" followed by <enter>.

at this point, you need to modify the makefile so that it'll compile the ROOT code; right now, it's only set to do Qt stuff.
exactly what you do differs by platform, and probably also depends on ROOT/Qt distribution and suchlike.
on both of them, if "toy_writer.cpp" comes with your distribution of the online monitor, you'll have to purge all mention of it from the makefile.

now for me, doing this works on Unix:
add "-std=c++0x" to "CXXFLAGS".
add "-I/opt/root/5.34.05/include" to "INCPATH".
add "-pthread -m64 -L/opt/root/5.34.05/lib -lGui -lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -pthread -lm -ldl -rdynamic" to "LIBS".
when in doubt, check to make sure that all options in "root-config --cflags --glibs" are located somewhere in the makefile.

on Windows, doing this to "Makefile.Debug" works for me:
add "-D_WINDOWS" to "DEFINES".
add "-Z7 -FIw32pragma.h" to "CFLAGS" and "CXXFLAGS" (only "CXXFLAGS" would probably be okay; i haven't tested it).
add "-I"..\..\..\..\..\..\Qt\5.2.1\5.2.1\msvc2012\include\QtWidgets" -I"C:\root\root_5.34_18\include"" to "INCPATH".
add "-LIBPATH:"C:\root\root_5.34_18\lib" libCore.lib libRIO.lib libHist.lib libGpad.lib" to "LIBS".
if this doesn't work then, sans changing the ROOT/Qt paths, i have no idea what to do.
the internet told me that ROOT executables need to be compiled with the exact same settings as their ROOT distribution, so ... try to figure out what those were.
see <https://root.cern.ch/phpBB3/viewtopic.php?f=3&t=3402>.

to run the makefile on Windows, run "vcvarsall.bat" (look it up on the internet) and type "nmake -f Makefile.Default".

also, if you're trying to port the Unix version to Windows, you might get a bunch of errors about "and" and "or" and stuff.
at least, my version of VC++ didn't recognize the alternative C++ logic keywords.
so, you might need to sub out "and -> &&", "or -> ||", and "not -> !".
also, there's one point in "monitor.cpp" where you need to replace "auto pixel = std::array <int, 2> {{col,row}};" with "auto pixel = std::array <int, 2> (); pixel.at(0) = col; pixel.at(1) = row;".
not really sure why; i think that the former version is valid C++11 syntax ...

you should always get a pair of warnings about some unused "data" parameters.

