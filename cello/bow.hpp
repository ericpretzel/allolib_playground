#pragma once

#include <cstdio>  // for printing to stdout
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

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

static float bowLength = 400.0f;

class MyApp;

class Bow {
public:
    friend class CelloString;
    Bow(MyApp *app) : app(app) {
        mesh.primitive(al::Mesh::LINE_STRIP);
        mesh.vertex(0, 0);
        mesh.vertex(bowLength, 0);
        mesh.colorFill(1);
    }

    void draw(al::Graphics &g);

    void update(float dt);
    bool attacking = true;
    MyApp *app;
    al::Mesh mesh;
    al::Vec2f pos{0.f, 0.f};
    al::Vec2f avgVel{0.f, 0.f};
    bool down = false;
    bool moving = false;
};