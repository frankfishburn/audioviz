#ifndef I_VISUAL_H
#define I_VISUAL_H

#include <string>

class IVisual {
   public:
    virtual ~IVisual(){};
    virtual void draw(const unsigned long) = 0;

    virtual std::string name() = 0;
    virtual void set_resolution(const float width, const float height) = 0;
    virtual void set_resolution(const int width, const int height) = 0;
};

#endif /* I_VISUAL_H */
