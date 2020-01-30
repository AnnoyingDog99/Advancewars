#include "Grid.h"
#include "precomp.h"

namespace Tmpl8
{

Grid::Grid(uint id, float width, float height, float area_x, float area_y)
    : width(width),
      height(height),
      area(area_x, area_y),
      id(id)
{
}

uint Grid::getID()
{
    return this->id;
}

float Grid::getWidth()
{
    return this->width;
}

float Grid::getHeight()
{
    return this->height;
}

vec2 Grid::getArea()
{
    return this->area;
}

/*
* Fills the vector all_tanks on first call (when the vector is empty) before returning
* Only returns all_tanks on all other calls
*/
vector<Tank*> Grid::getTanks()
{
    if (this->all_tanks.size() > 0) return all_tanks;
    this->all_tanks = blue_tanks;
    all_tanks.insert(all_tanks.end(), red_tanks.begin(), red_tanks.end());
    return all_tanks;
}

vector<Tank*> Grid::getBlueTanks()
{
    return this->blue_tanks;
}

vector<Tank*> Grid::getRedTanks()
{
    return this->red_tanks;
}

vector<Rocket*> Grid::getRockets()
{
    return this->rockets;
}

vector<Particle_beam*> Grid::getParticle_beams()
{
    return this->particle_beams;
}

/*
* Sorts and adds tank to it's corresponding vector
*/
void Grid::addTank(Tank* tank)
{
    if (tank->Get_Allignment() == BLUE)
        this->blue_tanks.push_back(std::move(tank));
    else
        this->red_tanks.push_back(std::move(tank));
}

void Grid::addBlueTank(Tank* tank)
{
    this->blue_tanks.push_back(std::move(tank));
}

void Grid::addRedTank(Tank* tank)
{
    this->red_tanks.push_back(std::move(tank));
}

void Grid::addRocket(Rocket* rocket)
{
    this->rockets.push_back(std::move(rocket));
}

void Grid::addParticle_beam(Particle_beam* particle_beam)
{
    this->particle_beams.push_back(std::move(particle_beam));
}

void Grid::clearTanks()
{
    this->all_tanks.shrink_to_fit();
    this->blue_tanks.shrink_to_fit();
    this->red_tanks.shrink_to_fit();
    this->all_tanks.clear();
    this->blue_tanks.clear();
    this->red_tanks.clear();
}

void Grid::clearRockets()
{
    this->rockets.shrink_to_fit();
    this->rockets.clear();
}

void Grid::clearParticle_beams()
{
    this->particle_beams.shrink_to_fit();
    this->particle_beams.clear();
}
} // namespace Tmpl8