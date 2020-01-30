#pragma once

namespace Tmpl8
{

class Rocket
{
  private:
    vec2 position;

    bool active;
    vec2 speed;

    float collision_radius;

    bool has_ticked;

    allignments allignment;

    int current_frame;
    Sprite* rocket_sprite;
  public:
    Rocket(vec2 position, vec2 direction, float collision_radius, allignments allignment, Sprite* rocket_sprite);
    ~Rocket();

    void Tick();
    void Draw(Surface* screen);

    bool Intersects(vec2 position_other, float radius_other) const;

    vec2 Get_Position() { return position; };

    float Get_collision_radius() { return collision_radius; };

    bool Is_Active() { return active; };
    void Deactivate() { this->active = false; };

    bool Has_Ticked() { return has_ticked; };
    void Set_Ticked(bool has_ticked) { this->has_ticked = has_ticked; };

    allignments Get_Allignment() { return allignment; };

};

} // namespace Tmpl8