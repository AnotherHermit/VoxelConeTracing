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
	drawVoxelOverlay = false;
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
	glClearColor(0.2f, 0.2f, 0.4f, 1.0f);

	dumpInfo();

	printError("after wrapper inits");

	glGenBuffers(1, &programBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, PROGRAM, programBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ProgramStruct), &param, GL_STREAM_DRAW);
	
	// TODO: See if it is possible to recude number of shaders by using subroutines
	// Load shaders for drawing
	shaders.simple = loadShaders("src/shaders/simpleModel.vert", "src/shaders/simpleModel.frag");
	shaders.texture = loadShaders("src/shaders/textureModel.vert", "src/shaders/textureModel.frag");
	shaders.mask = loadShaders("src/shaders/maskModel.vert", "src/shaders/maskModel.frag");

	// Load shaders for voxelization
	shaders.voxelize = loadShadersG("src/shaders/voxelizationSimple.vert", "src/shaders/voxelizationSimple.frag", "src/shaders/voxelizationSimple.geom");
	shaders.voxelizeTexture = loadShadersG("src/shaders/voxelizationTexture.vert", "src/shaders/voxelizationTexture.frag", "src/shaders/voxelizationTexture.geom");

	// Single triangle shader for deferred shading etc.
	shaders.singleTriangle = loadShaders("src/shaders/singleTriangle.vert", "src/shaders/singleTriangle.frag");

	// Draw voxels from 3D texture
	shaders.voxel = loadShaders("src/shaders/voxelSimple.vert", "src/shaders/voxelSimple.frag");

	// Calculate mipmaps
	shaders.mipmap = CompileComputeShader("src/shaders/mipmap.comp");

	// Create shadowmap
	shaders.shadowMap = loadShaders("src/shaders/shadowMap.vert", "src/shaders/shadowMap.frag");

	// Inject light(!)
	//shaders.lightInjection = CompileComputeShader("src/shaders/lightInjection.comp");

	// TODO: Make this a separate function
	// Set constant uniforms for the drawing programs
	glUseProgram(shaders.texture);
	glUniform1i(DIFF_UNIT, 0);

	glUseProgram(shaders.mask);
	glUniform1i(DIFF_UNIT, 0);
	glUniform1i(MASK_UNIT, 1);

	// Set constant uniforms for voxel programs
	glUseProgram(shaders.voxelize);
	glUniform1i(VOXEL_TEXTURE, 2);
	glUniform1i(VOXEL_DATA, 3);
	glUniform1i(SHADOW_UNIT, 5);

	// Set constant uniforms for voxel programs
	glUseProgram(shaders.voxelizeTexture);
	glUniform1i(DIFF_UNIT, 0);
	glUniform1i(VOXEL_TEXTURE, 2);
	glUniform1i(VOXEL_DATA, 3);

	// Set constant uniforms for simple triangle drawing
	glUseProgram(shaders.singleTriangle);
	glUniform1i(VOXEL_TEXTURE, 2);
	glUniform1i(VOXEL_DATA, 3);
	glUniform1i(SHADOW_UNIT, 5);

	// Set constant uniforms for drawing the voxel overlay
	glUseProgram(shaders.voxel);
	glUniform1i(VOXEL_DATA, 3);

	// Set constant uniforms for calculating mipmaps
	glUseProgram(shaders.mipmap);
	glUniform1i(VOXEL_DATA, 3);
	glUniform1i(VOXEL_DATA_NEXT, 4);

	// Set constants uniforms for calculating shadowmaps
	glUseProgram(shaders.shadowMap);
	glUniform1i(SHADOW_UNIT, 5);

	// Set constants uniforms for light injection
	//glUseProgram(shaders.lightInjection);
	// glUniform1i(VOXEL_DATA, 3);
	
	// Set up the AntBar
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(winWidth, winWidth);
	antBar = TwNewBar("VCT");
	TwDefine(" VCT refresh=0.1 size='300 520' valueswidth=140 ");
	TwDefine(" VCT help='This program simulates Global Illumination with Voxel Cone Tracing.' ");

	// Set up the camera
	cam = new Camera(cameraStartPos, &winWidth, &winHeight, cameraFrustumFar);
	if(!cam->Init()) return false;

	// Load scenes
	Scene* cornell = new Scene();
	if(!cornell->Init("resources/cornell.obj", &shaders)) return false;
	scenes.push_back(cornell);
	
	//Scene* sponza = new Scene();
	//if(!sponza->Init("resources/sponza.obj", &shaders)) return false;
	//scenes.push_back(sponza);
	
	// Initial Voxelization of the scenes
	cornell->Voxelize();
	cornell->InjectLight();
	cornell->MipMap();

	//sponza->Voxelize();
	//sponza->InjectLight();
	//sponza->MipMap();

	// Add information to the antbar
	TwAddVarRO(antBar, "FPS", TW_TYPE_FLOAT, &FPS, " group=Info ");
	TwAddVarRO(antBar, "Cam Pos", cam->GetCameraTwType(), cam->GetCameraInfo(), NULL);
	TwAddVarRW(antBar, "Cam Speed", TW_TYPE_FLOAT, cam->GetSpeedPtr(), " min=0 max=2000 step=10 group=Controls ");
	TwAddVarRW(antBar, "Cam Rot Speed", TW_TYPE_FLOAT, cam->GetRotSpeedPtr(), " min=0.0 max=0.010 step=0.001 group=Controls ");

	sceneType = TwDefineEnumFromString("Scene Selection", "Cornell, Sponza");
	TwAddVarCB(antBar, "Select Scene", sceneType, SetNewSceneCB, GetNewSceneCB, this, " group=Controls ");

	TwAddVarCB(antBar, "SceneOptions", Scene::GetSceneOptionTwType(), Scene::SetSceneOptionsCB, Scene::GetSceneOptionsCB, GetCurrentScene(), " group=Scene opened=true ");
	TwAddVarCB(antBar, "SceneToGPU", Scene::GetSceneTwType(), Scene::SetSceneCB, Scene::GetSceneCB, GetCurrentScene(), " group=Scene opened=true ");
	TwAddVarRW(antBar, "LightDir", TW_TYPE_DIR3F, GetCurrentScene()->GetLightDir(), "  group=Scene ");

#ifdef DEBUG
	TwAddVarCB(antBar, "DrawCmd", Scene::GetDrawIndTwType(), Scene::SetDrawIndCB, Scene::GetDrawIndCB, GetCurrentScene(), " group=Scene opened=true ");
	TwAddVarCB(antBar, "CompCmd", Scene::GetCompIndTwType(), Scene::SetCompIndCB, Scene::GetCompIndCB, GetCurrentScene(), " group=Scene opened=true ");
#endif // DEBUG

	// TODO: Add directional light as a control

	// Check if AntTweak Setup is ok
	if(TwGetLastError() != NULL) return false;

	return true;
}

void Program::Update() {
	// Upload program params (incl time update)
	UploadParams();

	// Update the camera
	cam->UpdateCamera();
}

void Program::Render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GetCurrentScene()->Voxelize();
	GetCurrentScene()->InjectLight();
	GetCurrentScene()->MipMap();
	GetCurrentScene()->Draw();

	TwDraw();

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
	glBindBufferBase(GL_UNIFORM_BUFFER, PROGRAM, programBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, NULL, sizeof(ProgramStruct), &param);
}

void TW_CALL Program::SetNewSceneCB(const void* value, void* clientData) {
	Program* obj = static_cast<Program*>(clientData);
	obj->sceneSelect = *static_cast<const GLuint*>(value);
	obj->GetCurrentScene()->UpdateBuffers();
	TwRemoveVar(obj->antBar, "SceneOptions");
	TwRemoveVar(obj->antBar, "SceneToGPU");
	TwAddVarCB(obj->antBar, "SceneOptions", Scene::GetSceneOptionTwType(), Scene::SetSceneOptionsCB, Scene::GetSceneOptionsCB, obj->GetCurrentScene(), " group=Scene opened=true ");
	TwAddVarCB(obj->antBar, "SceneToGPU", Scene::GetSceneTwType(), Scene::SetSceneCB, Scene::GetSceneCB, obj->GetCurrentScene(), " group=Scene opened=true ");

#ifdef DEBUG
	TwRemoveVar(obj->antBar, "DrawCmd");
	TwRemoveVar(obj->antBar, "CompCmd");
	TwAddVarCB(obj->antBar, "DrawCmd", Scene::GetDrawIndTwType(), Scene::SetDrawIndCB, Scene::GetDrawIndCB, obj->GetCurrentScene(), " group=Scene opened=true ");
	TwAddVarCB(obj->antBar, "CompCmd", Scene::GetCompIndTwType(), Scene::SetCompIndCB, Scene::GetCompIndCB, obj->GetCurrentScene(), " group=Scene opened=true ");
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