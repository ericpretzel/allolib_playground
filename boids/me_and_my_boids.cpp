/*
This is based off of Allocore Example: Flocking by Lance Putnam

Description:
This is an example implementation of a flocking algorithm. The original flocking
algorithm [1] consists of three main interactions between flockmates ("boids"):

    1) Collision avoidance (of nearby flockmates)
    2) Velocity matching (of nearby flockmates)
    3) Flock centering (of nearby flockmates)

Here, we implement 1) and 2) only. Another change from the reference source is
the use of Gaussian functions rather than inverse-squared functions for
calculating the "nearness" of flockmates. This is done primarily to avoid
infinities, but also to give smoother motions. Lastly, we give each boid a
random walk motion which helps both dissolve and redirect the flocks.

[1] Reynolds, C. W. (1987). Flocks, herds, and schools: A distributed behavioral
    model. Computer Graphics, 21(4):25–34.

Author:
Lance Putnam, Oct. 2014
*/

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
#include "boid.hpp"

using namespace al;
static const int Nb = 32;  // Number of boids
static int groupSizes[Nb];

struct MyApp : public App {
  Boid boids[Nb];
  Mesh heads, tails;
  Mesh box;

  PolySynth synth;

  void onCreate() {
    synth.allocatePolyphony<Boid>(Nb);

    box.primitive(Mesh::LINE_LOOP);
    box.vertex(-1, -1);
    box.vertex(1, -1);
    box.vertex(1, 1);
    box.vertex(-1, 1);
    box.colorFill(Color(1, 1, 1));
    nav().pullBack(4);

    resetBoids();

    for (int i = 0; i < Nb; ++i) {
      auto voice = synth.getVoice<Boid>(i);
      synth.triggerOn(voice, 0, i);
    }
  }

  void onSound(AudioIOData& io) override {
    synth.render(io);
  }

  // Randomize boid positions/velocities uniformly inside unit disc
  void resetBoids() {
    for (auto& b : boids) {
      b.pos = rnd::ball<Vec2f>();
      b.vel = rnd::ball<Vec2f>();
    }
  }

  void onAnimate(double dt_ms) {
    float dt = dt_ms;

    for (int i = 0; i < Nb; ++i) {
      boids[i].groupId = -1;
      groupSizes[i] = 0;
    }
    boids[0].groupId = 0;
    int numGroups = 1;


    // Compute boid-boid interactions
    for (int i = 0; i < Nb - 1; ++i) {
      for (int j = i + 1; j < Nb; ++j) {
        // printf("checking boids %d and %d\n", i,j);

        auto ds = boids[i].pos - boids[j].pos;
        auto dist = ds.mag();

        // form groups (incredibly inefficient)
        if (dist <= 0.25) {
          if (boids[i].groupId == -1 && boids[j].groupId == -1) {
            boids[i].groupId = numGroups;
            boids[j].groupId = numGroups;
            groupSizes[numGroups] = 2;
            numGroups++;
          } else if (boids[i].groupId == -1) {
            boids[i].groupId = boids[j].groupId;
            groupSizes[boids[j].groupId]++;
          } else if (boids[j].groupId == -1) {
            boids[j].groupId = boids[i].groupId;
            groupSizes[boids[i].groupId]++;
          } else {
            // Merge groups
            int oldGroup = boids[j].groupId;
            for (int k = 0; k < Nb; ++k) {
              if (boids[k].groupId == oldGroup) {
                boids[k].groupId = boids[i].groupId;
                groupSizes[boids[i].groupId]++;
              }
            }
          }
        }

        // Collision avoidance
        float pushRadius = 0.05;
        float pushStrength = 1;
        float push = exp(-al::pow2(dist / pushRadius)) * pushStrength;

        auto pushVector = ds.normalized() * push;
        boids[i].vel += pushVector;
        boids[j].vel -= pushVector;

        // Velocity matching
        float matchRadius = 0.125;
        float nearness = exp(-al::pow2(dist / matchRadius));
        Vec2f veli = boids[i].vel;
        Vec2f velj = boids[j].vel;

        // Take a weighted average of velocities according to nearness
        boids[i].vel = veli * (1 - 0.5 * nearness) + velj * (0.5 * nearness);
        boids[j].vel = velj * (1 - 0.5 * nearness) + veli * (0.5 * nearness);

        // TODO: Flock centering
      }
    }


    for (size_t i = 0; i < Nb; ++i) {
      auto& b = boids[i];
      b.update(dt);
    }
  }

  void onDraw(Graphics& g) {
    g.clear(0);
    g.depthTesting(true);
    g.pointSize(8);
    // g.nicest();
    // g.stroke(8);
    g.meshColor();

    // g.stroke(1);
    // g.color(1);
    g.draw(box);

    for (auto& b : boids) {
      b.draw(g);
    }
  }

  bool onKeyDown(const Keyboard& k) {
    switch (k.key()) {
      case 'r':
        resetBoids();
        break;
    }
    return true;
  }
};

void Boid::update(float dt) {
    pos += vel * dt;

    // Random "hunting" motion
    float huntUrge = 0.2;
    auto hunt = rnd::ball<Vec2f>();
    // Use cubed distribution to make small jumps more frequent
    hunt *= hunt.magSqr();
    vel += hunt * huntUrge;

    // Bound boid into a box
    if (pos.x > 1 || pos.x < -1) {
      pos.x = pos.x > 0 ? 1 : -1;
      vel.x = -vel.x;
      bounced = true;
    }
    if (pos.y > 1 || pos.y < -1) {
      pos.y = pos.y > 0 ? 1 : -1;
      vel.y = -vel.y;
      bounced = true;
    }
}

void Boid::draw(Graphics& g) {
  head.reset();
  head.primitive(Mesh::POINTS);
  head.vertex(pos);

  tail.reset();
  tail.primitive(Mesh::LINES);

  tail.vertex(pos);
  tail.vertex(pos - vel.normalized(0.07));

  head.color(HSV(1.0 / 32.0 * groupId, 1.0, 1.0));

  tail.color(head.colors()[0]);
  tail.color(RGB(0.5));

  g.draw(head);
  g.draw(tail);
}

void Boid::onProcess(al::AudioIOData& io) {
  while (io()) {
    mOsc.freq(440 + groupSizes[groupId] * 100);
    float s1 = mOsc() * mAmpEnv() * 0.5;
    float s2;
    mPan(s1, s1, s2);
    io.out(0) += s1 / Nb;
    io.out(1) += s2 / Nb;
  }

  // We need to let the synth know that this voice is done
  // by calling the free(). This takes the voice out of the
  // rendering chain
  if (mAmpEnv.done()) free();
}

void Boid::init() {
  mAmpEnv.curve(0);
  mAmpEnv.levels(0, 1, 1, 0);
  mAmpEnv.lengths()[0] = 0.5;
  mAmpEnv.lengths()[2] = 0.5;
  mAmpEnv.sustainPoint(2);
  
  mOsc.freq(440);

  mPan.pos(0);
}



int main() {
  MyApp app;
  app.configureAudio(44100, 512, 2, 0);
  app.start();
}
