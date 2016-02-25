#ifndef BAMMINIMALISTICREADER_H
#define BAMMINIMALISTICREADER_H 1

#include "Sequence.h"
#include "StringUtil.h" // for chomp
#include <cassert>
#include <cstdlib> // for exit
#include <fstream>
#include <istream>
#include <iostream> // for debugging, remove before committing
#include <limits> // for numeric_limits
#include <ostream>
#include <algorithm>


#include <cstdio>
#include <cstring>

#include "Uncompress.h"
#include "AbstractReader.h"



/** Read a FASTA, FASTQ, export, qseq or SAM file. */
class BamMinimalisticReader: public AbstractReader  {
public:

	BamMinimalisticReader(const char* path, int flags, 
                              bool secondary, int len = 0);

	~BamMinimalisticReader()
	{
		if (!eof() && !forceBreak) {
			std::string line;
			getline(line);
			die() << "expected end-of-file near\n" << line << '\n';
			exit(EXIT_FAILURE);
		}
		fclose(m_in);
		delete m_buff;
	}

	Sequence read(std::string& id, std::string& comment, char& anchor,
			std::string& qual);//escribir de 0


};




#endif //BAMMINIMALISTICREADER_H
