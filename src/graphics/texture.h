#pragma once

#include "base.h"

#include <string>

namespace fightinggame {

    struct texture
    {
    public:
        virtual ~texture() = default;

        virtual uint32_t get_width() const = 0;
        virtual uint32_t get_height() const = 0;
        virtual uint32_t get_renderer_id() const = 0;
        virtual std::string get_path() const = 0;

        virtual void set_type(std::string type) = 0;
        virtual std::string get_type() const = 0;

        virtual void set_data(void* data, uint32_t size) = 0;

        virtual void bind(uint32_t slot = 0) const = 0;

        virtual bool operator==(const texture& other) const = 0;     
    };

    class texture2D : public texture
    {
    public:
        static Ref<texture2D> Create(uint32_t width, uint32_t height);
        static Ref<texture2D> Create(const char* file_name, const std::string& directory);
    };

}
