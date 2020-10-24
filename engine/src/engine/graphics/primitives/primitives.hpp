
namespace fightingengine {

namespace primitives {

class Plane {
public:
    Plane() = default;

    void init();
    void draw();

private:
    unsigned int quadVAO_;
    unsigned int quadVBO_;
};

class Cube {
public:
    Cube() = default;

    void init();
    void draw();

private:
    unsigned int cubeVAO_;
    unsigned int cubeVBO_;
};

} //namespace primitives

} //namespace fightingengine