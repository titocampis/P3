/// @file

#include <iostream>
#include <fstream>
#include <string.h>
#include <errno.h>
#include <cmath>
#include "ffft/FFTReal.h"

#include "wavfile_mono.h"
#include "pitch_analyzer.h"

#include "docopt.h"

#define FRAME_LEN   0.030 /* 30 ms. */
#define FRAME_SHIFT 0.015 /* 15 ms. */

using namespace std;
using namespace upc;

static const char USAGE[] = R"(
get_pitch - Pitch Detector 

Usage:
    get_pitch [options] <input-wav> <output-txt>
    get_pitch (-h | --help)
    get_pitch --version

Options:
    -h, --help  Show this screen
    --version   Show the version of the project

Arguments:
    input-wav   Wave file with the audio signal
    output-txt  Output file: ASCII file with the result of the detection:
                    - One line per frame with the estimated f0
                    - If considered unvoiced, f0 must be set to f0 = 0
)";

int main(int argc, const char *argv[]) {
	/// \TODO 
	///  Modify the program syntax and the call to **docopt()** in order to
	///  add options and arguments to the program.
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
        {argv + 1, argv + argc},	// array of arguments, without the program name
        true,    // show help if requested
        "2.0");  // version string

	std::string input_wav = args["<input-wav>"].asString();
	std::string output_txt = args["<output-txt>"].asString();

  // Read input sound file
  unsigned int rate;
  vector<float> x;
  if (readwav_mono(input_wav, rate, x) != 0) {
    cerr << "Error reading input file " << input_wav << " (" << strerror(errno) << ")\n";
    return -2;
  }

  int n_len = rate * FRAME_LEN;
  int n_shift = rate * FRAME_SHIFT;

    /// \TODO
  /// Preprocess the input signal in order to ease pitch estimation. For instance,
  /// central-clipping or low pass filtering may be used. ===============================================

  // Define analyzer
  PitchAnalyzer analyzer(n_len, rate, PitchAnalyzer::HAMMING, 50, 500);

  // Real FFT
  ffft::FFTReal <float> fft_object (1024);

  // We make the FFT to x signal
  std::vector<float> X;
  std::vector<float> X_filt;
  std::vector<float> x_filt;
  X.resize(x.size());
  fft_object.do_fft(X.data(), x.data()); // X = fft(x)

  //Fistro 2 Paso Bajo siguiendo wikipedia
  //float alpha = 0.7;
 // for (unsigned int p = 2; p < x.size(); p++)
  //  x[p] = alpha*x[p]+(1-alpha)*x[p-1];

  // Fistro Paso Bajo
/*  std::vector<float> low_pass_filter;
  low_pass_filter.resize(x.size());
  float f_cut = 1000; //Threshold frequency
  int k_cut = (f_cut/rate)*X.size(); //Transform f into k

  // Calculate filter
  for(int n = 0; n < X.size(); n++) {
    if(n < k_cut) low_pass_filter[n] = 1;
    else low_pass_filter[n] = 0;
  }

  //Filter
  for(int n = 0; n < X.size(); n++) X_filt[n] = X[n]*low_pass_filter[n];

  // FFT-1
  fft_object.do_ifft(X_filt.data(), x.data());
  fft_object.rescale(x.data());*/

  //====================================================================================================

  // Iterate for each frame and save values in f0 vector
  vector<float>::iterator iX;
  vector<float> f0;
  for (iX = x.begin(); iX + n_len < x.end(); iX = iX + n_shift) {
    float f = analyzer(iX, iX + n_len);
    f0.push_back(f);
  }

  /// \TODO
  /// Postprocess the estimation in order to supress errors. For instance, a median filter
  /// or time-warping may be used.


  // Write f0 contour into the output file
  ofstream os(output_txt);
  if (!os.good()) {
    cerr << "Error reading output file " << output_txt << " (" << strerror(errno) << ")\n";
    return -3;
  }

  os << 0 << '\n'; //pitch at t=0
  for (iX = f0.begin(); iX != f0.end(); ++iX) 
    os << *iX << '\n';
  os << 0 << '\n';//pitch at t=Dur

  return 0;
}
