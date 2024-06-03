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
#include "al/ui/al_ControlGUI.hpp"

#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_Font.hpp"

#include "bow.hpp"
#include "cello_string.hpp"

static std::unordered_map<char, int> keyToIndex = {
    // for A string
    {'1', 0},
    {'2', 1},
    {'3', 2},
    {'4', 3},
    {'5', 4},
    {'6', 5},
    {'7', 6},
    {'8', 7},

    // for D string
    {'q', 8},
    {'w', 9},
    {'e', 10},
    {'r', 11},
    {'t', 12},
    {'y', 13},
    {'u', 14},
    {'i', 15},

    // for G string
    {'a', 16},
    {'s', 17},
    {'d', 18},
    {'f', 19},
    {'g', 20},
    {'h', 21},
    {'j', 22},
    {'k', 23},

    // for C string
    {'z', 24},
    {'x', 25},
    {'c', 26},
    {'v', 27},
    {'b', 28},
    {'n', 29},
    {'m', 30},
    {',', 31},
};

class MyApp : public al::App {
public:
    al::ControlGUI gui;
    al::Parameter filter_freq {"filter_freq", "", 2000.0f, "", 20.0f, 20000.0f};
    Bow *bow;
    std::vector<CelloString> strings;

    void onCreate() override;
    void onAnimate(double _dt) override;
    void onSound(al::AudioIOData &io) override;
    void onDraw(al::Graphics &g) override;

    bool onKeyDown(const al::Keyboard &k) override;
    bool onKeyUp(const al::Keyboard &k) override;

    void onExit() override;
};