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
static const int Nb = 16;  // Number of boids
static float ratios[5] = { // pentatonic scale ratios
  0,
  2,
  4,
  7,
  9
};

struct MyApp : public App {
  Boid *boids[Nb];
  Mesh heads, tails;
  Mesh box;

  PolySynth synth;

  std::unordered_set<int> inUseIds;

  MyApp() {
    for (int i = 0; i < Nb; ++i) {
      boids[i] = new Boid;
    }
  }

  ~MyApp() {
    for (int i = 0; i < Nb; ++i) {
      delete boids[i];
    }
  }

  void onCreate() {
    synth.allocatePolyphony<BoidGroup>(Nb * Nb);
    box.primitive(Mesh::LINE_LOOP);
    box.vertex(-1, -1);
    box.vertex(1, -1);
    box.vertex(1, 1);
    box.vertex(-1, 1);
    box.colorFill(Color(1, 1, 1));
    nav().pullBack(4);

    resetBoids();
  }

  void onSound(AudioIOData& io) override {
    synth.render(io);
  }

  bool group_is_intact(Boid *src, BoidGroup *group) {
    std::unordered_set<Boid *> visited;
    std::vector<Boid *> stack;
    stack.push_back(src);
    while (!stack.empty()) {
      auto b = stack.back();
      stack.pop_back();
      if (visited.find(b) != visited.end()) {
        continue;
      }
      visited.insert(b);
      //if (b->group != group) {
      //  return false;
      //}
      for (auto n : b->neighbors) {
        stack.push_back(n);
      }
    }
    return visited == group->boids;
  }

  void fill_group(Boid *src, BoidGroup *group) {
    std::unordered_set<Boid *> visited;
    std::vector<Boid *> stack;
    stack.push_back(src);
    while (!stack.empty()) {
      auto b = stack.back();
      stack.pop_back();
      if (visited.find(b) != visited.end()) {
        continue;
      }
      visited.insert(b);
      if (b->group != nullptr && b->group != group) {
        b->group->boids.erase(b);
        if (b->group->boids.empty()) {
          synth.triggerOff(b->group->id);
          inUseIds.erase(b->group->id);
        }
      }
      b->group = group;
      b->group->boids.insert(b);
      for (auto n : b->neighbors) {
        stack.push_back(n);
      }
    }
    group->mOsc.freq(440 * pow(2, ratios[group->boids.size() % 5] / 12.0));
  }

  // Randomize boid positions/velocities uniformly inside unit disc
  void resetBoids() {
    for (int i = 0; i < Nb; i++) {
      auto b = boids[i];
      b->neighbors.clear();
      b->group = nullptr;
      b->pos = rnd::ball<Vec2f>();
      b->vel = rnd::ball<Vec2f>();
    }
    for (int i = 0; i < Nb; i++) {
      for (int j = 0; j < Nb; j++) {
        if (i == j) {
          continue;
        }
        auto ds = boids[i]->pos - boids[j]->pos;
        auto dist = ds.mag();
        if (dist <= 0.25) {
          boids[i]->neighbors.insert(boids[j]);
          boids[j]->neighbors.insert(boids[i]);
        }
      }
    }
    for (int i = 0; i < Nb; i++) {
      auto b = boids[i];
      if (b->group == nullptr) {
        auto group = synth.getVoice<BoidGroup>();
        group->boids.clear();
        group->id = i;
        while (inUseIds.find(group->id) != inUseIds.end()) {
          group->id = group->id + Nb;
        }
        fill_group(b, group);
        inUseIds.insert(i);
        synth.triggerOn(b->group, 0, group->id);
      }
    }
  }  

  void onAnimate(double dt_ms) {
    float dt = dt_ms;

    for (int i = 0; i < Nb; ++i) {
      boids[i]->neighbors.clear();
    }

    // Compute boid-boid interactions
    for (int i = 0; i < Nb; ++i) {
      for (int j = 0; j < Nb; ++j) {
        // printf("checking boids %d and %d\n", i,j);
        if (i == j) {
          continue;
        }
        auto ds = boids[i]->pos - boids[j]->pos;
        auto dist = ds.mag();

        if (dist <= 0.25) {
          boids[i]->neighbors.insert(boids[j]);
          boids[j]->neighbors.insert(boids[i]);
        }

        // Collision avoidance
        float pushRadius = 0.05;
        float pushStrength = 1;
        float push = exp(-al::pow2(dist / pushRadius)) * pushStrength;

        auto pushVector = ds.normalized() * push;
        boids[i]->vel += pushVector;
        boids[j]->vel -= pushVector;

        // Velocity matching
        float matchRadius = 0.125;
        float nearness = exp(-al::pow2(dist / matchRadius));
        Vec2f veli = boids[i]->vel;
        Vec2f velj = boids[j]->vel;

        // Take a weighted average of velocities according to nearness
        boids[i]->vel = veli * (1 - 0.5 * nearness) + velj * (0.5 * nearness);
        boids[j]->vel = velj * (1 - 0.5 * nearness) + veli * (0.5 * nearness);

        // TODO: Flock centering
      }
    }
    
    // merge with any new groups, then check if our group has split
    for (int i = 0; i < Nb; i++) {
      auto b = boids[i];
      fill_group(b, b->group);
      if (!group_is_intact(b, b->group)) {
        auto original_group = b->group;
        auto new_group = synth.getVoice<BoidGroup>();
        new_group->boids.clear();
        new_group->id = i;
        while (inUseIds.find(new_group->id) != inUseIds.end()) {
          new_group->id += Nb;
        }
        fill_group(b, new_group);
        inUseIds.insert(new_group->id);
        for (auto &b : new_group->boids) {
          original_group->boids.erase(b);
        }
        synth.triggerOn(b->group, 0, new_group->id);
      }
    }

    for (size_t i = 0; i < Nb; ++i) {
      boids[i]->update(dt);
    }
  }

  void onDraw(Graphics& g) {
    g.clear(0);
    g.depthTesting(true);
    g.pointSize(8);
    // g.nicest();
    // g.stroke(8);
    g.meshColor();

    synth.render(g);
    // g.stroke(1);
    // g.color(1);
    g.draw(box);
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
    float huntUrge = 0.1 * group->boids.size();
    auto hunt = rnd::ball<Vec2f>();
    // Use cubed distribution to make small jumps more frequent
    hunt *= hunt.magSqr();
    vel += hunt * huntUrge;

    // Bound boid into a box
    if (pos.x > 1 || pos.x < -1) {
      pos.x = pos.x > 0 ? 1 : -1;
      vel.x = -vel.x;
    }
    if (pos.y > 1 || pos.y < -1) {
      pos.y = pos.y > 0 ? 1 : -1;
      vel.y = -vel.y;
    }
}

void Boid::draw(Graphics& g, HSV color) {
  head.reset();
  head.primitive(Mesh::POINTS);
  head.vertex(pos);

  tail.reset();
  tail.primitive(Mesh::LINES);

  tail.vertex(pos);
  tail.vertex(pos - vel.normalized(0.07));

  head.color(color);

  tail.color(head.colors()[0]);
  tail.color(RGB(0.5));

  g.draw(head);
  g.draw(tail);
}

void BoidGroup::onProcess(al::AudioIOData& io) {
  while (io()) {
    float s1 = mOsc() * (16.f / (float)Nb);
    // float s2;
    // mPan(s1, s1, s2);
    io.out(0) += s1 / (float)Nb;
    // io.out(1) += s2 / Nb;
  }
}

void BoidGroup::onProcess(al::Graphics &g) {
  for (auto b : boids) {
    b->draw(g, HSV(1.0 / (float)Nb * boids.size(), 1, 1));
  }
}

void BoidGroup::init() {
  mOsc.freq(440);
  mPan.pos(0);
}

int main() {
  MyApp app;
  app.configureAudio(44100, 512, 1, 0);
  app.start();
}
