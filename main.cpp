#include <weergaveSchermVierkant.h>
#include <iostream>
#include <random>

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
  
#define WIDTH 400
#define HEIGHT 240
#define SCREEN_SIZE WIDTH * HEIGHT * 2
#define BUF_SIZE SCREEN_SIZE * 2

#define PORT     1234 
#define MAXLINE BUF_SIZE 



int main()
{
	weergaveSchermVierkant scherm("3DS Receiver", 1280, 720);

	const glm::uvec3 dimensies(64);

	scherm.maakShader(			"toon3DS", 	"shaders/toon3DS.vert", 		"shaders/toon3DS.frag"		);
	//scherm.maakVolumeTextuur(	"terrarium", 		dimensies, 							vulVolumeVoorbeeld(dimensies).data());
	
	glClearColor(0,0,0,0);


	
	weergaveScherm::keyHandlerFunc toetsenbord = [&](int key, int scancode, int action, int mods)
	{
		const float tol = 0.05, tik = 0.1;

		/*if(action == GLFW_PRESS)
			switch(key)
			{
			case GLFW_KEY_SPACE:	waterStroomt 	= !waterStroomt;	break;
			case GLFW_KEY_R:		roteerMaar 		= !roteerMaar;		break;
			case GLFW_KEY_Z:		zonDraait 		= !zonDraait;		break;
			case GLFW_KEY_X:		tekenWater 		= !tekenWater;		break;
			}*/

	/*	switch(key)
		{
		case GLFW_KEY_A: case GLFW_KEY_LEFT: 	verdraaiing.x -= tol;				break;
		case GLFW_KEY_D: case GLFW_KEY_RIGHT:	verdraaiing.x += tol;				break;
		case GLFW_KEY_Q: case GLFW_KEY_UP: 		verdraaiing.y -= tol;				break;
		case GLFW_KEY_E: case GLFW_KEY_DOWN: 	verdraaiing.y += tol;				break;
		case GLFW_KEY_S:						kijkPlek.z	  -= tik; break;// * kijkRicht;	break;
		case GLFW_KEY_W:						kijkPlek.z 	  += tik; break;// * kijkRicht;	break;
		}	*/
	};

	scherm.setCustomKeyhandler(toetsenbord);

	auto basisShaderInfo = [&]()
	{
	/*	glUniform3fv(		glGetUniformLocation(scherm.huidigProgramma(), "kijkPlek"		),  1, 				glm::value_ptr(kijkPlek));
		glUniform3fv(		glGetUniformLocation(scherm.huidigProgramma(), "kijkRicht"		),  1, 				glm::value_ptr(kijkRicht));
		glUniformMatrix4fv(	glGetUniformLocation(scherm.huidigProgramma(), "kijkDraaiMat"	),  1, GL_FALSE, 	glm::value_ptr(kijkDraaiMat));

		ruisje0.zetKnooppunten(3, 4);
		ruisje0.zetKnooppunten(5, 6);*/
	};


	glErrorToConsole("Voordat we beginnen: ");
	std::cout << "Laten we beginnen..." << std::endl;
	while(!scherm.stopGewenst())
	{
		glDisable(GL_BLEND);
		scherm.bereidRenderVoor("toon3DS");
	//	scherm.bindTextuur("cam", 0);
		basisShaderInfo();
		scherm.geefWeer();


		scherm.rondRenderAf();
	}
}