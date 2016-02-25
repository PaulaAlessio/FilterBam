#include "AbstractReader.h"
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


/** Output an error message. */
ostream& AbstractReader::die()
{
	return cerr << m_path << ':' << m_line << ": error: ";
}

AbstractReader::AbstractReader(const char* path, int flags, bool secondary,int len)
	: m_path(path), secondary(secondary), m_blen(32768), 
        m_bstart(0), m_bend(0), m_fail(false), m_flags(flags), 
        m_line(0), m_unchaste(0),
	m_end(numeric_limits<streamsize>::max()),
	m_maxLength(len), forceBreak(false)
{
	m_buff = new char[m_blen];
	if (strcmp(path, "-") != 0)
		m_in = fopen(m_path, "r");
	else
		m_in = stdin;
	if (m_in == NULL)
		perror(m_path);
	if (fpeek(m_in) == EOF)
		cerr << m_path << ':' << m_line << ": warning: "
			"file is empty\n";
	fill_buff();
}

/** Return whether this read passed the chastity filter. */
bool AbstractReader::isChaste(const string& s, const string& line)
{
	if (s == "1" || s == "Y") {
		return true;
	} else if (s == "0" || s == "N") {
		return false;
	} else {
		die() << "chastity filter should be one of 0, 1, N or Y\n"
			"and saw `" << s << "' near\n" << line << endl;
		exit(EXIT_FAILURE);
	}
}

/** Check that the seqeuence and quality agree in length. */
void AbstractReader::checkSeqQual(const string& s, const string& q)
{
	if (s.length() != q.length()) {
		die() << "sequence and quality must be the same length near\n"
			<< s << '\n' << q << endl;
		exit(EXIT_FAILURE);
	}
}

