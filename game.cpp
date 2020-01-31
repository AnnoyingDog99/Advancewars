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
#define REF_PERFORMANCE 10600 //UPDATE THIS WITH YOUR REFERENCE PERFORMANCE (see console after 2k frames)
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

const static int NUM_OF_GRIDS_X = 10;
const static int NUM_OF_GRIDS_Y = 10;

const static int threadCount = std::thread::hardware_concurrency();
ThreadPool pool(threadCount);

future<void> add_tanks;
future<void> add_rockets;
future<void> add_particleBeams;

future<void> draw_screen;
future<void> draw_blue_tanks;
future<void> draw_red_tanks;
future<void> draw_rockets;
future<void> draw_explosions;
future<void> draw_smokes;
future<void> draw_healthbar;
future<void> draw_particlebeams;

future<void> sort_tanks;

// -----------------------------------------------------------
// Initialize the application
// -----------------------------------------------------------
void Game::Init() //2N
{
    std::cout << "init start: " << perf_timer.elapsed() << "\n";
    frame_count_font = new Font("assets/digital_small.png", "ABCDEFGHIJKLMNOPQRSTUVWXYZ:?!=-0123456789.");
    future<void> spawn_grids = pool.enqueue(move([&] {
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
    }));

    particle_beams.push_back(Particle_beam(vec2(SCRWIDTH / 2, SCRHEIGHT / 2), vec2(100, 50), &particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));
    particle_beams.push_back(Particle_beam(vec2(80, 80), vec2(100, 50), &particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));
    particle_beams.push_back(Particle_beam(vec2(1200, 600), vec2(100, 50), &particle_beam_sprite, PARTICLE_BEAM_HIT_VALUE));

    btanks.reserve(NUM_TANKS_BLUE);
    rtanks.reserve(NUM_TANKS_RED);

    uint rows = (uint)sqrt(NUM_TANKS_BLUE + NUM_TANKS_RED);
    uint max_rows = 12;

    float start_blue_x = tank_size.x + 10.0f;
    float start_blue_y = tank_size.y + 80.0f;

    float start_red_x = 980.0f;
    float start_red_y = 100.0f;

    float spacing = 15.0f;

    future<void> spawn_btanks = pool.enqueue([&] {
        //Spawn blue tanks
        for (int i = 0; i < NUM_TANKS_BLUE; i++) //N
        {
            //Tank(
            btanks.push_back(Tank(start_blue_x + ((i % max_rows) * spacing), start_blue_y + ((i / max_rows) * spacing), BLUE, &tank_blue, &smoke, 1200, 600, tank_radius, TANK_MAX_HEALTH, TANK_MAX_SPEED));
        }
    });

    //Spawn red tanks
    for (int i = 0; i < NUM_TANKS_RED; i++) //N
    {
        rtanks.push_back(Tank(start_red_x + ((i % max_rows) * spacing), start_red_y + ((i / max_rows) * spacing), RED, &tank_red, &smoke, 80, 80, tank_radius, TANK_MAX_HEALTH, TANK_MAX_SPEED));
    }

    spawn_grids.wait();
    spawn_btanks.wait();

    std::cout
        << "init end: " << perf_timer.elapsed() << "\n";
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
    if (current_tank.Get_Allignment() == BLUE)
    {
        for (int i = 0; i < rtanks.size(); i++)
        {
            if (rtanks[i].Is_Active())
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
        for (int i = 0; i < btanks.size(); i++)
        {
            if (btanks[i].Is_Active())
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

void Game::UpdateGrids()
{
    //Update Grids
    for (auto& grid : grids)
    {
        grid.clearTanks();
        grid.clearRockets();
        grid.clearParticle_beams();
    }
    add_rockets = pool.enqueue([&] {
        for (auto& grid : grids)
        {
            // Assigning Rockets to Grid
            for (Rocket& rocket : rockets)
            {
                rocket.Set_Ticked(false);

                // Overflow decides the amount of distance grids overlap eachother, this way rockets can be in multiple grids at once (making them check multiple grids for collisions with tanks)
                float overflow = rocket.Get_collision_radius();

                bool overlap = true ? (rocket.Get_Position().x >= grid.getArea().x - grid.getWidth() - overflow && rocket.Get_Position().x < grid.getArea().x + overflow && rocket.Get_Position().y >= grid.getArea().y - grid.getHeight() - overflow && rocket.Get_Position().y < grid.getArea().y + overflow) : false;
                if (overlap) grid.addRocket(&rocket);
            }
        }
    });
    add_particleBeams = pool.enqueue([&] {
        for (auto& grid : grids)
        {
            // Assigning Particle Beams to Grid
            for (Particle_beam& particle_beam : particle_beams)
            {
                particle_beam.Set_Ticked(false);

                bool overlap = true ? (particle_beam.Get_Rectangle().intersectsCircle(grid.getArea(), grid.getArea().x / 2 + grid.getArea().y / 2)) : false;
                if (overlap) grid.addParticle_beam(&particle_beam);
            }
        }
    });

    for (auto& grid : grids)
    {
        // Assigning Tanks to Grid
        for (Tank& btank : btanks)
        {
            btank.Set_Ticked(false);

            // Overflow decides the amount of distance grids overlap eachother, this way tanks can be in multiple grids at once (making them check multiple grids for collisions with other tanks)
            float overflow = btank.Get_collision_radius();

            bool overlap = true ? (btank.Get_Position().x >= grid.getArea().x - grid.getWidth() - overflow && btank.Get_Position().x < grid.getArea().x + overflow && btank.Get_Position().y >= grid.getArea().y - grid.getWidth() - overflow && btank.Get_Position().y < grid.getArea().y + overflow) : false;
            if (overlap) grid.addBlueTank(&btank);
        }
        for (Tank& rtank : rtanks)
        {
            rtank.Set_Ticked(false);

            // Overflow decides the amount of distance grids overlap eachother, this way tanks can be in multiple grids at once (making them check multiple grids for collisions with other tanks)
            float overflow = rtank.Get_collision_radius();

            bool overlap = true ? (rtank.Get_Position().x >= grid.getArea().x - grid.getWidth() - overflow && rtank.Get_Position().x < grid.getArea().x + overflow && rtank.Get_Position().y >= grid.getArea().y - grid.getWidth() - overflow && rtank.Get_Position().y < grid.getArea().y + overflow) : false;
            if (overlap) grid.addRedTank(&rtank);
        }
    }
}

void Game::UpdateTanks()
{
    for (auto& grid : grids)
    {
        //Update Tanks
        for (Tank* tank : grid.getTanks())
        {
            if (!tank->Is_Active()) continue;
            //Check tank collision and nudge tanks away from each other
            for (Tank* oTank : grid.getTanks())
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
            if (!tank->Has_Ticked())
            {
                tank->Tick();
            }

            //Shoot at closest target if reloaded
            if (tank->Rocket_Reloaded())
            {
                Tank& target = FindClosestEnemy(*tank);

                rockets.push_back(Rocket(tank->Get_Position(), (target.Get_Position() - tank->Get_Position()).normalized() * 3, rocket_radius, tank->Get_Allignment(), ((tank->Get_Allignment() == RED) ? &rocket_red : &rocket_blue)));

                tank->Reload_Rocket();
            }
        }
    }
}

void Game::UpdateRockets()
{
    for (auto& grid : grids)
    {
        //Update rockets
        for (Rocket* rocket : grid.getRockets())
        {
            if (!rocket->Has_Ticked())
            {
                rocket->Tick();
                rocket->Set_Ticked(true);
            }

            //Check if rocket collides with enemy tank, spawn explosion and if tank is destroyed spawn a smoke plume
            if (rocket->Get_Allignment() == BLUE)
            {
                for (Tank* tank : grid.getRedTanks())
                {
                    if (tank->Is_Active() && rocket->Intersects(tank->Get_Position(), tank->Get_collision_radius()))
                    {
                        explosions.push_back(Explosion(&explosion, tank->Get_Position()));

                        if (tank->hit(ROCKET_HIT_VALUE))
                        {
                            smokes.push_back(Smoke(smoke, tank->Get_Position() - vec2(0, 48)));
                        }

                        rocket->Deactivate();
                        break;
                    }
                }
            }
            else
            {
                for (Tank* tank : grid.getBlueTanks())
                {
                    if (tank->Is_Active() && rocket->Intersects(tank->Get_Position(), tank->Get_collision_radius()))
                    {
                        explosions.push_back(Explosion(&explosion, tank->Get_Position()));

                        if (tank->hit(ROCKET_HIT_VALUE))
                        {
                            smokes.push_back(Smoke(smoke, tank->Get_Position() - vec2(0, 48)));
                        }

                        rocket->Deactivate();
                        break;
                    }
                }
            }
        }
    }
}

void Game::UpdateParticleBeams()
{
    for (auto& grid : grids)
    {
        //Update particle beams
        for (Particle_beam* particle_beam : grid.getParticle_beams())
        {
            if (!particle_beam->Has_Ticked())
            {
                particle_beam->tick();
                particle_beam->Set_Ticked(true);
            }

            //Damage all tanks within the damage window of the beam (the window is an axis-aligned bounding box)
            for (Tank* tank : grid.getTanks())
            {
                if (tank->Is_Active() && particle_beam->Get_Rectangle().intersectsCircle(tank->Get_Position(), tank->Get_collision_radius()))
                {
                    if (tank->hit(particle_beam->Get_Damage()))
                    {
                        smokes.push_back(Smoke(smoke, tank->Get_Position() - vec2(0, 48)));
                    }
                }
            }
        }
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
    UpdateGrids(); // Assigns Tanks, Rockets and Particle Beams to Grid

    UpdateTanks();

    draw_screen.wait(); // Assigned in Tick()
    draw_blue_tanks = pool.enqueue([&] {
        DrawBlueTanks();
    });
    draw_red_tanks = pool.enqueue([&] {
        DrawRedTanks();
    });

    add_rockets.wait(); // Assigned in UpdateGrid()
    future<void> update_rockets = pool.enqueue([&] {
        UpdateRockets();
    });

    add_particleBeams.wait(); // Assigned in UpdateGrid()
    UpdateParticleBeams();

    update_rockets.wait();
    sort_tanks = pool.enqueue([&] {
        SortTanks();
    });

    draw_blue_tanks.wait();
    draw_red_tanks.wait();
    draw_rockets = pool.enqueue([&] {
        DrawRockets();
    });

    //Update explosion sprites and remove when done with remove erase idiom
    for (Explosion& explosion : explosions) //N
    {
        explosion.Tick();
    }
    explosions.erase(std::remove_if(explosions.begin(), explosions.end(), [](const Explosion& explosion) { return explosion.done(); }), explosions.end());

    //Remove exploded rockets with remove erase idiom
    rockets.erase(std::remove_if(rockets.begin(), rockets.end(), [](Rocket& rocket) {
                      return !rocket.Is_Active() || rocket.Get_Position().x < 0 || rocket.Get_Position().x > SCRWIDTH || rocket.Get_Position().y < 0 || rocket.Get_Position().y > SCRHEIGHT;
                  }),
                  rockets.end());

    //Update smoke plumes
    for (Smoke& smoke : smokes) //N
    {
        smoke.Tick();
    }
}

bool sortByHealth(const Tank& ltank, const Tank& rtank) { return ltank.Get_Health() < rtank.Get_Health(); }

void Game::DrawAll() //N^2
{
    sort_tanks = pool.enqueue([&] {
        SortTanks();
    });
    draw_screen.wait();
    DrawBlueTanks();
    DrawRedTanks();
    DrawRockets();
    DrawExplosions();
    DrawSmokes();
    DrawParticleBeams();
    DrawExplosions();
    DrawSmokes();
    sort_tanks.wait();
    DrawHealthBar();
}

void Game::DrawParticleBeams()
{
    for (Particle_beam& particle_beam : particle_beams) //N
    {
        particle_beam.Draw(screen);
    }
}

void Game::DrawScreen()
{
    // clear the graphics window
    screen->Clear(0);

    //Draw background
    background.Draw(screen, 0, 0);
}

void Game::DrawBlueTanks()
{
    for (int i = 0; i < NUM_TANKS_BLUE; i++) //N
    {
        // Don't draw tanks if out of view
        if (btanks.at(i).Get_Position().x < -btanks.at(i).Get_collision_radius() || btanks.at(i).Get_Position().x > SCRWIDTH + btanks.at(i).Get_collision_radius() ||
            btanks.at(i).Get_Position().y < -btanks.at(i).Get_collision_radius() || btanks.at(i).Get_Position().y > SCRHEIGHT + btanks.at(i).Get_collision_radius())
            continue;

        btanks[i].Draw(screen);

        vec2 tPos = btanks[i].Get_Position();
        // tread marks
        if ((tPos.x >= 0) && (tPos.x < SCRWIDTH) && (tPos.y >= 0) && (tPos.y < SCRHEIGHT))
            background.GetBuffer()[(int)tPos.x + (int)tPos.y * SCRWIDTH] = SubBlend(background.GetBuffer()[(int)tPos.x + (int)tPos.y * SCRWIDTH], 0x808080);
    }
}

void Game::DrawRedTanks()
{
    for (int i = 0; i < NUM_TANKS_RED; i++) //N
    {
        // Don't draw tanks if out of view
        if (rtanks.at(i).Get_Position().x < -rtanks.at(i).Get_collision_radius() || rtanks.at(i).Get_Position().x > SCRWIDTH + rtanks.at(i).Get_collision_radius() ||
            rtanks.at(i).Get_Position().y < -rtanks.at(i).Get_collision_radius() || rtanks.at(i).Get_Position().y > SCRHEIGHT + rtanks.at(i).Get_collision_radius())
            continue;

        rtanks[i].Draw(screen);

        vec2 tPos = rtanks[i].Get_Position();
        // tread marks
        if ((tPos.x >= 0) && (tPos.x < SCRWIDTH) && (tPos.y >= 0) && (tPos.y < SCRHEIGHT))
            background.GetBuffer()[(int)tPos.x + (int)tPos.y * SCRWIDTH] = SubBlend(background.GetBuffer()[(int)tPos.x + (int)tPos.y * SCRWIDTH], 0x808080);
    }
}

void Game::DrawRockets()
{
    for (Rocket& rocket : rockets) //N
    {
        rocket.Draw(screen);
    }
}

void Game::DrawExplosions()
{
    for (Explosion& explosion : explosions) //N
    {
        explosion.Draw(screen);
    }
}

void Game::DrawSmokes()
{
    for (Smoke& smoke : smokes) //N
    {
        smoke.Draw(screen);
    }
}

void Game::SortTanks()
{
    future<void> sort_btanks = pool.enqueue([&] {
        std::sort(btanks.begin(), btanks.end(), sortByHealth);
    });
    std::sort(rtanks.begin(), rtanks.end(), sortByHealth);
    sort_btanks.wait();
}

void Game::DrawHealthBar()
{
    //Draw sorted health bars
    std::future<void> draw_btanks_health = pool.enqueue(move([&] {
        for (int i = 0; i < NUM_TANKS_BLUE; i++) //N
        {
            int health_bar_start_x = i * (HEALTH_BAR_WIDTH + HEALTH_BAR_SPACING) + HEALTH_BARS_OFFSET_X;
            int health_bar_start_y = 0;
            int health_bar_end_x = health_bar_start_x + HEALTH_BAR_WIDTH;
            int health_bar_end_y = HEALTH_BAR_HEIGHT;

            screen->Bar(health_bar_start_x, health_bar_start_y, health_bar_end_x, health_bar_end_y, REDMASK);
            screen->Bar(health_bar_start_x, health_bar_start_y + (int)((double)HEALTH_BAR_HEIGHT * (1 - ((double)btanks.at(i).Get_Health() / (double)TANK_MAX_HEALTH))), health_bar_end_x, health_bar_end_y, GREENMASK);
        }
    }));
    for (int i = 0; i < NUM_TANKS_RED; i++) //N
    {
        int health_bar_start_x = i * (HEALTH_BAR_WIDTH + HEALTH_BAR_SPACING) + HEALTH_BARS_OFFSET_X;
        int health_bar_start_y = (SCRHEIGHT - HEALTH_BAR_HEIGHT) - 1;
        int health_bar_end_x = health_bar_start_x + HEALTH_BAR_WIDTH;
        int health_bar_end_y = SCRHEIGHT - 1;

        screen->Bar(health_bar_start_x, health_bar_start_y, health_bar_end_x, health_bar_end_y, REDMASK);
        screen->Bar(health_bar_start_x, health_bar_start_y + (int)((double)HEALTH_BAR_HEIGHT * (1 - ((double)rtanks.at(i).Get_Health() / (double)TANK_MAX_HEALTH))), health_bar_end_x, health_bar_end_y, GREENMASK);
    }
    draw_btanks_health.wait();
}

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
    draw_screen = pool.enqueue(move([&] {
        DrawScreen();
    }));
    if (!lock_update)
    {
        Update(deltaTime);
        draw_rockets.wait(); // Assigned in Update()
        DrawParticleBeams();
        DrawExplosions();
        DrawSmokes();
        sort_tanks.wait(); // Assigned in Update()
        DrawHealthBar();
    }
    else
        DrawAll();

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
