#pragma once

namespace Tmpl8
{

class Explosion
{
  private:
    vec2 position;

    int current_frame;
    Sprite* explosion_sprite;
  public:
    Explosion(Sprite* explosion_sprite, vec2 position) : current_frame(0), explosion_sprite(explosion_sprite), position(position) {}

    bool done() const;
    void Tick();
    void Draw(Surface* screen);

};

}
