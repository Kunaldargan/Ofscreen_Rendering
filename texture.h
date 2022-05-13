
#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <string>
#include <cassert>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

struct Size {
    int width=0;
    int height=0; 
    int nrChannels=0;
};

class Texture
{
public:
	Texture();
	virtual ~Texture();

    bool loadEmptyFrame();
    Size getSize();
	bool loadTexture(const std::string &fileName, bool generateMipMaps = true);
	bool loadTextureGeneric();
	void bind(GLuint texUnit = 0);
	void unbind(GLuint texUnit = 0);
    Size size;
    GLuint mTexture;

    
private:
	Texture(const Texture &rhs) {}
};

#endif
