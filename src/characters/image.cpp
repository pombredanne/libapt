#include "image.hpp"
#include "../util.hpp"
#include <iostream>
using namespace libapt;

void Image::Parse(uint8_t *& iter)
{
	m_image = read<uint32_t>(iter);
}

void Image::Update(const Transformation& t, std::shared_ptr<DisplayObject> dObj)
{
	std::cout << "WARNING: Image should not be able to be on displaylist" << std::endl;
}
