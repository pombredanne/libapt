#pragma once
#include "character.hpp"
#include <libapt/texture.hpp>
#include <vector>

namespace libapt
{
	class Button : public Character
	{
	public:
		virtual void Parse(uint8_t*& iter) override;
		virtual void Update(const Transformation& t, std::shared_ptr<DisplayObject> instance) override;
	private:
		const char* m_name;
		uint32_t m_glyphcount;
		uint8_t* m_glyphs;
	};
}