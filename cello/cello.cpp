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

static float bowLength = 300.0f;
static float stringLength = 250.0f;

class CelloString {

};

class MyApp : public al::App {

    al::Mesh bow;
    al::Vec2f bowPos = {0.f, 0.f};
    al::Vec2f avgBowVel = {0.f, 0.f};

    bool fingers[7] = {true, false, false, false, false, false, false};

    al::Mesh aString;

    bool bowDown = false;
    bool bowMoving = false;

    float amp = 0.1f;
    float freq = 440.0f;
    gam::Saw<> saw;

    gam::Sine<> vibrato;

public:

    void onCreate() override {
        bow.primitive(al::Mesh::LINE_STRIP);
        bow.vertex(0, 0);
        bow.vertex(bowLength, 0);

        aString.primitive(al::Mesh::LINE_STRIP);
        aString.vertex(0, 0);
        aString.vertex(0, stringLength);

        vibrato.freq(5.f);

        saw.freq(freq);
    }

    void onAnimate(double _dt) override {
        float dt = _dt;
        if (dt == 0) return;
        auto newBowPos = al::Vec2f(float(mouse().x()), float(height()) - mouse().y());
        auto v = (newBowPos - bowPos) / dt;
        avgBowVel = avgBowVel * 0.9 + v * 0.1;
        if (avgBowVel.mag() > 0.1f) {
            bowMoving = true;
        } else {
            bowMoving = false;
        }
        bowPos = newBowPos;
    }

    void onSound(al::AudioIOData &io) override {
        while (io()) {
            if (bowDown && bowMoving) {
                saw.freqAdd(vibrato() * 10.0f);
                float s = saw() * amp * avgBowVel.mag() * 0.01f;
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
        if (k.key() < '1' || k.key() > '6') return true;
        fingers[k.key() - '0'] = true;
        for (int i = 6; i >= 0; --i) {
            if (fingers[i]) {
                saw.freq(440.0f * powf(2.0f, i / 12.0f));
                break;
            }
        }
        return true;
    }

    bool onKeyUp(const al::Keyboard &k) override {
        if (k.key() < '1' || k.key() > '6') return true;
        fingers[k.key() - '0'] = false;
        for (int i = 6; i >= 0; --i) {
            if (fingers[i]) {
                saw.freq(440.0f * powf(2.0f, i / 12.0f));
                break;
            }
        }
        return true;
    }

    bool onMouseDrag(const al::Mouse &m) override {
        if (m.left() && m.x() + bowLength > width() / 2 && m.x() < width() / 2){
            bowDown = true;
        } else {
            bowDown = false;
        }
        return true;
    }

    bool onMouseMove(const al::Mouse &m) override {
        if (m.left() && m.x() + bowLength > width() / 2 && m.x() < width() / 2){
            bowDown = true;
        } else {
            bowDown = false;
        }
        return true;
    }

    bool onMouseUp(const al::Mouse &m) override {
        bowDown = false;
        return true;
    }

};

int main() {
  MyApp app;
  app.start();
  return 0;
}