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

#define STAPPEN  400
#define MAXLINE BUF_SIZE / STAPPEN 
#define PORT     1234 

#define EXTRA_INFO 8 

GLubyte * buffer, *beeld;
socklen_t len;
int n; 
int sockfd; 
struct sockaddr_in servaddr, cliaddr; 



void laadSockets()
{
    buffer = (GLubyte*)malloc(MAXLINE + EXTRA_INFO);

    char *hello = "Hello from server"; 
    
      
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    // Filling server information 
    servaddr.sin_family    		= AF_INET; // IPv4 
    servaddr.sin_addr.s_addr 	= INADDR_ANY; 
    servaddr.sin_port 			= htons(PORT); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    
  
    len = sizeof(cliaddr);  //len is value/resuslt 
}


uint32_t maxFrame = 0;

void ontvang(weergaveScherm & scherm)
{
	size_t maxAttempts = 100;

	for(size_t huidigeRegel = 0; huidigeRegel<STAPPEN; )
	{
		n = recvfrom(sockfd, (char *)buffer, MAXLINE + EXTRA_INFO,  MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len); 
		//buffer[n] = '\0'; 

		//printf("Received %d chars\n", n, buffer);

		if(n > 0)
		{
			uint32_t regel, frame;
			memcpy(&frame, buffer, 		4);
			memcpy(&regel, buffer + 4, 	4);

			if(frame < 128 && maxFrame >= 128) //rondje gemaakt
				maxFrame = frame;

			if(frame > maxFrame)
			{
				maxFrame = frame;
				huidigeRegel = 0;
			}

			if(frame == maxFrame)
			{
				//printf("was regel %d\n", regel);
				memcpy(beeld + (MAXLINE * regel), buffer + EXTRA_INFO, MAXLINE);

				huidigeRegel++;
			}

			
		}
		else if(--maxAttempts < 2)
			return;
	}

	//printf("Heel frame ontvangen (waarschijnlijk)!\n");
	scherm.maakTextuur("cam", WIDTH, HEIGHT * 2, beeld, GL_RGB,  GL_UNSIGNED_SHORT_5_6_5);
}


int main()
{
	using namespace glm;

	laadSockets();

	weergaveScherm scherm("3DS Receiver", 1280, 720);

	const glm::uvec3 dimensies(64);

	scherm.maakShader(			"toon3DS",		 	"shaders/toon3DS.vert", 			"shaders/toon3DS.frag"		);
	scherm.maakShader(			"stereophonics", 	"shaders/stereophonics.vert", 		"shaders/stereophonics.frag"		);
	//scherm.maakVolumeTextuur(	"terrarium", 		dimensies, 							vulVolumeVoorbeeld(dimensies).data());

	glErrorToConsole("Na shadershit: ");
	
	glClearColor(0,0,0,0);

	beeld = (GLubyte*)malloc(BUF_SIZE);
	memset(beeld, 64, BUF_SIZE);

	glErrorToConsole("Voordat we cam registreren: ");

	scherm.maakTextuur("cam", WIDTH, HEIGHT * 2, beeld, GL_RGB,  GL_UNSIGNED_SHORT_5_6_5);

	wrgvOpslag 				*_reeks		= nullptr;
	wrgvOnderOpslag<float>	*_punten	= nullptr,
							*_tex		= nullptr;



	_reeks = new wrgvOpslag();
	
	_punten = new wrgvOnderOpslag<float>(2, _reeks, 0);
	_punten->ggvPuntErbij(vec2( 0.0f, -1.0f));
	_punten->ggvPuntErbij(vec2( 1.0f, -1.0f));
	_punten->ggvPuntErbij(vec2( 1.0f,  0.0f));
	_punten->ggvPuntErbij(vec2( 0.0f,  0.0f));
	
	_punten->ggvPuntErbij(vec2(-1.0f, -1.0f));
	_punten->ggvPuntErbij(vec2( 0.0f, -1.0f));
	_punten->ggvPuntErbij(vec2( 0.0f,  0.0f));
	_punten->ggvPuntErbij(vec2(-1.0f,  0.0f));

	_punten->ggvPuntErbij(vec2(-1.0f,  0.0f));
	_punten->ggvPuntErbij(vec2( 0.0f,  0.0f));
	_punten->ggvPuntErbij(vec2( 0.0f,  1.0f));
	_punten->ggvPuntErbij(vec2(-1.0f,  1.0f));

	
	
	_tex = new wrgvOnderOpslag<float>(2, _reeks, 1);
	
	_tex->ggvPuntErbij(vec2( 0.0f,  0.0f));
	_tex->ggvPuntErbij(vec2( 1.0f,  0.0f));
	_tex->ggvPuntErbij(vec2( 1.0f,  0.5f));
	_tex->ggvPuntErbij(vec2( 0.0f,  0.5f));

	_tex->ggvPuntErbij(vec2( 0.0f,  0.5f));
	_tex->ggvPuntErbij(vec2( 1.0f,  0.5f));
	_tex->ggvPuntErbij(vec2( 1.0f,  1.0f));
	_tex->ggvPuntErbij(vec2( 0.0f,  1.0f));

	_tex->ggvPuntErbij(vec2( 0.0f,  0.0f));
	_tex->ggvPuntErbij(vec2( 1.0f,  0.0f));
	_tex->ggvPuntErbij(vec2( 1.0f,  1.0f));
	_tex->ggvPuntErbij(vec2( 0.0f,  1.0f));

	_punten->spoel();
	_tex->spoel();

	
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
		scherm.bindTextuur("cam", 0);
		basisShaderInfo();
		//scherm.geefWeer();

		glDisable(GL_DEPTH_TEST);
		static unsigned int indices[] = {1, 2, 0, 3 ,/**/ 5, 6, 4, 7, /**/ 9, 10, 8, 11};

		_reeks->bindPuntReeks();	
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, indices);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, indices + 4);

		scherm.bereidRenderVoor("stereophonics", false);
		scherm.bindTextuur("cam", 0);
		_reeks->bindPuntReeks();
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, indices + 8);


		scherm.rondRenderAf();

		ontvang(scherm);
	}
}