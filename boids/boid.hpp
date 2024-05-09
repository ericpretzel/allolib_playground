#pragma once

#include <cmath>
#include "al/app/al_App.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/math/al_Functions.hpp"
#include "al/math/al_Random.hpp"

// A "boid" (play on bird) is one member of a flock.
struct Boid : public al::SynthVoice {

  // Unit generators
  gam::Pan<> mPan;
  gam::Sine<> mOsc;
  gam::Env<3> mAmpEnv;
  
  al::Mesh head, tail;

  al::Vec2f pos, vel;

  bool bounced = false;

  // what "group" a boid is currently a part of
  int groupId;

  // Update boid independent behaviors - hunting "urge" and movement
  void update(float dt);

  void init() override;

  void onProcess(al::AudioIOData& io) override;
  void draw(al::Graphics& g);
  void onTriggerOn() override { mAmpEnv.reset(); }
  void onTriggerOff() override { mAmpEnv.release(); }

};