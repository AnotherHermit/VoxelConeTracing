﻿///////////////////////////////////////
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
	winWidth = 400;
	winHeight = 400;

	// Start state
	isRunning = true;

	// Time init
	time.startTimer();

	// Set program parameters
	cameraStartDistance = 2.9f;
	cameraStartAzimuth = (GLfloat)M_PI / 4.0f;//(GLfloat)M_PI / 2.0f; (GLfloat)M_PI / 4.0f;
	cameraStartPolar = (GLfloat)M_PI / 1.8f;//(GLfloat) M_PI / 2.3f; (GLfloat) M_PI / 1.8f;
	cameraStartTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraStartFov = 60.0f;
	cameraFrustumFar = 5000.0f;

	sceneSelect = 0;
	useOrtho = false;
	drawVoxelOverlay = false;

	takeTime = 0;
	runNumber = 0;
	runScene = 0;
	sceneAverage[0] = 0.0f;
	sceneAverage[1] = 0.0f;
	sceneAverage[2] = 0.0f;
	sceneAverage[3] = 0.0f;
	sceneAverage[4] = 0.0f;
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

void APIENTRY openglCallbackFunction(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam) {
	(void)source; (void)type; (void)id;
	(void)severity; (void)length; (void)userParam;

	switch(id) {
		case 131185: // Buffer detailed info
			return;
		case 131218: // Nvidia Shader complilation performance warning
			return;
		case 131186: // Memory copied from Device to Host
			return;		
		case 131204: // Texture state 0 is base level inconsistent
			return;
		case 131076: // Vertex attribute optimized away
			return;
		default:
			break;
	}
	
	fprintf(stderr, "ID: %d\n %s\n\n",id, message);
	if(severity == GL_DEBUG_SEVERITY_HIGH) {
		fprintf(stderr, "Aborting...\n");
		abort();
	}
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

#ifdef DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif // DEBUG

	glcontext = SDL_GL_CreateContext(screen);
	SDL_SetRelativeMouseMode(SDL_FALSE);

#ifdef _WINDOWS
	GLenum glewErr = glewInit();
	if(glewErr != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewErr) << std::endl;
		return false;
	}
#endif // _WINDOWS

#ifdef DEBUG
	// Enable the debug callback
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openglCallbackFunction, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
#endif // DEBUG

	// Activate depth test and blend for masking textures
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_3D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	dumpInfo();

	printError("after wrapper inits");

	glGenBuffers(1, &programBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, PROGRAM, programBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ProgramStruct), &param, GL_STREAM_DRAW);
	
	// Load shaders for drawing
	shaders.drawScene = loadShaders("src/shaders/drawModel.vert", "src/shaders/drawModel.frag");
	shaders.drawData = loadShaders("src/shaders/drawData.vert", "src/shaders/drawData.frag");

	// Load shaders for voxelization
	shaders.voxelize = loadShadersG("src/shaders/voxelization.vert", "src/shaders/voxelization.frag", "src/shaders/voxelization.geom");

	// Single triangle shader for deferred shading etc.
	shaders.singleTriangle = loadShaders("src/shaders/drawTriangle.vert", "src/shaders/drawTriangle.frag");

	// Draw voxels from 3D texture
	shaders.voxel = loadShaders("src/shaders/drawVoxel.vert", "src/shaders/drawVoxel.frag");

	// Calculate mipmaps
	shaders.mipmap = CompileComputeShader("src/shaders/mipmap.comp");

	// Create shadowmap
	shaders.shadowMap = loadShaders("src/shaders/shadowMap.vert", "src/shaders/shadowMap.frag");
		
	// Set up the AntBar
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(winWidth, winWidth);
	antBar = TwNewBar("VCT");
	TwDefine(" VCT refresh=0.1 size='300 700' valueswidth=140 ");
	TwDefine(" VCT help='This program simulates Global Illumination with Voxel Cone Tracing.' ");

	// Set up the camera
	cam = new OrbitCamera();
	if(!cam->Init(cameraStartTarget,cameraStartDistance,cameraStartPolar, cameraStartAzimuth, cameraStartFov, &winWidth, &winHeight, cameraFrustumFar)) return false;

	// Load scenes
	Scene* cornell = new Scene();
	if(!cornell->Init("resources/cornell.obj", &shaders)) return false;
	scenes.push_back(cornell);
	
	//Scene* sponza = new Scene();
	//if(!sponza->Init("resources/sponza.obj", &shaders)) return false;
	//scenes.push_back(sponza);
	
	// Initial Voxelization of the scenes
	cornell->CreateShadow();
	cornell->RenderData();
	cornell->Voxelize();
	cornell->MipMap();

	//sponza->CreateShadow();
	//sponza->RenderData();
	//sponza->Voxelize();
	//sponza->MipMap();

	// Add information to the antbar
	TwAddVarRO(antBar, "FPS", TW_TYPE_FLOAT, &FPS, " group=Info ");
	TwAddVarRO(antBar, "Cam Pos", cam->GetCameraTwType(), cam->GetCameraInfo(), NULL);
	//TwAddVarRW(antBar, "Cam Speed", TW_TYPE_FLOAT, cam->GetSpeedPtr(), " min=0 max=2000 step=10 group=Controls ");
	TwAddVarRW(antBar, "Cam Rot Speed", TW_TYPE_FLOAT, cam->GetRotSpeedPtr(), " min=0.0 max=0.010 step=0.001 group=Controls ");

	sceneType = TwDefineEnumFromString("Scene Selection", "Cornell, Sponza");
	TwAddVarCB(antBar, "Select Scene", sceneType, SetNewSceneCB, GetNewSceneCB, this, " group=Controls ");

	TwAddVarCB(antBar, "SceneOptions", Scene::GetSceneOptionTwType(), Scene::SetSceneOptionsCB, Scene::GetSceneOptionsCB, GetCurrentScene(), " group=Scene opened=true ");
	TwAddVarCB(antBar, "SceneToGPU", Scene::GetSceneTwType(), Scene::SetSceneCB, Scene::GetSceneCB, GetCurrentScene(), " group=Scene opened=true ");
	TwAddVarRW(antBar, "LightDir", TW_TYPE_DIR3F, GetCurrentScene()->GetLightDir(), "  group=Scene opened=true ");

#ifdef DEBUG
	//TwAddVarCB(antBar, "DrawCmd", Scene::GetDrawIndTwType(), Scene::SetDrawIndCB, Scene::GetDrawIndCB, GetCurrentScene(), " group=Scene opened=true ");
	//TwAddVarCB(antBar, "CompCmd", Scene::GetCompIndTwType(), Scene::SetCompIndCB, Scene::GetCompIndCB, GetCurrentScene(), " group=Scene opened=true ");
#endif // DEBUG
	
	// Check if AntTweak Setup is ok
	if(TwGetLastError() != NULL) return false;

	if(takeTime) {
		printf("Time per step logging\n");
		printf("Scene\t  CSA  \t  RDA  \t  V    \t  M    \t  DA\n");
	}

	return true;
}

void Program::Update() {
	// Upload program params (incl time update)
	UploadParams();

	GetCurrentScene()->UpdateBuffers();

	// Update the camera
	cam->Update(param.deltaT);
}

void Program::Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if(runNumber < 5 && takeTime) {
		glFinish();
		timer.startTimer();
	}

	GetCurrentScene()->CreateShadow();

	if(runNumber < 5 && takeTime) {
		glFinish();
		timer.endTimer();
		sceneAverage[0] += timer.getTimeMS() / 5.0f;

		timer.startTimer();
	}

	GetCurrentScene()->RenderData();

	if(runNumber < 5 && takeTime) {
		glFinish();
		timer.endTimer();
		sceneAverage[1] += timer.getTimeMS() / 5.0f;

		timer.startTimer();
	}

	GetCurrentScene()->Voxelize();

	if(runNumber < 5 && takeTime) {
		glFinish();
		timer.endTimer();
		sceneAverage[2] += timer.getTimeMS() / 5.0f;

		timer.startTimer();
	}

	GetCurrentScene()->MipMap();

	if(runNumber < 5 && takeTime) {
		glFinish();
		timer.endTimer();
		sceneAverage[3] += timer.getTimeMS() / 5.0f;

		timer.startTimer();
	}

	GetCurrentScene()->Draw();

	if(runNumber < 5 && takeTime) {
		glFinish();
		timer.endTimer();
		sceneAverage[4] += timer.getTimeMS() / 5.0f;
		runNumber++;
	}

	if(runNumber == 5 && takeTime) {
		printf("%4d \t%7.3f\t%7.3f\t%7.3f\t%7.3f\t%7.3f\n", runScene + 1, runScene > 0 ? sceneAverage[0] : 0.0f, sceneAverage[1], runScene > 1 ? sceneAverage[2] : 0.0f, runScene > 1 ? sceneAverage[3] : 0.0f, sceneAverage[4]);
		ToggleProgram();
	}

	TwDraw();

	SDL_GL_SwapWindow(screen);
}

void Program::ToggleProgram() {
	runScene = GetCurrentScene()->GetSceneParam()->voxelDraw;
	runScene++;
	runScene %= 5;
	GetCurrentScene()->GetSceneParam()->voxelDraw = runScene;
	runNumber = 0;
	sceneAverage[0] = 0.0f;
	sceneAverage[1] = 0.0f;
	sceneAverage[2] = 0.0f;
	sceneAverage[3] = 0.0f;
	sceneAverage[4] = 0.0f;
	if(runScene == 0 && takeTime) {
		takeTime++;
	}

	if(takeTime > 6) {
		takeTime = 0;
	}
}

void Program::Clean() {
	glDeleteBuffers(1, &programBuffer);
	TwTerminate();
	SDL_GL_DeleteContext(glcontext);
	SDL_Quit();
}

void Program::UploadParams() {
	// Update program parameters
	glBindBufferBase(GL_UNIFORM_BUFFER, PROGRAM, programBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, NULL, sizeof(ProgramStruct), &param);
}

void TW_CALL Program::SetNewSceneCB(const void* value, void* clientData) {
	Program* obj = static_cast<Program*>(clientData);
	obj->sceneSelect = *static_cast<const GLuint*>(value);
	obj->GetCurrentScene()->UpdateBuffers();
	TwRemoveVar(obj->antBar, "SceneOptions");
	TwRemoveVar(obj->antBar, "SceneToGPU");
	TwRemoveVar(obj->antBar, "LightDir");
	TwAddVarCB(obj->antBar, "SceneOptions", Scene::GetSceneOptionTwType(), Scene::SetSceneOptionsCB, Scene::GetSceneOptionsCB, obj->GetCurrentScene(), " group=Scene opened=true ");
	TwAddVarCB(obj->antBar, "SceneToGPU", Scene::GetSceneTwType(), Scene::SetSceneCB, Scene::GetSceneCB, obj->GetCurrentScene(), " group=Scene opened=true ");
	TwAddVarRW(obj->antBar, "LightDir", TW_TYPE_DIR3F, obj->GetCurrentScene()->GetLightDir(), "  group=Scene opened=true ");

#ifdef DEBUG
	//TwRemoveVar(obj->antBar, "DrawCmd");
	//TwRemoveVar(obj->antBar, "CompCmd");
	//TwAddVarCB(obj->antBar, "DrawCmd", Scene::GetDrawIndTwType(), Scene::SetDrawIndCB, Scene::GetDrawIndCB, obj->GetCurrentScene(), " group=Scene opened=true ");
	//TwAddVarCB(obj->antBar, "CompCmd", Scene::GetCompIndTwType(), Scene::SetCompIndCB, Scene::GetCompIndCB, obj->GetCurrentScene(), " group=Scene opened=true ");
#endif // DEBUG
}

void TW_CALL Program::GetNewSceneCB(void* value, void* clientData) {
	*static_cast<GLuint*>(value) = static_cast<Program*>(clientData)->sceneSelect;
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
					cam->Resize();
					GetCurrentScene()->SetupSceneTextures();
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
		case SDL_MOUSEWHEEL:
			cam->Zoom(1.0f + 0.05f * Event->wheel.y);
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
			break;
		case SDLK_f:
			cam->TogglePause();
			SDL_SetRelativeMouseMode(SDL_GetRelativeMouseMode() ? SDL_FALSE : SDL_TRUE);
			break;
		case SDLK_g:
			int isBarHidden;
			TwGetParam(antBar, NULL, "iconified", TW_PARAM_INT32, 1, &isBarHidden);
			if(isBarHidden) {
				TwDefine(" VCT iconified=false ");
			} else {
				TwDefine(" VCT iconified=true ");
			}
			break;
		default:
			break;
	}
}

void Program::OnMouseMove(SDL_Event *Event) {
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	if(!SDL_GetRelativeMouseMode()) {
		TwMouseMotion(Event->motion.x, Event->motion.y);
	} else {
		if(keystate[SDL_SCANCODE_LCTRL]) {
			GetCurrentScene()->PanLight((GLfloat)Event->motion.xrel, (GLfloat)Event->motion.yrel);
		} else {
			cam->Rotate(Event->motion.xrel, Event->motion.yrel);
		}
	}
}

void Program::CheckKeyDowns() {
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	if(keystate[SDL_SCANCODE_W]) {
		//cam->MoveForward();
	}
	if(keystate[SDL_SCANCODE_S]) {
		//cam->MoveBackward();
	}
	if(keystate[SDL_SCANCODE_A]) {
		//cam->MoveLeft();
	}
	if(keystate[SDL_SCANCODE_D]) {
		//cam->MoveRight();
	}
	if(keystate[SDL_SCANCODE_Q]) {
		//cam->MoveUp();
	}
	if(keystate[SDL_SCANCODE_E]) {
		//cam->MoveDown();
	}
}