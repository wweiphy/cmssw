#ifndef IntegrityClient_H
#define IntegrityClient_H

#include "DQWorkerClient.h"
#include "DQMServices/Core/interface/DQMEDHarvester.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "CondFormats/EcalObjects/interface/EcalChannelStatus.h"
#include "CondFormats/DataRecord/interface/EcalChannelStatusRcd.h"
//#include "FWCore/Framework/interface/ConsumesCollector.h"

namespace ecaldqm {
  class IntegrityClient :  public DQWorkerClient {
  public:
    IntegrityClient();
    ~IntegrityClient() override {}

    void producePlots(ProcessType) override;
    void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

  private:
    void setParams(edm::ParameterSet const&) override; 
    edm::ESGetToken<EcalChannelStatus, EcalChannelStatusRcd> chStatusToken;
    const EcalChannelStatus* chStatus;
    void setTokens(edm::ConsumesCollector&) ;

    float errFractionThreshold_;
  };
}  // namespace ecaldqm

#endif
