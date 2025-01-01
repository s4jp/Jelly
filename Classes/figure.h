#pragma once
#define _USE_MATH_DEFINES
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"

#include <glm/glm.hpp>
#include <tuple>
#include <vector>

const int dimensions = 3;

class Figure
{
protected:
  VAO vao;
  VBO vbo;
  EBO ebo;

  size_t indices_count;
  size_t vertices_count;

public:
  virtual void Render(int colorLoc) = 0;

  void Delete() {
    vao.Delete();
    vbo.Delete();
    ebo.Delete();
  }

  Figure(std::tuple<std::vector<GLfloat>, std::vector<GLuint>> data) {
	vertices_count = std::get<0>(data).size() / dimensions;
    indices_count = std::get<1>(data).size();

    vao.Bind();
    vbo = VBO(std::get<0>(data).data(), std::get<0>(data).size() * sizeof(GLfloat));
    ebo = EBO(std::get<1>(data).data(), std::get<1>(data).size() * sizeof(GLuint));

    vao.LinkAttrib(vbo, 0, dimensions, GL_FLOAT, 0, (void *)0);
    vao.Unbind();
    vbo.Unbind();
    ebo.Unbind();
  }

  void RefreshBuffers(std::tuple<std::vector<GLfloat>, std::vector<GLuint>> data) {
	  vertices_count = std::get<0>(data).size() / dimensions;
      indices_count = std::get<1>(data).size();

	  vao.Bind();

      vbo.ReplaceBufferData(std::get<0>(data).data(), std::get<0>(data).size() * sizeof(GLfloat));
      ebo.ReplaceBufferData(std::get<1>(data).data(), std::get<1>(data).size() * sizeof(GLuint));

	  vao.Unbind();
  }

  void RefreshVertices(std::vector<GLfloat> vertices) {
	  vertices_count = vertices.size() / dimensions;
	  vao.Bind();
	  vbo.ReplaceBufferData(vertices.data(), vertices.size() * sizeof(GLfloat));
	  vao.Unbind();
  }
};