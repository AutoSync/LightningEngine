// 2018 - 2022 AutoSync Lightning Engine | Erick Andrade - All Rights Reserved

//Lightning Engine
#include "Msg.h"
#include "Types.h"
#include "Engine.h"
#include "Camera.h"

#include <vector>

using namespace std;
using namespace Lightning;

class Cube
{
public:
	Cube()
	{

	}
	void Render()
	{

	}
private:
	vector<float> vertices =  {0};
	vector<float> positions = {0};
	uint VAO = 0, VBO = 0, EBO = 0, texture1, texture2;
};

// Main Window Instance
class GameEngine : Engine
{

public: 
	//Variables 
	Camera cam;

	GameEngine()
	{
		InitializeComponent();
	}
private:
	  void BeginPlay()
	  {
		  
		  Msg::Emit(Flow::PRINT, "Start Engine");
		  cam.position = V3(0.f, 0.f, 3.f);
		  
	  }
	  void Update()
	  {
		  V4* color = new V4(1.0f, 0.f, 0.f, 1.f);
		  glClearColor(color->x, color->y, color->z, color->w);
		  cam.Render();
		  
	  }
	  void EndPlay()
	  {
		  Msg::Emit(Flow::PRINT, "End Engine");
	  }

};


int main(int argc, const char* argv[])
{
	
	GameEngine* engine = new GameEngine();
	
	return 0;
}
