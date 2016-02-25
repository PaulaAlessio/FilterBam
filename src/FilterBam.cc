#include "BamMinimalisticReader.h"
#include "BufferOutput.h"
#include "CreateTreeFromFilter.h"
#include <cstdio>
#include <string.h>
#include <getopt.h>
#include <stdexcept>

void printHelpDialog()
{
   const char dialog[] =
      "Usage: FilterBam -i [INPUT_FILE.bam] -f [FILTER_FILE] -p [PREFIX] [OPTIONS]\n"
      "Extracts from INPUT_FILE.bam the reads with QNAMEs contained in [FILTER_FILE].\n"
      "[FILTER_FILE] is either a tsv file with QNAMEs in the first field or a *bam file.\n"
      "A prefix with the common characters of QNAME should be provided.\n"
      "Output is printed to stdout by default, unless an outputfile is specified.\n"
      " -i Input file. Required option\n"
      " -f Filter file. Either a *bam file or a tsv file with QNAME as first field. Required option.\n"
      " -p Prefix. String containing the common substring in QNAME. \n"
      "    e.g: \"HWI-ST933:227:C8C7YACXX:1:\". \n"
      "    Required option, since the matching allows only characters [0-9],:\n"
      " -o Output file. Optional. Written to stdout otherwise. If written to stdout,\n"
      "    the output can be piped to samtools to obtain a *bam file. \n"
      " -s Secondary alignments. Optional. If passed (with no arguments), then\n"
      "    secondary alignments will be considered.\n";
   std::cerr << dialog << std::endl;
   exit(EXIT_SUCCESS);
}


int main(int argc, char *argv[]){
    if (argc==1){
        printHelpDialog();
    }
    std::string inputfile, filter, prefix;
    bool secondary = false;
    std::string outputfile = "\0";
    char tmp;
    while ( (tmp=getopt(argc,argv,"hi:f:p:so:"))!=-1){
      switch(tmp){
         case 'h': //show the HelpDialog
            printHelpDialog();
            break;
         case 'i':
           inputfile=optarg;
           std::cerr << "Input File: "<< optarg << std::endl;
           if ((inputfile.substr(inputfile.size() - 4) != ".bam")
                    && (inputfile.substr(inputfile.size() - 4)
                    != ".sam"))
           std::cerr << "WARNING: Input file should be a *sam, or a *bam. Run at your own risk." <<std::endl;
           break; 
         case 'f':
           filter = optarg;
           std::cerr << "Filter File: " << filter << std::endl;
           break;
         case 'p':
           prefix = optarg;
           std::cerr << "Common Prefix: " << prefix << std::endl;
           break;
         case 'o':
           outputfile = optarg;
           std::cerr << "Output File: " << outputfile << std::endl;
           break;
         case 's':
           secondary = true;
           break;
         default:
            printHelpDialog();
            break;
      }         
            
   }

   std::cerr << "Outputting secondary alignments :" << secondary << std::endl;
   //std::string inputfile="/nfs/project/Projects/Feist16/tophat_results/topHatOut_00_Myclow-ctrl_A_1/accepted_hits.bam";
   //std::string filter="/nfs/project/Projects/Feist16/over_rep_sequences/rRNA_CRUnit/output_bbt/tests/00_Myclow-ctrl_A_1_bbt_gugu_rRNA_CRUnit_kmer50.sam";
   tree root = CreateTree();
   //std::string prefix="HWI-ST933:227:C8C7YACXX:1:";
   int pos_start = prefix.length(); 
   //Read Filter and construct tree
   CreateTreeFromFilter(root, filter, prefix);
   std::cerr << "filter read and tree constructed\n"; 

   // Open Output file if needed
   FILE *fout;
   if(!outputfile.empty()){
      fout = fopen(outputfile.c_str(), "w");
   } else {
      fout = stdout; 
   }
   if (fout==NULL){
      std::cerr << "Output file " << outputfile 
                << "could not be open." << std::endl;
      throw std::runtime_error(std::strerror(errno));
   }
   BamMinimalisticReader  sequence(inputfile.c_str(), 
                           BamMinimalisticReader::NO_FOLD_CASE, secondary);
   bool good = true;
   int line_in = 0;
   int line_out = 0;
   int branch[T_DEPTH];
   BamSamRecord rec;
   while (good){
      good = sequence >> rec;
      if (rec.id != ""){
         line_in++;
         if (line_in%1000000==0) std::cerr << line_in << " lines have been checked\n";
         for(size_t i=pos_start;i<rec.id.length();i++){
               branch[i-pos_start] = LT[rec.id[i]];
         }
         branch[rec.id.length()-pos_start] = 11;
         if(!CheckPath(root,branch,rec.id.length()-pos_start+1)) {
             line_out++;
             BufferOutput(fout, rec.full_line.c_str(), rec.full_line. length());
         }
     }
     else{
         line_out++;
         fprintf(fout, "%s\n", rec.full_line.c_str());    
     }
   }
   BufferOutput(fout, "\0", 0);
   std::cerr << "Input file has: " << line_in << " analyzed  lines." << std::endl;
   std::cerr << "Output has: " << line_out << " lines." << std::endl;
   assert(sequence.eof());
   fclose(fout);
   std::cerr << "Finished sucessfully!" << std::endl << std::endl;
   return 0;
}
