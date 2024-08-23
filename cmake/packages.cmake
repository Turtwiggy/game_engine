
function(find_packages)
  # Add VCPKG packages
  if(EMSCRIPTEN)
  else()
    # packages not supported or needed by emscripten
    # find_package(GameNetworkingSockets CONFIG REQUIRED)
    find_package(GLEW REQUIRED)
    find_package(SDL2 CONFIG REQUIRED)
  endif()

  find_package(box2d CONFIG REQUIRED)
  find_package(fmt CONFIG REQUIRED)
  find_package(glm CONFIG REQUIRED)
  find_package(nlohmann_json CONFIG REQUIRED)
  find_package(SDL2_mixer CONFIG REQUIRED)
  find_package(Stb REQUIRED)
endfunction()

function(link_libs project)
  if(EMSCRIPTEN)
  # target_link_libraries(${project} PRIVATE openal)
  else()
    # target_link_libraries(game_defence_tests PRIVATE GameNetworkingSockets::shared)
    target_link_libraries(${project} PRIVATE SDL2::SDL2 SDL2::SDL2main)
    target_link_libraries(${project} PRIVATE GLEW::GLEW)
    target_link_libraries(${project} PRIVATE $<IF:$<TARGET_EXISTS:SDL2_mixer::SDL2_mixer>,SDL2_mixer::SDL2_mixer,SDL2_mixer::SDL2_mixer-static>)
  endif()

  target_link_libraries(${project} PRIVATE box2d::box2d)
  target_link_libraries(${project} PRIVATE fmt::fmt)
  target_link_libraries(${project} PRIVATE glm::glm)
  target_link_libraries(${project} PRIVATE nlohmann_json::nlohmann_json)
endfunction()
