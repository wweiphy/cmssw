# cmsRun test.py maxEvents=10 inputFiles=file:/uscms/home/wwei/nobackup/LA/CMSSW_11_3_0/src/SiPixelCalSingleMuon.root

import FWCore.ParameterSet.Config as cms
import sys
import os

from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('analysis')
options.register( "outName", "lorentzangle.root", VarParsing.multiplicity.singleton, VarParsing.varType.string, "name and path of the output files (without extension)" )
options.parseArguments()

from Configuration.Eras.Era_Run2_2018_cff import Run2_2018

process = cms.Process("LA", Run2_2018)
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.GlobalTag.globaltag = "113X_dataRun2_v6"
#process.GlobalTag.globaltag = "112X_dataRun2_v7" CMSSW 11_2_0_pre10
# https://github.com/cms-sw/cmssw/blob/CMSSW_11_3_X/Configuration/AlCa/python/autoCond.py

process.load("RecoTracker.Configuration.RecoTracker_cff")
process.load("RecoVertex.BeamSpotProducer.BeamSpot_cff")
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideFindingBeamSpot
from RecoVertex.BeamSpotProducer.BeamSpot_cff import *
process.offlineBeamSpot = offlineBeamSpot

process.load("RecoTracker.TrackProducer.TrackRefitters_cff")
process.load("TrackingTools.TransientTrack.TransientTrackBuilder_cfi")
process.load("RecoTracker.MeasurementDet.MeasurementTrackerEventProducer_cfi")

process.MeasurementTrackerEvent.pixelClusterProducer = 'ALCARECOSiPixelCalSingleMuon'
process.MeasurementTrackerEvent.stripClusterProducer = 'ALCARECOSiPixelCalSingleMuon'
process.MeasurementTrackerEvent.inactivePixelDetectorLabels = cms.VInputTag()
process.MeasurementTrackerEvent.inactiveStripDetectorLabels = cms.VInputTag()


process.load("RecoTracker.TransientTrackingRecHit.TransientTrackingRecHitBuilderWithoutRefit_cfi")

process.TrackRefitter.src = 'ALCARECOSiPixelCalSingleMuon'
process.TrackRefitter.TrajectoryInEvent = True

process.MessageLogger = cms.Service("MessageLogger",
    destinations = cms.untracked.vstring('simul', 
        'cout'),
    simul = cms.untracked.PSet(
        threshold = cms.untracked.string('ERROR')
    ),
)

process.SiPixelLorentzAnglePCLWorker = cms.EDProducer(
    "SiPixelLorentzAnglePCLWorker",
    folder = cms.string('CalibTracker'),
#    fileName = cms.string(options.outName),
    src = cms.InputTag("TrackRefitter"),
    binsDepth    = cms.int32(50),
    binsDrift =    cms.int32(200),
    ptMin = cms.double(3),
    normChi2Max = cms.double(2),
    clustSizeYMin = cms.int32(4),
    residualMax = cms.double(0.005),
    clustChargeMax = cms.double(120000)
)

process.p = cms.Path(process.offlineBeamSpot*
                     process.MeasurementTrackerEvent*
                     process.TrackRefitter*
                     process.SiPixelLorentzAnglePCLWorker)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(int(options.maxEvents))
)

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles),
#   skipEvents = cms.untracked.uint32(100)
)

process.TFileService = cms.Service("TFileService",
      fileName = cms.string("Histograms.root"),
      closeFileFast = cms.untracked.bool(True)
  )
  
