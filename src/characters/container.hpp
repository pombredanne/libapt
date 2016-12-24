#pragma once
#include "character.hpp"
#include "../frame.hpp"
#include "../displaylist.hpp"
#include "../actionscript/engine.hpp"
#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <vector>
#include <stdint.h>


namespace libapt
{
    //the base class for movie & sprite
    class Container : public Character
    {
	public:
		Container();
		void Update(const Transformation& t, DisplayObject& dispO);
		virtual void Parse(uint8_t *&iter) = 0;
		inline int GetCurrentFrame() 
		{ 
			return m_currentFrame; 
		}
	protected:
		uint32_t m_framecount;
		std::vector<Frame> m_frames;
	private:
		void HandleAction(std::shared_ptr<FrameItem> fi, DisplayObject& dispO);
		void HandleBackground(std::shared_ptr<FrameItem> fi);
		void HandlePlaceObject(std::shared_ptr<FrameItem> fi);
		void HandleRemoveObject(std::shared_ptr<FrameItem> fi);
		void HandleInitAction(std::shared_ptr<FrameItem> fi);
    private:
		DisplayList m_dl;
		uint32_t m_currentFrame;
		glm::vec4 m_bgColor;
	private:
		static as::Engine s_engine;
    };
}