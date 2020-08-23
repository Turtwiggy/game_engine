#pragma once

#include <SDL2/SDL.h>

#include <cstdint>
#include <memory>
#include <string>

namespace fightingengine
{
	enum display_mode
	{
		Windowed,
		Fullscreen,
		Borderless
	};

	class GameWindow
	{
		struct SDLDestroyer
		{
			void operator()(SDL_Window* window) const { SDL_DestroyWindow(window); }
		};

	public:
		GameWindow(const std::string& title, const SDL_DisplayMode& display, display_mode displaymode);
		GameWindow(const std::string& title, int width, int height, display_mode displaymode);

		[[nodiscard]] SDL_Window* GetHandle() const;
		//void GetNativeHandles(void*& native_window, void*& native_display) const;

		[[nodiscard]] bool IsOpen() const;
		[[nodiscard]] float GetBrightness() const;
		void SetBrightness(float bright);
		[[nodiscard]] uint32_t GetID() const;
		[[nodiscard]] uint32_t GetFlags() const;
		void GrabInput(bool b = true);
		void SetMousePosition(int x, int y);
		[[nodiscard]] bool IsInputGrabbed() const;
		void ToggleMouseCaptured();
		[[nodiscard]] float GetAspectRatio() const;

		void SetTitle(const std::string& str);
		[[nodiscard]] std::string GetTitle() const;

		void Show();
		void Hide();

		void SetPosition(int x, int y);
		void GetPosition(int& x, int& y) const;
		void SetSize(int width, int height);
		void GetSize(int& width, int& height) const;
		void SetMinimumSize(int width, int height);
		void GetMinimumSize(int& width, int& height) const;
		void SetMaximumSize(int width, int height);
		void GetMaximumSize(int& width, int& height) const;

		void Minimise();
		void Maximise();
		void Restore();
		void Raise();
		void SetBordered(bool b = true);
		void SetFullscreen(bool f = true);
        bool ToggleFullscreen();    //returns the new value

        SDL_GLContext& get_gl_context();
        std::string get_glsl_version();

		void Close();

	private:
		std::unique_ptr<SDL_Window, SDLDestroyer> _window;

        SDL_GLContext gl_context;
        std::string glsl_version = "#version 430";
    };
}