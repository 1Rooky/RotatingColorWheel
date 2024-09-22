#include <iostream>
#include <gl/glew/glew.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include<gl\glm\glm.hpp>
#include<gl\glm\gtc\type_ptr.hpp>
#include<gl\glm\gtx\transform.hpp>
#include <cmath>

#define PI 3.14f
#define RADIUS 1

using namespace std;
using namespace glm;

float theta = 0;
int noOfSegments = 3;
bool red, green, blue = false;

enum DrawingMode
{
	Points,
	Lines,
	FilledTriangle
};

struct vertex
{
	vec3 position;
	vec3 color;
	vertex() {}
	vertex(vec3 _position, vec3 _color) :position{ _position }, color{ _color }
	{
	}
	vertex(vec3 _position) :position{ _position }, color{ _position }
	{
	}
};

GLuint InitShader(const char* vertex_shader_file_name, const char* fragment_shader_file_name);

const GLint WIDTH = 600, HEIGHT = 600;
GLuint VBO_Triangle, IBO, BasiceprogramId;
DrawingMode Current_DrawingMode = DrawingMode::FilledTriangle;

// transformation
GLuint modelMatLoc, viewMatLoc, projMatLoc;

vector<vertex> GetAllVertices() {
	vector<vertex> TriangleVertices(noOfSegments * 3 + 3);
	float angle = radians((360 / float(noOfSegments)));

	red, green, blue = false;
	float colorRatio = 0;
	float colorOffset = 3 / float(noOfSegments);
	colorRatio += colorOffset; // because I don't want to start from black

	TriangleVertices[0] = { vec3(0,0,0),vec3(1,1,1) };
	TriangleVertices[1].position = vec3(0, 1, 0);
	colorRatio += colorOffset;
	green = true;

	for (int i = 2; i < (noOfSegments * 3);) {
		if (green) {
			TriangleVertices[i].color = vec3(0, colorRatio, 1 - colorRatio);
			if (colorRatio >= 1 + colorOffset) {
				red = true;
				green = false;
				colorRatio = colorOffset;
			}
			colorRatio += colorOffset;
		}
		else if (red)
		{
			TriangleVertices[i].color = vec3(colorRatio, 1 - colorRatio, 0);

			if (colorRatio >= 1) {
				red = false;
				blue = true;
				colorRatio = colorOffset;
			}
			colorRatio += colorOffset;
		}
		else {
			TriangleVertices[i].color = vec3(1 - colorRatio, 0, colorRatio);
			colorRatio += colorOffset;
			if (colorRatio >= 1 + colorOffset) {
				green = true;
				red = false;
				colorRatio = colorOffset;
			}
		}

		TriangleVertices[i++].position =
			vec3((TriangleVertices[i - 1].position[0] * cos(angle)) + (-1 * TriangleVertices[i - 1].position[1] * sin(angle)), //x
				(1 * TriangleVertices[i - 1].position[0] * sin(angle)) + (1 * TriangleVertices[i - 1].position[1] * cos(angle)), //y
				0); // z
		TriangleVertices[i++] = { vec3(0,0,0),vec3(1,1,1) }; // center Point
		TriangleVertices[i++] = { TriangleVertices[i - 2].position,TriangleVertices[i - 2].color };
	}
	TriangleVertices[1].color = TriangleVertices[TriangleVertices.size() - 2].color;
	return TriangleVertices;
}

void CreateColorWheel()
{
	vector<vertex> colorWheel = GetAllVertices();

	glGenBuffers(1, &VBO_Triangle);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * colorWheel.size(), colorWheel.data(), GL_DYNAMIC_DRAW);

	// shader
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 6 * sizeof(GL_FLOAT), 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, false, 6 * sizeof(GL_FLOAT), (char*)(3 * sizeof(GL_FLOAT)));
	glEnableVertexAttribArray(1);
}

void updateBuffer() {
	vector<vertex> colorWheel = GetAllVertices();
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * colorWheel.size(), colorWheel.data(), GL_DYNAMIC_DRAW);
}

void CompileShader(const char* vertex_shader_file_name, const char* fragment_shader_file_namering, GLuint& programId)
{
	programId = InitShader(vertex_shader_file_name, fragment_shader_file_namering);
	glUseProgram(programId);
}

int Init()
{
	GLenum err = glewInit();

	if (err != GLEW_OK)
	{
		cout << "Error";
		getchar();
		return 1;
	}
	else
	{
		if (GLEW_VERSION_3_0)
			cout << "Driver support OpenGL 3.0\nDetails:\n";
	}
	cout << "\tUsing glew " << glewGetString(GLEW_VERSION) << endl;
	cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
	cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
	cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
	cout << "\tGLSL:" << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	CompileShader("VS.glsl", "FS.glsl", BasiceprogramId);

	CreateColorWheel();

	modelMatLoc = glGetUniformLocation(BasiceprogramId, "modelMat");
	viewMatLoc = glGetUniformLocation(BasiceprogramId, "viewMat");
	projMatLoc = glGetUniformLocation(BasiceprogramId, "projMat");

	glm::mat4 viewMat = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE, glm::value_ptr(viewMat));

	glm::mat4 projMat = glm::perspectiveFov(60.0f, (float)WIDTH, (float)HEIGHT, 0.1f, 100.0f);
	glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, glm::value_ptr(projMat));

	glClearColor(0, 0.5, 0.5, 1);
	glEnable(GL_DEPTH_TEST);

	return 0;
}

void Update()
{
	theta += 0.01f;
}

void Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (Current_DrawingMode)
	{
	case Points:
		glPointSize(10);
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	case Lines:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case FilledTriangle:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	default:
		break;
	}

	// draw triangle
	mat4 ModelMat = glm::rotate(theta * 180 / PI, glm::vec3(0, 0, 1)) *
		glm::scale(glm::vec3(0.5f, 0.5f, 0.5f));
	glUniformMatrix4fv(modelMatLoc, 1, GL_FALSE, glm::value_ptr(ModelMat));

	glDrawArrays(GL_TRIANGLES, 0, noOfSegments * 3);
}

int main()
{
	sf::ContextSettings context;
	context.depthBits = 24;
	sf::Window window(sf::VideoMode(WIDTH, HEIGHT), "Color Wheel!", sf::Style::Close, context);
	window.setVerticalSyncEnabled(1);
	if (Init()) return 1;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
			{
				window.close();
				break;
			}
			case sf::Event::KeyPressed:
			{
				if (event.key.code == sf::Keyboard::Num1)
				{
					Current_DrawingMode = DrawingMode::Points;
				}
				if (event.key.code == sf::Keyboard::Num2)
				{
					Current_DrawingMode = DrawingMode::Lines;
				}
				if (event.key.code == sf::Keyboard::Num3)
				{
					Current_DrawingMode = DrawingMode::FilledTriangle;
				}
				if (event.key.code == sf::Keyboard::Up)
				{
					noOfSegments++;
					updateBuffer();
				}
				if (event.key.code == sf::Keyboard::Down)
				{
					if (noOfSegments != 3) {
						noOfSegments--;
						updateBuffer();
					}
				}
				break;
			}
			}
		}

		Update();
		Render();

		window.display();
	}

	return 0;
}