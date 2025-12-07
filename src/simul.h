#ifndef Simul_H
#define Simul_H 1

class Simul {
public:
  Simul();
  double Pshape_Gaus(int j, double pos);
  double Pshape_RC(int j, double pos);
  double Pshape_Fourier(int j, double pos);
  void SimNameHist();
  void SimulatePulse(int ch, Long64_t tst, double pos);
  void SimulateOneEvent(Long64_t Tst);
  void SimulateEvents(Long64_t n_evts, Long64_t Tst0);
};

#endif
