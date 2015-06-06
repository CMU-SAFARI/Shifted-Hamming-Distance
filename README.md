# Shifted Hamming Distance

Shifted Hamming Distance (SHD) is an edit-distance based filter that can quickly check whether the
minimum number of edits (including insertions, deletions and substitutions) between two strings is
smaller than a user defined threshold **T** (the number of allowed edits between the two strings).

Testing if two stings differs by a small amount is a prevalent function that is used in many
applications. One of its biggest usage, perhaps, is in DNA or protein mapping, where a short
DNA or protein string is compared against an enormous database, in order to find similar matches. In
such applications, a query string is usually compared against multiple candidate strings in the
database. Only candidates that are similar to the query are considered **matches** and recorded.

SHD expands the basic Hamming distance computation, which only detects substitutions, into a
full-fledged edit-distance filter, which counts not only substitutions but **insertions and
deletions** as well.

SHD by itself, however, is *not* an edit-distance calculator. SHD is a filter that detects and
filters some of the string pairs that have edit-distances that are greater than **T**, but it **does
not** validate the string pairs that pass the filter regarding if they have edit-distances smaller
than T. In another word, it filters out some incorrect matches while passing some. In a DNA mapper
or protein mapper, SHD should be used in combination with an edit-distance calculator, where SHD
reduces the number of incorrect matches that reach to the more rigorous edit-distance calculator (or
filter).

As a filter, SHD achieves high accuracy with high speed. However, the accuracy drops as the
edit-distance threshold **T** increases. We recommend not setting the edit-distance threshold **T**
greater than 5% of the query string length.

Currently, SHD is implemented using Intel SSE, which uses 128-bit registers. Therefore, we recommend
using SHD for query strings that are not longer than 128 letters. SHD can also be easily expanded to
support longer query strings using future instruction sets such as AVX2 or AVX512. At the moment of
developing this code, however, we do not have a system that supports such ISAs. Hence SHD currently
supports a maximum of 128 letters.

**Important note:** Currently an AVX2 version of SHD is under development. We aim a release date by
October 1st. We will start developing an AVX512 version of the program once Intel releases the 2nd
generation Xeon Phi cards, the Knights Landing.

**Protein sequencing users:** Although currently SHD only supports mapping DNA, it can be easily
expanded to support protein matching. Please directly contact the author if you need such
functionality. Caveat: with SSE, SHD only supports matching strings that are shorter than 128. We
understand this length is typically too short for comparing proteins. We hope this problem will be
solved once wider SIMD ISAs come out.

**GPU support**: We do have a premature CUDA version of the program. The development of the GPU
version is temporarily on pause so we can focus on pushing forward other ideas. If the demand of a
GPU version is high, we will resume the development and push out the code.

The algorithm of SHD is described at: [ H. Xin et al., **Shifted Hamming Distance: A Fast and
Accurate SIMD-Friendly Filter to Accelerate Alignment Verification in Read Mapping**, Bioinformatics
](http://bioinformatics.oxfordjournals.org/content/early/2015/01/10/bioinformatics.btu856.abstract)

# Getting Started

To build SHD, simply do:

	$ make

# Running a test

To run a test using SHD, simply do:

	$ ./countPassFilter <edit-distance threshold T>

Then type in string pairs that are sent to SHD. Strings are delineated by a *new line (\n)*
character.

Input "end_of_file" to terminate execution.

String pairs that pass SHD is printed out to *stderr*. After the execution is finished, a summary
of the run is printed out to *stdout*, including the total number of string pairs processed and the
total number of string pairs pass SHD.

You can also run SHD by redirecting input from a file. An example input is provided in the
repository. To run, simply do:

	$ ./countPassFilter <edit-distance threshold T> < example_input

# Further Extensions 
	
Notice that this SHD build is specific to DNA strings (which can either be A, C, T or G). To modify
SHD for other type of strings, please change the masks in mask.h and mask.c files.

# Contributors

- Hongyi Xin (Carnegie Mellon University)
