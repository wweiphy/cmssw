# cmsRun test_Worker.py outName=Tree.root maxEvents=2 notInPCL=True PCLoutName=PCL_worker.root  inputFiles='/store/data/Run2018D/SingleMuon/ALCARECO/SiPixelCalSingleMuon-ForPixelALCARECO_UL2018-v1/250000/8313A9E7-F31A-1649-B080-6365B3948442.root'


# cmsRun test_Worker.py outName=Tree.root maxEvents=2 notInPCL=False PCLoutName=PCL_worker.root  inputFiles='/store/data/Run2018D/SingleMuon/ALCARECO/SiPixelCalSingleMuon-ForPixelALCARECO_UL2018-v1/250000/8313A9E7-F31A-1649-B080-6365B3948442.root','/store/data/Run2018D/SingleMuon/ALCARECO/SiPixelCalSingleMuon-ForPixelALCARECO_UL2018-v1/230000/77532669-589B-5D43-99E7-8D57C173752F.root','/store/data/Run2018D/SingleMuon/ALCARECO/SiPixelCalSingleMuon-ForPixelALCARECO_UL2018-v1/230000/77532669-589B-5D43-99E7-8D57C173752F.root'

# cmsRun test_Worker.py outName=Tree.root maxEvents=99999999 notInPCL=False PCLoutName=PCL_worker_full_new2_test.root inputFiles=/store/data/Run2018D/SingleMuon/ALCARECO/SiPixelCalSingleMuon-ForPixelALCARECO_UL2018-v1/230000/F33B7CA6-256A-B34F-9536-7594FBC6F75B.root inputFiles=/store/data/Run2018D/SingleMuon/ALCARECO/SiPixelCalSingleMuon-ForPixelALCARECO_UL2018-v1/230000/4040CCA0-912F-CC40-A865-A64ADBAAC671.root inputFiles=/store/data/Run2018D/SingleMuon/ALCARECO/SiPixelCalSingleMuon-ForPixelALCARECO_UL2018-v1/30000/53AA218C-DFD2-7741-8745-AE8BC60040E3.root

# cmsRun test_Worker.py maxEvents=-1 PCLoutName=PCL_worker_full.root inputFiles=/store/data/Run2018D/SingleMuon/ALCARECO/SiPixelCalSingleMuon-ForPixelALCARECO_UL2018-v1/230000/F33B7CA6-256A-B34F-9536-7594FBC6F75B.root  inputFiles=/store/data/Run2018D/SingleMuon/ALCARECO/SiPixelCalSingleMuon-ForPixelALCARECO_UL2018-v1/230000/4040CCA0-912F-CC40-A865-A64ADBAAC671.root inputFiles=/store/data/Run2018D/SingleMuon/ALCARECO/SiPixelCalSingleMuon-ForPixelALCARECO_UL2018-v1/30000/53AA218C-DFD2-7741-8745-AE8BC60040E3.root

import FWCore.ParameterSet.Config as cms
#import sys
#import os

from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('analysis')
options.register( "outName", "Tree.root", VarParsing.multiplicity.singleton, VarParsing.varType.string, "name and path of the Tree output files (without extension)" )
options.register( "PCLoutName", "DQM_Worker.root", VarParsing.multiplicity.singleton, VarParsing.varType.string, "name and path of the PCL Worker output files (without extension)" )

options.register( "notInPCL", False, VarParsing.multiplicity.singleton, VarParsing.varType.bool, "Is it in PCL or not ?" )
options.parseArguments()

from Configuration.Eras.Era_Run2_2018_cff import Run2_2018

process = cms.Process("LA", Run2_2018)
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

#process.GlobalTag.globaltag = "113X_dataRun2_v6"
process.GlobalTag.globaltag = "120X_dataRun2_v2" # for CMSSW 12_0_0_pre
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

#process.MessageLogger = cms.Service("MessageLogger",
#    destinations = cms.untracked.vstring('simul',
#        'cout'),
#    simul = cms.untracked.PSet(
#        threshold = cms.untracked.string('ERROR')
#    ),
#)

process.load("CalibTracker.SiPixelLorentzAngle.SiPixelLorentzAnglePCLWorker_cfi")
process.SiPixelLorentzAnglePCLWorker.folder = cms.string('AlCaReco/SiPixelLorentzAngleHarvesting/')
process.SiPixelLorentzAnglePCLWorker.fileName = cms.string(options.outName)
process.SiPixelLorentzAnglePCLWorker.notInPCL = cms.bool(options.notInPCL)


#process.SiPixelLorentzAnglePCLWorker = cms.EDProducer(
#    "SiPixelLorentzAnglePCLWorker",
#    folder = cms.string('AlCaReco/SiPixelLorentzAngleHarvesting/'),
#    notInPCL = cms.bool(options.notInPCL),
#    fileName = cms.string(options.outName),
#    newmodulelist = cms.vstring("BPix_BmI_SEC7_LYR2_LDR12F_MOD1", "BPix_BmI_SEC8_LYR2_LDR14F_MOD1", "BPix_BmO_SEC3_LYR2_LDR5F_MOD1", "BPix_BmO_SEC3_LYR2_LDR5F_MOD2", "BPix_BmO_SEC3_LYR2_LDR5F_MOD3", "BPix_BpO_SEC1_LYR2_LDR1F_MOD1", "BPix_BpO_SEC1_LYR2_LDR1F_MOD2", "BPix_BpO_SEC1_LYR2_LDR1F_MOD3"),
#    src = cms.InputTag("TrackRefitter"),
#    binsDepth    = cms.int32(50),
#    binsDrift =    cms.int32(200),
#    ptMin = cms.double(3),
#    normChi2Max = cms.double(2),
#    clustSizeYMin = cms.int32(4),
#    clustSizeYMinL4 = cms.int32(3),
#    clustSizeXMax = cms.int32(5),
#    residualMax = cms.double(0.005),
#    clustChargeMaxPerLength = cms.double(50000)
#)


process.DQMoutput = cms.OutputModule("DQMRootOutputModule",
                                     fileName = cms.untracked.string(options.PCLoutName))
                                     
process.p = cms.Path(process.offlineBeamSpot*
                     process.MeasurementTrackerEvent*
                     process.TrackRefitter*
                     process.SiPixelLorentzAnglePCLWorker)

process.DQMoutput_step = cms.EndPath(process.DQMoutput)

process.schedule = cms.Schedule(
    process.p,
    process.DQMoutput_step
    )
                                     
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)
    
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles),
#    fileNames = cms.untracked.vstring(lines),
    secondaryFileNames = cms.untracked.vstring()
)

#process.options.numberOfThreads=cms.untracked.uint32(4)
#process.options.numberOfStreams=cms.untracked.uint32(32)
