#include "IIRFilter.h"
#include <math.h>

extern void IIRFilterCalcLPFCoefficient(IIRFilter *self, double sampleRate, int16_t frequency_cent, int16_t q_cB)
{
    // 8.1.2 Generator Enumerators Defined
    // 8 initialFilterFc
    // When the cutoff frequency exceeds 20kHz and the Q (resonance) of the filter is zero,
    // the filter does not affect the signal.
    //
    // Actually, Hz of 13500 cent is approximately 19912 Hz,
    // but initialFilterFc default value is 13500 as 20kHz.
    // Even if it doesn't exceed 20kHz, default value should not affect the signal.
    if (13500 <= frequency_cent && 0 >= q_cB) {
        self->coef.b1 = 0.0;
        self->coef.b02 = 1.0;
        self->coef.a1 = 0.0;
        self->coef.a2 = 0.0;
        return;
    }

    // Convert cent to Hz
    double frequency = 440.0 * pow(2.0, (frequency_cent - 6900.0) / 1200.0);

    // Convert centibel Q to linear Q
    double q_dB = (double)q_cB / 10.0f;
    double q = pow(10.0f, q_dB / 20.0f);

    // -- The idea from here is based on Cookbook formulae for audio EQ biquad filter coefficients
    // http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt

    double omega = 2.0 * M_PI * frequency / sampleRate;
    double alpha = sin(omega) / (2.0 * q);

    double cos_omega = cos(omega);
    double one_minus_cos_omega = 1.0 - cos_omega;
    double one_minus_cos_omega_div_2 = one_minus_cos_omega / 2.0;

    // Pre normalization to reduce calculation
    double a0_inverter = 1.0 + alpha;

    self->coef.b1 = one_minus_cos_omega / a0_inverter;
    self->coef.b02 = one_minus_cos_omega_div_2 / a0_inverter;
    self->coef.a1 = (-2.0 * cos_omega) / a0_inverter;
    self->coef.a2 = (1.0 - alpha) / a0_inverter;

    // -- The idea up to here is Cookbook formulae for audio EQ biquad filter coefficients

    // 9.1.3 Low-pass Filter
    // The DC gain at any resonance is half of the resonance value
    // below the DC gain at zero resonance;
    // hence the peak height is half the resonance value above DC gain at zero resonance.
    double filter_gain = 1.0 / sqrt(q);
    self->coef.b1 *= filter_gain;
}

extern AudioSample IIRFilterApply(IIRFilter *self, AudioSample input)
{
    AudioSample output;

    output.L =
          self->coef.b02 * input.L
        + self->coef.b1  * self->history[0].input.L
        + self->coef.b02 * self->history[1].input.L
        - self->coef.a1  * self->history[0].output.L
        - self->coef.a2  * self->history[1].output.L;

    output.R =
          self->coef.b02 * input.R
        + self->coef.b1  * self->history[0].input.R
        + self->coef.b02 * self->history[1].input.R
        - self->coef.a1  * self->history[0].output.R
        - self->coef.a2  * self->history[1].output.R;

    self->history[1].input = self->history[0].input;
    self->history[1].output = self->history[0].output;

    self->history[0].input = input;
    self->history[0].output = output;

    return output;
}
