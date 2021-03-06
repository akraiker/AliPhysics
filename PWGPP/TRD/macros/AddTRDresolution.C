void AddTRDresolution(AliAnalysisDataContainer **ci, Int_t clErr, Int_t align)
{
  Info("AddTRDresolution", "[0]=\"%s\" [1]=\"%s\" [2]=\"%s\" [3]=\"%s\" [4]=\"%s\" [5]=\"%s\"", ci[0]->GetName(), ci[1]->GetName(), ci[2]->GetName(), ci[3]->GetName(), ci[4]->GetName(), ci[5]->GetName());
  AliAnalysisDataContainer *evInfoContainer = ci[3];

  AliAnalysisManager *mgr = AliAnalysisManager::GetAnalysisManager();
  if(!mgr) return;

  //AliLog::SetClassDebugLevel("AliTRDrecoTask", 2);
  //AliLog::SetClassDebugLevel("AliTRDresolution", 2);
  AliTRDresolution *res(NULL);
  const Char_t *suffix[]={"", "ITS", "K"};
  for(Int_t itq=0; itq<3; itq++){
    if(itq==1) continue;
    mgr->AddTask(res = new AliTRDresolution(Form("TRDresolution%s", suffix[itq])));
    res->SetMCdata((Bool_t)mgr->GetMCtruthEventHandler());
    res->SetPostProcess(kFALSE);
    res->SetDebugLevel(0);
    res->SetPtThreshold(0.2);
    //res->SetTriggerList("CINT7WUHJT-S-NOPF-CENT CINT7WUHSE-S-NOPF-CENT CINT7WUHQU-S-NOPF-CENT CEMC7WU-S-NOPF-ALL");
    res->SetNameId(suffix[itq]);
    mgr->ConnectInput(res, 0, mgr->GetCommonInputContainer()); // connect main (ESD) container 
    mgr->ConnectInput(res, 1, ci[itq]);                        // conect track info container
    mgr->ConnectInput(res, 2, evInfoContainer);                // conect event info container
    mgr->ConnectInput(res, 3, ci[4]);                          // conect onl.tracklets container
    mgr->ConnectInput(res, 4, ci[5]);                          // conect clusters container
    mgr->ConnectOutput(res,1, mgr->CreateContainer(res->GetName(), TObjArray::Class(), AliAnalysisManager::kOutputContainer, Form("%s:TRD_Performance", mgr->GetCommonFileName())));
  
    // Create output containers for calibration tasks
    AliAnalysisDataContainer *co(NULL);
    co = mgr->CreateContainer(Form("%sCl2Trk%s", res->GetName(), suffix[itq]), TObjArray::Class(), AliAnalysisManager::kExchangeContainer);
    mgr->ConnectOutput(res, AliTRDresolution::kClToTrk, co);
    co = mgr->CreateContainer(Form("%sCl2MC%s", res->GetName(), suffix[itq]), TObjArray::Class(), AliAnalysisManager::kExchangeContainer);
    mgr->ConnectOutput(res, AliTRDresolution::kClToMC, co);
    
    // Cluster Error Parameterization
    if(clErr){
      TObjArray *coa = mgr->GetContainers();
      AliTRDclusterResolution *taskCl(NULL);
      AliLog::SetClassDebugLevel("AliTRDclusterResolution", 2);
      for(Int_t idet(10); idet<11/*AliTRDgeometry::kNdet*/; idet++){
        mgr->AddTask(taskCl = new AliTRDclusterResolution(Form("ClErrCalib%03d", idet)));
        taskCl->SetCalibrationRegion(idet);
        taskCl->SetDebugLevel(0);
        mgr->ConnectInput(taskCl,  0, mgr->GetCommonInputContainer());  // connect main (ESD) container
        mgr->ConnectInput(taskCl,  1, (AliAnalysisDataContainer*)coa->FindObject(Form("%sCl2Trk%s", res->GetName(), suffix[itq])));
        mgr->ConnectInput(taskCl,  2, evInfoContainer);
        mgr->ConnectOutput(taskCl, 1, mgr->CreateContainer(taskCl->GetName(), TObjArray::Class(), AliAnalysisManager::kOutputContainer, Form("%s:TRD_Calibration", mgr->GetCommonFileName())));
        if(mgr->GetMCtruthEventHandler()){
          mgr->AddTask(taskCl = new AliTRDclusterResolution(Form("ClErrCalibMC%03d", idet)));
          taskCl->SetCalibrationRegion(idet);
          taskCl->SetDebugLevel(0);
          mgr->ConnectInput(taskCl,  0, mgr->GetCommonInputContainer());  
          mgr->ConnectInput(taskCl,  1, (AliAnalysisDataContainer*)coa->FindObject(Form("%sCl2MC%s", res->GetName(), suffix[itq])));
          mgr->ConnectInput(taskCl,  2, evInfoContainer);
          mgr->ConnectOutput(taskCl, 1, mgr->CreateContainer(taskCl->GetName(), TObjArray::Class(), AliAnalysisManager::kOutputContainer, Form("%s:TRD_Calibration", mgr->GetCommonFileName())));
        }
      }
    }
  }

  // TRD alignment
  if(align){
    AliTRDalignmentTask *taskAlign(NULL);
    mgr->AddTask(taskAlign = new AliTRDalignmentTask((char*)"TRDalignment"));
    taskAlign->SetDebugLevel(0);
    //AliLog::SetClassDebugLevel("AliTRDalignmentTask", 5);  
    mgr->ConnectInput(taskAlign,  0, mgr->GetCommonInputContainer());  
    mgr->ConnectInput(taskAlign,  1, ci[0]);
    mgr->ConnectInput(taskAlign,  2, evInfoContainer);
    mgr->ConnectInput(taskAlign,  3, ci[4]);                          // conect onl.tracklets container
    mgr->ConnectInput(taskAlign,  4, ci[5]);                          // conect clusters container
    mgr->ConnectOutput(taskAlign, 1, mgr->CreateContainer(Form("h%s", taskAlign->GetName()), TObjArray::Class(), AliAnalysisManager::kExchangeContainer));
    mgr->ConnectOutput(taskAlign, 2, mgr->CreateContainer(taskAlign->GetName(), TTree::Class(), AliAnalysisManager::kOutputContainer, Form("%s:TRD_Alignment",mgr->GetCommonFileName())));
  }
}

