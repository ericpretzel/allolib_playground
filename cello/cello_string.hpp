#pragma once

#include "Gamma/Analysis.h"
#include "Gamma/Effects.h"
#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"

#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/scene/al_PolySynth.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_ParameterGUI.hpp"
#include "al/ui/al_Parameter.hpp"

#include "al/sound/al_Reverb.hpp"

#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_Font.hpp"

#include "my_app.hpp"

static float stringLength = 250.0f;

class CelloString {
public:
    CelloString(MyApp *app, float baseFreq, al::Vec2f pos) : app(app), baseFreq(baseFreq), pos(pos) {
        saw.freq(baseFreq);
        vibrato.freq(5.f);
        mesh.primitive(al::Mesh::LINE_STRIP);
        for (int i = 0; i < 50; ++i) {
            mesh.vertex(0, i * stringLength / 50.f);
        }
        mesh.colorFill(al::HSV(1));

        lpf.type(gam::LOW_PASS);
        lpf.freq(1200);
        lpf.res(4);

        hpf.type(gam::HIGH_PASS);
        hpf.freq(1000);
        hpf.res(4);

        adsrEnv.levels(0, 2, 1, 0);
        adsrEnv.sustainPoint(2);
        adsrEnv.lengths()[0] = 0.01;
        adsrEnv.lengths()[1] = 0.1;
        adsrEnv.lengths()[2] = 0;
        adsrEnv.lengths()[3] = 1.0;
        adsrEnv.finish();

        // from allolib reverb example
        reverb.bandwidth(0.6f); // Low-pass amount on input, in [0,1]
        reverb.damping(0.5f);   // High-frequency damping, in [0,1]
        reverb.decay(0.6f);     // Tail decay factor, in [0,1]

        // Diffusion amounts
        // Values near 0.7 are recommended. Moving further away from 0.7 will lead
        // to more distinct echoes.
        reverb.diffusion(0.76, 0.666, 0.707, 0.571);
    }

    void process(al::AudioIOData &io);
    void draw(al::Graphics &g);
    void update(float dt);
    void lift_finger(int finger);
    void press_finger(int finger);

    void set_filter_freq(float freq) {
        lpf.freq(freq);
        hpf.freq(freq);
    }
private:
    MyApp *app;
    float baseFreq;
    al::Mesh mesh;
    al::Vec2f pos = {0.f, 0.f};
    gam::Saw<> saw;
    gam::Sine<> vibrato;
    gam::Env<4> adsrEnv;

    gam::Biquad<> lpf;
    gam::Biquad<> hpf;

    gam::NoisePink<> noise;

    al::Reverb<float> reverb;

    bool being_played = false;

    bool fingers[9] = {true, false, false, false, false, false, false, false, false};
};