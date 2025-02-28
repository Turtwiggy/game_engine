#include "spawner_components.hpp"

namespace game2d {

bool
operator==(const WaveKey& a, const WaveKey& b)
{
  return a.on_disk_spawns_index == b.on_disk_spawns_index && a.on_disk_waves_index == b.on_disk_waves_index;
};

} // namespace game2d