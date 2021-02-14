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


GLubyte * buffer, *beeld;
socklen_t len;
int n; 
int sockfd; 
struct sockaddr_in servaddr, cliaddr; 



void laadSockets()
{
    buffer = (GLubyte*)malloc(MAXLINE + 4);

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



void ontvang(weergaveScherm & scherm)
{
	
	for(size_t huidigeRegel = 0; huidigeRegel<STAPPEN; )
	{
		n = recvfrom(sockfd, (char *)buffer, MAXLINE + 4,  MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len); 
		//buffer[n] = '\0'; 

		//printf("Received %d chars\n", n, buffer);

		if(n > 0)
		{
			uint32_t regel;
			memcpy(&regel, buffer, 4);
			//printf("was regel %d\n", regel);
			memcpy(beeld + (MAXLINE * regel), buffer + 4, MAXLINE);

			huidigeRegel++;

			
		}
	}

	//printf("Heel frame ontvangen (waarschijnlijk)!\n");
	scherm.maakTextuur("cam", WIDTH, HEIGHT * 2, beeld, GL_RGB,  GL_UNSIGNED_SHORT_5_6_5);
}


int main()
{
	laadSockets();

	weergaveSchermVierkant scherm("3DS Receiver", 1280, 720, true);

	const glm::uvec3 dimensies(64);

	scherm.maakShader(			"toon3DS", 	"shaders/toon3DS.vert", 		"shaders/toon3DS.frag"		);
	//scherm.maakVolumeTextuur(	"terrarium", 		dimensies, 							vulVolumeVoorbeeld(dimensies).data());

	glErrorToConsole("Na shadershit: ");
	
	glClearColor(0,0,0,0);

	beeld = (GLubyte*)malloc(BUF_SIZE);
	memset(beeld, 0, BUF_SIZE);

	glErrorToConsole("Voordat we cam registreren: ");

	scherm.maakTextuur("cam", WIDTH * 2, HEIGHT, beeld, GL_RGB,  GL_UNSIGNED_SHORT_5_6_5);

	
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
		scherm.geefWeer();


		scherm.rondRenderAf();

		ontvang(scherm);
	}
}