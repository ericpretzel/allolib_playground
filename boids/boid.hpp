#pragma once

#include <cmath>
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Functions.hpp"
#include "al/math/al_Random.hpp"

struct Boid;
struct BoidGroup;

// A "boid" (play on bird) is one member of a flock.
struct Boid {
  BoidGroup *group;

  al::Mesh head, tail;
  al::Vec2f pos, vel;

  std::unordered_set<Boid *> neighbors;

  // Update boid independent behaviors - hunting "urge" and movement
  void update(float dt);
  void draw(al::Graphics &g, al::HSV color);
};

struct BoidGroup : public al::SynthVoice {

  int id;

  // Unit generators
  gam::Pan<> mPan;
  gam::Sine<> mOsc;
  gam::Env<3> mAmpEnv;

  std::unordered_set<Boid *> boids;

  void init() override;

  void onProcess(al::AudioIOData& io) override;
  void onProcess(al::Graphics& g) override;
  // void onTriggerOn() override { mAmpEnv.reset(); }
  // void onTriggerOff() override { free(); }
};