#include "peditor.h"
#include "histframe.h"
#include "libcrs.h"
#include <sstream>

extern Toptions opt;
extern MyMainFrame* myM;
extern CRS* crs;
extern HistFrame* HiFrm;
extern char startdir[200];

PEditor::PEditor(const TGWindow *main, MENU_COM mn, UInt_t w, UInt_t h)
{
  // Create an editor in a dialog.

  menu_id=mn;

  fMain = new TGTransientFrame(gClient->GetRoot(), main, w, h);
  fMain->Connect("CloseWindow()", "PEditor", this, "CloseWindow()");
  fMain->DontCallClose(); // to avoid double deletions.

  // use hierarchical cleaning
  fMain->SetCleanup(kDeepCleanup);

  fEdit = new TGTextEdit(fMain, w, h, kSunkenFrame | kDoubleBorder);
  fMain->AddFrame(fEdit, new TGLayoutHints(kLHintsExpandX|kLHintsExpandY,1,1,1,1));
  fEdit->Connect("Opened()", "PEditor", this, "DoOpen()");
  fEdit->Connect("Saved()",  "PEditor", this, "DoSave()");
  fEdit->Connect("Closed()", "PEditor", this, "DoClose()");

  // set selected text colors
  Pixel_t pxl;
  gClient->GetColorByName("#3399ff", pxl);
  fEdit->SetSelectBack(pxl);
  fEdit->SetSelectFore(TGFrame::GetWhitePixel());

  // TGTextButton* fRead = new TGTextButton(fMain, "  &Read  ");
  // fRead->Connect("Clicked()", "PEditor", this, "DoOpen()");
  // fL2 = new TGLayoutHints(kLHintsBottom | kLHintsCenterX, 0, 0, 5, 5);
  // fMain->AddFrame(fRead, fL2);

  switch (menu_id) {
  case M_EDIT_PROF8:
    LoadPar8();
    break;
  case M_EDIT_PROF64:
    LoadPar64();
    break;
  case M_EDIT_CUTG:
    LoadCuts();
    break;
  default:
    ;
  }

  TGLayoutHints* fLCB5 = new TGLayoutHints(kLHintsCenterX|kLHintsBottom, 5, 5, 0, 0);

  TGHorizontalFrame* fHor = new TGHorizontalFrame(fMain);
  fMain->AddFrame(fHor, new TGLayoutHints(kLHintsCenterX|kLHintsBottom, 0, 0, 5, 5));

  if (menu_id==M_EDIT_PROF64) {
    TGTextButton* fTC = new TGTextButton(fHor, "  &Ttime Calibration  ");
    fTC->Connect("Clicked()", "PEditor", this, "DoTCalibr()");
    fHor->AddFrame(fTC, fLCB5);
  }

  TGTextButton* fSave = new TGTextButton(fHor, "  &Save  ");
  fSave->Connect("Clicked()", "PEditor", this, "DoSavePar()");
  fHor->AddFrame(fSave, fLCB5);

  TGTextButton* fExit = new TGTextButton(fHor, "  Save && &Exit  ");
  fExit->Connect("Clicked()", "PEditor", this, "DoPExit()");
  fHor->AddFrame(fExit, fLCB5);

  TGTextButton* fCancel= new TGTextButton(fHor, "  &Cancel  ");
  fCancel->Connect("Clicked()", "PEditor", this, "CloseWindow()");
  fHor->AddFrame(fCancel, fLCB5);

  SetTitle();

  fMain->MapSubwindows();
  fMain->Resize();
  fMain->CenterOnParent(kTRUE, TGTransientFrame::kRight);
  fMain->MapWindow();

  // editor covers right half of parent window
}

PEditor::~PEditor()
{
  // Delete editor dialog.

  fMain->DeleteWindow();  // deletes fMain
  //delete this;
}

void PEditor::SetTitle()
{
  // Set title in editor window.

  TGText *txt = GetEditor()->GetText();
  Bool_t untitled = !strlen(txt->GetFileName()) ? kTRUE : kFALSE;

  char title[256];
  if (untitled)
    sprintf(title, "Channel map");
  else
    sprintf(title, "%s", txt->GetFileName());

  fMain->SetWindowName(title);
  fMain->SetIconName(title);
}

void PEditor::Popup()
{
  // Show editor.

  fMain->MapWindow();
}

// void PEditor::LoadBuffer(const char *buffer)
// {
//   // Load a text buffer in the editor.

//   fEdit->LoadBuffer(buffer);
// }

void PEditor::LoadFile(const char *file)
{
  // Load a file in the editor.
  fEdit->LoadFile(file);
}

void PEditor::Load_Ing(const char* header)
{
  char ss[100];
  fEdit->LoadBuffer(header);
  fEdit->AddLine("# N: Ing27 strip number");
  fEdit->AddLine("# X-ch: DAQ channel for the given X-strip");
  fEdit->AddLine("# Y-ch: DAQ channel for the given Y-strip");
  fEdit->AddLine("# Set to -1 if the strip is absent/not used");
  fEdit->AddLine("# Ing  N X-ch Y-ch");
  for (int i=0;i<16;i++) {
    sprintf(ss,"Ing  %2d %2d %2d",i,opt.Ing_x[i],opt.Ing_y[i]);
    fEdit->AddLine(ss);
  }
}

void PEditor::LoadPar8()
{
  char ss[100];
  Load_Ing("# Settings for 8x8 profilometer");
  fEdit->AddLine("# Prof N X-ch Y-ch");
  //fEdit->AddLine("#");
  for (int i=0;i<8;i++) {
    sprintf(ss,"Prof %2d %2d %2d",i,opt.Prof_x[i],opt.Prof_y[i]);
    fEdit->AddLine(ss);
  }
}

void PEditor::LoadPar64()
{
  char ss[100];
  Load_Ing("# Settings for 64x64 profilometer");
  fEdit->AddLine("");
  fEdit->AddLine("# Prof64:");
  fEdit->AddLine("# Start channel (Analysis/St) must be the \"threshold\" output");

  fEdit->AddLine("");
  fEdit->AddLine("# Prof64: four channels for Prof64 position signals");
  //fEdit->AddLine("#");
  sprintf(ss,"Prof64 %d # X P+(33-64)",opt.Prof64[0]);
  fEdit->AddLine(ss);
  sprintf(ss,"Prof64 %d # X P+(1-32)",opt.Prof64[1]);
  fEdit->AddLine(ss);
  sprintf(ss,"Prof64 %d # Y N+(33-64)",opt.Prof64[2]);
  fEdit->AddLine(ss);
  sprintf(ss,"Prof64 %d # Y N+(1-32)",opt.Prof64[3]);
  fEdit->AddLine(ss);
  sprintf(ss,"Prof64_T %d # Channel for clk output",opt.Prof64[4]);
  fEdit->AddLine(ss);

  fEdit->AddLine("");
  sprintf(ss,"Prof64_TSP %s # Prof64 clock spectrum",opt.Prof64_TSP);
  fEdit->AddLine(ss);
  sprintf(ss,"Prof64_PER %d # Period (in samples)",opt.Prof64_W[0]);
  fEdit->AddLine(ss);
  sprintf(ss,"Prof64_OFF %d # Time offset of the plateau",opt.Prof64_W[1]);
  fEdit->AddLine(ss);
  sprintf(ss,"Prof64_WID %d # Time width of the plateau",opt.Prof64_W[2]);
  fEdit->AddLine(ss);
  sprintf(ss,"Prof64_THR %d # Threshold",opt.Prof64_THR);
  fEdit->AddLine(ss);
  
  fEdit->AddLine("");
  fEdit->AddLine("# Run Profilometer time calibration (TCalibr)");
  fEdit->AddLine("# for period determination");
  fEdit->AddLine("# Prof64_TSP spectrum: start - threshold, stop - clk");
}

void PEditor::LoadCuts()
{
  string str = HiFrm->CutsToStr();
  fEdit->LoadBuffer(str.c_str());
}

void PEditor::CloseWindow()
{
  // Called when closed via window manager action.

  delete this;
  //myM->p_ed=0;
}

void PEditor::DoTCalibr() {
  new PopFrame(myM,600,600,M_PROF_TIME,this);
}

void PEditor::DoSavePar() {
  switch (menu_id) {
  case M_EDIT_PROF8:
  case M_EDIT_PROF64:
    DoSaveProf();
    break;
  case M_EDIT_CUTG:
    //LoadCuts();
    break;
  default:
    ;
  }
}

void PEditor::DoSaveProf() {
  // Handle Save button.

  TGText* tgt = fEdit->GetText();
  //cout << tgt->RowCount() << endl;
  int kk=0;
  for (int i=0;i<tgt->RowCount();i++) {
    char* chr = tgt->GetLine(TGLongPosition(0,i),100);
    if (chr) {
      std::stringstream ss(chr);
      TString ts,tj;
      int j,xx,yy;
      ss >> ts >> tj >> xx >> yy;
      j=tj.Atoi();
      //cout << i << " " << chr << " " << ts << " " << a << " " << b << " " << c << endl;
      ts.ToLower();
      delete[] chr;
      if (ts.EqualTo("ing") && j>=0 && j<16) {
	opt.Ing_x[j]=xx;
	opt.Ing_y[j]=yy;
      }
      else if (ts.EqualTo("prof") && j>=0 && j<8) {
	opt.Prof_x[j]=xx;
	opt.Prof_y[j]=yy;
      }
      else if (ts.EqualTo("prof64") && kk>=0 && kk<4) {
	opt.Prof64[kk]=j;
	++kk;
      }
      else if (ts.EqualTo("prof64_t")) {
	opt.Prof64[4]=j;
      }
      else if (ts.EqualTo("prof64_tsp")) {
	strcpy(opt.Prof64_TSP,tj.Data());
      }
      else if (ts.EqualTo("prof64_per")) {
	opt.Prof64_W[0]=j;
      }
      else if (ts.EqualTo("prof64_off")) {
	opt.Prof64_W[1]=j;
      }
      else if (ts.EqualTo("prof64_wid")) {
	opt.Prof64_W[2]=j;
      }
      else if (ts.EqualTo("prof64_thr")) {
	opt.Prof64_THR=j;
      }
    }
  }

  crs->Make_prof_ch();
}

void PEditor::DoPExit()
{
  // Handle Save&Exit button.

  DoSaveProf();
  CloseWindow();
}

void PEditor::DoOpen()
{
  SetTitle();
#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif
}

void PEditor::DoSave()
{
  SetTitle();
#ifdef LINUX
  if (chdir(startdir)) {}
#else
  _chdir(startdir);
#endif
}

void PEditor::DoClose()
{
  // Handle close button.

  CloseWindow();
}
