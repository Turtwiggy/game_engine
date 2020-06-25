#pragma once

#include "graphics/texture.h"
#include "singleton.h"
#include "base.h"

#include <array>
#include <vector>
#include <filesystem>
#include <iostream>

class texture_manager : public Singleton<texture_manager>
{
public:
    texture_manager() { std::cout << "texture manager constructed" << std::endl; }
    ~texture_manager() { std::cout << "texture manager destructed" << std::endl; }

    void init()
    {
        Ref<texture2D> white_texture;
        //create a texture default
        white_texture = texture2D::Create(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        white_texture->set_data(&whiteTextureData, sizeof(uint32_t));
        TextureSlots[0] = white_texture;

        //Load a texture
        std::string path = std::string(std::filesystem::current_path().generic_u8string());
        path += "/res/textures/Bamboo";
        Ref<texture2D> loaded_texture = texture2D::Create("BambooWall_1K_albedo.jpg", path);
        TextureSlots[1] = loaded_texture;

        NextTextureSlot = 2;

        //////Models
        ////FGTransform& lizard_transform = models[0];
        ////FGModel& lizard = lizard_transform.model;
        ////std::vector<Ref<texture2D>> lizard_textures = lizard.get_textures();
        ////for (auto tex_index = 0; tex_index < lizard_textures.size(); tex_index++)
        ////{
        ////    std::cout << "adding texture: " << s_Data.TextureSlotIndex << lizard_textures[tex_index]->get_path() << std::endl;
        ////    s_Data.TextureSlots[s_Data.TextureSlotIndex] = lizard_textures[tex_index];
        ////    s_Data.TextureSlotIndex += 1;
        ////}

        ////int32_t samplers[s_Data.MaxTextureSlots];
        ////for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
        ////    samplers[i] = i;

        //for (auto tex_index = 0; tex_index < s_Data.TextureSlots.size(); tex_index++)
        //{
        //    Ref<texture2D> texture = s_Data.TextureSlots[tex_index];
        //    if (texture != nullptr)
        //        std::cout << "idx: " << tex_index << "texture (id) " << texture->get_renderer_id() << " path: " << texture->get_path() << std::endl;
        //}
    };

    void add_texture(Ref<texture2D> texture)
    {
        if (TextureSlots.size() == MaxTextureSlots)
        {
            std::cout << "cannot add more textures!" << std::endl;
            return;
        }

        TextureSlots[NextTextureSlot] = texture;
        NextTextureSlot += 1;
    }


    void bind_textures()
    {
        std::cout << "binding textures" << std::endl;

        //// Bind textures
        //for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
        //{
        //    Ref<texture2D> texture = s_Data.TextureSlots[i];
        //    if (texture == nullptr)
        //        continue;

        //    //std::cout << "binding: " << s_Data.TextureSlots[i]->get_path() << " to id: " << i << std::endl;
        //    s_Data.TextureSlots[i]->bind(i);
        //}
    }

private:

    static const uint32_t MaxTextureSlots = 32;
    uint32_t NextTextureSlot = 0;
    std::array<Ref<texture2D>, MaxTextureSlots> TextureSlots;
};


