#ifndef _ESSENTIA_WRAPPER_H_
#define _ESSENTIA_WRAPPER_H_

#ifdef  __cplusplus
    #include "inc/essentia/algorithmfactory.h"
    #include "inc/essentia/pool.h"
    #include "inc/essentia/utils/synth_utils.h"

    using namespace std;
    using namespace essentia;
    using namespace standard;

    class WEssentia {
        private:
            vector<Real> magnitudes;
            vector<Real> frequencies;
            vector<Real> phases;

            vector<complex<Real> >  sfftframe;
            vector<Real> ifftframe;
            vector<Real> audioOutput;

            Algorithm* sinemodelsynth = 0;
            Algorithm* ifft = 0;
            Algorithm* overlapAdd = 0;

            unsigned int sine_model_dlen;

        public:
            WEssentia() {
                essentia::init();
            };

            ~WEssentia() {
                freeSineModel();

                essentia::shutdown();
            };

            void initializeSineModel(double sr, unsigned int framesize, unsigned int hopsize) {
                AlgorithmFactory& factory = AlgorithmFactory::instance();

                sinemodelsynth = factory.create("SineModelSynth", "sampleRate", sr, "fftSize", framesize, "hopSize", hopsize);
                ifft = factory.create("IFFT", "size", framesize);

                overlapAdd = factory.create("OverlapAdd", "frameSize", framesize, "hopSize", hopsize, "gain", 0.00001);

                sinemodelsynth->input("magnitudes").set(magnitudes);
                sinemodelsynth->input("frequencies").set(frequencies);
                sinemodelsynth->input("phases").set(phases);
                sinemodelsynth->output("fft").set(sfftframe);

                ifft->input("fft").set(sfftframe);
                ifft->output("frame").set(ifftframe);

                overlapAdd->input("signal").set(ifftframe);
                overlapAdd->output("signal").set(audioOutput);

                sine_model_dlen = floor(framesize / (hopsize * 2.f));
            };

            void freeSineModel() {
                delete sinemodelsynth;
                delete ifft;
                delete overlapAdd;
            };

            Real* computeSineModel(Real* m, Real* f, Real* p, unsigned int len) {
                frequencies.assign(f, f + len);
                magnitudes.assign(m, m + len);
                phases.assign(p, p + len);

                sinemodelsynth->compute();

                ifft->compute();
                overlapAdd->compute();

                return &audioOutput[0];
            };
    };
#endif

typedef void* CEssentia;

#ifdef __cplusplus
extern "C" {
#endif
    CEssentia newCEssentia();
    void delCEssentia(CEssentia);

    // Sine Model
    void initializeSineModelCEssentia(CEssentia e, double sample_rate, unsigned int framesize, unsigned int hopsize);
    void freeSineModelCEssentia(CEssentia e);
    float* computeSineModelCEssentia(CEssentia e, float* m, float* f, float* p, unsigned int len);
#ifdef __cplusplus
}
#endif

#endif
