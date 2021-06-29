// -*- C++ -*-
//
// Package:    CalibTracker/SiPixelLorentzAnglePCLWorker
// Class:      SiPixelLorentzAnglePCLWorker
//
/**\class SiPixelLorentzAnglePCLWorker SiPixelLorentzAnglePCLWorker.cc CalibTracker/SiPixelLorentzAnglePCLWorker/plugins/SiPixelLorentzAnglePCLWorker.cc
 Description: [one line class summary]
 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  mmusich
//         Created:  Sat, 29 May 2021 14:46:19 GMT
//
//

#include <string>

// user include files
#include "DQMServices/Core/interface/DQMStore.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "DQMServices/Core/interface/DQMGlobalEDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "DataFormats/TrajectorySeed/interface/TrajectorySeedCollection.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/GeometryVector/interface/GlobalVector.h"
#include "DataFormats/GeometryVector/interface/LocalVector.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/CommonDetUnit/interface/GeomDetType.h"
#include "DataFormats/TrackerRecHit2D/interface/SiPixelRecHit.h"
#include "DataFormats/TrackReco/interface/TrackExtra.h"
#include "DataFormats/SiStripDetId/interface/StripSubdetector.h"
#include "DataFormats/TrackerRecHit2D/interface/SiStripMatchedRecHit2D.h"
#include "Geometry/CommonTopologies/interface/StripTopology.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "Geometry/Records/interface/IdealGeometryRecord.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "TrackingTools/Records/interface/TransientRecHitRecord.h"
#include "TrackingTools/TrackFitters/interface/TrajectoryStateCombiner.h"
#include "TrackingTools/PatternTools/interface/TrajTrackAssociation.h"
#include "RecoTracker/TransientTrackingRecHit/interface/TkTransientTrackingRecHitBuilder.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "CalibTracker/Records/interface/SiPixelTemplateDBObjectESProducerRcd.h"
#include "CondFormats/SiPixelObjects/interface/SiPixelTemplateDBObject.h"
#include "CondFormats/SiPixelTransient/interface/SiPixelTemplateDefs.h"
#include "CondFormats/SiPixelTransient/interface/SiPixelTemplate.h"
#include "Geometry/TrackerGeometryBuilder/interface/PixelTopologyMap.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "../interface/SiPixelLorentzAngleCalibrationStruct.h"
#include <TTree.h>
#include <TFile.h>
//
// class declaration
//

static const int maxpix = 1000;
struct Pixinfo {
  int npix;
  float row[maxpix];
  float col[maxpix];
  float adc[maxpix];
  float x[maxpix];
  float y[maxpix];
};

struct Hit {
  float x;
  float y;
  double alpha;
  double beta;
  double gamma;
};
struct Clust {
  float x;
  float y;
  float charge;
  int size_x;
  int size_y;
  int maxPixelCol;
  int maxPixelRow;
  int minPixelCol;
  int minPixelRow;
};
struct Rechit {
  float x;
  float y;
};

class SiPixelLorentzAnglePCLWorker : public DQMGlobalEDAnalyzer<SiPixelLorentzAngleCalibrationHistograms> {
public:
  explicit SiPixelLorentzAnglePCLWorker(const edm::ParameterSet&);
  ~SiPixelLorentzAnglePCLWorker() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void bookHistograms(DQMStore::IBooker&,
                      edm::Run const&,
                      edm::EventSetup const&,
                      SiPixelLorentzAngleCalibrationHistograms&) const override;

  void dqmAnalyze(edm::Event const&,
                  edm::EventSetup const&,
                  SiPixelLorentzAngleCalibrationHistograms const&) const override;

  void dqmBeginRun(edm::Run const&, edm::EventSetup const&, SiPixelLorentzAngleCalibrationHistograms&) const override;

  void dqmEndRun(edm::Run const&,
                 edm::EventSetup const&,
                 SiPixelLorentzAngleCalibrationHistograms const&) const override;

  const Pixinfo fillPix(const SiPixelCluster& LocPix, const PixelTopology* topol) const;
  const std::pair<LocalPoint, LocalPoint> surface_deformation(const PixelTopology* topol,
                                                              TrajectoryStateOnSurface& tsos,
                                                              const SiPixelRecHit* recHitPix) const;
  // ------------ member data ------------
  std::string folder_;
  std::string filename_;

  // histogram etc
  int hist_x_;
  int hist_y_;
  double min_x_;
  double max_x_;
  double min_y_;
  double max_y_;
  double width_;
  double min_depth_;
  double max_depth_;
  double min_drift_;
  double max_drift_;
  double ypitch_;
  int bufsize;

  // parameters from config file
  double ptmin_;
  double normChi2Max_;
  int clustSizeYMin_;
  double residualMax_;
  double clustChargeMax_;
  int hist_depth_;
  int hist_drift_;

  std::unique_ptr<TFile> hFile_;
  std::shared_ptr<TTree> SiPixelLorentzAngleTreeBarrel_;
  std::shared_ptr<TTree> SiPixelLorentzAngleTreeForward_;

  // es consumes
  edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> geomEsToken_;
  edm::ESGetToken<TrackerTopology, TrackerTopologyRcd> topoEsToken_;
  edm::ESGetToken<SiPixelTemplateDBObject, SiPixelTemplateDBObjectESProducerRcd> siPixelTemplateEsToken_;
  edm::ESGetToken<TrackerTopology, TrackerTopologyRcd> topoPerEventEsToken_;
  edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> geomPerEventEsToken_;

  // event consumes
  edm::EDGetTokenT<TrajTrackAssociationCollection> t_trajTrack;
};

//
// constants, enums and typedefs
//

// tree branches barrel
int run_;
long int event_;
int lumiblock_;
int bx_;
int orbit_;
int module_;
int ladder_;
int layer_;
int isflipped_;
float pt_;
float eta_;
float phi_;
double chi2_;
double ndof_;
Pixinfo pixinfo_;
Hit simhit_, trackhit_;
Clust clust_;
Rechit rechit_;
Rechit rechitCorr_;
float trackhitCorrX_;
float trackhitCorrY_;
float qScale_;
float rQmQt_;

// tree branches forward
int runF_;
int eventF_;
int sideF_;
int diskF_;
int bladeF_;
int panelF_;
int moduleF_;
float ptF_;
float etaF_;
float phiF_;
double chi2F_;
double ndofF_;
Pixinfo pixinfoF_;
Hit simhitF_, trackhitF_;
Clust clustF_;
Rechit rechitF_;
Rechit rechitCorrF_;
float trackhitCorrXF_;
float trackhitCorrYF_;
float qScaleF_;
float rQmQtF_;
//
// static data member definitions
//

//
// constructors and destructor
//
SiPixelLorentzAnglePCLWorker::SiPixelLorentzAnglePCLWorker(const edm::ParameterSet& iConfig)
    : folder_(iConfig.getParameter<std::string>("folder")),
      filename_(iConfig.getParameter<std::string>("fileName")),
      ptmin_(iConfig.getParameter<double>("ptMin")),
      normChi2Max_(iConfig.getParameter<double>("normChi2Max")),
      clustSizeYMin_(iConfig.getParameter<int>("clustSizeYMin")),
      residualMax_(iConfig.getParameter<double>("residualMax")),
      clustChargeMax_(iConfig.getParameter<double>("clustChargeMax")),
      hist_depth_(iConfig.getParameter<int>("binsDepth")),
      hist_drift_(iConfig.getParameter<int>("binsDrift")),
      geomEsToken_(esConsumes<edm::Transition::BeginRun>()),
      topoEsToken_(esConsumes<edm::Transition::BeginRun>()),
      siPixelTemplateEsToken_(esConsumes()),
      topoPerEventEsToken_(esConsumes()),
      geomPerEventEsToken_(esConsumes()) {
  t_trajTrack = consumes<TrajTrackAssociationCollection>(iConfig.getParameter<edm::InputTag>("src"));

  // now do what ever initialization is needed
  hist_x_ = 50;
  hist_y_ = 100;
  min_x_ = -500.;
  max_x_ = 500.;
  min_y_ = -1500.;
  max_y_ = 500.;
  width_ = 0.0285;
  min_depth_ = -100.;
  max_depth_ = 400.;
  min_drift_ = -1000.;  //-200.;(iConfig.getParameter<double>("residualMax"))
  max_drift_ = 1000.;   //400.;

  bufsize = 64000;
  //    create tree structure
  //    Barrel pixel
  hFile_ = std::make_unique<TFile>(filename_.c_str(), "RECREATE");
  SiPixelLorentzAngleTreeBarrel_ =
      std::make_shared<TTree>("SiPixelLorentzAngleTreeBarrel_", "SiPixel LorentzAngle tree barrel", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("run", &run_, "run/I", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("event", &event_, "event/l", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("lumiblock", &lumiblock_, "lumiblock/I", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("bx", &bx_, "bx/I", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("orbit", &orbit_, "orbit/I", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("module", &module_, "module/I", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("ladder", &ladder_, "ladder/I", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("layer", &layer_, "layer/I", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("isflipped", &isflipped_, "isflipped/I", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("pt", &pt_, "pt/F", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("eta", &eta_, "eta/F", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("phi", &phi_, "phi/F", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("chi2", &chi2_, "chi2/D", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("ndof", &ndof_, "ndof/D", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("trackhit", &trackhit_, "x/F:y/F:alpha/D:beta/D:gamma_/D", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("npix", &pixinfo_.npix, "npix/I", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("rowpix", pixinfo_.row, "row[npix]/F", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("colpix", pixinfo_.col, "col[npix]/F", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("adc", pixinfo_.adc, "adc[npix]/F", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("xpix", pixinfo_.x, "x[npix]/F", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("ypix", pixinfo_.y, "y[npix]/F", bufsize);

  SiPixelLorentzAngleTreeBarrel_->Branch(
      "clust",
      &clust_,
      "x/F:y/F:charge/F:size_x/I:size_y/I:maxPixelCol/I:maxPixelRow:minPixelCol/I:minPixelRow/I",
      bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("rechit", &rechit_, "x/F:y/F", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("rechit_corr", &rechitCorr_, "x/F:y/F", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("trackhitcorr_x", &trackhitCorrX_, "trackhitcorr_x/F", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("trackhitcorr_y", &trackhitCorrY_, "trackhitcorr_y/F", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("qScale", &qScale_, "qScale/F", bufsize);
  SiPixelLorentzAngleTreeBarrel_->Branch("rQmQt", &rQmQt_, "rQmQt/F", bufsize);
  //    Forward pixel

  SiPixelLorentzAngleTreeForward_ =
      std::make_shared<TTree>("SiPixelLorentzAngleTreeForward_", "SiPixel LorentzAngle tree forward", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("run", &run_, "run/I", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("event", &event_, "event/l", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("lumiblock", &lumiblock_, "lumiblock/I", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("bx", &bx_, "bx/I", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("orbit", &orbit_, "orbit/I", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("side", &sideF_, "side/I", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("disk", &diskF_, "disk/I", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("blade", &bladeF_, "blade/I", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("panel", &panelF_, "panel/I", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("module", &moduleF_, "module/I", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("pt", &pt_, "pt/F", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("eta", &eta_, "eta/F", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("phi", &phi_, "phi/F", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("chi2", &chi2_, "chi2/D", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("ndof", &ndof_, "ndof/D", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("trackhit", &trackhitF_, "x/F:y/F:alpha/D:beta/D:gamma_/D", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("npix", &pixinfoF_.npix, "npix/I", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("rowpix", pixinfoF_.row, "row[npix]/F", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("colpix", pixinfoF_.col, "col[npix]/F", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("adc", pixinfoF_.adc, "adc[npix]/F", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("xpix", pixinfoF_.x, "x[npix]/F", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("ypix", pixinfoF_.y, "y[npix]/F", bufsize);

  SiPixelLorentzAngleTreeForward_->Branch(
      "clust",
      &clustF_,
      "x/F:y/F:charge/F:size_x/I:size_y/I:maxPixelCol/I:maxPixelRow:minPixelCol/I:minPixelRow/I",
      bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("rechit", &rechitF_, "x/F:y/F", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("rechit_corr", &rechitCorrF_, "x/F:y/F", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("trackhitcorr_x", &trackhitCorrXF_, "trackhitcorr_x/F", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("trackhitcorr_y", &trackhitCorrYF_, "trackhitcorr_y/F", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("qScale", &qScaleF_, "qScale/F", bufsize);
  SiPixelLorentzAngleTreeForward_->Branch("rQmQt", &rQmQtF_, "rQmQt/F", bufsize);
}

SiPixelLorentzAnglePCLWorker::~SiPixelLorentzAnglePCLWorker() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
}

//
// member functions
//

// ------------ method called for each event  ------------

void SiPixelLorentzAnglePCLWorker::dqmAnalyze(edm::Event const& iEvent,
                                              edm::EventSetup const& iSetup,
                                              SiPixelLorentzAngleCalibrationHistograms const& iHists) const {
  // Retrieve template stuff
  const SiPixelTemplateDBObject* templateDBobject_ = &iSetup.getData(siPixelTemplateEsToken_);
  std::vector<SiPixelTemplateStore> thePixelTemp_;
  SiPixelTemplate templ(thePixelTemp_);
  if (!SiPixelTemplate::pushfile(*templateDBobject_, thePixelTemp_)) {
    edm::LogError("SiPixelLorentzAnglePCLWorker")
        << "\nERROR: Templates not filled correctly. Check the sqlite file."
        << "Using SiPixelTemplateDBObject version " << (*templateDBobject_).version() << "\n\n";
  }

  // Retrieve tracker topology from geometry
  const TrackerTopology* const tTopo = &iSetup.getData(topoPerEventEsToken_);

  // Retrieve track geometry
  const TrackerGeometry* tracker = &iSetup.getData(geomPerEventEsToken_);

  // get the association map between tracks and trajectories
  edm::Handle<TrajTrackAssociationCollection> trajTrackCollectionHandle;
  iEvent.getByToken(t_trajTrack, trajTrackCollectionHandle);

  module_ = -1;
  layer_ = -1;
  ladder_ = -1;
  isflipped_ = -1;
  pt_ = -999;
  eta_ = 999;
  phi_ = 999;
  pixinfo_.npix = 0;

  run_ = iEvent.id().run();
  event_ = iEvent.id().event();
  lumiblock_ = iEvent.luminosityBlock();
  bx_ = iEvent.bunchCrossing();
  orbit_ = iEvent.orbitNumber();

  if (!trajTrackCollectionHandle->empty()) {
    for (TrajTrackAssociationCollection::const_iterator it = trajTrackCollectionHandle->begin();
         it != trajTrackCollectionHandle->end();
         ++it) {
      const reco::Track& track = *it->val;
      const Trajectory& traj = *it->key;

      // get the trajectory measurements
      std::vector<TrajectoryMeasurement> tmColl = traj.measurements();
      pt_ = track.pt();
      eta_ = track.eta();
      phi_ = track.phi();
      chi2_ = traj.chiSquared();
      ndof_ = traj.ndof();

      if (pt_ < ptmin_)
        continue;
      // iterate over trajectory measurements
      iHists.h_tracks_->Fill(0);
      bool pixeltrack = false;
      for (std::vector<TrajectoryMeasurement>::const_iterator itTraj = tmColl.begin(); itTraj != tmColl.end();
           itTraj++) {
        if (!itTraj->updatedState().isValid())
          continue;
        TransientTrackingRecHit::ConstRecHitPointer recHit = itTraj->recHit();
        if (!recHit->isValid() || recHit->geographicalId().det() != DetId::Tracker)
          continue;
        unsigned int subDetID = (recHit->geographicalId().subdetId());
        if (subDetID == PixelSubdetector::PixelBarrel || subDetID == PixelSubdetector::PixelEndcap) {
          if (!pixeltrack) {
            iHists.h_tracks_->Fill(1);
          }
          pixeltrack = true;
        }

        if (subDetID == PixelSubdetector::PixelBarrel) {
          DetId detIdObj = recHit->geographicalId();
          const PixelGeomDetUnit* theGeomDet = dynamic_cast<const PixelGeomDetUnit*>(tracker->idToDet(detIdObj));
          if (!theGeomDet)
            continue;

          const PixelTopology* topol = &(theGeomDet->specificTopology());

          if (!topol)
            continue;

          layer_ = tTopo->pxbLayer(detIdObj);
          ladder_ = tTopo->pxbLadder(detIdObj);
          module_ = tTopo->pxbModule(detIdObj);

          float tmp1 = theGeomDet->surface().toGlobal(Local3DPoint(0., 0., 0.)).perp();
          float tmp2 = theGeomDet->surface().toGlobal(Local3DPoint(0., 0., 1.)).perp();

          isflipped_ = (tmp2 < tmp1) ? 1 : 0;

          const SiPixelRecHit* recHitPix = dynamic_cast<const SiPixelRecHit*>((*recHit).hit());
          if (!recHitPix)
            continue;
          rechit_.x = recHitPix->localPosition().x();
          rechit_.y = recHitPix->localPosition().y();
          SiPixelRecHit::ClusterRef const& cluster = recHitPix->cluster();

          pixinfo_ = fillPix(*cluster, topol);

          // fill entries in clust_

          clust_.x = (cluster)->x();
          clust_.y = (cluster)->y();
          clust_.charge = (cluster->charge()) / 1000.;
          clust_.size_x = cluster->sizeX();
          clust_.size_y = cluster->sizeY();
          clust_.maxPixelCol = cluster->maxPixelCol();
          clust_.maxPixelRow = cluster->maxPixelRow();
          clust_.minPixelCol = cluster->minPixelCol();
          clust_.minPixelRow = cluster->minPixelRow();

          // fill the trackhit info
          TrajectoryStateOnSurface tsos = itTraj->updatedState();
          if (!tsos.isValid()) {
            edm::LogWarning("SiPixelLorentzAnglePCLWorker") << "tsos not valid" << std::endl;
            continue;
          }
          LocalVector trackdirection = tsos.localDirection();
          LocalPoint trackposition = tsos.localPosition();

          if (trackdirection.z() == 0)
            continue;
          // the local position and direction
          trackhit_.alpha = atan2(trackdirection.z(), trackdirection.x());
          trackhit_.beta = atan2(trackdirection.z(), trackdirection.y());
          trackhit_.gamma = atan2(trackdirection.x(), trackdirection.y());
          trackhit_.x = trackposition.x();
          trackhit_.y = trackposition.y();

          // get qScale_ = templ.qscale() and  templ.r_qMeas_qTrue();

          float cotalpha = trackdirection.x() / trackdirection.z();
          float cotbeta = trackdirection.y() / trackdirection.z();

          float locBx = 1.;
          if (cotbeta < 0.)
            locBx = -1.;
          float locBz = locBx;
          if (cotalpha < 0.)
            locBz = -locBx;

          auto detId = detIdObj.rawId();
          int TemplID = templateDBobject_->getTemplateID(detId);
          templ.interpolate(TemplID, cotalpha, cotbeta, locBz, locBx);
          qScale_ = templ.qscale();
          rQmQt_ = templ.r_qMeas_qTrue();

          // Surface deformation

          const auto& lp_pair = surface_deformation(topol, tsos, recHitPix);

          LocalPoint lp_track = lp_pair.first;
          LocalPoint lp_rechit = lp_pair.second;

          rechitCorr_.x = lp_rechit.x();
          rechitCorr_.y = lp_rechit.y();
          trackhitCorrX_ = lp_track.x();
          trackhitCorrY_ = lp_track.y();

          SiPixelLorentzAngleTreeBarrel_->Fill();

          // is one pixel in cluster a large pixel ? (hit will be excluded)
          bool large_pix = false;
          for (int j = 0; j < pixinfo_.npix; j++) {
            int colpos = static_cast<int>(pixinfo_.col[j]);
            if (pixinfo_.row[j] == 0 || pixinfo_.row[j] == 79 || pixinfo_.row[j] == 80 || pixinfo_.row[j] == 159 ||
                colpos % 52 == 0 || colpos % 52 == 51) {
              large_pix = true;
            }
          }

          double residual = TMath::Sqrt((trackhitCorrX_ - rechitCorr_.x) * (trackhitCorrX_ - rechitCorr_.x) +
                                        (trackhitCorrY_ - rechitCorr_.y) * (trackhitCorrY_ - rechitCorr_.y));

          double xlim1 = trackhitCorrX_ - width_ * cotalpha / 2.;
          double hypitch_ = ypitch_ / 2.;
          double ylim1 = trackhitCorrY_ - width_ * cotbeta / 2.;
          double ylim2 = trackhitCorrY_ + width_ * cotbeta / 2.;

          if (!large_pix && (chi2_ / ndof_) < normChi2Max_ && cluster->sizeY() >= clustSizeYMin_ &&
              residual < residualMax_ && (cluster->charge() < clustChargeMax_)) {
            // iterate over pixels in hit
            for (int j = 0; j < pixinfo_.npix; j++) {
              // use trackhits and include bowing correction
              float ypixlow = pixinfo_.y[j] - hypitch_;
              float ypixhigh = pixinfo_.y[j] + hypitch_;
              if (cotbeta > 0.) {
                if (ylim1 > ypixlow)
                  ypixlow = ylim1;
                if (ylim2 < ypixhigh)
                  ypixhigh = ylim2;
              } else {
                if (ylim2 > ypixlow)
                  ypixlow = ylim2;
                if (ylim1 < ypixhigh)
                  ypixhigh = ylim1;
              }
              float ypixavg = 0.5 * (ypixlow + ypixhigh);

              float dx = (pixinfo_.x[j] - xlim1) * 10000.;
              float dy = (ypixavg - ylim1) * 10000.;
              float depth = dy * tan(trackhit_.beta);
              float drift = dx - dy * tan(trackhit_.gamma);

              int i_index = module_ + (layer_ - 1) * iHists.nModules_[layer_ - 1];
              iHists.h_drift_depth_adc_.at(i_index)->Fill(drift, depth, pixinfo_.adc[j]);
              iHists.h_drift_depth_adc2_.at(i_index)->Fill(drift, depth, pixinfo_.adc[j] * pixinfo_.adc[j]);
              iHists.h_drift_depth_noadc_.at(i_index)->Fill(drift, depth);
            }
          }
        } else if (subDetID == PixelSubdetector::PixelEndcap) {
          // what to do here?
          edm::LogInfo("SiPixelLorentzAnglePCLWorker") << "do Nothing here?" << std::endl;
          DetId detIdObj = recHit->geographicalId();
          const PixelGeomDetUnit* theGeomDet = dynamic_cast<const PixelGeomDetUnit*>(tracker->idToDet(detIdObj));
          if (!theGeomDet)
            continue;

          const PixelTopology* topol = &(theGeomDet->specificTopology());

          if (!topol)
            continue;

          sideF_ = tTopo->pxfSide(detIdObj);
          diskF_ = tTopo->pxfDisk(detIdObj);
          bladeF_ = tTopo->pxfBlade(detIdObj);
          panelF_ = tTopo->pxfPanel(detIdObj);
          moduleF_ = tTopo->pxfModule(detIdObj);

          const SiPixelRecHit* recHitPix = dynamic_cast<const SiPixelRecHit*>((*recHit).hit());
          if (!recHitPix)
            continue;
          rechitF_.x = recHitPix->localPosition().x();
          rechitF_.y = recHitPix->localPosition().y();
          SiPixelRecHit::ClusterRef const& cluster = recHitPix->cluster();

          pixinfoF_ = fillPix(*cluster, topol);

          // fill entries in clust_

          clustF_.x = (cluster)->x();
          clustF_.y = (cluster)->y();
          clustF_.charge = (cluster->charge()) / 1000.;
          clustF_.size_x = cluster->sizeX();
          clustF_.size_y = cluster->sizeY();
          clustF_.maxPixelCol = cluster->maxPixelCol();
          clustF_.maxPixelRow = cluster->maxPixelRow();
          clustF_.minPixelCol = cluster->minPixelCol();
          clustF_.minPixelRow = cluster->minPixelRow();

          // fill the trackhit info
          TrajectoryStateOnSurface tsos = itTraj->updatedState();
          if (!tsos.isValid()) {
            edm::LogWarning("SiPixelLorentzAnglePCLWorker") << "tsos not valid" << std::endl;
            continue;
          }
          LocalVector trackdirection = tsos.localDirection();
          LocalPoint trackposition = tsos.localPosition();

          if (trackdirection.z() == 0)
            continue;
          // the local position and direction
          trackhitF_.alpha = atan2(trackdirection.z(), trackdirection.x());
          trackhitF_.beta = atan2(trackdirection.z(), trackdirection.y());
          trackhitF_.gamma = atan2(trackdirection.x(), trackdirection.y());
          trackhitF_.x = trackposition.x();
          trackhitF_.y = trackposition.y();

          float cotalpha = trackdirection.x() / trackdirection.z();
          float cotbeta = trackdirection.y() / trackdirection.z();

          float locBx = 1.;
          if (cotbeta < 0.)
            locBx = -1.;
          float locBz = locBx;
          if (cotalpha < 0.)
            locBz = -locBx;

          auto detId = detIdObj.rawId();
          int TemplID = templateDBobject_->getTemplateID(detId);
          templ.interpolate(TemplID, cotalpha, cotbeta, locBz, locBx);
          qScaleF_ = templ.qscale();
          rQmQtF_ = templ.r_qMeas_qTrue();

          // Surface deformation

          const auto& lp_pair = surface_deformation(topol, tsos, recHitPix);

          LocalPoint lp_track = lp_pair.first;
          LocalPoint lp_rechit = lp_pair.second;

          rechitCorrF_.x = lp_rechit.x();
          rechitCorrF_.y = lp_rechit.y();
          trackhitCorrXF_ = lp_track.x();
          trackhitCorrYF_ = lp_track.y();

          SiPixelLorentzAngleTreeForward_->Fill();
        }
      }  //end iteration over trajectory measurements
    }    //end iteration over trajectories
  }
}

void SiPixelLorentzAnglePCLWorker::dqmBeginRun(edm::Run const& run,
                                               edm::EventSetup const& iSetup,
                                               SiPixelLorentzAngleCalibrationHistograms& iHists) const {
  // geometry
  const TrackerGeometry* geom = &iSetup.getData(geomEsToken_);
  const TrackerTopology* tTopo = &iSetup.getData(topoEsToken_);

  PixelTopologyMap map = PixelTopologyMap(geom, tTopo);
  iHists.nlay = geom->numberOfLayers(PixelSubdetector::PixelBarrel);

  for (int i = 0; i < iHists.nlay; i++) {
    iHists.nModules_[i] = map.getPXBModules(i + 1);
  }
}

void SiPixelLorentzAnglePCLWorker::bookHistograms(DQMStore::IBooker& iBooker,
                                                  edm::Run const& run,
                                                  edm::EventSetup const& iSetup,
                                                  SiPixelLorentzAngleCalibrationHistograms& iHists) const {
  iBooker.setCurrentFolder(folder_);
  iHists.h_tracks_ = iBooker.book1D("h_tracks", "h_tracks", 2, 0., 2.);

  //book histograms
  char name[128];
  for (int i_layer = 1; i_layer <= iHists.nlay; i_layer++) {
    for (int i_module = 1; i_module <= iHists.nModules_[i_layer - 1]; i_module++) {
      unsigned int i_index = i_module + (i_layer - 1) * iHists.nModules_[i_layer - 1];

      sprintf(name, "h_drift_depth_adc_layer%i_module%i", i_layer, i_module);
      iHists.h_drift_depth_adc_[i_index] =
          iBooker.book2D(name, name, hist_drift_, min_drift_, max_drift_, hist_depth_, min_depth_, max_depth_);

      sprintf(name, "h_drift_depth_adc2_layer%i_module%i", i_layer, i_module);
      iHists.h_drift_depth_adc2_[i_index] =
          iBooker.book2D(name, name, hist_drift_, min_drift_, max_drift_, hist_depth_, min_depth_, max_depth_);

      sprintf(name, "h_drift_depth_noadc_layer%i_module%i", i_layer, i_module);
      iHists.h_drift_depth_noadc_[i_index] =
          iBooker.book2D(name, name, hist_drift_, min_drift_, max_drift_, hist_depth_, min_depth_, max_depth_);

      sprintf(name, "h_drift_depth_layer%i_module%i", i_layer, i_module);
      iHists.h_drift_depth_[i_index] =
          iBooker.book2D(name, name, hist_drift_, min_drift_, max_drift_, hist_depth_, min_depth_, max_depth_);

      sprintf(name, "h_mean_layer%i_module%i", i_layer, i_module);
      iHists.h_mean_[i_index] = iBooker.book1D(name, name, hist_depth_, min_depth_, max_depth_);
    }
  }
}

void SiPixelLorentzAnglePCLWorker::dqmEndRun(edm::Run const& run,
                                             edm::EventSetup const& iSetup,
                                             const SiPixelLorentzAngleCalibrationHistograms& iHists) const {
  hFile_->cd();
  hFile_->Write();
  hFile_->Close();
}

// method used to fill per pixel info
const Pixinfo SiPixelLorentzAnglePCLWorker::fillPix(const SiPixelCluster& LocPix, const PixelTopology* topol) const {
  static Pixinfo pixinfo;
  const std::vector<SiPixelCluster::Pixel>& pixvector = LocPix.pixels();
  pixinfo.npix = 0;
  for (std::vector<SiPixelCluster::Pixel>::const_iterator itPix = pixvector.begin(); itPix != pixvector.end();
       itPix++) {
    pixinfo.row[pixinfo.npix] = itPix->x;
    pixinfo.col[pixinfo.npix] = itPix->y;
    pixinfo.adc[pixinfo.npix] = itPix->adc;
    LocalPoint lp = topol->localPosition(MeasurementPoint(itPix->x + 0.5, itPix->y + 0.5));
    pixinfo.x[pixinfo.npix] = lp.x();
    pixinfo.y[pixinfo.npix] = lp.y();
    pixinfo.npix++;
  }
  return pixinfo;
}

// method used to correct for the surface deformation
const std::pair<LocalPoint, LocalPoint> SiPixelLorentzAnglePCLWorker::surface_deformation(
    const PixelTopology* topol, TrajectoryStateOnSurface& tsos, const SiPixelRecHit* recHitPix) const {
  LocalPoint trackposition = tsos.localPosition();
  const LocalTrajectoryParameters& ltp = tsos.localParameters();
  const Topology::LocalTrackAngles localTrackAngles(ltp.dxdz(), ltp.dydz());

  std::pair<float, float> pixels_track = topol->pixel(trackposition, localTrackAngles);
  std::pair<float, float> pixels_rechit = topol->pixel(recHitPix->localPosition(), localTrackAngles);

  LocalPoint lp_track = topol->localPosition(MeasurementPoint(pixels_track.first, pixels_track.second));

  LocalPoint lp_rechit = topol->localPosition(MeasurementPoint(pixels_rechit.first, pixels_rechit.second));

  static std::pair<LocalPoint, LocalPoint> lps = std::make_pair(lp_track, lp_rechit);
  return lps;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void SiPixelLorentzAnglePCLWorker::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<std::string>("folder", "AlCaReco/SiPixelLorentzAngle");
  desc.add<std::string>("fileName", "testrun.root");
  desc.add<edm::InputTag>("src", edm::InputTag("TrackRefitter"));
  desc.add<double>("ptMin", 3.);
  desc.add<double>("normChi2Max", 2.);
  desc.add<int>("clustSizeYMin", 4);
  desc.add<double>("residualMax", 0.005);
  desc.add<double>("clustChargeMax", 120000);
  desc.add<int>("binsDepth", 50);
  desc.add<int>("binsDrift", 200);
  descriptions.addWithDefaultLabel(desc);
}

// define this as a plug-in
DEFINE_FWK_MODULE(SiPixelLorentzAnglePCLWorker);
