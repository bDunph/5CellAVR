#ifndef CSOUNDSESSION_H
#define CSOUNDSESSION_H

#include <string>

#include "csound.hpp"
#include "csPerfThread.hpp"

using namespace std;

class CsoundSession : public Csound{

	string m_csd;
	CsoundPerformanceThread *m_pt;

public:

	CsoundSession(string const &csdFileName) : Csound() {
	m_pt = NULL;
	m_csd = "";
	if(!csdFileName.empty()){
		m_csd = csdFileName;
		startThread();
		}
	};
	void startThread();
	void resetSession(string const &csdFileName);
	void stopPerformance();
	void audioLoop();

};

#endif
