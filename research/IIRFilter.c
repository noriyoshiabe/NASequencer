#include "IIRFilter.h"
#include <math.h>
#include <string.h>

extern void IIRFilterInitialize(IIRFilter *self, double sampleRate)
{
    memset(self, 0, sizeof(IIRFilter));
    self->sampleRate = sampleRate;
}

extern void IIRFilterCalcLPFCoefficient(IIRFilter *self, double frequency, double q)
{
    // Based on Cookbook formulae for audio EQ biquad filter coefficients
    // http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt

    double omega = 2.0 * M_PI * frequency / self->sampleRate;
    double alpha = sin(omega) / (2.0 * q);

    double cos_omega = cos(omega);
    double one_minus_cos_omega = 1.0 - cos_omega;
    double one_minus_cos_omega_div_2 = one_minus_cos_omega / 2.0;

    self->coef.b0 = one_minus_cos_omega_div_2;
    self->coef.b1 = one_minus_cos_omega;
    self->coef.b2 = one_minus_cos_omega_div_2;
    self->coef.a0 = 1.0 + alpha;
    self->coef.a1 = -2.0 * cos_omega;
    self->coef.a2 = 1.0 - alpha;
}

extern AudioSample IIRFilterApply(IIRFilter *self, AudioSample input)
{
    AudioSample output;

    output.L =
          self->coef.b0 / self->coef.a0 * input.L
        + self->coef.b1 / self->coef.a0 * self->history[0].input.L
        + self->coef.b2 / self->coef.a0 * self->history[1].input.L
        - self->coef.a1 / self->coef.a0 * self->history[0].output.L
        - self->coef.a2 / self->coef.a0 * self->history[1].output.L;

    output.R =
          self->coef.b0 / self->coef.a0 * input.R
        + self->coef.b1 / self->coef.a0 * self->history[0].input.R
        + self->coef.b2 / self->coef.a0 * self->history[1].input.R
        - self->coef.a1 / self->coef.a0 * self->history[0].output.R
        - self->coef.a2 / self->coef.a0 * self->history[1].output.R;

    self->history[1].input = self->history[0].input;
    self->history[1].output = self->history[0].output;

    self->history[0].input = input;
    self->history[0].output = output;

    return output;
}
