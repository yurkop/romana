#pragma link C++ class MainFrame + ;
#pragma link C++ class MyMainFrame + ;
#pragma link C++ class EventFrame + ;
#pragma link C++ class HistFrame + ;
#pragma link C++ class ErrFrame + ;
// #pragma link C++ class PopFrame+;

// #pragma link C++ class SockClass+;

#pragma link C++ class ParDlg + ;
#pragma link C++ class ParParDlg + ;
#pragma link C++ class HistParDlg + ;
#pragma link C++ class ChanParDlg + ;
#pragma link C++ class CRS + ;
#pragma link C++ class HMap + ;
#pragma link C++ class HClass + ;
// #pragma link C++ class rpeak_type+;
// #pragma link C++ class vector<rpeak_type73>+;
// #pragma link C++ class Hbuf+;
#pragma link C++ class Hdef + ;
// #pragma link C++ class HCuts+;
#pragma link C++ class Coptions + ;
#pragma link C++ class Toptions + ;
#pragma link C++ class PEditor + ;
// #pragma link C++ class Hoptions+;
#pragma link C++ class Mdef + ;

#pragma link C++ class TGMatrixLayout2;

#pragma link C++ function DynamicExec;

// ========== I/O Rules for schema evolution ==========
#ifdef __CINT__

// Prof64_W: Int_t[3] → Float_t[3]
#pragma read sourceClass="Toptions" version="[1,184]" \
         targetClass="Toptions" \
         source="Int_t Prof64_W[3]" target="Prof64_W" \
         code="{ Prof64_W[0] = (Float_t)onfile.Prof64_W[0]; \
                 Prof64_W[1] = (Float_t)onfile.Prof64_W[1]; \
                 Prof64_W[2] = (Float_t)onfile.Prof64_W[2]; }"

// Prof64_THR: Int_t → Float_t
#pragma read sourceClass="Toptions" version="[1,184]" \
         targetClass="Toptions" \
         source="Int_t Prof64_THR" target="Prof64_THR" \
         code="{ Prof64_THR = (Float_t)onfile.Prof64_THR; }"

// Prof64_GAT: Int_t → Float_t
#pragma read sourceClass="Toptions" version="[1,184]" \
         targetClass="Toptions" \
         source="Int_t Prof64_GAT" target="Prof64_GAT" \
         code="{ Prof64_GAT = (Float_t)onfile.Prof64_GAT; }"

// Prof64_X: Int_t → Float_t
#pragma read sourceClass="Toptions" version="[1,184]" \
         targetClass="Toptions" \
         source="Int_t Prof64_X" target="Prof64_X" \
         code="{ Prof64_X = (Float_t)onfile.Prof64_X; }"

// Prof64_Y: Int_t → Float_t
#pragma read sourceClass="Toptions" version="[1,184]" \
         targetClass="Toptions" \
         source="Int_t Prof64_Y" target="Prof64_Y" \
         code="{ Prof64_Y = (Float_t)onfile.Prof64_Y; }"

#endif