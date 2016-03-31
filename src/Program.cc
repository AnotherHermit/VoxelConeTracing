///////////////////////////////////////
//
//	Computer Graphics TSBK03
//	Conrad Wahlén - conwa099
//
///////////////////////////////////////

#include "Program.h"

#include "GL_utilities.h"


#include <iostream>


Program::Program() {
	// Set all pointers to null
	screen = NULL;
	glcontext = NULL;
	cam = NULL;

	// Window init size
	winWidth = 800;
	winHeight = 800;

	// Start state
	isRunning = true;

	// Time init
	time.startTimer();

	// Set program parameters
	cameraStartPos = glm::vec3(0.0, 0.0, 2.0);
	cameraFrustumFar = 5000.0f;

	sceneSelect = 0;
	useOrtho = false;
	voxelRes = 32;
}

int Program::Execute() {
	if(!Init()) {
		std::cout << "\nInit failed. Press enter to quit ..." << std::endl;
		getchar();
		return -1;
	}

	SDL_Event Event;

	while(isRunning) {
		timeUpdate();
		while(SDL_PollEvent(&Event)) OnEvent(&Event);
		CheckKeyDowns();
		Update();
		Render();
	}

	Clean();

	return 0;
}

void Program::timeUpdate() {
	time.endTimer();
	param.deltaT = time.getLapTime();
	param.currentT = time.getTime();
	FPS = 1.0f / time.getLapTime();
}

bool Program::Init() {
	// SDL, glew and OpenGL init
	if(SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
		return false;
	}
	screen = SDL_CreateWindow("Voxel Cone Tracing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, winWidth, winHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if(screen == 0) {
		std::cerr << "Failed to set Video Mode: " << SDL_GetError() << std::endl;
		return false;
	}

	glcontext = SDL_GL_CreateContext(screen);
	SDL_SetRelativeMouseMode(SDL_FALSE);

#ifdef _WINDOWS
	GLenum glewErr = glewInit();
	if(glewErr != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewErr) << std::endl;
		return false;
	}
#endif

	dumpInfo();

	printError("after wrapper inits");

	glGenBuffers(1, &programBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 12, programBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, programBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ProgramStruct), &param, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);


	// Load shaders for drawing
	GLint err;
	shaders.simple = loadShaders("src/shaders/simpleModel.vert", "src/shaders/simpleModel.frag");
	glGetProgramiv(shaders.simple, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	shaders.texture = loadShaders("src/shaders/textureModel.vert", "src/shaders/textureModel.frag");
	glGetProgramiv(shaders.texture, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	shaders.mask = loadShaders("src/shaders/maskModel.vert", "src/shaders/maskModel.frag");
	glGetProgramiv(shaders.mask, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	// Load shaders for voxelization
	shaders.voxel = loadShadersG("src/shaders/voxelizationSimple.vert", "src/shaders/voxelizationSimple.frag", "src/shaders/voxelizationSimple.geom");
	glGetProgramiv(shaders.voxel, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	shaders.voxelTexture = loadShadersG("src/shaders/voxelizationTexture.vert", "src/shaders/voxelizationTexture.frag", "src/shaders/voxelizationTexture.geom");
	glGetProgramiv(shaders.voxelTexture, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	// Single triangle shader for deferred shading etc.
	shaders.singleTriangle = loadShaders("src/shaders/singleTriangle.vert", "src/shaders/singleTriangle.frag");
	glGetProgramiv(shaders.singleTriangle, GL_LINK_STATUS, &err);
	if(err == GL_FALSE) return false;

	printError("after shader load");

	// Set up the AntBar
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(winWidth, winWidth);
	antBar = TwNewBar("VCT");
	TwDefine(" VCT refresh=0.1 size='300 420' valueswidth=140 ");
	TwDefine(" VCT help='This program simulates Global Illumination with Voxel Cone Tracing.' ");

	// Set up the camera
	cam = new Camera(cameraStartPos, &winWidth, &winHeight, cameraFrustumFar);
	if(!cam->Init()) return false;


	// Load scenes
	//Scene* cornell = new Scene();
	//if(!cornell->Init("resources/cornell.obj", &shaders)) return false;
	//scenes.push_back(cornell);
	
	Scene* sponza = new Scene();
	if(!sponza->Init("resources/sponza.obj", &shaders)) return false;
	scenes.push_back(sponza);
	


	// Add information to the antbar
	TwAddVarRO(antBar, "FPS", TW_TYPE_FLOAT, &FPS, " group=Info ");
	TwAddVarRO(antBar, "Cam Pos", cam->GetCameraTwType(), cam->GetCameraInfo(), NULL);
	TwAddVarRW(antBar, "Cam Speed", TW_TYPE_FLOAT, cam->GetSpeedPtr(), " min=0 max=2000 step=10 group=Controls ");
	TwAddVarRW(antBar, "Cam Rot Speed", TW_TYPE_FLOAT, cam->GetRotSpeedPtr(), " min=0.0 max=0.010 step=0.001 group=Controls ");
	//TwAddVarRW(antBar, "Skip No Texture", TW_TYPE_BOOL8, scenes[1]->GetSkipNoTexturePtr(), " group=Controls ");
	TwAddVarRW(antBar, "Select Scene", TW_TYPE_UINT32, &sceneSelect, " min=0 max=1 group=Controls ");
	TwAddVarRO(antBar, "Use Ortho", TW_TYPE_BOOL8, &useOrtho, " group=Controls ");
	TwAddVarRW(antBar, "Select View Cornell", TW_TYPE_UINT32, scenes[0]->GetViewPtr(), " min=0 max=2 group=Controls ");
	//TwAddVarRW(antBar, "Select View Sponza", TW_TYPE_UINT32, scenes[1]->GetViewPtr(), " min=0 max=2 group=Controls ");
	TwAddVarRW(antBar, "Select Voxel Res", TW_TYPE_UINT32, scenes[0]->GetVoxelResPtr(), " min=16 max=512 step=16 group=Controls ");
	TwAddVarRW(antBar, "Select Voxel Layer", TW_TYPE_UINT32, scenes[0]->GetLayerPtr(), " min=0 max=127 group=Controls ");
	TwAddVarRW(antBar, "Draw Voxel Data", TW_TYPE_UINT32, scenes[0]->GetVoxelDataDrawPtr(), " min=0 max=1 group=Controls ");

	// Check if AntTweak Setup is ok
	if(TwGetLastError() != NULL) return false;

	// Activate depth test and blend for masking textures
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

void Program::Update() {
	// Upload program params (incl time update)
	UploadParams();

	// Update the camera
	if(!useOrtho) {
		cam->UpdateCamera();
	}

	printError("after update");
}

void Program::Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(useOrtho) {
		scenes[sceneSelect]->Voxelize();
	} else {
		scenes[sceneSelect]->Draw();
	}

	TwDraw();

	printError("after display");

	SDL_GL_SwapWindow(screen);
}

void Program::Clean() {
	glDeleteBuffers(1, &programBuffer);
	TwTerminate();
	SDL_GL_DeleteContext(glcontext);
	SDL_Quit();
}

void Program::UploadParams() {
	// Update program parameters
	glBindBuffer(GL_UNIFORM_BUFFER, programBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, NULL, sizeof(ProgramStruct), &param);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	printError("program param upload");
}

void Program::OnEvent(SDL_Event *Event) {
	switch(Event->type) {
		case SDL_QUIT:
			isRunning = false;
			break;
		case SDL_WINDOWEVENT:
			switch(Event->window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					SDL_SetWindowSize(screen, Event->window.data1, Event->window.data2);
					SDL_GetWindowSize(screen, &winWidth, &winHeight);
					glViewport(0, 0, winWidth, winHeight);
					TwWindowSize(winWidth, winHeight);
					cam->SetFrustum();
					break;
			}
		case SDL_KEYDOWN:
			OnKeypress(Event);
			break;
		case SDL_MOUSEMOTION:
			OnMouseMove(Event);
			break;
		case SDL_MOUSEBUTTONDOWN:
			TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_LEFT);
			break;
		case SDL_MOUSEBUTTONUP:
			TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_LEFT);
			break;
		default:
			break;
	}
}

void Program::OnKeypress(SDL_Event *Event) {
	TwKeyPressed(Event->key.keysym.sym, TW_KMOD_NONE);
	switch(Event->key.keysym.sym) {
		case SDLK_ESCAPE:
			isRunning = false;
			break;
		case SDLK_SPACE:
			useOrtho = !useOrtho;
			scenes[sceneSelect]->SetDrawVoxels(useOrtho);
			break;
		case SDLK_t:
			break;
		case SDLK_f:
			cam->TogglePause();
			SDL_SetRelativeMouseMode(SDL_GetRelativeMouseMode() ? SDL_FALSE : SDL_TRUE);
			break;
		case SDLK_g:
			int isBarHidden;
			TwGetParam(antBar, NULL, "iconified", TW_PARAM_INT32, 1, &isBarHidden);
			if(isBarHidden) {
				TwDefine(" Particles iconified=false ");
			} else {
				TwDefine(" Particles iconified=true ");
			}
			break;
		default:
			break;
	}
}

void Program::OnMouseMove(SDL_Event *Event) {
	if(!SDL_GetRelativeMouseMode())
		TwMouseMotion(Event->motion.x, Event->motion.y);
	else
		cam->RotateCamera(Event->motion.xrel, Event->motion.yrel);
}

void Program::CheckKeyDowns() {
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	if(keystate[SDL_SCANCODE_W]) {
		cam->MoveForward(param.deltaT);
	}
	if(keystate[SDL_SCANCODE_S]) {
		cam->MoveForward(-param.deltaT);
	}
	if(keystate[SDL_SCANCODE_A]) {
		cam->MoveRight(-param.deltaT);
	}
	if(keystate[SDL_SCANCODE_D]) {
		cam->MoveRight(param.deltaT);
	}
	if(keystate[SDL_SCANCODE_Q]) {
		cam->MoveUp(param.deltaT);
	}
	if(keystate[SDL_SCANCODE_E]) {
		cam->MoveUp(-param.deltaT);
	}
}