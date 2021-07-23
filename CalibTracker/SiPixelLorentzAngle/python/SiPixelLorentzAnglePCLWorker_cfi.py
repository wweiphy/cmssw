import FWCore.ParameterSet.Config as cms

from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
SiPixelLorentzAnglePCLWorker = DQMEDAnalyzer(
    "SiPixelLorentzAnglePCLWorker",
    folder = cms.string('AlCaReco/SiPixelLorentzAngle'),
    notInPCL = cms.bool(False),
    fileName = cms.string('testrun.root'),
    src = cms.InputTag("TrackRefitter"),
    binsDepth    = cms.int32(50),
    binsDrift =    cms.int32(200),
    ptMin = cms.double(3),
    normChi2Max = cms.double(2),
    clustSizeYMin = cms.double(3.999999),
    clustSizeYMinL4 = cms.double(2.999999),
    clustSizeXMax = cms.double(5),
    residualMax = cms.double(0.005),
    clustChargeMax = cms.double(50000)
)
