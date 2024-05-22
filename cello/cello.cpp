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

static float bowLength = 100.0f;
static float stringLength = 250.0f;

class CelloString {

};

class MyApp : public al::App {

    al::Mesh bow;
    al::Vec2f bowPos;
    al::Vec2f avgBowVel = {0, 0};

    al::Mesh aString;

    bool playing;

    float amp = 0.1f;
    float freq = 440.0f;
    gam::Saw<> saw;

public:

    void onCreate() override {
        bow.primitive(al::Mesh::LINE_STRIP);
        bow.vertex(0, 0);
        bow.vertex(bowLength, 0);

        aString.primitive(al::Mesh::LINE_STRIP);
        aString.vertex(0, 0);
        aString.vertex(0, stringLength);

        saw.freq(freq);
    }

    void onAnimate(double dt) override {
        auto newBowPos = al::Vec2f(mouse().x(), height() - mouse().y());
        auto v = (newBowPos - bowPos) / dt;
        avgBowVel = 0.9f * avgBowVel + 0.1f * v;

        bowPos = newBowPos;
    }

    void onSound(al::AudioIOData &io) override {
        while (io()) {
            if (playing) {
                std::cout << "avgBowVel: " << avgBowVel.mag() << std::endl;
                float s = saw() * amp;
                io.out(0) = s;
                io.out(1) = s;
            }
        }
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

        g.pushMatrix();
        g.translate(width() / 2, height() / 4);
        g.color(1);
        g.draw(aString);
        g.popMatrix();
    }

    bool onKeyDown(const al::Keyboard &k) override {
        return true;
    }

    bool onMouseDrag(const al::Mouse &m) override {
        if (m.left() && m.x() + bowLength > width() / 2 && m.x() < width() / 2){
            playing = true;
        } else {
            playing = false;
        }

        return true;
    }

    bool onMouseMove(const al::Mouse &m) override {
        if (m.left() && m.x() + bowLength > width() / 2 && m.x() < width() / 2){
            playing = true;
        } else {
            playing = false;
        }
        return true;
    }

};

int main() {
  MyApp app;
  app.start();
  return 0;
}