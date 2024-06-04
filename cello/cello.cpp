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
    navControl().disable();

    bow = new Bow(this);
    dimensions(1200, 1000);
    strings.push_back(CelloString(this, 220.0f, al::Vec2f(width() / 8.0 + 300, height() / 8.0 * 7.0 - 175))); // A3
    strings.push_back(CelloString(this, 146.8f, al::Vec2f(width() / 8.0 * 3 + 100, height() / 8.0 * 5 - 150))); // D3
    strings.push_back(CelloString(this, 98.0f, al::Vec2f(width() / 8.0 * 5 - 100, height() / 8.0 * 3 - 125))); // G2
    strings.push_back(CelloString(this, 65.41f, al::Vec2f(width() / 8.0 * 7 - 300, height() / 8.0 - 100))); // C2

    filter_freq.registerChangeCallback([this](float value) {
        for (auto &s : strings) {
            s.set_filter_freq(value);
        }
    });

    // gui << filter_freq;

    // gui.init();
}

void MyApp::onExit() {
    delete bow;
}

void MyApp::onAnimate(double _dt) {
    float dt = _dt;
    if (dt == 0) return;
    bow->update(dt);
    bool consume_attack = false;
    for (auto &s : strings) {
        s.update(dt);
        consume_attack |= s.being_played;
    }
    if (consume_attack) {
        bow->attacking = false;
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
    g.lineWidth(2.f);
    g.depthTesting(true);
    g.pointSize(5.f);
    // g.nicest();
    // g.stroke(8);
    g.meshColor();
    
    bow->draw(g);
    for (auto &s : strings) {
        s.draw(g);
    }
    // gui.draw(g);
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
    if (adsrEnv.done()) return;
    
    saw.freqAdd(vibrato());
    saw.freqAdd(detune() * detuneAmount);
    float s = saw();

    s *= abs(app->bow->avgVel.x) * 0.001f * adsrEnv();

    float wet1, wet2;
    reverb(s, wet1, wet2);
    s += wet1 + wet2;
    s = lpf(s);
    s = hpf(s);
    // s = res(s);

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
    being_played = app->bow->down && 
        app->bow->moving && 
        app->bow->pos.x + bowLength > pos.x && 
        app->bow->pos.x < pos.x &&
        app->bow->pos.y > pos.y && 
        app->bow->pos.y < pos.y + stringLength;

    if (being_played) {
        if (app->bow->attacking) {
            adsrEnv.levels(0, 2, 1, 0);
            adsrEnv.resetSoft();
        } else if (adsrEnv.released() || adsrEnv.done()) { 
            adsrEnv.levels(1, 1, 1, 0);
            adsrEnv.resetSoft(); 
        }
    } else if (!adsrEnv.released()) {
        adsrEnv.release();
    }
    
    if (!adsrEnv.done()) {
        mesh.vertices().clear();
        for (int i = 0; i < 100; ++i) {
            mesh.vertex(sin(M_PI * i / 100.f) * vibrato() * 3.5f, i * stringLength / 100.f);
        }
    } else {
        mesh.vertices().clear();
        for (int i = 0; i < 100; ++i) {
            mesh.vertex(0, i * stringLength / 100.f);
        }
    }
}

void CelloString::lift_finger(int finger) {
    fingers[finger] = false;
    for (int i = 8; i >= 0; --i) {
        if (fingers[i]) {
            saw.freq(baseFreq * powf(2.0f, i / 12.0f));
            mesh.colors().clear();
            mesh.colorFill(al::HSV(i / 8.f));
            break;
        }
    }
}

void CelloString::press_finger(int finger) {
    fingers[finger] = true;
    for (int i = 8; i >= 0; --i) {
        if (fingers[i]) {
            saw.freq(baseFreq * powf(2.0f, i / 12.0f));
            mesh.colors().clear();
            mesh.colorFill(al::HSV(i / 8.f));
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
    al::Vec2f newPos = {(float)app->mouse().x(), (float)app->height() - (float)app->mouse().y()};
    auto v = (newPos - pos) / dt;
    avgVel = avgVel * 0.9 + v * 0.1;
    if ((v.x > 0 && avgVel.x <= 0) || (v.x < 0 && avgVel.x >= 0)) {
        avgVel = v;
        attacking = true;
    } else if (!down && app->mouse().left()) {
        attacking = true;
    }
    pos = newPos;
    moving = avgVel.mag() > 0.1f;
    down = app->mouse().left();
}

int main() {
  MyApp app;
  app.start();
  return 0;
}