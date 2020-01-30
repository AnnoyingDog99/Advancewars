#include "Tank.h"
#include "precomp.h"

namespace Tmpl8
{
mutex mtx;
condition_variable cond_reloadrocket;
condition_variable cond_hasticked;
condition_variable cond_push;
condition_variable cond_hit;
condition_variable cond_tick;

bool reload_ready = true;
bool hasticked_ready = true;
bool push_ready = true;
bool hit_ready = true;
bool tick_ready = true;

Tank::Tank(
    float pos_x,
    float pos_y,
    allignments allignment,
    Sprite* tank_sprite,
    Sprite* smoke_sprite,
    float tar_x,
    float tar_y,
    float collision_radius,
    int health,
    float max_speed)
    : position(pos_x, pos_y),
      allignment(allignment),
      target(tar_x, tar_y),
      health(health),
      collision_radius(collision_radius),
      max_speed(max_speed),
      force(0, 0),
      reload_time(1),
      reloaded(false),
      speed(0),
      active(true),
      current_frame(0),
      tank_sprite(tank_sprite),
      smoke_sprite(smoke_sprite)
{
}

Tank::~Tank()
{
}

void Tank::Tick()
{
    std::unique_lock<std::mutex> uLock(mtx);
    while (!tick_ready)
    {
        cond_tick.wait(uLock);
        this_thread::yield();
    }
    tick_ready = false;
    vec2 direction = (target - position).normalized();

    //Update using accumulated force
    speed = direction + force;
    position += speed * max_speed * 0.5f;

    //Update reload time
    if (--reload_time <= 0.0f)
    {
        reloaded = true;
    }

    force = vec2(0.f, 0.f);

    if (++current_frame > 8) current_frame = 0;
    this->has_ticked = true;
    tick_ready = true;
    cond_tick.notify_all();
}

//Start reloading timer
void Tank::Reload_Rocket()
{
    std::unique_lock<std::mutex> uLock(mtx);
    while (!reload_ready)
    {
        cond_reloadrocket.wait(uLock);
        this_thread::yield();
    }
    reload_ready = false;
    reloaded = false, reload_time = 200.0f;
    reload_ready = true;
    cond_reloadrocket.notify_all();
}


void Tank::Deactivate()
{
    active = false;
}

//Remove health
bool Tank::hit(int hit_value)
{
    std::unique_lock<std::mutex> uLock(mtx);
    while (!hit_ready)
    {
        cond_hit.wait(uLock);
        this_thread::yield();
    }
    hit_ready = false;
    health -= hit_value;

    if (health <= 0)
    {
        this->Deactivate();
        hit_ready = true;
        cond_hit.notify_all();
        return true;
    }
    hit_ready = true;
    cond_hit.notify_all();
    return false;
}

//Draw the sprite with the facing based on this tanks movement direction
void Tank::Draw(Surface* screen)
{
    vec2 direction = (target - position).normalized();
    tank_sprite->SetFrame(((abs(direction.x) > abs(direction.y)) ? ((direction.x < 0) ? 3 : 0) : ((direction.y < 0) ? 9 : 6)) + (current_frame / 3));
    tank_sprite->Draw(screen, (int)position.x - 14, (int)position.y - 18);
}

int Tank::CompareHealth(const Tank& other) const
{
    return ((health == other.health) ? 0 : ((health > other.health) ? 1 : -1));
}

//Add some force in a given direction
void Tank::Push(vec2 direction, float magnitude)
{
    std::unique_lock<std::mutex> uLock(mtx);
    while (!push_ready)
    {
        cond_push.wait(uLock);
        this_thread::yield();
    }
    push_ready = false;
    force += direction * magnitude;
    push_ready = true;
    cond_push.notify_all();
}

} // namespace Tmpl8