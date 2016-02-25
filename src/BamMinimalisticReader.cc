#include "BamMinimalisticReader.h"
#include "Options.h"
#include "IOUtil.h"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;


BamMinimalisticReader::BamMinimalisticReader(const char* path, 
      int flags, bool secondary,int len)
	: AbstractReader(path,flags,secondary,len){}


/** Read a single record. */
Sequence BamMinimalisticReader::read(string& id, string& comment,
		char& anchor, string& full_line)
{
next_record:
	id.clear();
	comment.clear();
	anchor = 0;
	full_line.clear();

	signed char recordType = peek();
	Sequence s;

	if (eof() || recordType == EOF || ftell(m_in) >= m_end) {
		string header;
		getline(header);
		return s;
	} else if ( recordType == '@') {
		// Read the header.
		getline(full_line);
                id="";
                comment="";
                return "";
	} else {
		string line;
		vector<string> fields;
		fields.reserve(22);
		getline(full_line);
		istringstream in(full_line);
		string field;
		while (std::getline(in, field, '\t'))
			fields.push_back(field);
		unsigned flags = strtoul(fields[1].c_str(), NULL, 0);
		if (!secondary && (flags & 0x100) ) // FSECONDARY
			goto next_record;
		id = fields[0];
		s = fields[9];
	}

	return s;
}
