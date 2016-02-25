#include "GenericReader.h"
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

namespace opt {
	/** Discard reads that failed the chastity filter. */
	int chastityFilter = 1;

	/** Trim masked (lower case) characters from the ends of
	 * sequences.
	 */
	int trimMasked = 1;

	/** minimum quality threshold */
	int qualityThreshold;

	/** quality offset, usually 33 or 64 */
	int qualityOffset;

	/** The size of length cutoff threshold */
	size_t minLength = 0;
}

/** Return whether the read seq is in colour space. */
static bool isColourSpace(const std::string& seq)
{
	assert(!seq.empty());
	size_t i = seq.find_first_of("ACGTacgt0123", 1);
	return i != std::string::npos && isdigit(seq[i]);
}


GenericReader::GenericReader(const char* path, int flags, bool secondary,int len)
	: AbstractReader(path,flags,secondary,len){}


/** Read a single record. */
Sequence GenericReader::read(string& id, string& comment,
		char& anchor, string& q)
// q: quality if fastq
//    fullline if bam/sam
{
next_record:
	id.clear();
	comment.clear();
	anchor = 0;
	q.clear();

	// Discard comments.
	while (peek() == '#')
		ignoreLines(1);

	signed char recordType = peek();
	Sequence s;

	unsigned qualityOffset = 0;
	if (eof() || recordType == EOF || ftell(m_in) >= m_end) {
		string header;
		getline(header);

		return s;
	} else if (recordType == '>' || recordType == '@') {
		// Read the header.
		string header;
		getline(header);
		istringstream headerStream(header);
		headerStream >> recordType >> id >> ws;
		std::getline(headerStream, comment);

		// Read SAM headers as a whole
		if (id.length() == 2 && isupper(id[0]) && isupper(id[1])
				&& comment.length() > 2 && comment[2] == ':')
                        q=header;
                        id="";
                        comment="";
                        return "";
			//goto next_record;

		// Ignore @CO (One-line text comment) header in particular,
		// https://samtools.github.io/hts-specs/SAMv1.pdf
		if (id.length() == 2 && id[0] == 'C' && id[1] == 'O')
			goto next_record;

		// Casava FASTQ format
		if (comment.size() > 3
				&& comment[1] == ':' && comment[3] == ':') {
			// read, chastity, flags, index: 1:Y:0:AAAAAA
			if (opt::chastityFilter && comment[2] == 'Y') {
				m_unchaste++;
				if (recordType == '@') {
					ignoreLines(3);
				} else {
					while (peek() != '>' && peek() != '#'
							&& ignoreLines(1))
						;
				}
				goto next_record;
			}
			if (id.size() > 2 && id.rbegin()[1] != '/') {
				// Add the read number to the ID.
				id += '/';
				id += comment[0];
			}
		}

		getline(s);
		if (recordType == '>') {
			// Read a multi-line FASTA record.
			string line;
			while (peek() != '>' && peek() != '#'
					&& getline(line))
				s += line;
			if (eof())
				clear();
		}

		if (recordType == '@') {
			char c = peek();
			if (c != '+') {
				die() << s << '\n' << header << '\n';
				string line;
				getline(line);
				die() << "expected `+' and saw ";
				if (eof())
					cerr << "end-of-file\n";
				else
					cerr << "`" << c << "' near\n"
					<< c << line << "\n";
				exit(EXIT_FAILURE);
			}
			ignoreLines(1);
			getline(q);
		} else
			q.clear();

		if (s.empty()) {
			die() << "sequence with ID `" << id << "' is empty\n";
			exit(EXIT_FAILURE);
		}

		if (s.length() < opt::minLength) {
			goto next_record;
		}

		bool colourSpace = isColourSpace(s);
		if (colourSpace && !isdigit(s[0])) {
			// The first character is the primer base. The second
			// character is the dibase read of the primer and the
			// first base of the sample, which is not part of the
			// assembly.
			assert(s.length() > 2);
			anchor = colourToNucleotideSpace(s[0], s[1]);
			s.erase(0, 2);
			q.erase(0, 1);
		}

		if (!q.empty())
			checkSeqQual(s, q);

		if (opt::trimMasked && !colourSpace) {
			// Removed masked (lower case) sequence at the beginning
			// and end of the read.
			size_t trimFront = 0;
			while (trimFront <= s.length() && islower(s[trimFront]))
				trimFront++;
			size_t trimBack = s.length();
			while (trimBack > 0 && islower(s[trimBack - 1]))
				trimBack--;
			s.erase(trimBack);
			s.erase(0, trimFront);
			if (!q.empty()) {
				q.erase(trimBack);
				q.erase(0, trimFront);
			}
		}
		if (flagFoldCase())
			transform(s.begin(), s.end(), s.begin(), ::toupper);

		qualityOffset = 33;
	} else {
		string line;
		vector<string> fields;
		fields.reserve(22);
		getline(line);
		istringstream in(line);
		string field;
		while (std::getline(in, field, '\t'))
			fields.push_back(field);

		if (fields.size() >= 11
				&& (fields[9].length() == fields[10].length()
					|| fields[10] == "*")) {
			// SAM
			unsigned flags = strtoul(fields[1].c_str(), NULL, 0);
			if (!secondary && (flags & 0x100) ) // FSECONDARY
				goto next_record;
		//	if (opt::chastityFilter && (flags & 0x200)) { // FQCFAIL
		//		m_unchaste++;
		//		goto next_record;
		//	}
			id = fields[0];
			switch (flags & 0xc1) { // FPAIRED|FREAD1|FREAD2
			  case 0: case 1: // FPAIRED
				comment = "0:";
				break;
			  case 0x41: // FPAIRED|FREAD1
				id += "/1";
				comment = "1:";
				break;
			  case 0x81: // FPAIRED|FREAD2
				id += "/2";
				comment = "2:";
				break;
			  default:
				die() << "invalid flags: `" << id << "' near"
					<< line << endl;
				exit(EXIT_FAILURE);
			}
			comment += flags & 0x200 ? "Y:0:" : "N:0:"; // FQCFAIL

			s = fields[9];
			q = fields[10];
			if (s == "*")
				s.clear();
			if (q == "*")
				q.clear();
			if (flags & 0x10) { // FREVERSE
				s = reverseComplement(s);
				reverse(q.begin(), q.end());
			}
			qualityOffset = 33;
			if (!q.empty())
				checkSeqQual(s, q);
                        q=line;
		} else if (fields.size() == 11 || fields.size() == 22) {
			// qseq or export
			if (opt::chastityFilter
					&& !isChaste(fields.back(), line)) {
				m_unchaste++;
				goto next_record;
			}

			ostringstream o;
			o << fields[0];
			for (int i = 1; i < 6; i++)
				if (!fields[i].empty())
					o << ':' << fields[i];
			if (!fields[6].empty() && fields[6] != "0")
				o << '#' << fields[6];
			// The reverse read is typically the second read, but is
			// the third read of an indexed run.
			o << '/' << (fields[7] == "3" ? "2" : fields[7]);
			id = o.str();
			comment = fields[7];
			comment += isChaste(fields.back(), line)
				? ":N:0:" : ":Y:0:";
			s = fields[8];
			q = fields[9];
			qualityOffset = 64;
			checkSeqQual(s, q);
		} else {
			die() << "Expected either `>' or `@' or 11 fields\n"
					"and saw `" << recordType << "' and "
					<< fields.size() << " fields near\n"
					<< line << endl;
			exit(EXIT_FAILURE);
		}
	}

	if (opt::qualityOffset > 0)
		qualityOffset = opt::qualityOffset;

	// Trim from the 3' end to the maximum length. Then, trim based on
	// quality.
	if (m_maxLength > 0) {
		s.erase(m_maxLength);
		q.erase(m_maxLength);
	}

	if (opt::qualityThreshold > 0 && !q.empty()) {
		assert(s.length() == q.length());
		static const char ASCII[] =
			" !\"#$%&'()*+,-./0123456789:;<=>?"
			"@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
			"`abcdefghijklmnopqrstuvwxyz{|}~";
		assert(qualityOffset > (unsigned)ASCII[0]);
		const char* goodQual = ASCII + (qualityOffset - ASCII[0])
			+ opt::qualityThreshold;

		size_t trimFront = q.find_first_of(goodQual);
		size_t trimBack = q.find_last_of(goodQual) + 1;
		if (trimFront >= trimBack) {
			// The entire read is poor quality.
			s.erase(1);
			q.erase(1);
		} else if (trimFront > 0 || trimBack < q.length()) {
			s.erase(trimBack);
			s.erase(0, trimFront);
			q.erase(trimBack);
			q.erase(0, trimFront);
		}
	}

	assert(qualityOffset >= 33);
	if (flagConvertQual() && qualityOffset != 33) {
		// Convert to standard quality (ASCII 33).
		for (string::iterator it = q.begin(); it != q.end(); ++it) {
			int x = *it - qualityOffset;
			if (x < -5 || x > 41) {
				die() << "quality " << x
					<< " is out of range -5 <= q <= 41 near\n"
					<< q << '\n'
					<< string(it - q.begin(), ' ') << "^\n";
				exit(EXIT_FAILURE);
			}
			*it = 33 + max(0, x);
		}
	}

	return s;
}
