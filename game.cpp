#include "precomp.h" // include (only) this in every .cpp file

#define NUM_TANKS_BLUE 1200
#define NUM_TANKS_RED 1200

#define TANK_MAX_HEALTH 1000
#define ROCKET_HIT_VALUE 60
#define PARTICLE_BEAM_HIT_VALUE 50

#define TANK_MAX_SPEED 1.5

#define HEALTH_BARS_OFFSET_X 0
#define HEALTH_BAR_HEIGHT 70
#define HEALTH_BAR_WIDTH 1
#define HEALTH_BAR_SPACING 0

#define MAX_FRAMES 2000

//Global performance timer
#define REF_PERFORMANCE 41550.5 //UPDATE THIS WITH YOUR REFERENCE PERFORMANCE (see console after 2k frames)
static timer perf_timer;
static float duration;

//Load sprite files and initialize sprites
static Surface* background_img = new Surface("assets/Background_Grass.png");
static Surface* tank_red_img = new Surface("assets/Tank_Proj2.png");
static Surface* tank_blue_img = new Surface("assets/Tank_Blue_Proj2.png");
static Surface* rocket_red_img = new Surface("assets/Rocket_Proj2.png");
static Surface* rocket_blue_img = new Surface("assets/Rocket_Blue_Proj2.png");
static Surface* particle_beam_img = new Surface("assets/Particle_Beam.png");
static Surface* smoke_img = new Surface("assets/Smoke.png");
static Surface* explosion_img = new Surface("assets/Explosion.png");

static Sprite background(background_img, 1);
static Sprite tank_red(tank_red_img, 12);
static Sprite tank_blue(tank_blue_img, 12);
static Sprite rocket_red(rocket_red_img, 12);
static Sprite rocket_blue(rocket_blue_img, 12);
static Sprite smoke(smoke_img, 4);
static Sprite explosion(explosion_img, 9);
static Sprite particle_beam_sprite(particle_beam_img, 3);

const static vec2 tank_size(14, 18);
const static vec2 rocket_size(25, 24);

const static float tank_radius = 12.f;
const static float rocket_radius = 10.f;

const int NUM_OF_GRIDS_X = 10;
const int NUM_OF_GRIDS_Y = 10;

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init() //2N
{
    std::cout << "init start: " << perf_timer.elapsed() << "\n";
    frame_count_font = new Font("assets/digital_small.png", "ABCDEFGHIJKLMNOPQRSTUVWXYZ:?!=-0123456789.");

    btanks.reserve(NUM_TANKS_BLUE);
    rtanks.reserve(NUM_TANKS_RED);

    uint rows = (uint)sqrt(NUM_TANKS_BLUE + NUM_TANKS_RED);
    uint max_rows = 12;

    float start_blue_x = tank_size.x + 10.0f;
    float start_blue_y = tank_size.y + 80.0f;

    float start_red_x = 980.0f;
    float start_red_y = 100.0f;

    float spacing = 15.0f;

    //Spawn blue tanks
    for (int i = NUM_TANKS_BLUE; i--;) //N
    {
        btanks.push_back(Tank(start_blue_x + ((i % max_rows) * spacing), start_blue_y + ((i / max_rows) * spacing), BLUE, &tank_blue, &smoke, 1200, 600, tank_radius, TANK_MAX_HEALTH, TANK_MAX_SPEED));
    }
    //Spawn red tanks
    for (int i = NUM_TANKS_RED; i--;) //N
    {
        rtanks.push_back(Tank(start_red_x + ((i % max_rows) * spacing), start_red_y + ((i / max_rows) * spacing), RED, &tank_red, &smoke, 80, 80, tank_radius, TANK_MAX_HEALTH, TANK_MAX_SPEED));
    }

    particle_beams.push_back(Particle_beam(vec2(SCRWIDTH / 2, SCRHEIGHT / 2), vec2(100, 50), &particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));
    particle_beams.push_back(Particle_beam(vec2(80, 80), vec2(100, 50), &particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));
    particle_beams.push_back(Particle_beam(vec2(1200, 600), vec2(100, 50), &particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));
    std::cout << "init end: " << perf_timer.elapsed() << "\n";

    float GRID_WIDTH = (SCRWIDTH) / NUM_OF_GRIDS_X;
    float GRID_HEIGHT = (SCRHEIGHT + 300) / NUM_OF_GRIDS_Y;
    vector<const Tank*> assigned_tanks;
    //Spawn grids
    int id = 1;
    for (int i = 1; i <= NUM_OF_GRIDS_X + 1; i++)
    {
        for (int j = 1; j <= NUM_OF_GRIDS_Y + 1; j++)
        {
            grids.push_back(Grid(id, GRID_WIDTH, GRID_HEIGHT, i * GRID_WIDTH, j * GRID_HEIGHT));
            id++;
        }
    }
    /* Grid Layout example
         X
    |/////////|/////////|/////////|/////////|
    |//     //|//     //|//     //|//     //|
    |//  1  //|//  4  //|//  7  //|//  10 //| Y
    |//     //|//     //|//     //|//     //|
    |/////////|/////////|/////////|/////////|
    |//     //|//     //|//     //|//     //|
    |//  2  //|//  5  //|//  8  //|//  11 //|
    |//     //|//     //|//     //|//     //|
    |/////////|/////////|/////////|/////////|
    |//     //|//     //|//     //|//     //|
    |//  3  //|//  6  //|//  9  //|//  12 //|
    |//     //|//     //|//     //|//     //|
    |/////////|/////////|/////////|/////////|
    */
}

// -----------------------------------------------------------
// Close down application
// -----------------------------------------------------------
void Game::Shutdown()
{
}

// -----------------------------------------------------------
// Iterates through all tanks and returns the closest enemy tank for the given tank
// -----------------------------------------------------------

Tank& Game::FindClosestEnemy(Tank& current_tank) //N
{
    float closest_distance = numeric_limits<float>::infinity();
    int closest_index = 0;
    if (current_tank.allignment == BLUE)
    {
        for (int i = rtanks.size(); i--;)
        {
            if (rtanks[i].active)
            {
                float sqrDist = fabsf((rtanks[i].Get_Position() - current_tank.Get_Position()).sqrLength());
                if (sqrDist < closest_distance)
                {
                    closest_distance = sqrDist;
                    closest_index = i;
                }
            }
        }
        return rtanks[closest_index];
    }
    else
    {
        for (int i = btanks.size(); i--;)
        {
            if (btanks[i].active)
            {
                float sqrDist = fabsf((btanks[i].Get_Position() - current_tank.Get_Position()).sqrLength());
                if (sqrDist < closest_distance)
                {
                    closest_distance = sqrDist;
                    closest_index = i;
                }
            }
        }
        return btanks[closest_index];
    }
}

// -----------------------------------------------------------
// Update the game state:
// Move all objects
// Update sprite frames
// Collision detection
// Targeting etc..
// -----------------------------------------------------------
void Game::Update(float deltaTime) //N^2
{
    //Update Grids
    for (auto& grid : grids)
    {
        // Assigning Tanks to Grid
        grid.clearTanks();
        for (auto& btank : btanks)
        {
            btank.has_ticked = false;

            // Overflow decides the amount of distance grids overlap eachother, this way tanks can be in multiple grids at once (making them check multiple grids for collisions with other tanks)
            float overflow = btank.collision_radius;

            bool overlap = true ? (btank.position.x >= grid.area.x - grid.width - overflow && btank.position.x < grid.area.x + overflow && btank.position.y >= grid.area.y - grid.height - overflow && btank.position.y < grid.area.y + overflow) : false;
            if (overlap) grid.addBlueTank(&btank);
        }
        for (auto& rtank : rtanks)
        {
            rtank.has_ticked = false;

            // Overflow decides the amount of distance grids overlap eachother, this way tanks can be in multiple grids at once (making them check multiple grids for collisions with other tanks)
            float overflow = rtank.collision_radius;

            bool overlap = true ? (rtank.position.x >= grid.area.x - grid.width - overflow && rtank.position.x < grid.area.x + overflow && rtank.position.y >= grid.area.y - grid.height - overflow && rtank.position.y < grid.area.y + overflow) : false;
            if (overlap) grid.addRedTank(&rtank);
        }

        // Assigning Rockets to Grid
        grid.clearRockets();
        for (auto& rocket : rockets)
        {
            rocket.has_ticked = false;

            // Overflow decides the amount of distance grids overlap eachother, this way rockets can be in multiple grids at once (making them check multiple grids for collisions with tanks)
            float overflow = rocket.collision_radius;

            bool overlap = true ? (rocket.position.x >= grid.area.x - grid.width - overflow && rocket.position.x < grid.area.x + overflow && rocket.position.y >= grid.area.y - grid.height - overflow && rocket.position.y < grid.area.y + overflow) : false;
            if (overlap) grid.addRocket(&rocket);
        }

        // Assigning Particle Beams to Grid
        grid.clearParticle_beams();
        for (auto& particle_beam : particle_beams)
        {
            particle_beam.has_ticked = false;

            bool overlap = true ? (particle_beam.rectangle.intersectsCircle(grid.area, grid.area.x / 2 + grid.area.y / 2)) : false;
            if (overlap) grid.addParticle_beam(&particle_beam);
        }
    }

    for (auto& grid : grids)
    {
        //Update Tanks
        for (auto* tank : grid.getTanks())
        {
            if (!tank->active) continue;
            //Check tank collision and nudge tanks away from each other
            for (auto* oTank : grid.getTanks())
            {
                if (tank == oTank) continue;
                vec2 dir = tank->Get_Position() - oTank->Get_Position();
                float dirSquaredLen = dir.sqrLength();

                float colSquaredLen = (tank->Get_collision_radius() * tank->Get_collision_radius()) + (oTank->Get_collision_radius() * oTank->Get_collision_radius());

                if (dirSquaredLen < colSquaredLen)
                {
                    tank->Push(dir.normalized(), 1.f);
                }
            }

            //Move tanks according to speed and nudges (see above) also reload
            if (!tank->has_ticked)
            {
                tank->Tick();
                tank->has_ticked = true;
            }

            //Shoot at closest target if reloaded
            if (tank->Rocket_Reloaded())
            {
                Tank& target = FindClosestEnemy(*tank);

                rockets.push_back(Rocket(tank->position, (target.Get_Position() - tank->position).normalized() * 3, rocket_radius, tank->allignment, ((tank->allignment == RED) ? &rocket_red : &rocket_blue)));

                tank->Reload_Rocket();
            }
        }

        //Update rockets
        for (auto* rocket : grid.getRockets())
        {
            if (!rocket->has_ticked)
            {
                rocket->Tick();
                rocket->has_ticked = true;
            }

            //Check if rocket collides with enemy tank, spawn explosion and if tank is destroyed spawn a smoke plume
            if (rocket->allignment == BLUE)
            {
                for (auto* tank : grid.getRedTanks())
                {
                    if (tank->active && rocket->Intersects(tank->position, tank->collision_radius))
                    {
                        explosions.push_back(Explosion(&explosion, tank->position));

                        if (tank->hit(ROCKET_HIT_VALUE))
                        {
                            smokes.push_back(Smoke(smoke, tank->position - vec2(0, 48)));
                        }

                        rocket->active = false;
                        break;
                    }
                }
            }
            else
            {
                for (auto* tank : grid.getBlueTanks())
                {
                    if (tank->active && rocket->Intersects(tank->position, tank->collision_radius))
                    {
                        explosions.push_back(Explosion(&explosion, tank->position));

                        if (tank->hit(ROCKET_HIT_VALUE))
                        {
                            smokes.push_back(Smoke(smoke, tank->position - vec2(0, 48)));
                        }

                        rocket->active = false;
                        break;
                    }
                }
            }
        }

        //Update particle beams
        for (auto* particle_beam : grid.getParticle_beams())
        {
            if (!particle_beam->has_ticked)
            {
                particle_beam->tick();
                particle_beam->has_ticked = true;
            }

            //Damage all tanks within the damage window of the beam (the window is an axis-aligned bounding box)
            for (auto* tank : grid.getTanks())
            {
                if (tank->active && particle_beam->rectangle.intersectsCircle(tank->Get_Position(), tank->Get_collision_radius()))
                {
                    if (tank->hit(particle_beam->damage))
                    {
                        smokes.push_back(Smoke(smoke, tank->position - vec2(0, 48)));
                    }
                }
            }
        }
    }

    //Update tanks
    /*for (Tank& tank : btanks) //N^2
    {
        if (tank.active)
        {
            //Check tank collision and nudge tanks away from each other
            for (Tank &oTank : btanks) //N
            {
                if (&tank == &oTank) continue;

                vec2 dir = tank.Get_Position() - oTank.Get_Position();
                float dirSquaredLen = dir.sqrLength();

                float colSquaredLen = (tank.Get_collision_radius() * tank.Get_collision_radius()) + (oTank.Get_collision_radius() * oTank.Get_collision_radius());

                if (dirSquaredLen < colSquaredLen)
                {
                    tank.Push(dir.normalized(), 1.f);
                }
            }

            //Move tanks according to speed and nudges (see above) also reload
            tank.Tick();

            //Shoot at closest target if reloaded
            if (tank.Rocket_Reloaded())
            {
                Tank& target = FindClosestEnemy(tank);

                rockets.push_back(Rocket(tank.position, (target.Get_Position() - tank.position).normalized() * 3, rocket_radius, tank.allignment, ((tank.allignment == RED) ? &rocket_red : &rocket_blue)));

                tank.Reload_Rocket();
            }
        }
    }*/

    /*for (Tank& tank : rtanks) //N^2
    {
        if (tank.active)
        {
            //Check tank collision and nudge tanks away from each other
            for (Tank& oTank : rtanks) //N
            {
                if (&tank == &oTank) continue;

                vec2 dir = tank.Get_Position() - oTank.Get_Position();
                float dirSquaredLen = dir.sqrLength();

                float colSquaredLen = (tank.Get_collision_radius() * tank.Get_collision_radius()) + (oTank.Get_collision_radius() * oTank.Get_collision_radius());

                if (dirSquaredLen < colSquaredLen)
                {
                    tank.Push(dir.normalized(), 1.f);
                }
            }

            //Move tanks according to speed and nudges (see above) also reload
            tank.Tick();

            //Shoot at closest target if reloaded
            if (tank.Rocket_Reloaded())
            {
                Tank& target = FindClosestEnemy(tank);

                rockets.push_back(Rocket(tank.position, (target.Get_Position() - tank.position).normalized() * 3, rocket_radius, tank.allignment, ((tank.allignment == RED) ? &rocket_red : &rocket_blue)));

                tank.Reload_Rocket();
            }
        }
    }*/

    //Update smoke plumes
    for (auto& smoke : smokes) //N
    {
        smoke.Tick();
    }

    //Update rockets
    /*for (Rocket& rocket : rockets) //N^2
    {
        rocket.Tick();

        //Check if rocket collides with enemy tank, spawn explosion and if tank is destroyed spawn a smoke plume
        for (Tank& tank : btanks) //N
        {
            if (tank.active && (tank.allignment != rocket.allignment) && rocket.Intersects(tank.position, tank.collision_radius))
            {
                explosions.push_back(Explosion(&explosion, tank.position));

                if (tank.hit(ROCKET_HIT_VALUE))
                {
                    smokes.push_back(Smoke(smoke, tank.position - vec2(0, 48)));
                }

                rocket.active = false;
                break;
            }
        }
        for (Tank& tank : rtanks) //N
        {
            if (tank.active && (tank.allignment != rocket.allignment) && rocket.Intersects(tank.position, tank.collision_radius))
            {
                explosions.push_back(Explosion(&explosion, tank.position));

                if (tank.hit(ROCKET_HIT_VALUE))
                {
                    smokes.push_back(Smoke(smoke, tank.position - vec2(0, 48)));
                }

                rocket.active = false;
                break;
            }
        }
    }*/

    //Remove exploded rockets with remove erase idiom
    rockets.erase(std::remove_if(rockets.begin(), rockets.end(), [](const Rocket& rocket) {
                      return !rocket.active || rocket.position.x < 0 || rocket.position.x > SCRWIDTH || rocket.position.y < 0 || rocket.position.y > SCRHEIGHT;
                  }),
                  rockets.end());

    //Update particle beams
    /*for (Particle_beam& particle_beam : particle_beams) //N^2
    {
        particle_beam.tick(btanks);
        particle_beam.tick(rtanks);

        //Damage all tanks within the damage window of the beam (the window is an axis-aligned bounding box)
        for (Tank &tank : btanks, rtanks) //N
        {
            if (tank.active && particle_beam.rectangle.intersectsCircle(tank.Get_Position(), tank.Get_collision_radius()))
            {
                if (tank.hit(particle_beam.damage))
                {
                    smokes.push_back(Smoke(smoke, tank.position - vec2(0, 48)));
                }
            }
        }
    }*/

    //Update explosion sprites and remove when done with remove erase idiom
    for (auto& explosion : explosions) //N
    {
        explosion.Tick();
    }

    explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](const Explosion& explosion) { return explosion.done(); }), explosions.end());
}

bool sortByHealth(const Tank& ltank, const Tank& rtank) { return ltank.health < rtank.health; }

void Game::Draw() //N^2
{
    // clear the graphics window
    screen->Clear(0);

    //Draw background
    background.Draw(screen, 0, 0);

    //Draw sprites
    for (int i = NUM_TANKS_BLUE; i--;) //N
    {
        btanks[i].Draw(screen);

        vec2 tPos = btanks[i].Get_Position();
        // tread marks
        if ((tPos.x >= 0) && (tPos.x < SCRWIDTH) && (tPos.y >= 0) && (tPos.y < SCRHEIGHT))
            background.GetBuffer()[(int)tPos.x + (int)tPos.y * SCRWIDTH] = SubBlend(background.GetBuffer()[(int)tPos.x + (int)tPos.y * SCRWIDTH], 0x808080);
    }

    for (int i = NUM_TANKS_RED; i--;) //N
    {
        rtanks[i].Draw(screen);

        vec2 tPos = rtanks[i].Get_Position();
        // tread marks
        if ((tPos.x >= 0) && (tPos.x < SCRWIDTH) && (tPos.y >= 0) && (tPos.y < SCRHEIGHT))
            background.GetBuffer()[(int)tPos.x + (int)tPos.y * SCRWIDTH] = SubBlend(background.GetBuffer()[(int)tPos.x + (int)tPos.y * SCRWIDTH], 0x808080);
    }

    for (auto& rocket : rockets) //N
    {
        rocket.Draw(screen);
    }

    for (auto& smoke : smokes) //N
    {
        smoke.Draw(screen);
    }

    for (auto& particle_beam : particle_beams) //N
    {
        particle_beam.Draw(screen);
    }

    for (auto& explosion : explosions) //N
    {
        explosion.Draw(screen);
    }

    //Draw sorted health bars
    //std::vector<const Tank*> sorted_btanks;
    //std::vector<const Tank*> sorted_rtanks;
    std::sort(btanks.begin(), btanks.end(), sortByHealth);
    std::sort(rtanks.begin(), rtanks.end(), sortByHealth);
    //insertion_sort_tanks_health(btanks, sorted_btanks, 0, NUM_TANKS_BLUE);
    //insertion_sort_tanks_health(rtanks, sorted_rtanks, 0, NUM_TANKS_RED);
    for (int i = NUM_TANKS_BLUE; i--;) //N
    {
        int health_bar_start_x = i * (HEALTH_BAR_WIDTH + HEALTH_BAR_SPACING) + HEALTH_BARS_OFFSET_X;
        int health_bar_start_y = 0;
        int health_bar_end_x = health_bar_start_x + HEALTH_BAR_WIDTH;
        int health_bar_end_y = HEALTH_BAR_HEIGHT;

        screen->Bar(health_bar_start_x, health_bar_start_y, health_bar_end_x, health_bar_end_y, REDMASK);
        screen->Bar(health_bar_start_x, health_bar_start_y + (int)((double)HEALTH_BAR_HEIGHT * (1 - ((double)btanks[i].health / (double)TANK_MAX_HEALTH))), health_bar_end_x, health_bar_end_y, GREENMASK);
    }

    for (int i = NUM_TANKS_RED; i--;) //N
    {
        int health_bar_start_x = i * (HEALTH_BAR_WIDTH + HEALTH_BAR_SPACING) + HEALTH_BARS_OFFSET_X;
        int health_bar_start_y = (SCRHEIGHT - HEALTH_BAR_HEIGHT) - 1;
        int health_bar_end_x = health_bar_start_x + HEALTH_BAR_WIDTH;
        int health_bar_end_y = SCRHEIGHT - 1;

        screen->Bar(health_bar_start_x, health_bar_start_y, health_bar_end_x, health_bar_end_y, REDMASK);
        screen->Bar(health_bar_start_x, health_bar_start_y + (int)((double)HEALTH_BAR_HEIGHT * (1 - ((double)rtanks[i].health / (double)TANK_MAX_HEALTH))), health_bar_end_x, health_bar_end_y, GREENMASK);
    }
}

// -----------------------------------------------------------
// Sort tanks by health value using insertion sort
// -----------------------------------------------------------
//void Tmpl8::Game::insertion_sort_tanks_health(const std::vector<Tank>& original, std::vector<const Tank*>& sorted_tanks, UINT16 begin, UINT16 end)
//{
//    const UINT16 NUM_TANKS = end - begin;
//    sorted_tanks.reserve(NUM_TANKS);
//    sorted_tanks.emplace_back(&original.at(begin));
//
//    for (int i = begin + 1; i < (begin + NUM_TANKS); i++) //N^2
//    {
//        const Tank& current_tank = original.at(i);
//
//        for (int s = (int)sorted_tanks.size() - 1; s >= 0; s--) //N
//        {
//            const Tank* current_checking_tank = sorted_tanks.at(s);
//
//            if ((current_checking_tank->CompareHealth(current_tank) <= 0))
//            {
//                sorted_tanks.insert(1 + sorted_tanks.begin() + s, &current_tank);
//                break;
//            }
//
//            if (s == 0)
//            {
//                sorted_tanks.insert(sorted_tanks.begin(), &current_tank);
//                break;
//            }
//        }
//    }
//}

// -----------------------------------------------------------
// When we reach MAX_FRAMES print the duration and speedup multiplier
// Updating REF_PERFORMANCE at the top of this file with the value
// on your machine gives you an idea of the speedup your optimizations give
// -----------------------------------------------------------
void Tmpl8::Game::MeasurePerformance()
{
    char buffer[128];
    if (frame_count >= MAX_FRAMES)
    {
        if (!lock_update)
        {
            duration = perf_timer.elapsed();
            cout << "Duration was: " << duration << " (Replace REF_PERFORMANCE with this value)" << endl;
            lock_update = true;
        }

        frame_count--;
    }

    if (lock_update)
    {
        screen->Bar(420, 170, 870, 430, 0x030000);
        int ms = (int)duration % 1000, sec = ((int)duration / 1000) % 60, min = ((int)duration / 60000);
        sprintf(buffer, "%02i:%02i:%03i", min, sec, ms);
        frame_count_font->Centre(screen, buffer, 200);
        sprintf(buffer, "SPEEDUP: %4.1f", REF_PERFORMANCE / duration);
        frame_count_font->Centre(screen, buffer, 340);
    }
}

// -----------------------------------------------------------
// Main application tick function
// -----------------------------------------------------------
void Game::Tick(float deltaTime)
{
    if (!lock_update)
    {
        Update(deltaTime);
    }
    Draw();

    MeasurePerformance();

    // print something in the graphics window
    //screen->Print("hello world", 2, 2, 0xffffff);

    // print something to the text window
    //cout << "This goes to the console window." << std::endl;

    //Print frame count
    frame_count++;
    //cout << frame_count << "\n";
    string frame_count_string = "FRAME: " + std::to_string(frame_count);
    frame_count_font->Print(screen, frame_count_string.c_str(), 350, 580);
}
