Contents
========
1. Introduction
2. Copying and Modifying
3. Compiling the CCL-Parser
4. Running the CCL-Parser
5. Execution Sequence
6. Global Configuration
7. Example Execution Sequences and Configuration
8. Reproducing Reported Results
9. Contact
10. Bibliography

Introduction
============

This distribution contains the source code of an incremental common cover 
link parser ("ccl-parser"). This is an unsupervised parser which learns
to parse a language from unannotated example sentences of that language.
The parser uses a link-based representation of syntactic structure which
is similar, but not identical, to dependencies. This representation can
then be converted to an equivalent bracketing, and this is the 
output produced by the parser. For more details, see Seginer (2007a,2007b),
the first paper being very short and the second very long.

The source code is made available in order to make the results reported 
in these papers fully reproducible. Some minor differences between
the code in this distribution and the original code used in the experiments
are described below. The results remain practically unchanged.
The source code also demonstrates how the algorithm can be implemented 
efficiently. Finally, this should allow other researchers to implement
their own variants and extensions of the algorithm. If you wish to do so
and have any questions about the code or the algorithm, I will
be happy to answer them.

This file contains information on how to install, configure and run the
ccl-parser. For more information about the algorithm itself, please
see Seginer (2007a,2007b) or just read the source code.

Copying and Modifying
=====================

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

A copy of the GNU General Public License can be found in the file
'License.txt' which is part of this distribution.

Compiling the CCL-Parser
========================

The source code is written in C++. While most of the code should be platform 
independent, I have only compiled it on Linux, using gcc versions 4.0.2 and
higher (up to 4.3.0). The instructions below are therefore restricted 
to compilation on Linux. If you either succeed or fail to compile it 
on some other platform, I will be happy to hear about it.

Unpacking
---------

1. Copy cclparser.src.v1.0.0.2.tar.gz to the directory in which the 
   distribution should be unpacked. From now on I will refer to 
   this directory as <root>.
2. Use 'tar xvfz cclparser.src.v1.0.0.2.tar.gz' to unpack the distribution.
   The source code tree should now be in <root>/cclparser.

Compiling
---------

1. In order to compile the code, you need to have a C++ compiler.
   I used gcc versions 4.0.2 and higher (up to 4.3.0). If you are using 
   a standard installation of Linux, you probably have some version of gcc 
   installed. The name of the compiler to be used is specified by 
   the following line in the file <root>/cclparser/tools/prs.mk:

   CC =	g++

   By default this will look for an executable named 'g++' to compile the
   program with. If the compiler has a different name on your system,
   replace 'g++' on this line by the name of your compiler.

2. Now, simply run 'make' at <root>/cclparser (yes, you also need to have
   'make' installed). This should compile the code and the compiler should 
   not generate any warnings.

3. If all went well, the compiler created the following executable:

   <root>/cclparser/main/<OS name>/cclparser

   <OS name> is the name of your operating system (something like 'Linux').
   It is determined based on the setting of the environment variables.
   If $O is defined in the environment, the content of this variable
   is taken as <OS name>. If $O is not defined, $ARCH is used and if this
   too is not defined, $OSTYPE is used. If none of these is defined,
   <OS name> is set to 'UnknownOS'.

Running the CCL-Parser
======================

To run the parser, you need to specify the 'execution sequence' which should
be followed. Each step in the execution sequence specifies a set of input 
sentences to be processed and the operations which should be applied to
these sentences. A typical (simple) execution sequence contains two
steps: a learning step and a parsing step. The state of the parser at the
end of one step is the initial state of the parser at the beginning of
the next step. Since the parser is lexicalized, it is actually the
lexicon which is passed from one step to the next.

To run the parser, simply run:

cclparser [options] <execution sequence file> [<execution sequence file> ...]

<execution sequence file> is the name of a file containing the execution
sequence for the parser, as specified in the next section. The options 
available at the command line are exactly those which are available
at every line of the execution sequence file (and are described in the next 
section). When an option is given on the command line, it serves as
a default value for that option. This means that if no value is specified 
(in the execution sequence file) for an option at a given step, the value
given at the command line will be used at that step. If no value is given 
at the command line, the system's default value is used as default.

It is possible to specify more than one execution sequence file. The
files are then treated as if they form a single concatenated file
(with concatenation in the order the files are given).

Execution Sequence
==================

The execution sequence file should contain one line for every step in the
execution sequence. Empty lines and lines beginning with '#' are ignored.
Every line in the execution sequence file should have the following form:

<input file pattern> <input file type> <action> [options]

<input file name>
-----------------

This defines the input files for this step. A single regular expression 
may be given here. The format of this regular expression is the same
as used with shell commands (e.g. 'combined/wsj/*/*.mrg'). The files
are processed in the order defined by the system (usually alphabetic order).

<input file type>
-----------------

This specifies the format of the input files. All files in a single step 
must have the same format, but each step may have a different format. 
In all formats, the system comment symbol(s) can be used to indicate a comment
line (which is ignored). This symbol (or symbols) must appear at the beginning
of the line. By default, the comment symbol is '#' (see 'CommentStr' below
for modifying this).

The following formats are currently supported:

'word':

In this format every line in the input file should contain a single input
token (word or punctuation). Leading and trailing white space are ignored, 
but internal white space is not. An utterance is terminated by an empty line
(if you read in several files, make sure the last utterance in every file
is terminated by an empty line - it is only the end of the last file which
terminates an utterance even if there is no empty line). This format is
the most general input format, because it allows any text (including 
sequences of words separated by white space) to be considered as an input
token. If you want your token to begin with the system comment symbol
('#' by default), you can do one of two things: either change the
the system comment symbol (see 'CommentStr' below) or add white space
at the beginning of the line (the comment symbol must be the first symbol 
in the line for it to be recognized as a comment).

'line':

In this format, each line is a single utterance, with the tokens separated
by white space. In particular, punctuation should be separated by white
space from the surrounding text. Therefore, the standard text 
"No, thank you." should be written as "No , thank you ." in this format.

The following three formats take trees as input. While the tree structures 
are not used by the parser itself, they are needed if you want to use 
the built-in evaluation modules.

'wsj':

This format takes in Penn Treebank Wall Street Journal (Version 2) trees 
as input (this is the format available in the directory 'combined/wsj' 
of the Penn Treebank distribution).

'negra':

This format takes in the Negra corpus (version 2.0) Penn-style trees as input.

'ctb':

This format takes in the Chinese Treebank (version 5.0) trees as input.

<action>
--------

Four actions are currently defined:

'learn':

This applies the learning algorithm to the input files and updates the lexicon.
The lexicon is not cleared at the beginning of this step, so consecutive
learning steps may be defined to cumulatively learn from different inputs.

'parse':

This action parses the input files based on the lexicon inherited from 
previous steps. The lexicon is not changed during a parse step.

'learn+parse':

This combines the two actions, 'learn' and 'parse'. Since the algorithm
always parses while learning, this is internally equivalent to a 
'learn' step, only that it allows the parses to be evaluated and printed.
Because learning continues while parsing, each parse output by this
step uses a (slightly) different lexicon. This usually makes it more
difficult to understand why any given parse was produced by the parser.
For research purposes it is therefore recommended to first run a 'learn'
step and then run a 'parse' step. 

'filter':

Various filtering options are defined (see below) to allow a subset of
the input to be extracted. If the action specified is 'filter', the
utterances which match the filtering criteria are printed to the
output file. The format is the same as that found in the input file,
except that empty and comment lines are omitted.

options
-------

The following options may be used at any step in the execution sequence
and at the command line. Any combination of options may be given, but
options may be ignored if they do not make sense with the given action
or conflict with other options.

Some options may take a sequence of strings as an argument
(e.g. '-e PR PRTag'). The sequence is terminated by the next option
(anything beginning with '-') or the end of the line. If this option
is used on the command line, it may be followed by the name of
the execution sequence file. In this case, the end of the argument 
sequence can be indicated by the dummy option '--' (e.g.
'cclparser -e PR PRTag -- ExecutionSequence').

-B <number>

Indicates the number of the first utterance to be processed. For example,
if the input file(s) contain 1000 utterances, -B 400 will cause the first
399 utterances to be skipped, and processing begins with the 400th
utterance. The first 399 still need to be read (in order to find the
400th utterance) so, if you want to process only the last utterances
in a very large input file, it is best to extract those sentences into
a separate file (for example, by using the 'filter' action with
the '-B' option).

-c <number>

This is a counting filtering option. Only utterances containing no more
than <number> words are processed. Punctuation symbols are not included
in this count.

-C

Non-trivial tree structure filter. When this filter is applied, 
only utterances with a non-trivial tree structure (containing more than 
one non-terminal) are processed. If the input files do not contain
tree structures (but only a plain text format), all utterances are
rejected by this filter.

-e <evaluator> [<evaluator> .. <evaluator>] 

This option specifies the list of evaluators to be used. This is 
applicable only to a 'parse' (or 'learn+parse') step. Each evaluator has 
the format [<eval name>:]<eval type> where <eval name> is a name assigned 
to the evaluator and <eval type> is the type of evaluator to use.
When a <eval name> is given (it is always optional), the same evaluator
can be used in several different steps and the evaluation scores are
accumulated over the different steps. If no name is given, a new
(empty) evaluation is started for each step.
Currently, the following types of evaluators are supported:

'PR': 
Unlabeled precision and recall.

'PRTag': 
Precision and recall by non-terminal name. The precision and recall are 
calculated separately for every type of bracket, as determined by the 
non-terminal label of the bracket. Since no non-terminals appear in 
the output of the parser, all the non-terminals of the output are considered
empty. Therefore, for all non-empty non-terminals, precision is always 1.
          
'PRSTag':
Same as 'PRTag' only that in case non-terminal labels consist of several
parts (e.g. NP-SBJ), only the first part of the non-terminal is used
(e.g. NP). 

'PRTagSeq':
Precision and recall by part-of-speech sequence. This is similar to 'PRTag',
but instead of grouping brackets by their non-terminal label, brackets 
are groups by the part-of-speech sequence assigned to the words the bracket
covers (e.g. 'NNP NNP').

'PRSTagSeq':
Same as 'PRTagSeq' only that in case the parts-of-speech are compound
(e.g. 'ART-NK NN-NK') only the first part of each part of speech is
used (e.g. 'ART NN').

-G <configuration file name>

This option specifies the name of a configuration file containing values
for various global settings and options of the parser. The format of
this file and the values it may contain are specified in  
'Global Configuration' below. Only the values of those options specified
in the file are changed. The values of other options remain unchanged
from the previous step (if the value of a given option was never specified,
the system default is used). In most typical execution sequences, the global 
configuration file is read once by the first step in the execution sequence.

-L <number>

Indicates the number of the last utterance to be processed. For example,
'-L 1000' will only process the first 1000 utterances in the input file.
This may be combined with the -B option. For example, -B 401 -L 1000
processes the 600 utterances number 401 to 1000 in the input. 

-n <tag> [<tag> ... <tag>] 

This option filters out utterances which have one of the given tags as 
the tag of the top tagged bracket.

-o <base file name>

Base name of the output file to be used in this step. The output file name
is <base file name>.<suffix> where <suffix> is specified by the -s option
below. If the '-o' option is omitted, 'Unnamed' is used as the base
file name.

-p

Prints the lexicon at the end of the execution step. The output is written
to the file <base file name>.<suffix>.lexicon (where <base file name>
is set by the -o option and <suffix> is set by the -s option). If <suffix>
is empty, the output is written to <base file name>.lexicon.

-R <number>

If this is set to non-zero, the parser prints out a progress report
(number of objects processed) every <number> objects processed. Typically, 
this should be set to 100 or 1000.

-s <suffix>

Suffix of the output file name to be used in this step. The output file
name is <base file name>.<suffix>, where <base file name> is set by the
-o option. If the '-s' option is omitted, the current date and time is
used as a suffix (this ensures that every step writes to a different file,
unless the user has explicitly chosen to do otherwise, by selecting 
the same output file name in different steps).

-t <tag> [<tag> ... <tag>] 

This option filters out utterances which do not have one of the given tags as 
the tag of the top tagged bracket.

Global Configuration
====================

The configuration file may be used to modify the values of the global
parameters of the parser. These parameters control the way the input is read
and determine the output format (including debug printing). They also allow
you to choose between different variants of the algorithm and to set
various parameters of the evaluation metrics.

The configuration file may be reloaded for each step in the execution 
sequence and its name is specified by the '-G' option of that step (see above).
Parameters which are not specified in the configuration file of 
a given step inherit their value from the previous step (or are set to
the system default if this is the first execution step). 
Therefore, the configuration file need only specify values for those 
parameters which should be changed (with respect to the previous step).
Often, the configuration parameters remain unchanged throughout the
execution sequence and one only needs to specify a configuration file for 
the first step.

Every global variable in the parser has a name of the form g_X in the
the source code and can be set by adding the following line to the 
configuration file:

X <value>

The <value> can be a non-negative integer number, a string or
a sequence of strings, depending on the definition of X. 

Lines beginning with '#' are considered comments and ignored.

The following global parameters are currently available (group by 
functionality):


Input reading
-------------

UseTagsAsWords <number>:

If <number> is not 0 and the input contains a tag for each word 
(e.g. part-of-speech tags in the tree formats), these tags are used 
instead of the words. The default value is 0 (that is, using words, not tags).
Because tags do not have the Zipfian distribution, the algorithm does not work 
well from tags (so this default value should not be normally changed).

UseTagsAsLabels <number>:

If <number> is not 0 and the input contains a tag for each word 
(e.g. part-of-speech tags in the tree formats), these tags are added to
the labels of each word. The default value is 0 and it does not seem
useful to change it.

CurrencySymbolIsPunct <number>:

If <number> is not 0, currency symbols (in the Wall Street Journal Corpus)
are treated as punctuation (rather than words). By default, this is set
to non-zero, because this is the way it is standardly evaluated, following 
Klein and Manning (2002,2004) and Klein (2005). 

UseStoppingPunct <number>:

If <number> is not 0, the algorithm uses stopping punctuation 
(full stop, question mark, exclamation mark, semicolon, comma and dash)
in the way described in section 6.3 of Seginer (2007b). If this is set
to 0, all punctuation (including stopping punctuation) is ignored. The default
is non-zero (use stopping punctuation).

DiscardTerminatingPunct <number>:

In <number> is not 0, terminating punctuation (full stop, question mark
and exclamation mark) is ignored even if UseStoppingPunct is on.
This parameter was introduced because the original implementation used
in Seginer (2007a,2007b) had a bug which caused terminal punctuation to be
ignored. Therefore, to reproduce the results of those experiments accurately,
this parameter should be set to non-zero. Otherwise, this parameter should 
normally be set to 0 (which is also the default).
Discarding terminating punctuation only matters when this punctuation
appears sentence internally. Since this is relatively rare, the effect 
on the parses produced is minor.

ReversePennObjs <number>:

When <number> is not 0 and the input is in Penn Treebank style
(the 'wsj', 'negra' or 'ctb' formats) the utterances are processed
backwards ('from right to left'). This is used in the 'right-to-left'
parsing experiments reported in Seginer (2007a,2007b). This has no influence
when using the plain text formats ('word' and 'line'). The default
value is zero.

Parser Specification
--------------------

ParserType <string>:

The argument <string> specifies the name of the parsing algorithm to be used.
Currently, the only possible value here is "CCL", because this is the only
parser implemented. This is, of course, also the default value for this
parameter and should not be changed.

CCLBasicUseBothInValues <number>:

When <number> is not 0, the 'basic parsing function family' of section 6.3
of Seginer (2007b) is used (this is also the algorithm used in Seginer 2007a).
When <number> is 0, the 'simplified basic parsing function family' of
section 6.3.3 of Seginer (2007b) is used. The default is non-zero 
(the 'basic parsing function family').

StatisticsTopListMaxLen <number>:
MaxLabels <number>:

These parameters specify the <number> of highest-weight labels which are
used for updating the labels (section 6.3.1 of Seginer, 2007b) and matching
labels ('calculating the best matching label' in section 6.3.2 of
Seginer, 2007b). Though the two parameters can technically be set to different
values, they should both be set to the same value. The default value is 10
and this is also the value in all experiments in Seginer (2007a, 2007b).
Increasing this number slows down the parser but does not seem 
to significantly change the parsing results.  

Evaluation
----------

CountTopBracket <number>:

When the 'PR' precision and recall evaluator is used, this parameter determines
whether the top bracket (covering the whole utterance) should be included
in the scores. A non-zero <number> indicates that the top brackets should
be included. The default value is 1, which means that the top bracket is
indeed included. Since the top bracket is always correct, this tends 
to inflate the scores, especially for short sentences, but this is 
the standard evaluation measure for this task since Klein and Manning (2004).

EvalGroupedOutputSorting <string> [<string> ... <string>]:

When any of the grouped precision and recall evaluators ('PRTag', 'PRSTag',
'PRTagSeq' or 'PRSTagSeq') is used, this parameter determines the way their
output is sorted. Output is always in decreasing order. Possible values are:
'Expected': Sort by the number of times the given bracket category appears
            in the gold standard.
'Observed': Sort by the number of times the given bracket category appears
            in the parser's output.
'OverObserved': Sort by the number of times the given bracket category 
            was incorrectly suggested by the parser.
'UnderObserved': Sort by the number of times the given bracket category
            appeared in the gold standard but was not suggested by the parser.
For the 'PRTag' and 'PRSTag' evaluators, only the first sorting method 
('Expected') makes any sense. This is also the default value.
Several sorting methods may be specified. In that case, the evaluation
is printed multiple times, each time sorted according to the appropriate
sorting criterion.

GroupedEvalMaxPrint <number>:

When any of the grouped precision and recall evaluators ('PRTag', 'PRSTag',
'PRTagSeq' or 'PRSTagSeq') is used, this parameter determines how many
bracket categories to print (the top categories according to the selected
sorting method). Use 0 for all categories.

Output printing
---------------

CommentStr <string>:

This specifies the <string> which is prefixed to comment lines in
the output. By default this is "#". Lines beginning with this string
are ignored when the file is used as input to the parser.

LexMinPrint <number>:

When printing the lexicon ('-p' option above) this determines the minimal
frequency a word should have in order for its entry to be printed. The default
is 5. Be aware that selecting a smaller value may result in a very large
output file.

PrintingMode [<string> ... <string>]:

Specifies what extra output should be printed by the parser. The parse itself
(in a parse step) and the evaluation (if an evaluator is defined) are always
printed. Any combination of the following values may be specified:

  'timing': 
  For each step in the execution sequence, this outputs the start time, 
  end time and total time of the step.

  'obj_count': 
  This prints how many objects (utterances) were processed (if a filter 
  is defined, only utterances matched by the filter are counted).

  'extra_parse': 
  This prints extra information about the parse structure. In addition
  to the bracketing (which is the standard format) this also prints
  the common cover links on which the brackets are based, as follows:

  (
    (
      (there (0) >>1 )
      (is (1) >2 >4 >7
        (no (2) >3  2< asbestos (3) )
        (in (4) >5
          (our (5) >6  5< products (6) ))
        (now (7) ))))

  The numbers in parentheses are the position of each word in the sentence.
  The arrows represent the common cover links and the number appearing next
  to each arrow indicates the word which this arrow points to. A single
  arrow (>5) represents a link of depth 0 and a double arrow (>>1) represents
  a link of depth 1.

  'more_extra_parse': 
  This prints even more information about the parse structure:

  (
    (
      (there (0) >>1 )
      (0>> is (1) >2 >4 >7
        (1> no (2) >3 <3  2> 2< asbestos (3) )
        (1> in (4) >5
          (4> our (5) >6 <6  5> 5< products (6) ))
        (1> now (7) ))))

  In addition to the outbound links, this format also indicates the
  inbound links for each word (this is mainly useful with long sentences).

  'source_text':
  This prints out the source text of each utterance (as it appear in the
  input file) before printing the parse generated by the parser.

  'extra_eval':
  This prints the contribution of each utterance to the total evaluation score.

  'config':
  Prints the values of the global parameters used in generating the output. 

The default value of this parameter is empty (none of the above options).

TraceBits <number>:

The bits in <number> determine which debug information (tracing) should be
printed out. See include/Tracing.h for the possible bits. The default
value is zero (no tracing).

Example Execution Sequences and Configuration
=============================================

Assume that $WORKDIR is your working directory and that you have the
Penn Treebank at $WORKDIR/PennTB. The following execution sequence can be
used to reproduce the results reported in Seginer (2007a) for the WSJ10:

PennTB/combined/wsj/*/*.mrg wsj learn -o WSJLearn -s log -R 100 -p
PennTB/combined/wsj/*/*.mrg wsj parse -o WSJ10Parse -s prs -c 10 -R 100 -e PR 

The first line learns from the full Wall Street Journal corpus. The second
line then uses the lexicon created by the first step to parse all
sentences of length 10 or shorter. The parsing results are evaluated using
the precision and recall evaluator ('PR'). The lexicon (at the end of 
the learning step) is printed to $WORKDIR/WSJLearn.log.lexicon and 
the parses are printed to $WORKDIR/WSJ10Parse.prs.

Put these two lines into the file $WORKDIR/WSJ10.exec (or any other file 
name or location) and run 'cclparser WSJ10.exec'.

If you want some extra information to be printed, you can put the following
line into the file  $WORKDIR/Print.conf (or any other file name or location):

PrintingMode     timing obj_count extra_parse source_text

Now set $WORKDIR/Print.conf as the configuration file for the appropriate
step. If you want this configuration to hold both for the learning and the
parsing step, add the '-G' option to the first step:

PennTB/combined/wsj/*/*.mrg wsj learn -G Print.conf -o WSJLearn -s log -R 100 -p
PennTB/combined/wsj/*/*.mrg wsj parse -o WSJ10Parse -s prs -c 10 -R 100 -e PR 

If you want the extra printing to apply only to the parsing step,
you can add the '-G' option to the second step only:

PennTB/combined/wsj/*/*.mrg wsj learn -o WSJLearn -s log -R 100 -p
PennTB/combined/wsj/*/*.mrg wsj parse -G Print.conf -o WSJ10Parse -s prs -c 10 -R 100 -e PR 

Note: Even if the lines become very long, they may not be broken across several
lines.

Reproducing Reported Results
============================

The implementation of the parsing algorithm in this distribution is identical,
in principle, to that used in Seginer (2007a,2007b). However, as a result
of cleaning up and reorganization of the code, small differences appear
in the output. These differences are very small: not more than two dozen
brackets (and probably even fewer links) out of the tens of thousands
of brackets in the parses. These differences are probably due to rounding
effects of the floating point arithmetic and appear when parsing decisions
are marginal and depend on very small differences between weights.

A second difference between the default output of the parser and the results
of Seginer (2007a,2007b) is due to the fixing of a small bug in the handling
of terminating punctuation (full stop, exclamation mark, question mark)
which appear inside an utterance. In the original version of the parser,
these were discarded. In the present version of the parser, these are handled
like any other stopping punctuation symbol. Because terminating punctuation
seldom appears inside utterances, this does not change the results 
significantly. If you wish to go back to the original setting (where
terminating punctuation is ignored), you can set the 'DiscardTerminatingPunct'
parameter (as described above).

Assuming $WORKDIR is the working directory (where you run the parser)
and that you have the Penn Treebank CD at $WORKDIR/PennTB, the Negra
Corpus (in Penn Treebank format) as a single file $WORKDIR/NegraCorpus and
the Chinese Treebank (V5.0) at $WORKDIR/CTB, you can reproduce the
reported results by using the following lines in your execution sequence file
(you should run the parser separately for each corpus, to avoid the lexicons
of the different languages being mixed up):

# WSJ
PennTB/combined/wsj/*/*.mrg wsj learn -o WSJLearn -s log -R 100 -p
PennTB/combined/wsj/*/*.mrg wsj parse -o WSJ10Parse -s prs -c 10 -R 100 -e PR
PennTB/combined/wsj/*/*.mrg wsj parse -o WSJ20Parse -s prs -c 20 -R 100 -e PR
PennTB/combined/wsj/*/*.mrg wsj parse -o WSJ40Parse -s prs -c 40 -R 100 -e PR
PennTB/combined/wsj/*/*.mrg wsj parse -o WSJFullParse -s prs -R 100 -e PR

# Negra
NegraCorpus negra learn -o NegraLearn -s log -R 100 -p
NegraCorpus negra parse -o Negra10Parse -s prs -c 10 -R 100 -e PR
NegraCorpus negra parse -o Negra20Parse -s prs -c 20 -R 100 -e PR
NegraCorpus negra parse -o Negra40Parse -s prs -c 40 -R 100 -e PR
NegraCorpus negra parse -o NegraFullParse -s prs -R 100 -e PR

#CTB
data/bracketed/* ctb learn -o CTBLearn -s log -R 100 -p
data/bracketed/* ctb parse -o CTB10Parse -s prs -c 10 -R 100 -e PR
data/bracketed/* ctb parse -o CTB20Parse -s prs -c 20 -R 100 -e PR
data/bracketed/* ctb parse -o CTB40Parse -s prs -c 40 -R 100 -e PR
data/bracketed/* ctb parse -o CTBFullParse -s prs -R 100 -e PR

For each corpus, this defines a sequence of steps which begins with 
learning from the full corpus and continues with parsing different subsets
of each corpus (defined by an upper bound on the number of words in a 
sentence). The lines for each corpus should be put into a separate file and run
separately.

Contact
=======

If you have any questions, remarks, bug reports, bug fixes or extensions,
I will be happy to hear from you.

Yoav Seginer
cclparser@gmail.com 

Bibliography
============

Klein, D. (2005). The Unsupervised Learning of Natural Language Structure.
Ph. D. thesis, Stanford University.

Klein, D. and C. D. Manning (2002). A Generative Constituent-Context Model
for Improved Grammar Induction. In Proceedings of the 40th Annual Meeting
of the Association for Computational Linguistics, pp. 128-135.

Klein, D. and C. D. Manning (2004). Corpus-based Induction of Syntactic 
Structure: Models of Dependency and Constituency. In Proceedings of the 42nd
Annual Meeting of the Association for Computational Linguistics, pp. 478-485.

Seginer, Y.(2007a). Fast Unsupervised Incremental Parsing. In proceedings 
of the 45th Annual Meeting of the Association of Computational Linguistics, 
pp. 384-391. (http://www.aclweb.org/anthology-new/P/P07/P07-1049.pdf)

Seginer, Y.(2007b). Learning Syntactic Structure. Ph.D. Thesis,
Universiteit van Amsterdam. (http://dare.uva.nl/document/52554) 
