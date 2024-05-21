#include <cstdio>  // for printing to stdout
#include <unordered_map>
#include <unordered_set>

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

class CelloString : public al::SynthVoice {
public:
    al::Parameter freq {"freq", "", 440.0, "", 20.0, 5000.0};
    al::Parameter amp {"amp", "", 0.5, "", 0.0, 1.0};

    al::Mesh string;

    gam::Saw<> saw;

    void init() override {
        saw.freq(freq.get());
        string.primitive(al::Mesh::POINTS);
    }

    void onProcess(al::AudioIOData& io) override {

        while(io()) {
            float s = saw() * amp.get();
            io.out(0) += s;
            io.out(1) += s;
        }
        if (!saw.phase()) free();
    }

    // standing wave equation is 2sin(kx)cos(ft)
    void update(double dt) {
        string.reset();
        for (int x = 0; x < 100; x++) {
            string.vertex(x/100.0, 0.5 + 0.5 * sin(2 * M_PI * x * freq.get() / 100.0));
        }
    }

    void onProcess(al::Graphics &g) override {
        g.pushMatrix();
        g.draw(string);
        g.popMatrix();
    }
};

class MyApp : public al::App {

    al::SynthGUIManager<CelloString> synthManager {"cello"};

    al::Mesh bow;
    al::Vec2f bowPos;
public:

    void onCreate() override {
        bow.primitive(al::Mesh::LINE_STRIP);
        bow.vertex(0, 0);
        bow.vertex(100, 0);
    }

    void onAnimate(double dt) override {
        auto newBowPos = al::Vec2f(mouse().x(), height() - mouse().y());
        auto v = (newBowPos - bowPos) / dt;

        

        bowPos = newBowPos;
    }

    void onDraw(al::Graphics &g) override {
        g.camera(al::Viewpoint::ORTHO_FOR_2D);
        g.clear(0);
        g.lineWidth(2);
        g.depthTesting(true);
        g.pointSize(8);
        // g.nicest();
        // g.stroke(8);
        
        g.pushMatrix();
        g.translate(bowPos);
        g.color(1);
        g.draw(bow);
        g.popMatrix();
    }

    bool onKeyDown(const al::Keyboard &k) override {
        return true;
    }

};

int main() {
  MyApp app;
  app.start();
  return 0;
}