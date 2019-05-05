#include "objBasicLoader.hpp"

bool loadOBJ(const char * path, std::vector <glm::vec3> & out_vertices, std::vector <glm::vec2> & out_uvs, std::vector <glm::vec3> & out_normals)
{
	std::vector <unsigned int> vertexIndices, uvIndicies, normalIndicies;
	std::vector <vec3> temp_vertices, temp_normals; 
	std::vector <vec2> temp_uvs;

	// Open a file 
	FILE *file = fopen(path, "r");

	if (file == NULL)
	{
		printf("Could not open OBJ file\n");
		return false;
	}

	// Read the file line by line 
	while (true)
	{
		// Naive but sufficient assumption that the first word of a line will not be longer than 128 bytes
		char lineHeader[128];

		int res = fscanf(file, "%s", lineHeader);
		
		// Last line in the file
		if (res == EOF)
			break;

		// Vertex data
		if (strcmp(lineHeader, "v") == 0)
		{
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		// Vertex texture data
		else if (strcmp(lineHeader, "vt") == 0)
		{
			vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y *= -1.0f; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		// Normal data
		else if (strcmp(lineHeader, "vn") == 0) 
		{
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		// Face data
		else if (strcmp(lineHeader, "f") == 0)
		{
			std::string vertex1, vertex2, vertex3; 
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];

			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			
			if (matches != 9)
			{
				printf("File could not be ready by parser\n");
				fclose(file);
				return false;
			}

			// Populate the arrays
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);

			uvIndicies.push_back(uvIndex[0]);
			uvIndicies.push_back(uvIndex[1]);
			uvIndicies.push_back(uvIndex[2]);

			normalIndicies.push_back(normalIndex[0]);
			normalIndicies.push_back(normalIndex[1]);
			normalIndicies.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			// This should really be made better later
			char tempBuffer[1000];
			fgets(tempBuffer, 1000, file);
		}
	}

	// Now we match the indicies of the faces with the actual data
	// Note obj's index at 1 and our arrays index at 0
	for (unsigned int i = 0; i < vertexIndices.size(); ++i)
	{
		out_vertices.push_back(temp_vertices[vertexIndices[i] - 1]);
		out_uvs.push_back(temp_uvs[uvIndicies[i] - 1]);
		out_normals.push_back(temp_normals[normalIndicies[i] - 1]);
	}

	fclose(file);
	return true;
}