#pragma once

#include <string>
#include <memory>

namespace fightinggame {

    struct Texture
    {
    public:
        virtual ~Texture() = default;

        virtual uint32_t get_width() const = 0;
        virtual uint32_t get_height() const = 0;
        virtual uint32_t get_renderer_id() const = 0;
        virtual std::string get_path() const = 0;

        virtual void set_type(std::string type) = 0;
        virtual std::string get_type() const = 0;

        virtual void set_data(void* data, uint32_t size) = 0;

        virtual void bind(uint32_t slot = 0) const = 0;
        virtual void unbind(uint32_t slot = 0) const = 0;

        virtual bool operator==(const Texture& other) const = 0;     
    };

    class Texture2D : public Texture
    {
    public:
        static std::shared_ptr<Texture2D> Create(uint32_t width, uint32_t height, std::string directory, std::string unique_name);
        static std::shared_ptr<Texture2D> Create(const char* file_name, const std::string& directory);
    };

}
