AliAnalysisTaskHFECal* ConfigHFECal(Bool_t useMC,Bool_t MassConst,Bool_t MassWidthCut,Bool_t MassCal,Bool_t MassNonlinear,Double_t asspTCut,Double_t angleCut,Double_t MassCut, Double_t NsigCut, Int_t fqa){
  //
  // HFE standard task configuration
  //

  printf("Setting in this analysis\n");
  printf("MassConst = %d/n",MassConst); 
  printf("MassCal = %d/n", MassCal); 
  printf("MassWidth = %d/n",MassWidthCut); 
  printf("asspTCut = %d/n",asspTCut); 
  printf("angleCut = %d/n",angleCut); 
  printf("MassCut = %d/n",MassCut); 
  printf("NsigCut = %d/n", NsigCut); 
  printf("qa = %d/n", fqa); 

  Bool_t kAnalyseTaggedTracks = kTRUE;
  
  AliHFEcuts *hfecuts = new AliHFEcuts("hfeCutsEMCAL","HFE Standard Cuts");
  hfecuts->CreateStandardCuts();
  hfecuts->SetMinNClustersTPC(100);
  hfecuts->SetMinRatioTPCclusters(0.6);
  hfecuts->SetTPCmodes(AliHFEextraCuts::kFound, AliHFEextraCuts::kFoundOverFindable);
  hfecuts->SetMinNClustersITS(3);
  hfecuts->SetCutITSpixel(AliHFEextraCuts::kAny);
  hfecuts->SetCheckITSLayerStatus(kFALSE);
  hfecuts->SetVertexRange(10.);
  hfecuts->SetTOFPIDStep(kFALSE);
  hfecuts->SetPtRange(2, 50);
  hfecuts->SetMaxImpactParam(3.,3.);
  
 
  AliAnalysisTaskHFECal *task = new AliAnalysisTaskHFECal("HFEanalysisEMCal");
  printf("task ------------------------ %p\n ", task);
  //task->SetHFECuts(hfecuts);
  task->SetMassConstraint(MassConst);
  task->SetMassWidthCut(MassWidthCut);
  task->SetMassNonlinear(MassNonlinear);
  //Double_t masscut = 0.05;
  //if(!MassConst)masscut = 0.1;
  Double_t masscut = MassCut;
  task->SetInvariantMassCut(masscut);
  task->SetOpeningAngleCut(angleCut);
  task->SetMimpTassCut(asspTCut);
  task->SetMimNsigassCut(NsigCut); 
  task->SetMassCalMethod(MassCal);
  task->SetQAHist(fqa);

  // Define PID
  AliHFEpid *pid = task->GetPID();
  if(useMC) pid->SetHasMCData(kTRUE);
  pid->AddDetector("TPC", 0);
  //pid->AddDetector("EMCAL", 1);

  Double_t params[4];
  char *cutmodel;
  if(useMC){
	  // Monte-Carlo needs modelling of the falling mean with momentum at low momentum
	  // for high momentum it is consistent with a flat -0.94
	  //cutmodel = "[0]*TMath::Exp([1]*x) + [2] + [3]*x";
	  //Double_t paramsMC[4] = {0.7174, -1.588, -0.9395, 0.0246};
	  //for(int ipar = 0; ipar < 4; ipar++) params[ipar] = paramsMC[ipar];
	  cutmodel = "pol0";
	  params[0] = -1.0; //sigma min
  } else {
	  // Data is consistent with a flat 0.12
	  cutmodel = "pol0";
	  //params[0] = -0.0015;
	  //params[0] = -3.0;
	  //params[0] = -0.05; //sigma min
	  params[0] = -1.0; //sigma min
  }
  //pid->ConfigureTPCdefaultCut(cutmodel, params,3.0); 
  for(Int_t a=0;a<11;a++)pid->ConfigureTPCcentralityCut(a,cutmodel,params,3.0);
  

  // change E/p cuts
  AliHFEpidEMCAL *emcpid = pid->AliHFEpid::GetDetPID(AliHFEpid::kEMCALpid);
  emcpid->SetEoPMax(1.3);
  emcpid->SetEoPMim(0.9);

  printf("*************************************\n");
  printf("Configuring standard Task:\n");
//  task->PrintStatus();
  pid->PrintStatus();
  printf("*************************************\n"); 
  return task;
}
