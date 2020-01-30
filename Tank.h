#pragma once

namespace Tmpl8
{

enum allignments
{
    BLUE,
    RED
};



class Tank
{
  private:
    vec2 position;
    vec2 speed;
    vec2 target;

    int health;

    float collision_radius;
    vec2 force;

    float max_speed;
    float reload_time;

    bool reloaded;
    bool active;
    bool has_ticked;

    allignments allignment;

    int current_frame;
    Sprite* tank_sprite;
    Sprite* smoke_sprite;
  public:
    Tank(float pos_x, float pos_y, allignments allignment, Sprite* tank_sprite, Sprite* smoke_sprite, float tar_x, float tar_y, float collision_radius, int health, float max_speed);

    ~Tank();

    void Tick();

    vec2 Get_Position() const { return position; };
    vec2 Get_Speed() const { return speed; };
    vec2 Get_Target() const { return target; };

    int Get_Health() const { return health; };

    float Get_collision_radius() const { return collision_radius; };

    bool Is_Active() const { return active; };
    void Set_Ticked(bool has_ticked) { this->has_ticked = has_ticked; };
    bool Has_Ticked() const { return has_ticked; };

    allignments Get_Allignment() { return allignment; };

    bool Rocket_Reloaded() const { return reloaded; };

    void Reload_Rocket();

    void Deactivate();
    bool hit(int hit_value);

    void Draw(Surface* screen);

    int CompareHealth(const Tank& other) const;

    void Push(vec2 direction, float magnitude);
};

} // namespace Tmpl8