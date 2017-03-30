#ifndef chandlg_H
#define chandlg_H 1

#include <TGFrame.h>
#include <TGLabel.h>
#include <TGTextEntry.h>
#include <TGColorSelect.h>



//-----------------------------------------------
class ChanDlg {

protected:
   TGTransientFrame     *fMain;
   TGHorizontalFrame    *fFr1;
   TGVerticalFrame      *fFr2[2];
   TGHorizontalFrame    *fHor3;
   TGHorizontalFrame    *fHor4;

   TGHorizontalFrame    *fHor[MAX_CH+2];
   TGLayoutHints        *fLay1,*fLay2,*fLay3,*fLay4;

   TGLabel              *fLabel[2];
   TGLabel              *fLab2[MAX_CH+1];
   TGLabel              *fLab4[MAX_L];
   TGRadioButton        *fR[MAX_R*(MAX_CH+1)];
   TGCheckButton        *fChk[(MAX_CH+1)];
   TGTextEntry          *fName[MAX_CH+1];
   TGColorSelect        *fColor[MAX_CH+1];
   TGColorSelect        *fLColor[MAX_L];

public:
   ChanDlg(const TGWindow *p, const TGWindow *main, const char* title);
   virtual ~ChanDlg();

   void Map();
   void AddColumn(int);
   void AddLine(int, TGVerticalFrame*);
   void FillLine(int, TGHorizontalFrame*);
   void FillAllCh(int, TGHorizontalFrame*);
   //void FillCuts();
   //void FillLast(int, TGHorizontalFrame*);
   void DoRadio();
   void DoChk();
   void DoSetName();
   void DoColor();
   void CloseWindow();
   void DoOK();

   ClassDef(ChanDlg, 0)
};

#endif
