#pragma once

#include "precomp.h"

namespace Tmpl8
{
class Grid
{
  public:
    Grid(uint, float, float, float, float);
    float width;
    float height;
    uint getID();
    vec2 area;
    vector<Tank*> getTanks();
    vector<Tank*> getBlueTanks();
    vector<Tank*> getRedTanks();
    vector<Rocket*> getRockets();
    vector<Particle_beam*> getParticle_beams();
    void addTank(Tank*);
    void addBlueTank(Tank*);
    void addRedTank(Tank*);
    void addRocket(Rocket*);
    void addParticle_beam(Particle_beam*);
    void clearTanks();
    void clearRockets();
    void clearParticle_beams();

  private:
    uint id;
    vector<Tank*> all_tanks;
    vector<Tank*> red_tanks;
    vector<Tank*> blue_tanks;
    vector<Rocket*> rockets;
    vector<Particle_beam*> particle_beams;
};
} // namespace Tmpl8