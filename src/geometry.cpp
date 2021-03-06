#include "geometry.hpp"
#include <libapt/apt.hpp>
#include "graphics/flextGL.hpp"
#include "graphics/clipmask.hpp"
#include "graphics/buffer.hpp"
#include "util.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <sstream>
using namespace libapt;

Shader Geometry::s_shader;
uint32_t Geometry::s_vao = 0;
std::string Geometry::s_vertSrc =
"#version 330\n"
"layout(location = 0)in vec2 vert;\n"
"out vec2 fragVert;\n"
"uniform vec2 ptranslate;\n"
"uniform vec2 translate;\n"
"uniform mat2 protscale;\n"
"uniform mat2 rotscale;\n"
"uniform mat4 ortho;\n"
"void main()\n"
"{\n"
"	fragVert = rotscale*vert+translate;\n"
"   vec2 sc=protscale*vert+ptranslate;\n"
"	gl_Position = ortho*vec4(sc,0,1);\n"
"}";

std::string Geometry::s_fragSrc =
"#version 330\n"
"in vec2 fragVert;\n"
"out vec4 fragColor;\n"
"uniform vec4 color;\n"
"uniform sampler2D tex;\n"
"uniform sampler2D mask;\n"
"uniform bool textured;\n"
"uniform bool masked;\n"
"void main()\n"
"{\n"
"	ivec2 ts = textureSize(tex,0);\n"						//texture size
"	ivec2 ms = textureSize(mask,0);\n"						//mask size
"	fragColor = color;\n"
"	if(textured) {\n"										//if shape is textured
"	vec2 tc = vec2(fragVert.x/ts.x,-fragVert.y/ts.y);\n"	//calculate texture coordinates
"	fragColor *= texture(tex,tc);\n"						//get texture color
"	}\n"
"   if(masked) {\n"
"   vec4 fc = gl_FragCoord;\n"								//masked shape
"	vec2 tc = vec2(fc.x/ms.x,fc.y/ms.y);\n"					//calculate texture coords
"   vec4 maskColor = texture(mask,tc);\n"
"   fragColor.a *= maskColor.a;\n"							//mask alpha channel
"   }\n"
"}";

Geometry::Geometry(uint32_t width, uint32_t height)
{
	if (!s_shader.Valid())
	{
		s_shader.Load(s_vertSrc, s_fragSrc);
		s_shader.addUniform("translate");
		s_shader.addUniform("rotscale");
		s_shader.addUniform("ptranslate");
		s_shader.addUniform("protscale");
		s_shader.addUniform("ortho");
		s_shader.addUniform("color");
		s_shader.addUniform("tex");
		s_shader.addUniform("textured");
		s_shader.addUniform("mask");
		s_shader.addUniform("masked");

		glGenVertexArrays(1, &s_vao);
		glBindVertexArray(s_vao);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0,
			2,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0
		);
		glBindVertexArray(0);
	}

	m_ortho = glm::ortho(0.0, static_cast<double>(width), 
		static_cast<double>(height),0.0,0.0,10.0);
}

Error Geometry::Load(const std::string & data)
{
	std::stringstream filestream(data);
	std::stringstream linestream;
	std::string line;
	bool valid = false;
	char parseChar;
	std::string param;
	std::vector<Triangle> triangles;
	std::vector<Line> lines;
	Style currentStyle = UNDEFINED;
	glm::u8vec4 color;
	int thickness = 1;
	int image = 0;
	std::vector<std::string> params;
	glm::mat2 rotscale;
	glm::vec2 translate;

	while (std::getline(filestream, line))
	{
		//Parse the line
		trim(line);
		linestream.str(line);

		linestream.get(parseChar);

		while (std::getline(linestream, param, ':'))
		{
			trim(param);
			params.push_back(param);
		}

		switch (parseChar)
		{
		case 'c':
			switch (currentStyle)
			{
			case LINES:
				AddLines(lines, color, thickness);
				break;
			case SOLID_TRIS:
				AddSolidTriangles(triangles, color);
				break;
			case TEXTURED_TRIS:
				AddTexturedTriangles(triangles, color, image, rotscale, translate);
				break;
			case UNDEFINED:
				//don't do anything in this case
				break;
			}
			break;
		case 's':
		{
			if (params.front() == "s")
			{
				if (params.size() != 5)
				{
					std::cout << "Invalid parameter count in geometry file!" << std::endl;
					return INVALID_GEOM;
				}

				currentStyle = SOLID_TRIS;
				color.r = std::stoi(params[1]);
				color.g = std::stoi(params[2]);
				color.b = std::stoi(params[3]);
				color.a = std::stoi(params[4]);
			}
			else if (params.front() == "l")
			{
				if (params.size() != 6)
				{
					std::cout << "Invalid parameter count in geometry file!" << std::endl;
					return INVALID_GEOM;
				}

				currentStyle = LINES;
				thickness = std::stoi(params[1]);
				color.r = std::stoi(params[2]);
				color.g = std::stoi(params[3]);
				color.b = std::stoi(params[4]);
				color.a = std::stoi(params[5]);
			}
			else if (params.front() == "tc")
			{
				if (params.size() != 12)
				{
					std::cout << "Invalid parameter count in geometry file!" << std::endl;
					return INVALID_GEOM;
				}

				currentStyle = TEXTURED_TRIS;
				color.r = std::stoi(params[1]);
				color.g = std::stoi(params[2]);
				color.b = std::stoi(params[3]);
				color.a = std::stoi(params[4]);
				image = std::stoi(params[5]);
				rotscale[0][0] = std::stof(params[6]);
				rotscale[0][1] = std::stof(params[7]);
				rotscale[1][0] = std::stof(params[8]);
				rotscale[1][1] = std::stof(params[9]);
				translate[0] = std::stof(params[10]);
				translate[1] = std::stof(params[11]);
			}
			else
			{
				std::cout << "Invalid geometry file!" << std::endl;
				return INVALID_GEOM;
			}
		}
		break;
		case 't':
		{
			Triangle t;
			t.v1 = glm::vec2(std::stoi(params[0]), std::stoi(params[1]));
			t.v2 = glm::vec2(std::stoi(params[2]), std::stoi(params[3]));
			t.v3 = glm::vec2(std::stoi(params[4]), std::stoi(params[5]));
			triangles.push_back(t);
			break;
		}
		case 'l':
		{
			Line l;
			l.v1 = glm::vec2(std::stoi(params[0]), std::stoi(params[1]));
			l.v2 = glm::vec2(std::stoi(params[2]), std::stoi(params[3]));
			lines.push_back(l);
			break;
		}
		}
		linestream.clear();
		params.clear();
	}

	switch (currentStyle)
	{
	case LINES:
		AddLines(lines, color, thickness);
		break;
	case SOLID_TRIS:
		AddSolidTriangles(triangles, color);
		break;
	case TEXTURED_TRIS:
		AddTexturedTriangles(triangles, color, image, rotscale, translate);
		break;
	case UNDEFINED:
		//do nothing
		break;
	}
	return NO_ERROR;
}

void Geometry::AddLines(std::vector<Line>& lines, glm::u8vec4 color, int thickness)
{
	Entry ls;
	ls.style = LINES;
	ls.color = color;
	ls.lines = lines;
	ls.thickness = thickness;
	m_entries.push_back(ls);
	lines.clear();
}

void Geometry::AddSolidTriangles(std::vector<Triangle>& tris, glm::u8vec4 color)
{
	Entry st;
	st.style = SOLID_TRIS;
	st.color = color;
	st.triangles = tris;
	m_entries.push_back(st);
	tris.clear();
}

void Geometry::AddTexturedTriangles(std::vector<Triangle>& tris, glm::u8vec4 color, uint32_t image,
	glm::mat2 rotscale, glm::vec2 translate)
{
	Entry tt;
	tt.style = TEXTURED_TRIS;
	tt.color = color;
	tt.image = image;
	tt.triangles = tris;
	tt.rotscale = rotscale;
	tt.translate = translate;
	m_entries.push_back(tt);
	tris.clear();
}

Geometry::Entry::Entry() : thickness(0), image(0),style(UNDEFINED)
{
}

//compile to OpenGL buffer objects
void Geometry::Compile(std::shared_ptr<Apt> apt)
{
	auto buffer = apt->GetGeometryBuffer();
	
	for (auto& e : m_entries)
	{
		Object obj;
		obj.color = glm::vec4(e.color.r / 255.0, e.color.g / 255.0,
			e.color.b / 255.0, e.color.a / 255.0);
		obj.rotscale = e.rotscale;
		obj.translate = e.translate;

		switch (e.style)
		{
		case LINES:
			for (auto& line : e.lines)
			{
				auto tris = TriangulateLine(line, e.thickness);
				e.triangles.insert(e.triangles.end(),tris.begin(),tris.end());
			}
			break;
		case TEXTURED_TRIS:
			obj.textured = true;
			obj.texture = apt->GetTexture(e.image);
			break;
		case SOLID_TRIS:
			//nothing to do
			break;
		case UNDEFINED:
			std::cout << "Can't compile undefined geometry" << std::endl;
			break;
		}

		std::vector<glm::vec2> verts;
		for (auto& t : e.triangles)
		{
			verts.push_back(t.v1);
			verts.push_back(t.v2);
			verts.push_back(t.v3);
		}

		//store offset in the vbo
		obj.start = buffer->Append(verts);
		//add triangles of this object to the vbo
		obj.numVerts = verts.size();
		m_objects.push_back(obj);
	}
	m_entries.clear();
}

std::vector<Geometry::Triangle> Geometry::TriangulateLine(Line l, uint32_t thickness)
{
	std::vector<Triangle> tris;
	glm::vec2 lineDir = l.v2 - l.v1;
	lineDir = glm::normalize(lineDir);
	glm::vec2 lineNormal = glm::vec2(lineDir.y, -lineDir.x);
	Triangle t1,t2;
	t1.v1 = l.v1;
	t1.v2 = l.v2;
	t1.v3 = l.v1 + lineNormal*static_cast<float>(thickness);
	t2.v1 = l.v1 + lineNormal*static_cast<float>(thickness);
	t2.v2 = l.v2;
	t2.v3 = l.v2 + lineNormal*static_cast<float>(thickness);
	tris.push_back(t1);
	tris.push_back(t2);
	return tris;
}


void Geometry::Draw(const Transformation& t, std::shared_ptr<Apt> apt)
{
	s_shader.Use();
	glBindVertexArray(s_vao);
	apt->GetGeometryBuffer()->Bind();
	glUniformMatrix2fv(s_shader.uniform("protscale"), 1, GL_FALSE, glm::value_ptr(t.rotscale));
	glUniform2fv(s_shader.uniform("ptranslate"), 1, glm::value_ptr(t.translate));
	glUniformMatrix4fv(s_shader.uniform("ortho"), 1, GL_FALSE, glm::value_ptr(m_ortho));
	glUniform1i(s_shader.uniform("tex"), 0);

	if (t.mask != nullptr)
	{
		glUniform1i(s_shader.uniform("mask"), 1);
		glUniform1i(s_shader.uniform("masked"), true);
		glActiveTexture(GL_TEXTURE1);
		t.mask->BindMask();
	}
	else
	{
		glUniform1i(s_shader.uniform("masked"), false);
	}

	for (const auto& obj : m_objects)
	{	
		glVertexAttribPointer(
			0,
			2,
			GL_FLOAT,
			GL_FALSE,
			0,
			nullptr
		);

		glUniform1i(s_shader.uniform("textured"), obj.textured);
			
		if (obj.textured)
		{
			glActiveTexture(GL_TEXTURE0);
			obj.texture->Bind();
		}
				
		glm::vec4 finalColor = obj.color * t.color;
		glUniform4fv(s_shader.uniform("color"),1,glm::value_ptr(finalColor));
		glUniformMatrix2fv(s_shader.uniform("rotscale"), 1, GL_FALSE, glm::value_ptr(obj.rotscale));
		glUniform2fv(s_shader.uniform("translate"), 1, glm::value_ptr(obj.translate));
		glDrawArrays(GL_TRIANGLES, obj.start, obj.numVerts);
	}

	glBindVertexArray(0);
}

Geometry::Object::Object() : textured(false),texture(nullptr),start(0)
{
}