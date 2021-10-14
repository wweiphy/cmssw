# cmsRun test_Harvester.py maxEvents=-1 inputFiles=file:PCL_worker.root

import FWCore.ParameterSet.Config as cms
import sys
import os

from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('analysis')
options.parseArguments()

process = cms.Process('HARVESTING')
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load("DQMServices.Core.DQM_cfg")
process.load("DQMServices.Components.DQMEnvironment_cfi")
process.load("CondCore.CondDB.CondDB_cfi")

# check for the correct tag on https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideFrontierConditions
#process.GlobalTag.globaltag = "113X_dataRun2_v6"
process.GlobalTag.globaltag = "120X_dataRun2_v2"


process.CondDB.connect = 'sqlite_file:Harvester_5000.db'

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)



process.PoolDBOutputService = cms.Service("PoolDBOutputService",
    process.CondDB,
    timetype = cms.untracked.string('runnumber'),
    toPut = cms.VPSet(cms.PSet(
        record = cms.string("SiPixelLorentzAngleRcd"),
        tag = cms.string('Harvester')
    ))
)

from CalibTracker.SiPixelLorentzAngle.SiPixelLorentzAnglePCLHarvester_cfi import SiPixelLorentzAnglePCLHarvester
process.SiPixelLorentzAnglePCLHarvester = SiPixelLorentzAnglePCLHarvester.clone()
#process.load("CalibTracker.SiPixelLorentzAngle.SiPixelLorentzAnglePCLHarvester_cfi")
process.SiPixelLorentzAnglePCLHarvester.dqmDir = cms.string('AlCaReco/SiPixelLorentzAngleHarvesting/')

#process.SiPixelLorentzAnglePCLHarvester.dqmDir = cms.string('AlCaReco/SiPixelLorentzAngle')
#from DQMServices.Core.DQMEDHarvester import DQMEDHarvester
#process.PixelLorentzAnglePCLHarvester = cms.EDProducer(
#    "SiPixelLorentzAnglePCLHarvester",
#    newmodulelist = cms.vstring("BPix_BmI_SEC7_LYR2_LDR12F_MOD1", "BPix_BmI_SEC8_LYR2_LDR14F_MOD1", "BPix_BmO_SEC3_LYR2_LDR5F_MOD1", "BPix_BmO_SEC3_LYR2_LDR5F_MOD2", "BPix_BmO_SEC3_LYR2_LDR5F_MOD3", "BPix_BpO_SEC1_LYR2_LDR1F_MOD1", "BPix_BpO_SEC1_LYR2_LDR1F_MOD2", "BPix_BpO_SEC1_LYR2_LDR1F_MOD3"),
#    dqmDir = cms.string('AlCaReco/SiPixelLorentzAngleHarvesting/'),
#    record = cms.string("SiPixelLorentzAngleRcd"),
#    fitProbCut = cms.double(0.5)
#)

#process.DQMoutput = cms.OutputModule("DQMRootOutputModule",
#                                     fileName = cms.untracked.string("OUT_step2.root"))

process.p = cms.Path(process.SiPixelLorentzAnglePCLHarvester)
#process.DQMoutput_step = cms.EndPath(process.DQMoutput)
process.load("DQMServices.Components.DQMFileSaver_cfi")
process.dqmsave_step = cms.Path(process.dqmSaver)

process.schedule = cms.Schedule(
    process.p,
#    process.DQMoutput_step
    process.dqmsave_step
    )
    
process.source = cms.Source("DQMRootSource",
                            fileNames = cms.untracked.vstring(options.inputFiles))

process.dqmSaver.workflow = '/CalibTracker/SiPixelLorentzAngle/test'
