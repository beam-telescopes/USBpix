#ifndef STCONTROLPRODUCER_H
#define STCONTROLPRODUCER_H


#include <eudaq/Producer.hh>
#include <eudaq/Configuration.hh>
#include "STCdefines.h"
#include <QString>

namespace PixLib {
class PixController; 
}
class STControlEngine;

class STControlProducer : public eudaq::Producer {


public:
	STControlProducer(STControlEngine& engine, std::string const& prdname, std::string const& runctrl);
	STControlProducer() = delete;
	~STControlProducer() = default;
	// EUDAQ methods, must be re-implemented here
	void OnConfigure (const eudaq::Configuration & config);
	void OnStartRun (unsigned param);
	void OnStopRun ();
	void OnTerminate ();
	void OnUnrecognised(const std::string & cmd, const std::string & param);
	void OnPrepareRun(unsigned runnumber);
	void OnStatus();

private:
	bool InitControllers(extScanOptions ScanOptions);
	QString CreateMultiBoardConfig(extScanOptions& ScanOptions);
	std::vector<PixLib::PixController*> getPixControllers();
	STControlEngine& m_STControlEngine;
	std::string m_rcAddress;
	extScanOptions scan_options;
};
#endif