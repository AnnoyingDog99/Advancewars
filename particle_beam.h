#pragma once

namespace Tmpl8
{
class Particle_beam
{
  private:
    vec2 min_position;
    vec2 max_position;

    Rectangle2D rectangle;

    int sprite_frame;

    int damage;

    bool has_ticked;

    Sprite* particle_beam_sprite;
  public:
    Particle_beam();
    Particle_beam(vec2 min, vec2 max, Sprite* particle_beam_sprite, int damage);

    void tick();
    void Draw(Surface* screen);

    bool Has_Ticked() { return has_ticked; };
    void Set_Ticked(bool has_ticked) { this->has_ticked = has_ticked; };

    Rectangle2D Get_Rectangle() { return rectangle; };

    int Get_Damage() { return damage; };
};
} // namespace Tmpl8