// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cstdio>
#include <string>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;

#include <shader.h>
#include <texture.h>

#define width       1024 
#define height      768

int key=3;

void apply_color(GLuint vertexbuffer, GLuint colorbuffer){

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glVertexAttribPointer(
        1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );

}

void apply_texture(GLuint vertexbuffer, GLuint uvbuffer, bool blur=false){

    // pass attributes to the shader
    
    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(
        1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        2,                                // size : U+V => 2
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );
    
}

void FBO_2_PPM_file()
{
    FILE    *output_image;
    int     output_width, output_height;

    output_width = width;
    output_height = height;

    /// READ THE PIXELS VALUES from FBO AND SAVE TO A .PPM FILE
    int             i, j, k;
    unsigned char   *pixels = (unsigned char*)malloc(output_width*output_height*3);

    /// READ THE CONTENT FROM THE FBO
    glReadBuffer(GL_COLOR_ATTACHMENT0);//(GL_BACK);
    glReadPixels(0, 0, output_width, output_height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    output_image = fopen("/home/kunal/Desktop.ppm", "wt");
    fprintf(output_image,"P3\n");
    fprintf(output_image,"# Created by Ricao\n");
    fprintf(output_image,"%d %d\n",output_width,output_height);
    fprintf(output_image,"255\n");

    k = 0;
    for(i=0; i<output_width; i++)
    {
        for(j=0; j<output_height; j++)
        {
            fprintf(output_image,"%u %u %u ",(unsigned int)pixels[k],(unsigned int)pixels[k+1],
                                             (unsigned int)pixels[k+2]);
            k = k+3;
        }
        fprintf(output_image,"\n");
    }
    free(pixels);
}


// void saveImage(char* filepath, GLFWwindow* w) {
    
//     glfwGetFramebufferSize(w, &width, &height);
//     GLsizei nrChannels = 3;
//     GLsizei stride = nrChannels * width;
    
//     stride += (stride % 4) ? (4 - stride % 4) : 0;
//     GLsizei bufferSize = stride * height;
    
//     std::vector<char> buffer(bufferSize);
//     glPixelStorei(GL_PACK_ALIGNMENT, 4);
//     glReadBuffer(GL_FRONT);
//     glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
    
//     stbi_flip_vertically_on_write(true);
//     stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
// }

void checkGLError()
{
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR){
        std::cout << "error:"<<err <<endl;
    }  
}

void render_to_defaultFBO(Texture &tex,GLuint programID_passthrough, GLuint quadbuffer, GLuint TextureIDPass){

	// Bind the output texture from the previous shader program.
	tex.bind(0);
	
	// Use shader
	glUseProgram(programID_passthrough);

	// Set our "renderedTexture" sampler to use Texture Unit 0
	glUniform1i(TextureIDPass, 0);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
	glDisableVertexAttribArray(0);
}
int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( width, height, "Tutorial", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader" );

    // Create and compile our GLSL program from the shaders
	GLuint programID_Texture = LoadShaders( "TextureTransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader" );

    // Create and compile our GLSL program from the shaders
	GLuint programID_passthrough = LoadShaders( "Passthrough.vertexshader", "TextureFragmentShader.fragmentshader" );

    // Create and compile our GLSL program from the shaders
	GLuint programID_Texture_Blur = LoadShaders( "Passthrough.vertexshader", "TextureGaussianBlurShader.fragmentshader" );
	
	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	glm::mat4 View       = glm::lookAt(
								glm::vec3(4,3,-3), // Camera is at (4,3,-3), in World Space
								glm::vec3(0,0,0), // and looks at the origin
								glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
						   );
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model      = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around

	// Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
	// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
	static const GLfloat g_vertex_buffer_data[] = { 
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		 1.0f,-1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f,-1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f,-1.0f, 1.0f
	};

    // One color for each vertex.
	static const GLfloat g_color_buffer_data[] = { 
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f,
		1.0f, 0.5f, 0.5f
	};

	// One color for each vertex. They were generated randomly.
    static const GLfloat g_color_buffer_data_random[] = { 
		0.583f,  0.771f,  0.014f,
		0.609f,  0.115f,  0.436f,
		0.327f,  0.483f,  0.844f,
		0.822f,  0.569f,  0.201f,
		0.435f,  0.602f,  0.223f,
		0.310f,  0.747f,  0.185f,
		0.597f,  0.770f,  0.761f,
		0.559f,  0.436f,  0.730f,
		0.359f,  0.583f,  0.152f,
		0.483f,  0.596f,  0.789f,
		0.559f,  0.861f,  0.639f,
		0.195f,  0.548f,  0.859f,
		0.014f,  0.184f,  0.576f,
		0.771f,  0.328f,  0.970f,
		0.406f,  0.615f,  0.116f,
		0.676f,  0.977f,  0.133f,
		0.971f,  0.572f,  0.833f,
		0.140f,  0.616f,  0.489f,
		0.997f,  0.513f,  0.064f,
		0.945f,  0.719f,  0.592f,
		0.543f,  0.021f,  0.978f,
		0.279f,  0.317f,  0.505f,
		0.167f,  0.620f,  0.077f,
		0.347f,  0.857f,  0.137f,
		0.055f,  0.953f,  0.042f,
		0.714f,  0.505f,  0.345f,
		0.783f,  0.290f,  0.734f,
		0.722f,  0.645f,  0.174f,
		0.302f,  0.455f,  0.848f,
		0.225f,  0.587f,  0.040f,
		0.517f,  0.713f,  0.338f,
		0.053f,  0.959f,  0.120f,
		0.393f,  0.621f,  0.362f,
		0.673f,  0.211f,  0.457f,
		0.820f,  0.883f,  0.371f,
		0.982f,  0.099f,  0.879f
	};

    // Two UV coordinatesfor each vertex. They were created with Blender.
	static const GLfloat g_uv_buffer_data[] = { 
		0.000059f, 1.0f-0.000004f, 
		0.000103f, 1.0f-0.336048f, 
		0.335973f, 1.0f-0.335903f, 
		1.000023f, 1.0f-0.000013f, 
		0.667979f, 1.0f-0.335851f, 
		0.999958f, 1.0f-0.336064f, 
		0.667979f, 1.0f-0.335851f, 
		0.336024f, 1.0f-0.671877f, 
		0.667969f, 1.0f-0.671889f, 
		1.000023f, 1.0f-0.000013f, 
		0.668104f, 1.0f-0.000013f, 
		0.667979f, 1.0f-0.335851f, 
		0.000059f, 1.0f-0.000004f, 
		0.335973f, 1.0f-0.335903f, 
		0.336098f, 1.0f-0.000071f, 
		0.667979f, 1.0f-0.335851f, 
		0.335973f, 1.0f-0.335903f, 
		0.336024f, 1.0f-0.671877f, 
		1.000004f, 1.0f-0.671847f, 
		0.999958f, 1.0f-0.336064f, 
		0.667979f, 1.0f-0.335851f, 
		0.668104f, 1.0f-0.000013f, 
		0.335973f, 1.0f-0.335903f, 
		0.667979f, 1.0f-0.335851f, 
		0.335973f, 1.0f-0.335903f, 
		0.668104f, 1.0f-0.000013f, 
		0.336098f, 1.0f-0.000071f, 
		0.000103f, 1.0f-0.336048f, 
		0.000004f, 1.0f-0.671870f, 
		0.336024f, 1.0f-0.671877f, 
		0.000103f, 1.0f-0.336048f, 
		0.336024f, 1.0f-0.671877f, 
		0.335973f, 1.0f-0.335903f, 
		0.667969f, 1.0f-0.671889f, 
		1.000004f, 1.0f-0.671847f, 
		0.667979f, 1.0f-0.335851f
	};

	// The fullscreen quad's FBO : Triangles covering the screen
	static const GLfloat g_quad_vertex_buffer_data[] = { 
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
	};

	/////////////////////////////////////////////////////////////////////////////
	// TEXTURE INITIALIZATION
	//
	/////////////////////////////////////////////////////////////////////////////

	//Intialize texture object
    Texture texture[4];
    
    const char imagePath[] = "/home/kunal/Desktop/Haptics/opengl_resources/opengl-tutorials/tutorial05_textured_cube/uvtemplate.tga";
    bool loaded = texture[0].loadTexture(imagePath, true);
    
    if (!loaded){
        cout << "error in loading texture :" << imagePath <<endl;
        return -1;
    }

	// For saving first pass results
    texture[1].loadEmptyFrame();

	const char imagePath1[] = "/home/kunal/Desktop/Haptics/opengl_resources/opengl-tutorials/tutorial05_textured_cube/uvtemplate.bmp";
    bool loaded1 = texture[2].loadTexture(imagePath1, true);
    
    if (!loaded1){
        cout << "error in loading texture :" << imagePath1 <<endl;
        return -1;
    }
	
	// For saving Blur results
	texture[3].loadEmptyFrame();


	/////////////////////////////////////////////////////////////////////////////
	// ARRAY Buffer INITIALIZATION
	// FRAME BUFFER INITIALIZATION
	/////////////////////////////////////////////////////////////////////////////

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

    GLuint randomColorbuffer;
	glGenBuffers(1, &randomColorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, randomColorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data_random), g_color_buffer_data_random, GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

	GLuint quadbuffer;
	glGenBuffers(1, &quadbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

	// Define a FBO
    GLuint frameBuffer;
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	glFramebufferTexture(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,texture[1].mTexture,0);
    /*
        Name

        glFramebufferTexture — attach a level of a texture object as a logical buffer of a framebuffer object
        C Specification
        void glFramebufferTexture( 	
            GLenum target,
            GLenum attachment,
            GLuint texture,
            GLint level);
        
        Parameters

        target

            Specifies the target to which the framebuffer is bound.
        attachment

            Specifies the attachment point of the framebuffer.
        texture

            Specifies the name of an existing texture object to attach.
        level

            Specifies the mipmap level of the texture object to attach.
    */
	

	
	// Set the list of draw buffers.
    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    // Always check that our framebuffer is ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    	return false;

	// Define a FBO : to blur
	GLuint frameBufferBlur;
	glGenFramebuffers(1, &frameBufferBlur);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferBlur);

	glFramebufferTexture(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,texture[3].mTexture,0);
    // Set the list of draw buffers.
    GLenum DrawBuffersBlur[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, DrawBuffersBlur); // "1" is the size of DrawBuffers

    // Always check that our framebuffer is ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    	return false;


	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    
    // Get a handle for our "MVP" uniform
	GLuint MatrixIDTexture = glGetUniformLocation(programID_Texture, "MVP");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID_Texture, "myTextureSampler");

    // Get a handle for our "MVP" uniform
	// GLuint MatrixIDTextureBlur = glGetUniformLocation(programID_Texture_Blur, "MVP");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureIDBlur  = glGetUniformLocation(programID_Texture_Blur, "myTextureSampler");


	GLuint TextureIDPass = glGetUniformLocation(programID_passthrough, "myTextureSampler");
    
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);


	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 ){

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS || key==0 ){
            key=0;

            // Use our shader
            glUseProgram(programID);

            // Send our transformation to the currently bound shader, 
            // in the "MVP" uniform
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            apply_color(vertexbuffer, colorbuffer);
            glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles
        }

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS || key==1){
            key=1;
            // Use our shader
            glUseProgram(programID);

            // Send our transformation to the currently bound shader, 
            // in the "MVP" uniform
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            apply_color(vertexbuffer, randomColorbuffer);
            glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles
        }
    
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS || key==2){
            key=2;

            // Use our shader
            glUseProgram(programID_Texture);

            // Send our transformation to the currently bound shader, 
            // in the "MVP" uniform
            glUniformMatrix4fv(MatrixIDTexture, 1, GL_FALSE, &MVP[0][0]);

            // bind 0th texture
            texture[0].bind(0);

            // Set our "myTextureSampler" sampler to use Texture Unit 0
            glUniform1i(TextureID, 0);
            apply_texture(vertexbuffer, uvbuffer);
            glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles
            // Only incase one want to remove a texture:    texture.unbind(0);
		
        }

        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS || key==3){
            key=3;

            // Render to our framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
            glViewport(0,0,width, height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
			
			// Clear the screen
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Use our shader
            glUseProgram(programID_Texture);

            // Send our transformation to the currently bound shader, 
            // in the "MVP" uniform
            glUniformMatrix4fv(MatrixIDTexture, 1, GL_FALSE, &MVP[0][0]);

            // bind 0th texture
            texture[2].bind(0);

            // Set our "myTextureSampler" sampler to use Texture Unit 0
            glUniform1i(TextureID, 0);

            apply_texture(vertexbuffer, uvbuffer);
		    glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles
            
			FBO_2_PPM_file();

			// Render to the screen
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // Render on the whole framebuffer, complete from the lower left corner to the upper right
            glViewport(0,0,width, height);

			// Clear the screen
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			// Use shader
            glUseProgram(programID_passthrough);

			// Bind the output texture from the previous shader program.
            texture[1].bind(0);

			// Set our "renderedTexture" sampler to use Texture Unit 0
			glUniform1i(TextureIDPass, 0);

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
			glVertexAttribPointer(
				0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
				3,                  // size
				GL_FLOAT,           // type
				GL_FALSE,           // normalized?
				0,                  // stride
				(void*)0            // array buffer offset
			);

			// Draw the triangles !
			glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

			glDisableVertexAttribArray(0);

        }

        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS || key == 4 || key == 5 ){

			if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
				key=4;

            // Render to our framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
            glViewport(0,0,width, height); // Render on the whole framebuffer, complete from the lower left corner to the upper right
			
			// Clear the screen
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Use our shader
            glUseProgram(programID_Texture);

            // Send our transformation to the currently bound shader, 
            // in the "MVP" uniform
            glUniformMatrix4fv(MatrixIDTexture, 1, GL_FALSE, &MVP[0][0]);

            // bind 0th texture
            texture[2].bind(0);

            // Set our "myTextureSampler" sampler to use Texture Unit 0
            glUniform1i(TextureID, 0);

            apply_texture(vertexbuffer, uvbuffer);
		    glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles
            
			// take screenshot :
			//FBO_2_PPM_file();

			if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS || key==5){
            	key=5;
				
				// Render to our framebuffer
            	glBindFramebuffer(GL_FRAMEBUFFER, frameBufferBlur);
				glViewport(0,0,width, height); // Render on the whole framebuffer, complete from the lower left corner to the upper right

				// Clear the screen
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				///// 2nd pass ////////////////////////////////////////
				
				// Bind the output texture from the previous shader program.
            	texture[1].bind(0);

				// Use shader
				glUseProgram(programID_Texture_Blur);

				
				// Set our "myTextureSampler" sampler to use Texture Unit 0
				glUniform1i(TextureIDBlur, 0);

				// 1rst attribute buffer : vertices
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
				glVertexAttribPointer(
					0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
				);

				// Draw the triangles !
				glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

				
				// Render to our framebuffer
            	glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glViewport(0,0,width, height); // Render on the whole framebuffer, complete from the lower left corner to the upper right

				// Clear the screen
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// Bind the output texture from the previous shader program.
				texture[3].bind(0);
				
				// Use shader
				glUseProgram(programID_passthrough);

				// Set our "renderedTexture" sampler to use Texture Unit 0
				glUniform1i(TextureIDPass, 0);

				// 1rst attribute buffer : vertices
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
				glVertexAttribPointer(
					0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
				);

				// Draw the triangles !
				glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
				// take screenshot :
				FBO_2_PPM_file();

				glDisableVertexAttribArray(0);

			} 
			else{
				// Render to the screen
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
				// Clear the screen
				glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// Bind the output texture from the previous shader program.
				texture[1].bind(0);
				
				// Use shader
				glUseProgram(programID_passthrough);

				// Set our "renderedTexture" sampler to use Texture Unit 0
				glUniform1i(TextureIDPass, 0);

				// 1rst attribute buffer : vertices
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, quadbuffer);
				glVertexAttribPointer(
					0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
					3,                  // size
					GL_FLOAT,           // type
					GL_FALSE,           // normalized?
					0,                  // stride
					(void*)0            // array buffer offset
				);

				// Draw the triangles !
				glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles
				glDisableVertexAttribArray(0);
			}
        }

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();


	} // Check if the ESC key was pressed or the window was closed
	

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteBuffers(1, &randomColorbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteProgram(programID_Texture);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

