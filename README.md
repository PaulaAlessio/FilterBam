# FilterBam user manual 


This software filters out a set of reads characterized by 
their QNAME from a  `*.bam` or a  `*.sam` file, and produces
 a `*sam` file as an output. 


1. Compiling and installing FilterBam. 
-------

##### Dependencies: 

 * g++ (tested on versions 4.9.2 and 4.7.2).
 * samtools (tested on version 1.1).

##### Compilation 
 
  Run 
  ```
   make
   ```
   If all works correctly, an executable should land under the folder `bin`.
   If not, the `Makefile` is rather straight forward, modify it to meet your
   requirements. 

2. Running the program 
-------
   Usage:  

   ```   FilterBam -i [INPUT_FILE.bam] -f [FILTER_FILE] -p [PREFIX] [OPTIONS]```

 ```
   Extracts from INPUT_FILE.bam the reads with QNAMEs contained in [FILTER_FILE].  
   [FILTER_FILE] is either a tsv file with QNAMEs in the first field or a  *.bam file.  
   A prefix with the common characters of QNAME should be provided.
   Output is printed to stdout by default, unless an outputfile is specified.
    -h Print help dialog.
    -i Input file. Required option
    -f Filter file. Either a *bam file or a tsv file with QNAME as first field. Required option.
    -p Prefix. String containing the common substring in QNAME. 
       e.g: "HWI-EAS746_0004". 
       Required option, since the matching allows only characters [0-9],:
    -o Output file. Optional. Written to stdout otherwise. If written to stdout,
       the output can be piped to samtools to obtain a *.bam file. 
    -s Secondary alignments. Optional. If passed (with no arguments), then
       secondary alignments will be considered.
```

  **Warning:** This software is only valid if QNAME has the form: 
   `Prefix+string([0:9],:)`    

   If QNAME has a different structure, the program has to be modified. 


3. Output
-------

  The output is written to `stdout` by default unless an output file name 
is passed as an argument. The output has the structure of a `*sam` file. 
If written to `stdout`, it can be piped to be used directly by `samtools`
and produce a `*bam` output, namely: 

 ```  FilterBam -i input.bam -f filter.fai -p HWI-EAS746_0004  | samtools -bS -o output.bam  - ```

4. Tests
---------
   Under folder `test_files`, one can find files and a bash script for testing.
