#pragma once

namespace Tmpl8
{
//forward declarations
class Grid;
class Tank;
class Rocket;
class Smoke;
class Particle_beam;

class Game
{
  public:
    void SetTarget(Surface* surface) { screen = surface; }
    void Init();
    void Shutdown();
    void Update(float deltaTime);

    void UpdateGrids();
    void UpdateTanks();
    void UpdateRockets();
    void UpdateParticleBeams();

    void DrawAll();
    void DrawParticleBeams();
    void DrawScreen();
    void DrawBlueTanks();
    void DrawRedTanks();
    void DrawRockets();
    void DrawExplosions();
    void DrawSmokes();

    void SortTanks();
    void DrawHealthBar();
    void Tick(float deltaTime);
    void insertion_sort_tanks_health(const std::vector<Tank>& original, std::vector<const Tank*>& sorted_tanks, UINT16 begin, UINT16 end);
    int Partition(std::vector<Tank>& tanks, UINT16 left, UINT16 right);
    void QuickSort(std::vector<Tank>& tanks, UINT16 left, UINT16 right);
    void MeasurePerformance();

    Tank& FindClosestEnemy(Tank& current_tank);

    void MouseUp(int button)
    { /* implement if you want to detect mouse button presses */
    }

    void MouseDown(int button)
    { /* implement if you want to detect mouse button presses */
    }

    void MouseMove(int x, int y)
    { /* implement if you want to detect mouse movement */
    }

    void KeyUp(int key)
    { /* implement if you want to handle keys */
    }

    void KeyDown(int key)
    { /* implement if you want to handle keys */
    }

  private:
    Surface* screen;

    vector<Grid> grids;
    vector<Tank> btanks;
    vector<Tank> rtanks;
    vector<Rocket> rockets;
    vector<Smoke> smokes;
    vector<Explosion> explosions;
    vector<Particle_beam> particle_beams;

    Font* frame_count_font;
    long long frame_count = 0;

    bool lock_update = false;
};

}; // namespace Tmpl8