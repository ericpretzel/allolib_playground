
/*
For most complex applications you will want to have a common state that
synchronizes data at frame rate. Through the cuttlebone library, this
can be done very efficiently. The DistributedAppWithState class encapsulates
and make use of cuttlebone very simple.

(Note for Windows: cuttlebone only runs on *nix platforms, so Windows has an
OSC backup for this. However, the size of the state is severly limited due
to a small buffer size managed by OSC and it also makes interoperability
between Windows and *nix machines impossible)

The DistributedAppWithState must be templated on a struct that defines the
state data. you can then get the state using state().

Notice how the nav from the primary application is syncrhonized for all others
writing if primary or reading from the state if otherwise.

Rule of thumb: parameter vs. state

You should use state synchronization for large data and data that is generated
and consumed at frame rate (i.e. simulation data and graphics parameters).

You should use parameters when you need near instantaneous updates (usually
good for audio), or if your state is getting large and you don't require
updating values on every frame.

*/

#include "Gamma/Oscillator.h"
#include "al/app/al_DistributedApp.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"

using namespace al;

struct CommonState {
  float xPosition = 0.0;
  float mod = 0.5; // modulation value
  Nav nav;
};

class MyApp : public DistributedAppWithState<CommonState> {
public:
  Mesh m;
  bool rising{false};
  gam::Square<> osc;

  // We will make the modulation "factor" a parameter
  Parameter factor{"factor", "", 1.03f, 1.001f, 1.08f};
  // The "mod" parameter will only be used to move data
  // it will not be visible in the controls.
  Parameter mod{"mod", "", 0.5};
  ControlGUI gui;

  void onInit() override {}
  void onCreate() override {
    addIcosphere(m);
    gui.init();
    gui << factor; // display factor parameter in gui
    // This application will not use the paramter server to synchronize data.
  }

  void onAnimate(double dt) override {
    // We only want to update the mod parameter if we are the main machine

    if (isPrimary()) {
      if (rising) {
        // We can no longer use *= operators with Parameter classes...
        mod = mod * factor;
      } else {
        mod = mod * (1 / factor);
      }
      if (mod > 0.8) {
        rising = false;
      } else if (mod < 0.2) {
        rising = true;
      }
      state().mod = mod;

      state().xPosition = (factor - 1) * 10;
      state().nav = nav();
    } else {
      nav() = state().nav;
    }
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    g.pushMatrix();
    // Notice that I query variables through state(). In the case of the primary
    // node, this is local data, in the case of renderers, this is data
    // received through state synchronization.
    g.translate(state().xPosition, 0, -4);
    g.scale(state().mod);
    g.polygonLine();
    g.draw(m);
    if (hasCapability(Capability::CAP_2DGUI)) {
      gui.draw(g);
    }
    g.popMatrix();
  }

  void onSound(AudioIOData &io) override {
    // factor affects frequency
    // Even though we are using the "factor" parameter here, we don't need
    // to syncrhonize it in this case, as audio is run by the same node that
    // updates it.
    // But you must always be aware of who is synchronizing what to ensure
    // that everything gets synchronized properly.
    osc.freq(220 * (factor - 1) * 10);
    while (io()) {
      io.out(0) = osc() * mod;
    }
  }

  void onExit() override { gui.cleanup(); }
};

int main() {
  MyApp app;
  app.start();
  return 0;
}