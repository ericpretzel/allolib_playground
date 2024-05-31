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

#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_Font.hpp"

#include "my_app.hpp"

static float stringLength = 250.0f;

class CelloString {
public:
    CelloString(MyApp *app, float baseFreq, al::Vec2f pos) : app(app), baseFreq(baseFreq), pos(pos) {
        saw.freq(baseFreq);
        mesh.primitive(al::Mesh::LINE_STRIP);
        mesh.vertex(0, 0);
        mesh.vertex(0, stringLength);
        mesh.colorFill(1);
    }

    void process(al::AudioIOData &io);
    void draw(al::Graphics &g);
    void update(float dt);
    void lift_finger(int finger);
    void press_finger(int finger);
private:
    MyApp *app;
    float baseFreq;
    al::Mesh mesh;
    al::Vec2f pos = {0.f, 0.f};
    gam::Saw<> saw;
    gam::Sine<> vibrato;
    bool vibrating;

    bool fingers[9] = {true, false, false, false, false, false, false, false, false};
};