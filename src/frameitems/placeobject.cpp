#include "placeobject.hpp"
#include "../util.hpp"
#include <iostream>
using namespace libapt;

void PlaceObject::Parse(uint8_t* iter, const uint8_t *base)
{
	m_flags = read<Flags>(iter);
	m_depth = read<int32_t>(iter);
	m_character = read<int32_t>(iter);
	m_rotscale = read<glm::f32mat2>(iter);

	m_translate = read<glm::f32vec2>(iter);
	m_color = read<glm::u8vec4>(iter);
	m_unknown = read<uint32_t>(iter);
	m_ratio = read<glm::f32>(iter);
	uint32_t nameOffset = read<uint32_t>(iter);
	if(nameOffset)
		m_name = readString(base+nameOffset);
	m_clipdepth = read<int32_t>(iter);
	uint32_t poaOffset = read<uint32_t>(iter);
	
	if (m_flags.PlaceFlagHasClipActions && poaOffset)
	{
		uint8_t* poa_offset = const_cast<uint8_t*>(base) + poaOffset;
		auto fi = FrameItem::Create(poa_offset, base);
		if (fi->GetType() != ACTION)
		{
			std::cout << "WTF mate" << std::endl;
		}

		m_action = std::dynamic_pointer_cast<Action>(fi);
	}
}