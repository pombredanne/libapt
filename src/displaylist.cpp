#include "displaylist.hpp"
#include <iostream>
using namespace libapt;

void DisplayList::Insert(uint32_t depth, std::shared_ptr<Character> ch,
	const glm::vec2 & translate, const glm::mat2 & rotscale, const std::string& name,
	std::shared_ptr<Container> parent)
{
	DisplayObject obj;
	obj.Create(ch,translate,rotscale,name,parent);
	m_objects[depth] = obj;
	#ifndef  NDEBUG
	std::cout << "Placed object at depth: " << depth << std::endl;
	std::cout << "Name: " << name << std::endl;
	#endif // ! NDEBUG
}

void DisplayList::Erase(uint32_t depth)
{
	m_objects.erase(depth);
	#ifndef  NDEBUG
	std::cout << "Removed object at depth: " << depth << std::endl;
	#endif // ! NDEBUG
}

void DisplayList::Move(uint32_t depth, const glm::vec2 & translate, const glm::mat2 & rotscale)
{
	auto& obj = m_objects[depth];
	obj.SetRotscale(rotscale);
	obj.SetTranslate(translate);
	#ifndef NDEBUG
	std::cout << "Moved object at depth: " << depth <<
		" Translate: (" << translate.x << "-" << translate.y << ")" << std::endl;
	#endif // !NDEBUG
}

void DisplayList::Render(const Transformation & t)
{
	for (auto& pair : m_objects)
	{
		pair.second.Render(t);
	}
}
