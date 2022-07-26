#pragma once

namespace game2d {

struct SINGLETON_NetworkingUIComponent
{
  bool start_server = false;
  bool start_client = false;
  bool close_networking = false;
};

} // namespace game2d