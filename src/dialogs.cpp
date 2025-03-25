#include <iostream>

#include <TGFileDialog.h>
#include <TGFSComboBox.h>
#include <TGListView.h>
#include <TGFSContainer.h>

#include <TGLabel.h>
#include <TGTextBuffer.h>
#include <TGTextEntry.h>


#include "dialogs.h"


extern Pixel_t fYellow;
extern Pixel_t fRed;

enum EFileFialog {
   kIDF_CDUP,
   kIDF_NEW_FOLDER,
   kIDF_LIST,
   kIDF_DETAILS,
   kIDF_CHECKB,
   kIDF_FSLB,
   kIDF_FTYPESLB,
   kIDF_OK,
   kIDF_CANCEL
};

static const char *gDefTypes[] = { "All files",     "*",
                                   "ROOT files",    "*.root",
                                   "ROOT macros",   "*.C",
                                    0,               0 };

static TGFileInfo gInfo;

//______________________________________________________________________________
TGFileDialog2::TGFileDialog2(const TGWindow *p, const TGWindow *main,
			     EFileDialogMode dlg_type, TGFileInfo *file_info)
//: TGFileDialog(p, main, dlg_type, file_info) - не работает

//  TGTransientFrame(p, main, 10, 10, kVerticalFrame),

//: fTbfname(0), fName(0),
  // fTypes(0), fTreeLB(0), fCdup(0), fNewf(0), fList(0), fDetails(0), fCheckB(0),
  // fPcdup(0), fPnewf(0), fPlist(0), fPdetails(0), fOk(0), fCancel(0), fFv(0),
  // fFc(0), fFileInfo(0)

{
   // Create a file selection dialog. Depending on the dlg_type it can be
   // used for opening or saving a file.
   // About the first two arguments, p is the parent Window, usually the 
   // desktop (root) window, and main is the main (TGMainFrame) application 
   // window (the one opening the dialog), onto which the dialog is 
   // usually centered, and which is waiting for it to close. 

  TGTransientFrame(p, main, 10, 10, kVerticalFrame);
  fTbfname=0; fName=0;
  fTypes=0; fTreeLB=0; fCdup=0; fNewf=0; fList=0; fDetails=0; fCheckB=0;
  fPcdup=0; fPnewf=0; fPlist=0; fPdetails=0; fOk=0; fCancel=0; fFv=0;
  fFc=0; fFileInfo=0;


   SetCleanup(kDeepCleanup);
   Connect("CloseWindow()", "TGFileDialog", this, "CloseWindow()");
   DontCallClose();

   int i;

   if (!p && !main) {
      MakeZombie();
      return;
   }
   if (!file_info) {
      Error("TGFileDialog", "file_info argument not set");
      fFileInfo = &gInfo;
      if (fFileInfo->fIniDir) {
         delete [] fFileInfo->fIniDir;
         fFileInfo->fIniDir = 0;
      }
      if (fFileInfo->fFilename) {
         delete [] fFileInfo->fFilename;
         fFileInfo->fFilename = 0;
      }
      fFileInfo->fFileTypeIdx = 0;
   } else
      fFileInfo = file_info;

   if (!fFileInfo->fFileTypes)
      fFileInfo->fFileTypes = gDefTypes;

   if (!fFileInfo->fIniDir)
      fFileInfo->fIniDir = StrDup(".");

   TGHorizontalFrame *fHtext = new TGHorizontalFrame(this, 10, 10);
   AddFrame(fHtext, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 4, 4, 3, 1));
   TGTextEntry *fTxt = new TGTextEntry(fHtext, "Enter the directory where to save the files");
   fTxt->SetTextColor(fRed);
   fTxt->ChangeBackground(fYellow);
   fTxt->SetState(false);
   fHtext->AddFrame(fTxt, new TGLayoutHints(kLHintsTop | kLHintsCenterX, 2, 2, 2, 2));
   

   TGHorizontalFrame *fHtop = new TGHorizontalFrame(this, 10, 10);
   
   //--- top toolbar elements
   TGLabel *fLookin = new TGLabel(fHtop, new TGHotString((dlg_type == kFDSave)
                                                  ? "S&ave in:" : "&Look in:"));
   fTreeLB = new TGFSComboBox(fHtop, kIDF_FSLB);
   fTreeLB->Associate(this);

   fPcdup = fClient->GetPicture("tb_uplevel.xpm");
   fPnewf = fClient->GetPicture("tb_newfolder.xpm");
   fPlist = fClient->GetPicture("tb_list.xpm");
   fPdetails = fClient->GetPicture("tb_details.xpm");

   if (!(fPcdup && fPnewf && fPlist && fPdetails))
      Error("TGFileDialog", "missing toolbar pixmap(s).\n");

   fCdup    = new TGPictureButton(fHtop, fPcdup, kIDF_CDUP);
   fNewf    = new TGPictureButton(fHtop, fPnewf, kIDF_NEW_FOLDER);
   fList    = new TGPictureButton(fHtop, fPlist, kIDF_LIST);
   fDetails = new TGPictureButton(fHtop, fPdetails, kIDF_DETAILS);

   fCdup->SetStyle(gClient->GetStyle());
   fNewf->SetStyle(gClient->GetStyle());
   fList->SetStyle(gClient->GetStyle());
   fDetails->SetStyle(gClient->GetStyle());

   fCdup->SetToolTipText("Up One Level");
   fNewf->SetToolTipText("Create New Folder");
   fList->SetToolTipText("List");
   fDetails->SetToolTipText("Details");

   fCdup->Associate(this);
   fNewf->Associate(this);
   fList->Associate(this);
   fDetails->Associate(this);

   fList->AllowStayDown(kTRUE);
   fDetails->AllowStayDown(kTRUE);

   fTreeLB->Resize(200, fTreeLB->GetDefaultHeight());

   fHtop->AddFrame(fLookin, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 5, 2, 2));
   fHtop->AddFrame(fTreeLB, new TGLayoutHints(kLHintsLeft | kLHintsExpandY, 3, 0, 2, 2));
   fHtop->AddFrame(fCdup, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 3, 0, 2, 2));
   fHtop->AddFrame(fNewf, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 3, 0, 2, 2));
   fHtop->AddFrame(fList, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 3, 0, 2, 2));
   fHtop->AddFrame(fDetails, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 0, 8, 2, 2));

   if (dlg_type == kFDSave) {
      fCheckB = new TGCheckButton(fHtop, "&Overwrite", kIDF_CHECKB);
      fCheckB->SetToolTipText("Overwrite a file without displaying a message if selected");
   } else {
      fCheckB = new TGCheckButton(fHtop, "&Multiple files", kIDF_CHECKB);
      fCheckB->SetToolTipText("Allows multiple file selection when SHIFT is pressed");
      fCheckB->Connect("Toggled(Bool_t)","TGFileInfo",fFileInfo,"SetMultipleSelection(Bool_t)");
   }
   fHtop->AddFrame(fCheckB, new TGLayoutHints(kLHintsLeft | kLHintsCenterY));
   fCheckB->SetOn(fFileInfo->fMultipleSelection);
   AddFrame(fHtop, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 4, 4, 3, 1));

   //--- file view

   fFv = new TGListView(this, 400, 161);

   fFc = new TGFileContainer(fFv->GetViewPort(),
                             10, 10, kHorizontalFrame, fgWhitePixel);
   fFc->Associate(this);

   fFv->GetViewPort()->SetBackgroundColor(fgWhitePixel);
   fFv->SetContainer(fFc);
   fFv->SetViewMode(kLVList);
   fFv->SetIncrements(1, 19); // set vertical scroll one line height at a time

   TGTextButton** buttons = fFv->GetHeaderButtons();
   if (buttons) {
      buttons[0]->Connect("Clicked()", "TGFileContainer", fFc, "Sort(=kSortByName)");
      buttons[1]->Connect("Clicked()", "TGFileContainer", fFc, "Sort(=kSortByType)");
      buttons[2]->Connect("Clicked()", "TGFileContainer", fFc, "Sort(=kSortBySize)");
      buttons[3]->Connect("Clicked()", "TGFileContainer", fFc, "Sort(=kSortByOwner)");
      buttons[4]->Connect("Clicked()", "TGFileContainer", fFc, "Sort(=kSortByGroup)");
      buttons[5]->Connect("Clicked()", "TGFileContainer", fFc, "Sort(=kSortByDate)");
   }

   fFc->SetFilter(fFileInfo->fFileTypes[fFileInfo->fFileTypeIdx+1]);
   fFc->Sort(kSortByName);
   fFc->ChangeDirectory(fFileInfo->fIniDir);
   fFc->SetMultipleSelection(fFileInfo->fMultipleSelection);
   fTreeLB->Update(fFc->GetDirectory());

   fList->SetState(kButtonEngaged);

   AddFrame(fFv, new TGLayoutHints(kLHintsTop | kLHintsExpandX | kLHintsExpandY, 4, 4, 3, 1));

   if (dlg_type == kFDOpen) {
      fCheckB->Connect("Toggled(Bool_t)","TGFileContainer",fFc,"SetMultipleSelection(Bool_t)");
      fCheckB->Connect("Toggled(Bool_t)","TGFileContainer",fFc,"UnSelectAll()");
   }
   
   //--- file name and types

   TGHorizontalFrame *fHf = new TGHorizontalFrame(this, 10, 10);

   TGVerticalFrame *fVf = new TGVerticalFrame(fHf, 10, 10);

   TGHorizontalFrame *fHfname = new TGHorizontalFrame(fVf, 10, 10);

   TGLabel *fLfname = new TGLabel(fHfname, new TGHotString("File &name:"));
   fTbfname = new TGTextBuffer(1034);
   fName = new TGTextEntry(fHfname, fTbfname);
   fName->Resize(230, fName->GetDefaultHeight());
   fName->Associate(this);

   fHfname->AddFrame(fLfname, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 5, 2, 2));
   fHfname->AddFrame(fName, new TGLayoutHints(kLHintsRight | kLHintsCenterY, 0, 20, 2, 2));

   fVf->AddFrame(fHfname, new TGLayoutHints(kLHintsLeft | kLHintsCenterY | kLHintsExpandX));

   TGHorizontalFrame *fHftype = new TGHorizontalFrame(fVf, 10, 10);

   TGLabel *fLftypes = new TGLabel(fHftype, new TGHotString("Files of &type:"));
   fTypes = new TGComboBox(fHftype, kIDF_FTYPESLB);
   fTypes->Associate(this);
   fTypes->Resize(230, fName->GetDefaultHeight());

   TString s;
   for (i = 0; fFileInfo->fFileTypes[i] != 0; i += 2) {
      s.Form("%s (%s)", fFileInfo->fFileTypes[i], fFileInfo->fFileTypes[i+1]);
      fTypes->AddEntry(s.Data(), i);
   }
   fTypes->Select(fFileInfo->fFileTypeIdx);

   // Show all items in combobox without scrollbar
   //TGDimension fw = fTypes->GetListBox()->GetContainer()->GetDefaultSize();
   //fTypes->GetListBox()->Resize(fw.fWidth, fw.fHeight);

   if (fFileInfo->fFilename && fFileInfo->fFilename[0])
      fTbfname->AddText(0, fFileInfo->fFilename);
   else {
      fTbfname->Clear();
      if (dlg_type == kFDSave) {
         fTbfname->AddText(0, "unnamed");
         fName->SelectAll();
         if (fFileInfo->fFileTypes[fFileInfo->fFileTypeIdx+1] &&
             strstr(fFileInfo->fFileTypes[fFileInfo->fFileTypeIdx+1], "*.")) {
            TString ext = fFileInfo->fFileTypes[fFileInfo->fFileTypeIdx+1];
            ext.ReplaceAll("*.", ".");
            fTbfname->AddText(7, ext.Data());
         }
         fName->SetFocus();
      }
   }

   fTypes->GetListBox()->Resize(230, 120);
   fHftype->AddFrame(fLftypes, new TGLayoutHints(kLHintsLeft | kLHintsCenterY, 2, 5, 2, 2));
   fHftype->AddFrame(fTypes, new TGLayoutHints(kLHintsRight | kLHintsCenterY, 0, 20, 2, 2));

   fVf->AddFrame(fHftype, new TGLayoutHints(kLHintsLeft | kLHintsCenterY | kLHintsExpandX));

   fHf->AddFrame(fVf, new TGLayoutHints(kLHintsLeft | kLHintsCenterY | kLHintsExpandX));

   //--- Open/Save and Cancel buttons

   TGVerticalFrame *fVbf = new TGVerticalFrame(fHf, 10, 10, kFixedWidth);

   fOk = new TGTextButton(fVbf, new TGHotString((dlg_type == kFDSave)
                                                 ? "&Save" : "&Open"), kIDF_OK);
   fCancel = new TGTextButton(fVbf, new TGHotString("Cancel"), kIDF_CANCEL);

   fOk->Associate(this);
   fCancel->Associate(this);

   fVbf->AddFrame(fOk, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 0, 0, 2, 2));
   fVbf->AddFrame(fCancel, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 0, 0, 2, 2));

   UInt_t width = TMath::Max(fOk->GetDefaultWidth(), fCancel->GetDefaultWidth()) + 20;
   fVbf->Resize(width + 20, fVbf->GetDefaultHeight());

   fHf->AddFrame(fVbf, new TGLayoutHints(kLHintsLeft | kLHintsCenterY));

   AddFrame(fHf, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 4, 4, 3, 1));
   SetEditDisabled(kEditDisable);

   MapSubwindows();

   TGDimension size = GetDefaultSize();

   Resize(size);

   //---- position relative to the parent's window

   CenterOnParent();

   //---- make the message box non-resizable

   SetWMSize(size.fWidth, size.fHeight);
   SetWMSizeHints(size.fWidth, size.fHeight, 10000, 10000, 1, 1);

   const char *wname = (dlg_type == kFDSave) ? "Save As..." : "Open";
   SetWindowName(wname);
   SetIconName(wname);
   SetClassHints("ROOT", "FileDialog");

   SetMWMHints(kMWMDecorAll | kMWMDecorResizeH  | kMWMDecorMaximize |
                              kMWMDecorMinimize | kMWMDecorMenu,
               kMWMFuncAll |  kMWMFuncResize    | kMWMFuncMaximize |
                              kMWMFuncMinimize,
               kMWMInputModeless);

   MapWindow();
   fFc->DisplayDirectory();
   if (dlg_type == kFDSave)
      fName->SetFocus();
   fClient->WaitFor(this);
}









//-------------------------
// TGFileDialog2::TGFileDialog2(const TGWindow* p, const TGWindow* main, EFileDialogMode dlg_type, TGFileInfo* file_info)
//   : TGFileDialog(p,main,dlg_type,file_info) {
//     //cout << "TG1: " << endl;
//     //TGFileDialog(p,main,dlg_type,file_info);
//   cout << "TG2: " << endl;
// }


// void TGFileDialog2::SetWindowName(const char* name) {
//   cout << "TG3: " << name << endl;
// }

