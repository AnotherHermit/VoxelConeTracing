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

	cameraStartPos = glm::vec3(800.0, 200.0, 0.0);
	cameraFrustumFar = 5000.0f;
}

int Program::Execute() {
	if (!Init()) {
		std::cout << "\nInit failed. Press enter to quit ..." << std::endl;
		getchar();
		return -1;
	}

	SDL_Event Event;

	while (isRunning) {
		timeUpdate();
		while (SDL_PollEvent(&Event)) OnEvent(&Event);
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
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
		return false;
	}
	screen = SDL_CreateWindow("Voxel Cone Tracing", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, winWidth, winHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (screen == 0) {
		std::cerr << "Failed to set Video Mode: " << SDL_GetError() << std::endl;
		return false;
	}

	glcontext = SDL_GL_CreateContext(screen);
	SDL_SetRelativeMouseMode(SDL_FALSE);
	
#ifdef _WINDOWS
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
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

	// Set up the AntBar
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(winWidth, winWidth);
	antBar = TwNewBar("VCT");
	TwDefine(" VCT refresh=0.1 size='300 420' valueswidth=140 ");
	TwDefine(" VCT help='This program simulates Global Illumination with Voxel Cone Tracing.' ");

	// Set up the camera
	cam = new Camera(cameraStartPos, &winWidth, &winHeight, cameraFrustumFar);
	if (!cam->Init()) return false;

	modelLoader = new ModelLoader();
	if(!modelLoader->Init("resources/sponza.obj")) return false;

	TwAddVarRO(antBar, "FPS", TW_TYPE_FLOAT, &FPS, " group=Info ");
	TwAddVarRW(antBar, "Cam Speed", TW_TYPE_FLOAT, cam->GetSpeedPtr(), " min=0 max=2000 step=10 group=Controls ");
	TwAddVarRW(antBar, "Cam Rot Speed", TW_TYPE_FLOAT, cam->GetRotSpeedPtr(), " min=0.0 max=0.010 step=0.001 group=Controls ");

	// Check if AntTweak Setup is ok
	if (TwGetLastError() != NULL) return false;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

void Program::Update() {
	// Upload program params (incl time update)
	UploadParams();

	// Update the camera
	cam->UpdateCamera();
	
	printError("after update");
}

void Program::Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	modelLoader->Draw();

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
	switch (Event->type) {
	case SDL_QUIT:
		isRunning = false;
		break;
	case SDL_WINDOWEVENT:
		switch (Event->window.event) {
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
	switch (Event->key.keysym.sym) {
	case SDLK_ESCAPE:
		isRunning = false;
		break;
	case SDLK_SPACE:
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
		if (isBarHidden) {
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
	if (!SDL_GetRelativeMouseMode())
		TwMouseMotion(Event->motion.x, Event->motion.y);
	else
		cam->RotateCamera(Event->motion.xrel, Event->motion.yrel);
}

void Program::CheckKeyDowns() {
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	if (keystate[SDL_SCANCODE_W]) {
		cam->MoveForward(param.deltaT);
	}
	if (keystate[SDL_SCANCODE_S]) {
		cam->MoveForward(-param.deltaT);
	}
	if (keystate[SDL_SCANCODE_A]) {
		cam->MoveRight(-param.deltaT);
	}
	if (keystate[SDL_SCANCODE_D]) {
		cam->MoveRight(param.deltaT);
	}
	if (keystate[SDL_SCANCODE_Q]) {
		cam->MoveUp(param.deltaT);
	}
	if (keystate[SDL_SCANCODE_E]) {
		cam->MoveUp(-param.deltaT);
	}
}