#include <weergaveSchermVierkant.h>
#include <iostream>
#include <random>
#include <perlinRuis.h>

std::vector<unsigned char> vulVolumeVoorbeeld(glm::uvec3 dimensies)
{
	std::vector<unsigned char> volume(dimensies.x * dimensies.y * dimensies.z * 4, 0);

	std::random_device rd;
    std::mt19937 gen(rd());

	for(int i=0; i<volume.size() / 33; i++)
	{
		glm::uvec3 hier(gen()%dimensies.x, gen()%dimensies.y, gen()%dimensies.z);

		size_t deze = hier.x + (hier.y * dimensies.x) + (hier.z * dimensies.x * dimensies.y);

		glm::uvec3 vanafMidden = hier - (dimensies / glm::uvec3(2));

		//if((length(glm::vec2(vanafMidden.yz())) < 0.8 * (dimensies.x/2 - abs(vanafMidden.x))))
			for(size_t dus=0; dus < 4; dus++)
				volume[dus + (deze * 4)] = gen()%255;
	}

	return volume;
}

int main()
{
	weergaveSchermVierkant scherm("Terrarium", 1280, 720);

	const glm::uvec3 dimensies(64);

	scherm.maakShader(			"toonTerrarium", 	"shaders/toonTerrarium.vert", 		"shaders/toonTerrarium.frag"		);
	scherm.maakVolumeTextuur(	"terrarium", 		dimensies, 							vulVolumeVoorbeeld(dimensies).data());
	
	glClearColor(0,0,0,0);

	perlinRuis 		ruisje0, 
					ruisje1;

	glm::vec3 	kijkPlek		(0.0f, 0.0f, -3.0f)	,
				kijkRicht		(0.0f, 0.0f,  1.0f) ;
	glm::vec2 	verdraaiing		(0.0f, 0.0f)		;
	glm::mat4	kijkDraaiMat;

	
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

		switch(key)
		{
		case GLFW_KEY_A: case GLFW_KEY_LEFT: 	verdraaiing.x -= tol;				break;
		case GLFW_KEY_D: case GLFW_KEY_RIGHT:	verdraaiing.x += tol;				break;
		case GLFW_KEY_Q: case GLFW_KEY_UP: 		verdraaiing.y -= tol;				break;
		case GLFW_KEY_E: case GLFW_KEY_DOWN: 	verdraaiing.y += tol;				break;
		case GLFW_KEY_S:						kijkPlek.z	  -= tik; break;// * kijkRicht;	break;
		case GLFW_KEY_W:						kijkPlek.z 	  += tik; break;// * kijkRicht;	break;
		}	
	};

	scherm.setCustomKeyhandler(toetsenbord);

	auto basisShaderInfo = [&]()
	{
		glUniform3fv(		glGetUniformLocation(scherm.huidigProgramma(), "kijkPlek"		),  1, 				glm::value_ptr(kijkPlek));
		glUniform3fv(		glGetUniformLocation(scherm.huidigProgramma(), "kijkRicht"		),  1, 				glm::value_ptr(kijkRicht));
		glUniformMatrix4fv(	glGetUniformLocation(scherm.huidigProgramma(), "kijkDraaiMat"	),  1, GL_FALSE, 	glm::value_ptr(kijkDraaiMat));

		ruisje0.zetKnooppunten(3, 4);
		ruisje0.zetKnooppunten(5, 6);
	};

	auto berekenModelZicht = [&]()
	{
		kijkDraaiMat = 	glm::rotate(
							glm::rotate(
								glm::mat4(1.0f),
								verdraaiing.x,
								glm::vec3(0.0f, 1.0f, 0.0f)
							), 
							verdraaiing.y, 
							glm::vec3(1.0f, 0.0f, 0.0f)
						);

		kijkRicht = glm::mat3(kijkDraaiMat) * glm::vec3(0.0f, 0.0f, 1.0f);
	};

	glErrorToConsole("Voordat we beginnen: ");
	std::cout << "Laten we beginnen..." << std::endl;
	while(!scherm.stopGewenst())
	{
		berekenModelZicht();

		glDisable(GL_BLEND);
		scherm.bereidRenderVoor("toonTerrarium");
		scherm.bindTextuur("terrarium", 0);
		basisShaderInfo();
		scherm.geefWeer();

		/*if(tekenWater)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			scherm.bereidRenderVoor("tekenWater", false);
			basisShaderInfo();
			scherm.geefWeer();
		}*/

		scherm.rondRenderAf();
	}
}