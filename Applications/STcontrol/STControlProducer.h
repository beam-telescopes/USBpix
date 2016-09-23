#ifndef STCONTROLPRODUCER_H
#define STCONTROLPRODUCER_H


#include "eudaq/Producer.hh"
#include "eudaq/Configuration.hh"
#include "eudaq/CommandReceiver.hh"

#include "STCdefines.h"
#include "STEUDAQDataSender.h"

#include <QString>

#include <memory>

namespace PixLib {
class PixController; 
}
class STControlEngine;

class STControlProducer : public eudaq::CommandReceiver {


public:
	STControlProducer(STControlEngine& engine, std::string const& prdname, std::string const& runctrl);
	STControlProducer() = delete;
	~STControlProducer() = default;
	// EUDAQ methods, must be re-implemented here
	void OnInitialise(const eudaq::Configuration &param);
	void OnConfigure (const eudaq::Configuration & config);
	void OnStartRun (unsigned param);
	void OnStopRun ();
	void OnTerminate ();
	void OnUnrecognised(const std::string & cmd, const std::string & param);
	void OnPrepareRun(unsigned runnumber);
	void OnStatus();

private:
	bool InitControllers(extScanOptions& ScanOptions);
	QString CreateMultiBoardConfig(extScanOptions& ScanOptions);
	std::vector<PixLib::PixController*> getPixControllers();
	STControlEngine& m_STControlEngine;
	std::string m_rcAddress;
	extScanOptions scan_options;
	
	std::vector<std::unique_ptr<STEUDAQDataSender>> m_dataSenders;

};
#endif
