/// @file

#include <iostream>
#include <math.h>
#include "pitch_analyzer.h"
#include <cmath>
#include "ffft/FFTReal.h"

using namespace std;

/// Name space of UPC
namespace upc {
  void PitchAnalyzer::autocorrelation(const vector<float> &x, vector<float> &r) const {

    for (unsigned int l = 0; l < r.size(); ++l) {
  		/// \TODO Compute the autocorrelation r[l]
      for(unsigned int n = 0; n < x.size()-l; n++) r[l] += x[n]*x[n+l];
      r[l] = r[l]/(r.size()-l);
    }
  //  r = r/r.size();
    if (r[0] == 0.0F) //to avoid log() and divide zero 
      r[0] = 1e-10; 
  }

  void PitchAnalyzer::set_window(Window win_type) {
    if (frameLen == 0)
      return;

    window.resize(frameLen);

    switch (win_type) {
    case HAMMING:
      /// \TODO Implement the Hamming window
      for(int i = 0; i < frameLen; i++) 
        window[i] = 0.54 - 0.46*cos((2*M_PI*i)/(frameLen-1));
      break;
    case RECT:
      // Rectangular Window case
      window.assign(frameLen, 1);
    default:
      // Rectangular Window too
      window.assign(frameLen, 1);
    }
  }

  void PitchAnalyzer::set_f0_range(float min_F0, float max_F0) {
    npitch_min = (unsigned int) samplingFreq/max_F0;
    if (npitch_min < 2)
      npitch_min = 2;  // samplingFreq/2

    npitch_max = 1 + (unsigned int) samplingFreq/min_F0;

    //frameLen should include at least 2*T0
    if (npitch_max > frameLen/2)
      npitch_max = frameLen/2;
  }

  bool PitchAnalyzer::unvoiced(float pot, float r1norm, float rmaxnorm) const {
    /// \TODO Implement a rule to decide whether the sound is voiced or not.
    /// * You can use the standard features (pot, r1norm, rmaxnorm),
    ///   or compute and use other ones.
    if ((pot > -30 && r1norm > 0.85) || rmaxnorm > 0.63) return false;
    else return true;
  }

  float PitchAnalyzer::compute_pitch(vector<float> & x) const {
    if (x.size() != frameLen)
      return -1.0F;

    //Window input frame
    for (unsigned int i=0; i<x.size(); ++i)
      x[i] *= window[i];

    vector<float> r(npitch_max);

    //Compute correlation
    autocorrelation(x, r);

    vector<float>::const_iterator iR = r.begin(), iRMax = iR;

    /// \TODO 
	/// Find the lag of the maximum value of the autocorrelation away from the origin.<br>
	/// Choices to set the minimum value of the lag are:
	///    - The first negative value of the autocorrelation.
	///    - The lag corresponding to the maximum value of the pitch.
    ///	   .
	/// In either case, the lag should not exceed that of the minimum value of the pitch.

    unsigned int lag = iRMax - r.begin();
    float pot = 10 * log10(r[0]);

    float maximum = 0;
    // Buscamos los periodos de repetición entre los cuales estará nuestra señal
    // Es decir entre los valores que encontraremos nuestro pitch expresados en r[l]})
    int max_l = samplingFreq/75;
    int min_l = samplingFreq/500;

    for (int i = min_l; i <= max_l; i++) {
      if (r[i] >= maximum) {
        lag = i;
        maximum = r[i];
      }
    }
    //You can print these (and other) features, look at them using wavesurfer
    //Based on that, implement a rule for unvoiced
    //change to #if 1 and compile
#if 0
    if (r[0] > 0.0F)
      cout << pot << '\t' << r[1]/r[0] << '\t' << r[lag]/r[0] << endl;
#endif
    
    if (unvoiced(pot, r[1]/r[0], r[lag]/r[0]))
      return 0;
    else
      return (float) samplingFreq/(float) lag;
  }
}
