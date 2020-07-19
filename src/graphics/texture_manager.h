#pragma once

#include "graphics/texture.h"
#include "util/singleton.h"
#include "util/base.h"

#include <array>
#include <vector>
#include <filesystem>
#include <iostream>

namespace fightinggame {

    class TextureManager : public Singleton<TextureManager>
    {
    public:
        TextureManager() { std::cout << "texture manager constructed" << std::endl; }
        ~TextureManager() { std::cout << "texture manager destructed" << std::endl; }

        void init()
        {
            Ref<Texture2D> white_texture = Texture2D::Create(1, 1, "white_texture");
            uint32_t whiteTextureData = 0xffffffff;
            white_texture->set_data(&whiteTextureData, sizeof(uint32_t));
            TextureSlots[0] = white_texture;

            //Load a texture
            std::string path = std::string(std::filesystem::current_path().generic_u8string());
            path += "/assets/textures/Bamboo";
            Ref<Texture2D> loaded_texture = Texture2D::Create("BambooWall_1K_albedo.jpg", path);
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

        void add_texture(Ref<Texture2D> texture)
        {
            if (TextureSlots.size() == MaxTextureSlots)
            {
                std::cout << "cannot add more textures!" << std::endl;
                return;
            }

            TextureSlots[NextTextureSlot] = texture;
            NextTextureSlot += 1;
        }


        void bind_texture(std::string texture_name)
        {
            for (uint32_t i = 0; i < MaxTextureSlots; i++)
            {
                Ref<Texture2D> texture = TextureSlots[i];
                if (texture == nullptr)
                    continue;

                if (texture->get_path().compare(texture_name) == 0) {
                    std::cout << "binding: " << TextureSlots[i]->get_path() << " to id: " << i << std::endl;
                    TextureSlots[i]->bind(i);
                    return;
                }
            }
        }

        void unbind_texture(std::string texture_name) {
            for (uint32_t i = 0; i < MaxTextureSlots; i++)
            {
                Ref<Texture2D> texture = TextureSlots[i];
                if (texture == nullptr)
                    continue;

                if (texture->get_path().compare(texture_name) == 0) {
                    std::cout << "binding: " << TextureSlots[i]->get_path() << " to id: " << i << std::endl;
                    TextureSlots[i]->unbind(i);
                    return;
                }
            }
        }

    private:

        static const uint32_t MaxTextureSlots = 32;
        uint32_t NextTextureSlot = 0;
        std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
    };

}
