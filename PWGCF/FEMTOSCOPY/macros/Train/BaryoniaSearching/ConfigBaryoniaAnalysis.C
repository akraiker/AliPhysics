#if !defined(__CINT__) || defined(__MAKECINT_)
#include "AliFemtoManager.h"
#include "AliFemtoModelManager.h"
#include "AliFemtoEventReaderAODMultSelection.h"
#include "AliFemtoBaryoniaAnalysis.h"
#include "AliFemtoBasicEventCut.h"
#include "AliFemtoESDTrackCut.h"
#include "AliFemtoTrioMinvFctn.h"
#include "AliFemtoTrioCut.h"
#include "AliFemtoTrio.h"
#include "AliESDtrack.h"
#endif


enum ESys  { kAPL, kPAL, kKKpi, nSys };

const char *sysNames[nSys]      = { "APL", "PAL", "KKpi"};
const bool runSys[nSys]         = {   1  ,   1  ,   1  };
const double distMin[nSys]      = {  1.5 ,  1.5 ,  1.0 }; // everything in GeV here
const double distMax[nSys]      = {  2.5 ,  2.5 ,  2.0 };
const double distBinWidth[nSys] = { 0.001, 0.001, 0.001};

bool separationCuts;
bool ppCollisions;

AliFemtoEventReaderAODMultSelection* GetReader2015(bool mcAnalysis);
AliFemtoEventReaderAODChain* GetReader2011(bool mcAnalysis);
AliFemtoEventReaderAODChain* GetReaderPP(bool mcAnalysis);
AliFemtoBaryoniaAnalysis* GetAnalysis(bool doEventMixing);
AliFemtoBasicEventCut* GetEventCut();
AliFemtoESDTrackCut* GetTrackCut(AliFemtoTrio::EPart particle);
AliFemtoTrioCut* GetTrioCut(ESys system);
void GetParticlesForSystem(ESys system, AliFemtoTrio::EPart &firstParticle, AliFemtoTrio::EPart &secondParticle, AliFemtoTrio::EPart &thirdParticle);

//________________________________________________________________________
AliFemtoManager* ConfigFemtoAnalysis(bool mcAnalysis=false, bool sepCuts=false, int year=2015, bool ppAnalysis=false, bool eventMixing=false)
{
  separationCuts = sepCuts;
  ppCollisions = ppAnalysis;
  
  // create analysis managers
  AliFemtoManager* Manager = new AliFemtoManager();
  AliFemtoModelManager *modelMgr = new AliFemtoModelManager();
  
  // add event reader
  if(ppAnalysis){
    AliFemtoEventReaderAODChain* ReaderPP = GetReaderPP(mcAnalysis);
    Manager->SetEventReader(ReaderPP);
  }
  else if(year==2015){
    AliFemtoEventReaderAODMultSelection* Reader2015 = GetReader2015(mcAnalysis);
    Manager->SetEventReader(Reader2015);
  }
  else if(year==2011){
    AliFemtoEventReaderAODChain* Reader2011 = GetReader2011(mcAnalysis);
    Manager->SetEventReader(Reader2011);
  }
  
  // declare necessary objects
  AliFemtoBaryoniaAnalysis  *baryoniaAnalysis[nSys];
  AliFemtoBasicEventCut     *eventCut[nSys];
  
  AliFemtoTrioMinvFctn *distribution[nSys];
  
  // setup analysis
  int anIter = 0;
  
  for (int iSys=0; iSys<nSys; iSys++)
  {
    if (!runSys[iSys]) continue;
    anIter = iSys;
    
    // create new analysis
    baryoniaAnalysis[anIter] = GetAnalysis(eventMixing);
    
    // get event cut
    eventCut[anIter] = GetEventCut();
    
    // get particle cuts
    AliFemtoTrio::EPart firstParticle, secondParticle, thirdParticle;
    GetParticlesForSystem((ESys)iSys,firstParticle,secondParticle, thirdParticle);
    
    AliFemtoESDTrackCut *firstTrackCut  = GetTrackCut(firstParticle);
    AliFemtoESDTrackCut *secondTrackCut = GetTrackCut(secondParticle);
    AliFemtoESDTrackCut *thirdTrackCut  = GetTrackCut(thirdParticle);
    
    // get trio cut
    AliFemtoTrioCut *trioCut = GetTrioCut((ESys)iSys);
    
    // setup anallysis cuts
    baryoniaAnalysis[anIter]->SetEventCut(eventCut[anIter]);
    baryoniaAnalysis[anIter]->SetV0SharedDaughterCut(true);
    
    baryoniaAnalysis[anIter]->SetFirstParticleCut(firstTrackCut);
    baryoniaAnalysis[anIter]->SetSecondParticleCut(secondTrackCut);
    baryoniaAnalysis[anIter]->SetThirdParticleCut(thirdTrackCut);
    
    baryoniaAnalysis[anIter]->SetCollection1type(firstParticle);
    baryoniaAnalysis[anIter]->SetCollection2type(secondParticle);
    baryoniaAnalysis[anIter]->SetCollection3type(thirdParticle);
    
    // create m_inv distribution and add to the analysis
    distribution[anIter] = new AliFemtoTrioMinvFctn(sysNames[iSys],(distMax[iSys]-distMin[iSys])/distBinWidth[iSys],distMin[iSys],distMax[iSys]);
    distribution[anIter]->SetTrioCut(trioCut);
    
    baryoniaAnalysis[anIter]->AddDistribution(distribution[anIter]);
    
    // add analysis to the manager
    Manager->AddAnalysis(baryoniaAnalysis[anIter]);
  }
  return Manager;
}

AliFemtoEventReaderAODMultSelection* GetReader2015(bool mcAnalysis)
{
  AliFemtoEventReaderAODMultSelection* Reader = new AliFemtoEventReaderAODMultSelection();
  Reader->SetFilterBit(7);
  Reader->SetReadV0(1);
  Reader->SetUseMultiplicity(AliFemtoEventReaderAOD::kCentrality);
  Reader->SetEPVZERO(kTRUE);
  Reader->SetCentralityFlattening(kTRUE);
  Reader->SetReadMC(mcAnalysis);
  
  return Reader;
}

AliFemtoEventReaderAODChain* GetReader2011(bool mcAnalysis)
{
  AliFemtoEventReaderAODChain* Reader = new AliFemtoEventReaderAODChain();
  Reader->SetFilterBit(7);
  Reader->SetReadV0(1);
  Reader->SetUseMultiplicity(AliFemtoEventReaderAOD::kCentrality);
  Reader->SetEPVZERO(kTRUE);
  Reader->SetCentralityFlattening(kTRUE);
  Reader->SetReadMC(mcAnalysis);
  
  return Reader;
}

AliFemtoEventReaderAODChain* GetReaderPP(bool mcAnalysis)
{
  AliFemtoEventReaderAODChain* Reader = new AliFemtoEventReaderAODChain();
  Reader->SetFilterBit(128);
  Reader->SetReadV0(true);
  Reader->SetUseMultiplicity(AliFemtoEventReaderAOD::kReference);
  Reader->SetMinPlpContribSPD(3);
  Reader->SetIsPileUpEvent(true);
  Reader->SetReadMC(mcAnalysis);
  
  return Reader;
}

AliFemtoBaryoniaAnalysis* GetAnalysis(bool doEventMixing)
{
  AliFemtoBaryoniaAnalysis *analysis = new AliFemtoBaryoniaAnalysis();
  analysis->SetDoEventMixing(doEventMixing);
  // here one can put some additional analysis settings in the future
  
  return analysis;
}

AliFemtoBasicEventCut* GetEventCut()
{
  AliFemtoBasicEventCut *eventCut = new AliFemtoBasicEventCut();
  eventCut->SetEventMult(3,100000);
  eventCut->SetVertZPos(-8,8);
  eventCut->SetEPVZERO(-TMath::Pi()/2.,TMath::Pi()/2.);
  
  return eventCut;
}

AliFemtoESDTrackCut* GetTrackCut(AliFemtoTrio::EPart particle)
{
  AliFemtoESDTrackCut *particleCut = new AliFemtoESDTrackCut();
  
  if(particle == AliFemtoTrio::kKaonPlus || particle==AliFemtoTrio::kKaonMinus){
    particleCut->SetMostProbableKaon();
    particleCut->SetMass(0.493677);
  }
  else if(particle == AliFemtoTrio::kPionPlus || particle==AliFemtoTrio::kPionMinus){
    particleCut->SetMostProbablePion();
    particleCut->SetMass(0.13956995);
  }
  
  if(particle == AliFemtoTrio::kKaonPlus || particle == AliFemtoTrio::kPionPlus){ particleCut->SetCharge( 1.0); }
  else                                              { particleCut->SetCharge(-1.0); }
  
  particleCut->SetPt(0.14,1.5);
  particleCut->SetEta(-0.8, 0.8);
  particleCut->SetMaxImpactZ( ppCollisions ? 9999999 : 0.25);
  particleCut->SetMaxImpactXY(ppCollisions ? 9999999 : 0.20);
  particleCut->SetStatus(AliESDtrack::kTPCin);
  particleCut->SetminTPCncls(80);
  particleCut->SetRemoveKinks(kTRUE);
  particleCut->SetLabel(kFALSE);
  particleCut->SetMaxTPCChiNdof(4.0);
  
//  particleCut->SetNsigma(3.0);
//  particleCut->SetNsigmaTPCTOF(kTRUE);
  return particleCut;
}

AliFemtoTrioCut* GetTrioCut(ESys system)
{
  AliFemtoTrioCut *trioCut = new AliFemtoTrioCut();
  
  
  // ππ cuts
  // ρ
  trioCut->SetExcludePair(0.775,0.145,AliFemtoTrio::kPionPlus,AliFemtoTrio::kPionMinus);
  // ω
  trioCut->SetExcludePair(0.783,0.008,AliFemtoTrio::kPionPlus,AliFemtoTrio::kPionMinus);
  // f0
  trioCut->SetExcludePair(0.990,0.010,AliFemtoTrio::kPionPlus,AliFemtoTrio::kPionMinus);
  // f2
  trioCut->SetExcludePair(1.276,0.187,AliFemtoTrio::kPionPlus,AliFemtoTrio::kPionMinus);
  // f0
  trioCut->SetExcludePair(1.504,0.109,AliFemtoTrio::kPionPlus,AliFemtoTrio::kPionMinus);
  // ρ3
  trioCut->SetExcludePair(1.689,0.161,AliFemtoTrio::kPionPlus,AliFemtoTrio::kPionMinus);
  // f0
  trioCut->SetExcludePair(1.723,0.139,AliFemtoTrio::kPionPlus,AliFemtoTrio::kPionMinus);
  // f4
  trioCut->SetExcludePair(2.018,0.237,AliFemtoTrio::kPionPlus,AliFemtoTrio::kPionMinus);
  // K0_s
  trioCut->SetExcludePair(0.497,0.005,AliFemtoTrio::kPionPlus,AliFemtoTrio::kPionMinus);
  
  // Kπ cuts:
  // K*(892)
  trioCut->SetExcludePair(0.294,0.050,AliFemtoTrio::kKaonPlus,AliFemtoTrio::kPionMinus);
  trioCut->SetExcludePair(0.294,0.050,AliFemtoTrio::kKaonMinus,AliFemtoTrio::kPionPlus);
  // K*(1410)
  trioCut->SetExcludePair(1.421,0.236,AliFemtoTrio::kKaonPlus,AliFemtoTrio::kPionMinus);
  trioCut->SetExcludePair(1.421,0.236,AliFemtoTrio::kKaonMinus,AliFemtoTrio::kPionPlus);
  // K*2(1430)
  trioCut->SetExcludePair(1.425,0.099,AliFemtoTrio::kKaonPlus,AliFemtoTrio::kPionMinus);
  trioCut->SetExcludePair(1.425,0.099,AliFemtoTrio::kKaonMinus,AliFemtoTrio::kPionPlus);
  // K*(1680)
  trioCut->SetExcludePair(1.718,0.322,AliFemtoTrio::kKaonPlus,AliFemtoTrio::kPionMinus);
  trioCut->SetExcludePair(1.718,0.322,AliFemtoTrio::kKaonMinus,AliFemtoTrio::kPionPlus);
  // K*3(1780)
  trioCut->SetExcludePair(1.776,0.159,AliFemtoTrio::kKaonPlus,AliFemtoTrio::kPionMinus);
  trioCut->SetExcludePair(1.776,0.159,AliFemtoTrio::kKaonMinus,AliFemtoTrio::kPionPlus);
  // K*4(2045)
  trioCut->SetExcludePair(2.045,0.198,AliFemtoTrio::kKaonPlus,AliFemtoTrio::kPionMinus);
  trioCut->SetExcludePair(2.045,0.198,AliFemtoTrio::kKaonMinus,AliFemtoTrio::kPionPlus);
  // D0
  trioCut->SetExcludePair(1.865,0.010,AliFemtoTrio::kKaonPlus,AliFemtoTrio::kPionMinus);
  trioCut->SetExcludePair(1.865,0.010,AliFemtoTrio::kKaonMinus,AliFemtoTrio::kPionPlus);
  
//  pairCut->SetPhiStarDifferenceMinimum(0.012);
//  pairCut->SetEtaDifferenceMinimum(0.017);
//  pairCut->SetShareQualityMax(1.0);
//  pairCut->SetShareFractionMax(0.05);
//  pairCut->SetRemoveSameLabel(kFALSE);
//  pairCut->SetMaxEEMinv(0.002);
//  pairCut->SetMaxThetaDiff(0.008);
//  pairCut->SetDataType(AliFemtoPairCut::kAOD);
//  if(separationCuts)
//  {
//    pairCut->SetTPCEntranceSepMinimum(0.00001);
//    pairCut->SetAvgsepMinimum(5.0);
//  }
  
  return trioCut;
}

void GetParticlesForSystem(ESys system, AliFemtoTrio::EPart &firstParticle, AliFemtoTrio::EPart &secondParticle, AliFemtoTrio::EPart &thirdParticle)
{
  if(system == kAPL){
    firstParticle  = AliFemtoTrio::kKaonMinus;
    secondParticle = AliFemtoTrio::kPionPlus;
    thirdParticle  = AliFemtoTrio::kPionMinus;
  }
  if(system == kPAL){
    firstParticle  = AliFemtoTrio::kKaonPlus;
    secondParticle = AliFemtoTrio::kPionPlus;
    thirdParticle  = AliFemtoTrio::kPionMinus;
  }
  if(system == kKKpi){
    firstParticle  = AliFemtoTrio::kKaonPlus;
    secondParticle = AliFemtoTrio::kKaonMinus;
    thirdParticle  = AliFemtoTrio::kPionMinus;
  }
}










