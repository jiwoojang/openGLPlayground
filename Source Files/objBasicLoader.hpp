#pragma once
#include <stdlib.h>
#include <glm/glm.hpp>
#include <vector>

using namespace glm;

bool loadOBJ(
	const char * path,
	std::vector <vec3> & out_vertices,
	std::vector <vec2> & out_uvs,
	std::vector <vec3> & out_normals
);
