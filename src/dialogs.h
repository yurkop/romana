#ifndef dialogs_H
#define dialogs_H 1

//--------------------------------------
class TGFileDialog2 : public TGFileDialog {
public:
  TGFileDialog2(const TGWindow *p = 0, const TGWindow *main = 0,
                EFileDialogMode dlg_type = kFDOpen, TGFileInfo *file_info = 0);
  // using TGFileDialog::fTbfname;
  // virtual void SetWindowName(const char* name = 0);
};

#endif
