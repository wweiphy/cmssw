#include "DQMServices/Core/interface/DQMEDHarvester.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "Geometry/TrackerGeometryBuilder/interface/PixelTopologyMap.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"

#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "../interface/SiPixelLorentzAngleCalibrationStruct.h"
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"
#include "CondFormats/SiPixelObjects/interface/SiPixelLorentzAngle.h"
#include "CondFormats/DataRecord/interface/SiPixelLorentzAngleRcd.h"

#include <fmt/format.h>
#include <fmt/printf.h>

//------------------------------------------------------------------------------

class SiPixelLorentzAnglePCLHarvester : public DQMEDHarvester {
public:
  SiPixelLorentzAnglePCLHarvester(const edm::ParameterSet&);
  void beginRun(const edm::Run&, const edm::EventSetup&) override;

  static void fillDescriptions(edm::ConfigurationDescriptions&);

private:
  void dqmEndJob(DQMStore::IBooker&, DQMStore::IGetter&) override;
  void findMean(MonitorElement* h_drift_depth_adc_slice_, int i, int i_ring);

  // es tokens
  edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> geomEsToken_;
  edm::ESGetToken<TrackerTopology, TrackerTopologyRcd> topoEsToken_;
  edm::ESGetToken<SiPixelLorentzAngle, SiPixelLorentzAngleRcd> siPixelLAEsToken_;

  const std::string dqmDir_;
  const double fitProbCut_;
  const std::string recordName_;

  SiPixelLorentzAngleCalibrationHistograms hists;
  const SiPixelLorentzAngle* currentLorentzAngle;
};

//------------------------------------------------------------------------------

SiPixelLorentzAnglePCLHarvester::SiPixelLorentzAnglePCLHarvester(const edm::ParameterSet& iConfig)
    : geomEsToken_(esConsumes<edm::Transition::BeginRun>()),
      topoEsToken_(esConsumes<edm::Transition::BeginRun>()),
      siPixelLAEsToken_(esConsumes<edm::Transition::BeginRun>()),
      dqmDir_(iConfig.getParameter<std::string>("dqmDir")),
      fitProbCut_(iConfig.getParameter<double>("fitProbCut")),
      recordName_(iConfig.getParameter<std::string>("record")) {
  // first ensure DB output service is available
  edm::Service<cond::service::PoolDBOutputService> poolDbService;
  if (!poolDbService.isAvailable())
    throw cms::Exception("SiPixelLorentzAnglePCLHarvester") << "PoolDBService required";
}

//------------------------------------------------------------------------------

void SiPixelLorentzAnglePCLHarvester::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
  // geometry
  const TrackerGeometry* geom = &iSetup.getData(geomEsToken_);
  const TrackerTopology* tTopo = &iSetup.getData(topoEsToken_);

  currentLorentzAngle = &iSetup.getData(siPixelLAEsToken_);

  PixelTopologyMap map = PixelTopologyMap(geom, tTopo);
  hists.nlay = geom->numberOfLayers(PixelSubdetector::PixelBarrel);

  for (int i = 0; i < hists.nlay; i++) {
    hists.nModules_[i] = map.getPXBModules(i + 1);
  }

  std::vector<uint32_t> treatedIndices;

  for (auto det : geom->detsPXB()) {
    const PixelGeomDetUnit* pixelDet = dynamic_cast<const PixelGeomDetUnit*>(det);
    const auto& layer = tTopo->pxbLayer(pixelDet->geographicalId());
    const auto& module = tTopo->pxbModule(pixelDet->geographicalId());
    int i_index = module + (layer - 1) * hists.nModules_[layer - 1];

    uint32_t rawId = pixelDet->geographicalId().rawId();

    if (std::find(treatedIndices.begin(), treatedIndices.end(), i_index) != treatedIndices.end()) {
      hists.detIdsList.at(i_index).push_back(rawId);
    } else {
      hists.detIdsList.insert(std::pair<uint32_t, std::vector<uint32_t>>(i_index, {rawId}));
      treatedIndices.push_back(i_index);
    }
  }
}

//------------------------------------------------------------------------------

void SiPixelLorentzAnglePCLHarvester::dqmEndJob(DQMStore::IBooker& iBooker, DQMStore::IGetter& iGetter) {
  // go in the right directory
  iGetter.cd();
  iGetter.setCurrentFolder(dqmDir_);

  /*
  const auto listOfHistos = iGetter.getMEs();
  for(const auto& hname : listOfHistos){
    const auto& histo = iGetter.get(dqmDir_+"/"+hname);
    std::cout << hname << " name: " << histo->getName() << std::endl;
  }
  */

  for (int i_layer = 1; i_layer <= hists.nlay; i_layer++) {
    for (int i_module = 1; i_module <= hists.nModules_[i_layer - 1]; i_module++) {
      int i_index = i_module + (i_layer - 1) * hists.nModules_[i_layer - 1];

      hists.h_drift_depth_[i_index] =
          iGetter.get(fmt::format("{}/h_drift_depth_layer{}_module{}", dqmDir_, i_layer, i_module));

      hists.h_drift_depth_adc_[i_index] =
          iGetter.get(fmt::format("{}/h_drift_depth_adc_layer{}_module{}", dqmDir_, i_layer, i_module));

      hists.h_drift_depth_adc2_[i_index] =
          iGetter.get(fmt::format("{}/h_drift_depth_adc2_layer{}_module{}", dqmDir_, i_layer, i_module));

      hists.h_drift_depth_noadc_[i_index] =
          iGetter.get(fmt::format("{}/h_drift_depth_noadc_layer{}_module{}", dqmDir_, i_layer, i_module));

      hists.h_mean_[i_index] = iGetter.get(fmt::format("{}/h_mean_layer{}_module{}", dqmDir_, i_layer, i_module));

      hists.h_drift_depth_[i_index]->divide(
          hists.h_drift_depth_adc_[i_index], hists.h_drift_depth_noadc_[i_index], 1., 1., "");
    }
  }

  /*
  for(const auto& [index,histo] : hists.h_drift_depth_adc_){
    std::cout << index << " => " << histo->getName();
  }
  */

  int hist_drift_ = hists.h_drift_depth_adc_[1]->getNbinsX();
  int hist_depth_ = hists.h_drift_depth_adc_[1]->getNbinsY();
  double min_drift_ = hists.h_drift_depth_adc_[1]->getAxisMin(1);
  double max_drift_ = hists.h_drift_depth_adc_[1]->getAxisMax(1);

  iBooker.setCurrentFolder("AlCaReco/SiPixelLorentzAngleHarvesting/");
  MonitorElement* h_drift_depth_adc_slice_ =
      iBooker.book1D("h_drift_depth_adc_slice", "slice of adc histogram", hist_drift_, min_drift_, max_drift_);

  TF1* f1 = new TF1("f1", "[0] + [1]*x", 50., 235.);
  f1->SetParName(0, "p0");
  f1->SetParName(1, "p1");
  f1->SetParameter(0, 0);
  f1->SetParameter(1, 0.4);
  std::cout << "module"
            << "\t"
            << "layer"
            << "\t"
            << "offset"
            << "\t"
            << "error"
            << "\t"
            << "slope"
            << "\t"
            << "error"
            << "\t"
               "rel.err"
            << "\t"
               "pull"
            << "\t"
            << "chi2"
            << "\t"
            << "prob" << std::endl;

  SiPixelLorentzAngle* LorentzAngle = new SiPixelLorentzAngle();

  //loop over modlues and layers to fit the lorentz angle
  for (int i_layer = 1; i_layer <= hists.nlay; i_layer++) {
    for (int i_module = 1; i_module <= hists.nModules_[i_layer - 1]; i_module++) {
      int i_index = i_module + (i_layer - 1) * hists.nModules_[i_layer - 1];
      //loop over bins in depth (z-local-coordinate) (in order to fit slices)
      for (int i = 1; i <= hist_depth_; i++) {
        //std::cout << i_layer << " " << i_module << " " << i << std::endl;

        findMean(h_drift_depth_adc_slice_, i, i_index);
      }  // end loop over bins in depth
      hists.h_mean_[i_index]->getTH1()->Fit(f1, "ERQ");
      double p0 = f1->GetParameter(0);
      double e0 = f1->GetParError(0);
      double p1 = f1->GetParameter(1);
      double e1 = f1->GetParError(1);
      double chi2 = f1->GetChisquare();
      double prob = f1->GetProb();
      std::cout << std::setprecision(4) << i_module << "\t" << i_layer << "\t" << p0 << "\t" << e0 << "\t" << p1
                << std::setprecision(3) << "\t" << e1 << "\t" << e1 / p1 * 100. << "\t" << (p1 - 0.424) / e1 << "\t"
                << chi2 << "\t" << prob << std::endl;

      const auto& detIdsToFill = hists.detIdsList.at(i_index);

      /*
      std::cout << i_index << " ";
      for (const auto& id : detIdsToFill) {
        std::cout << id << " ";
      }
      std::cout << std::endl;
      */

      float bPixLorentzAnglePerTesla_;
      // if the fit quality is OK
      if (prob > fitProbCut_) {
        for (const auto& id : detIdsToFill) {
          bPixLorentzAnglePerTesla_ = p1 / 3.8f;
          if (!LorentzAngle->putLorentzAngle(id, bPixLorentzAnglePerTesla_)) {
            edm::LogError("SiPixelLorentzAnglePCLHarvester")
                << "[SiPixelLorentzAnglePCLHarvester::dqmEndRun] detid already exists" << std::endl;
          }
        }
      } else {
        // just copy the values from the existing payload
        for (const auto& id : detIdsToFill) {
          bPixLorentzAnglePerTesla_ = currentLorentzAngle->getLorentzAngle(id);
          if (!LorentzAngle->putLorentzAngle(id, bPixLorentzAnglePerTesla_)) {
            edm::LogError("SiPixelLorentzAnglePCLHarvester")
                << "[SiPixelLorentzAnglePCLHarvester::dqmEndRun] detid already exists" << std::endl;
          }
        }
      }
    }
  }  // end loop over modules and layers

  // fill the rest of DetIds not filled above (for the moment FPix)
  const auto& currentLAMap = currentLorentzAngle->getLorentzAngles();
  const auto& newLAMap = LorentzAngle->getLorentzAngles();
  std::vector<unsigned int> currentLADets;
  std::vector<unsigned int> newLADets;

  std::transform(currentLAMap.begin(),
                 currentLAMap.end(),
                 std::back_inserter(currentLADets),
                 [](const std::map<unsigned int, float>::value_type& pair) { return pair.first; });

  std::transform(newLAMap.begin(),
                 newLAMap.end(),
                 std::back_inserter(newLADets),
                 [](const std::map<unsigned int, float>::value_type& pair) { return pair.first; });

  std::vector<unsigned int> notCommon;
  std::set_symmetric_difference(
      currentLADets.begin(), currentLADets.end(), newLADets.begin(), newLADets.end(), std::back_inserter(notCommon));

  for (const auto& id : notCommon) {
    float fPixLorentzAnglePerTesla_ = currentLorentzAngle->getLorentzAngle(id);
    if (!LorentzAngle->putLorentzAngle(id, fPixLorentzAnglePerTesla_)) {
      edm::LogError("SiPixelLorentzAnglePCLHarvester")
          << "[SiPixelLorentzAnglePCLHarvester::dqmEndRun] detid already exists" << std::endl;
    }
  }

  // book histogram of differences
  MonitorElement* h_diffLA = iBooker.book1D(
      "h_diffLA", "difference in #mu_{H}; #Delta #mu_{H}/#mu_{H} (old-new)/old [%];n. modules", 100, -10, 10);

  for (const auto& id : newLADets) {
    float deltaMuHoverMuH = (currentLorentzAngle->getLorentzAngle(id) - LorentzAngle->getLorentzAngle(id)) /
                            currentLorentzAngle->getLorentzAngle(id);
    h_diffLA->Fill(deltaMuHoverMuH);
  }

  // fill the DB object record
  edm::Service<cond::service::PoolDBOutputService> mydbservice;
  if (mydbservice.isAvailable()) {
    try {
      mydbservice->writeOne(LorentzAngle, mydbservice->currentTime(), recordName_);
    } catch (const cond::Exception& er) {
      edm::LogError("SiPixelLorentzAngleDB") << er.what() << std::endl;
    } catch (const std::exception& er) {
      edm::LogError("SiPixelLorentzAngleDB") << "caught std::exception " << er.what() << std::endl;
    } catch (...) {
      edm::LogError("SiPixelLorentzAngleDB") << "Funny error" << std::endl;
    }
  } else {
    edm::LogError("SiPixelLorentzAngleDB") << "Service is unavailable" << std::endl;
  }
}

void SiPixelLorentzAnglePCLHarvester::findMean(MonitorElement* h_drift_depth_adc_slice_, int i, int i_ring) {
  double nentries = 0;
  h_drift_depth_adc_slice_->Reset();
  int hist_drift_ = h_drift_depth_adc_slice_->getNbinsX();

  // determine sigma and sigma^2 of the adc counts and average adc counts
  //loop over bins in drift width
  for (int j = 1; j <= hist_drift_; j++) {
    if (hists.h_drift_depth_noadc_[i_ring]->getBinContent(j, i) >= 1) {
      /*
      std::cout << hists.h_drift_depth_adc_[i_ring]->getBinContent(j, i)  << std::endl;
      std::cout << hists.h_drift_depth_noadc_[i_ring]->getBinContent(j, i) << std::endl;
      std::cout << hists.h_drift_depth_adc2_[i_ring]->getBinContent(j, i) << std::endl;
      */

      double adc_error2 = (hists.h_drift_depth_adc2_[i_ring]->getBinContent(j, i) -
                           hists.h_drift_depth_adc_[i_ring]->getBinContent(j, i) *
                               hists.h_drift_depth_adc_[i_ring]->getBinContent(j, i) /
                               hists.h_drift_depth_noadc_[i_ring]->getBinContent(j, i)) /
                          hists.h_drift_depth_noadc_[i_ring]->getBinContent(j, i);

      hists.h_drift_depth_adc_[i_ring]->setBinError(j, i, sqrt(adc_error2));
      double error2 = adc_error2 / (hists.h_drift_depth_noadc_[i_ring]->getBinContent(j, i) - 1.);
      hists.h_drift_depth_[i_ring]->setBinError(j, i, sqrt(error2));
    } else {
      hists.h_drift_depth_[i_ring]->setBinError(j, i, 0);
      hists.h_drift_depth_adc_[i_ring]->setBinError(j, i, 0);
    }
    h_drift_depth_adc_slice_->setBinContent(j, hists.h_drift_depth_adc_[i_ring]->getBinContent(j, i));
    h_drift_depth_adc_slice_->setBinError(j, hists.h_drift_depth_adc_[i_ring]->getBinError(j, i));
    nentries += hists.h_drift_depth_noadc_[i_ring]->getBinContent(j, i);
  }  // end loop over bins in drift width

  double mean = h_drift_depth_adc_slice_->getMean(1);
  double error = 0;
  if (nentries != 0) {
    error = h_drift_depth_adc_slice_->getRMS(1) / std::sqrt(nentries);
  }

  hists.h_mean_[i_ring]->setBinContent(i, mean);
  hists.h_mean_[i_ring]->setBinError(i, error);
}

//------------------------------------------------------------------------------
void SiPixelLorentzAnglePCLHarvester::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<std::string>("dqmDir", "AlCaReco/SiPixelLorentzAngle");
  desc.add<double>("fitProbCut", 0.5);
  desc.add<std::string>("record", "SiPixelLorentzAngleRcd");
  descriptions.addWithDefaultLabel(desc);
}

DEFINE_FWK_MODULE(SiPixelLorentzAnglePCLHarvester);
