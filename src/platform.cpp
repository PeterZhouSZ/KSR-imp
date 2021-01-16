#include "platform.h"

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;

//bool rotate = true;
ImVec4 clear_color = ImVec4(0.2f, 0.3f, 0.3f, 1.00f);
float depth = 1;
bool rotate = false;
bool grow = false;
float grow_speed = -1;

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

//whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

//whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
	{
		lastX = xpos;
		lastY = ypos;
		return;
	}

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

//whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

Platform::Platform() { platform_init(); }

Platform::~Platform() { platform_shutdown(); }

void Platform::platform_init()
{

	if (!glfwInit())
	{
		std::cout << "glfw init failure" << std::endl;
	}
	const char *glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "KSR-imp", NULL, NULL);
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1); // Enable vsync
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	if (!gladLoadGL())
	{
		std::cout << "glad init failure" << std::endl;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;

	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	glEnable(GL_DEPTH_TEST);
	glClearDepth(depth);
}

void Platform::platform_shutdown()
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
}

void Platform::begin_frame()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void Platform::complete_frame()
{

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Platform::render_imgui(Kinetic_queue &k_queue, KPolygons_SET &kpolys_set, FT &kinetic_time)
{

	static float f = 0.0f;
	static int counter = 0;
	static double last_time = glfwGetTime();
	static FT next_kinetic_t = k_queue.next_time();
	// static auto k_poly_bak = kpolys_set;

	double dt = glfwGetTime() - last_time;
	last_time = last_time + dt;
	dt = std::powf(10, grow_speed) * dt;
	if (grow)
	{
		kpolys_set.move_dt(dt);
		kinetic_time += dt;
		// if (kinetic_time >= next_kinetic_t)
		// {
		// 	k_polys = k_poly_bak;
		// 	kinetic_time = k_queue.to_next_event();
		// 	k_poly_bak = k_polys;
		// 	next_kinetic_t = k_queue.next_time();
		// }
		// else
		// {
		// 	for (auto &k_poly : k_polys)
		// 		k_poly.update(k_poly.move_dt(dt));
		// }
	}

	ImGui::Begin("Hello, world!");		// Create a window called "Hello, world!" and append into it.
	ImGui::Checkbox("rotate", &rotate); // Edit bools storing our window open/close state
	ImGui::SliderFloat("depth", &depth, -1, 1);

	ImGui::Checkbox("growing", &grow);
	ImGui::SliderFloat("grow speed", &grow_speed, -2, 1);

	if (ImGui::Button("next event"))
	{
		kinetic_time = k_queue.to_next_event();
		next_kinetic_t = k_queue.next_time();
	}
	ImGui::SameLine();
	ImGui::Text("queue size = %d", k_queue.size());


	ImGui::Text("kinetic time = %.3f", CGAL::to_double(kinetic_time));
	ImGui::Text("next time = %.3f", CGAL::to_double(next_kinetic_t));

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}

void Platform::render_3d(Shader &shader, KPolygons_SET &kpolys_set)
{

	shader.use();
	glm::mat4 model(1); //model矩阵，局部坐标变换至世界坐标
	if (rotate)
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
	glm::mat4 view(1); //view矩阵，世界坐标变换至观察坐标系
	view = camera.GetViewMatrix();
	glm::mat4 projection(1); //projection矩阵，投影矩阵
	projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

	// 向着色器中传入参数
	shader.setMat4("model", model);
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);

	auto mesh = kpolys_set.Get_mesh();
	mesh.render(shader);
}

void Platform::clear()
{

	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearDepth(depth);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void Platform::render(Shader &shader, Kinetic_queue &k_queue, KPolygons_SET& kpolys_set, FT &kinetic_time)
{

	render_imgui(k_queue, kpolys_set, kinetic_time);
	clear();
	render_3d(shader, kpolys_set);
}

void Platform::loop(Shader &shader, Kinetic_queue &k_queue, KPolygons_SET& kpolys_set, FT &kinetic_time)
{

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		processInput(window);

		begin_frame();

		render(shader, k_queue, kpolys_set, kinetic_time);

		complete_frame();

		glfwSwapBuffers(window);
	}
}
