#include <cstdio>  // for printing to stdout
#include <unordered_map>
#include <unordered_set>
#include <set>
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
#include "bow.hpp"
#include "cello_string.hpp"

void MyApp::onCreate() {
    bow = new Bow(this);
    strings.push_back(CelloString(this, 220.0f, al::Vec2f(width() / 4, height() / 2 - 50))); // A3
    strings.push_back(CelloString(this, 146.8f, al::Vec2f(width() / 4 + 100, height() / 2 - 100))); // D3
    strings.push_back(CelloString(this, 98.0f, al::Vec2f(width() / 4 + 200, height() / 2 - 150))); // G2
    strings.push_back(CelloString(this, 65.41f, al::Vec2f(width() / 4 + 300, height() / 2 - 200))); // C2
}

void MyApp::onExit() {
    delete bow;
}

void MyApp::onAnimate(double _dt) {
    float dt = _dt;
    if (dt == 0) return;
    bow->update(dt);
    for (auto &s : strings) {
        s.update(dt);
    }
}

void MyApp::onSound(al::AudioIOData &io) {
    while (io()) {
        for (auto &s : strings) {
            s.process(io);
        }
    }
}

void MyApp::onDraw(al::Graphics &g) {
    g.camera(al::Viewpoint::ORTHO_FOR_2D);
    g.clear(0);
    g.lineWidth(2);
    g.depthTesting(true);
    g.pointSize(8);
    // g.nicest();
    // g.stroke(8);
    g.meshColor();
    
    bow->draw(g);
    for (auto &s : strings) {
        s.draw(g);
    }
}

bool MyApp::onKeyDown(const al::Keyboard &k) {
    auto it = keyToIndex.find(k.key());
    if (it != keyToIndex.end()) {
        strings[it->second / 8].press_finger((it->second % 8) + 1);
    }
    return true;
}

bool MyApp::onKeyUp(const al::Keyboard &k) {
    auto it = keyToIndex.find(k.key());
    if (it != keyToIndex.end()) {
        strings[it->second / 8].lift_finger((it->second % 8) + 1);
    }
    return true;
}

void CelloString::process(al::AudioIOData &io) {
    if (!vibrating) return;
    saw.freqAdd(vibrato() * 10.0f);
    float s = saw() * 0.1f;
    io.out(0) += s;
    io.out(1) += s;
}

void CelloString::draw(al::Graphics &g) {
    g.pushMatrix();
    g.translate(pos);
    g.draw(mesh);
    g.popMatrix();
}

void CelloString::update(float dt) {
    vibrating = app->bow->down && 
        app->bow->moving && 
        app->bow->pos.x + bowLength > pos.x && 
        app->bow->pos.x < pos.x &&
        app->bow->pos.y > pos.y && 
        app->bow->pos.y < pos.y + stringLength;
}

void CelloString::lift_finger(int finger) {
    fingers[finger] = false;
    for (int i = 8; i >= 0; --i) {
        if (fingers[i]) {
            saw.freq(baseFreq * powf(2.0f, i / 12.0f));
            break;
        }
    }
}

void CelloString::press_finger(int finger) {
    fingers[finger] = true;
    for (int i = 8; i >= 0; --i) {
        if (fingers[i]) {
            saw.freq(baseFreq * powf(2.0f, i / 12.0f));
            break;
        }
    }
}

void Bow::draw(al::Graphics &g) {
    g.pushMatrix();
    g.translate(pos);
    g.draw(mesh);
    g.popMatrix();
}

void Bow::update(float dt) {
    al::Vec2f newPos = {float(app->mouse().x()), app->height() - float(app->mouse().y())};
    auto v = (newPos - pos) / dt;
    avgVel = avgVel * 0.9 + v * 0.1;
    pos = newPos;
    moving = avgVel.mag() > 0.1f;
    down = app->mouse().left();
}

int main() {
  MyApp app;
  app.start();
  return 0;
}