#pragma once

#include "Core/Types/CommonTypes.h"
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <algorithm>

/* a virtual camera, this is you */
class Camera
{
public:
	Camera() = default;
	Camera(const float& verticalFOV, const float& near, const float& far)
	{
		nearPlane = near;
		farPlane = far;
		setFOVh(verticalFOV);
		createConversionMatrices();

	};

	Camera& operator=(const Camera&) = default;
	Camera& operator=(Camera&&) = default;
	bool operator==(Camera* comparePtr) const { return comparePtr == this; }

	Transform transform;

	/* camera settings */
	float nearPlane = 1.f;
	float farPlane = 500.f;
	float vFOV = 45.f;
	float aspectRatio = 1920 / 1080;//1.333f;

	float testValue = 1.f; // debug test value, set by user input

	void setFOVh(const float& deg) 
	{ 
		vFOV = (float)Transform::degToRad((float)deg); 
	}

	glm::mat4 blenderToVulkanMatrix1, blenderToVulkanMatrix2;

	void createConversionMatrices()
	{
		auto& m1 = blenderToVulkanMatrix1;
		auto& m2 = blenderToVulkanMatrix2;
		// zero-initialize
		m1 = m2 = glm::mat4(0.f);

		// to swap x and z
		// a,b,c -> c,b,a
		// 0, 0, 1, 0
		// 0, 1, 0, 0
		// 1, 0, 0, 0
		// 0, 0, 0, 1
		m1[2][0] = 1.f;
		m1[1][1] = 1.f;
		m1[0][2] = 1.f;
		m1[3][3] = 1.f;

		// to swap and invert y and z
		// c,b,a -> c,-a,-b
		// 1, 0, 0, 0
		// 0, 0,-1, 0
		// 0,-1, 0, 0
		// 0, 0, 0, 1
		m2[0][0] = 1.f;
		m2[2][1] = -1.f;
		m2[1][2] = -1.f;
		m2[3][3] = 1.f;
	}


	//glm::mat4 getProjectionMatrix_legacy() 
	//{
	//	/*
	//	float left = -aspectRatio;
	//	float right = aspectRatio;
	//	float top = -1.f;
	//	float bottom = 1.f;
	//
	//	float Xdelta = right - left;
	//	float Ydelta = top - bottom;
	//	float Zdelta = farPlane - nearPlane;
	//
	//	glm::mat4 mat{0.f};
	//	mat[0][0] = nearPlane * 2.f / Xdelta;
	//	mat[1][1] = nearPlane * 2.f / Ydelta;
	//	mat[2][0] = (right + left) / Xdelta; // note: negate Z
	//	mat[2][1] = (top + bottom) / Ydelta;
	//	mat[2][2] = -(farPlane + nearPlane) / Zdelta;
	//	mat[2][3] = -1.f;
	//	mat[3][2] = (-2.f * nearPlane * farPlane) / Zdelta;
	//	return mat;
	//	*/
	//
	//	// lve: up = -Y,  right = +X,  forward = +Z
	//	const float tanHalfFovy = tan(vFOV / 2.f);
	//	float x = 1.f / (aspectRatio * tanHalfFovy);
	//	float y = 1.f / (tanHalfFovy);
	//	float z = farPlane / (farPlane - nearPlane);
	//	float w = -(farPlane * nearPlane) / (farPlane - nearPlane);
	//
	//	glm::mat4 lveMat
	//	{
	//		x,   0.f, 0.f, 0.f,
	//		0.f, y,   0.f, 0.f,
	//		0.f, 0.f, z,   1.f,
	//		0.f, 0.f, w,   0.f,
	//	};
	//
	//	return lveMat;
	//}


	// first variation of the Blender-style projection matrix
	glm::mat4 getProjectionMatrixB1(float nearDistance, float farDistance)
	{
		// for X-forward Z-up: rotate camera 90 deg counter-clockwise on Y, 90 deg clockwise on X
		const float b = nearDistance * tan(vFOV / 2);
		const float X_d = (aspectRatio * b) * 2;
		const float Y_d = -b * 2;
		const float Z_d = farDistance - nearDistance;

		const float x = nearDistance * 2.f / X_d;
		const float y = nearDistance * 2.f / Y_d;
		const float A = -(farDistance + nearDistance) / Z_d;
		const float B = (-2.f * nearDistance * farDistance) / Z_d;

		glm::mat4 mat
		{
			x,   0.f, 0.f,  0.f, // X
			0.f, y,   0.f,  0.f, // Y
			0.f, 0.f, A,    B,   // Z
			0.f, 0.f, -1.f, 0.f, // w
		};
		return mat;
	}

	glm::mat4 getProjectionMatrix(float aspectRatio)
	{
		const auto& f = farPlane;
		const auto& n = nearPlane;
		const auto& a = aspectRatio;
		const auto& fov = (float)Transform::degToRad(80.f);
		glm::mat4 m{ 0.f };
		m[0][0] = a / tan(fov / 2);
		m[1][1] = 1 / tan(fov / 2);
		m[2][2] = f / (f - n);
		m[3][2] = -((n * f) / (f - n));
		m[2][3] = 1.f;
		return m;
	}

	glm::mat4 getProjectionMatrixB2()
	{
		//void BKE_camera_params_compute_viewplane(CameraParams* params, int winx, int winy, float aspx, float aspy)
		// https://gamedev.stackexchange.com/questions/23395/how-to-convert-screen-space-into-3d-world-space
		// https://github.com/blender/blender/blob/9c0bffcc89f174f160805de042b00ae7c201c40b/source/blender/blenkernel/intern/camera.cc#L300
		//return getProjectionMatrix(nearPlane, farPlane);
		// TODO: EXPERIMENTAL!!!
		CameraParams p{};
		p.sensor_x = 1.f;
		p.lens = fov_to_focallength((float)Transform::degToRad(80.0), p.sensor_x);
		farPlane = testValue;

		glm::mat4 m{};
		compute_matrix(m, compute_viewplane(p, 1920, 1080), nearPlane, farPlane);
		return m;
	}
	
	// used for getProjectionMatrixB2
	float fov_to_focallength(float h_fov, float sensor)
	{
		return (sensor / 2.0f) / tanf(h_fov * 0.5f);
	}
	struct ViewPlane { float xmin, xmax, ymin, ymax; };
	struct CameraParams
	{
		float lens = 100.f;
		float sensor_x = 50.f;
		float zoom = 1.f;
	};
	// used for getProjectionMatrixB2
	void getAspxAspy(float w, float h, float& aspx, float& aspy) 
	{
		const auto aspectRatioY = w / h;
		aspx = 1;
		aspy = aspectRatioY;
		aspx *= w;
		aspy *= h;
	}
	// used for getProjectionMatrixB2. same as BKE_camera_params_compute_viewplane
	ViewPlane compute_viewplane(CameraParams& params, int winx, int winy)
	{
		ViewPlane viewplane;
		//float dx, dy;

		float aspx, aspy;
		getAspxAspy(winx, winy, aspx, aspy);

		float ycor = aspy / aspx;

		// just use horizontal sensor fit
		float sensor_size = params.sensor_x;
		float pixsize = (sensor_size * nearPlane) / params.lens;
		float viewfac = winx;

		pixsize /= viewfac;

		/* extra zoom factor */
		pixsize *= params.zoom;

		/* compute view plane:
		 * Fully centered, Z-buffer fills in jittered between `-.5` and `+.5`. */
		viewplane.xmin = -0.5f * float(winx); // "left"
		viewplane.xmax = 0.5f * float(winx); // "right"
		viewplane.ymin = -0.5f * ycor * float(winy); // "bottom"
		viewplane.ymax = 0.5f * ycor * float(winy); // "top"

		/* lens shift and offset */
		//dx = params.shiftx * viewfac + winx * params.offsetx;
		//dy = params.shifty * viewfac + winy * params.offsety;
		//viewplane.xmin += dx;
		//viewplane.ymin += dy;
		//viewplane.xmax += dx;
		//viewplane.ymax += dy;

		/* the window matrix is used for clipping, and not changed during OSA steps */
		/* using an offset of +0.5 here would give clip errors on edges */
		viewplane.xmin *= pixsize;
		viewplane.ymin *= pixsize;
		viewplane.xmax *= pixsize;
		viewplane.ymax *= pixsize;

		/* Used for rendering (offset by near-clip with perspective views), passed to RE_SetPixelSize.
		 * For viewport drawing 'RegionView3D.pixsize'. */
		//params.viewdx = pixsize;
		//params.viewdy = ycor * pixsize;
		return viewplane;
	}
	// used for getProjectionMatrixB2. does the same thing as BKE_camera_params_compute_matrix() / perspective_m4()
	void compute_matrix(glm::mat4& mat, const ViewPlane& viewplane, float near, float far)
	{
		const auto& left	= viewplane.xmin;
		const auto& right	= viewplane.xmax;
		const auto& bottom	= viewplane.ymin;
		const auto& top		= viewplane.ymax;

		float Xdelta = right - left;
		float Ydelta = top - bottom;
		float Zdelta = far - near;

		if (Xdelta == 0.0f || Ydelta == 0.0f || Zdelta == 0.0f) { return; }
		mat[0][0] = near * 2.0f / Xdelta;
		mat[1][1] = near * 2.0f / Ydelta;
		mat[2][0] = (right + left) / Xdelta; /* NOTE: negate Z. */
		mat[2][1] = (top + bottom) / Ydelta;
		mat[2][2] = -(far + near) / Zdelta;
		mat[2][3] = -1.0f;
		mat[3][2] = (-2.0f * near * far) / Zdelta;
		mat[0][1] = mat[0][2] = mat[0][3] = mat[1][0] = mat[1][2] = mat[1][3] = mat[3][0] = mat[3][1] = mat[3][3] = 0.0f;
	}



	glm::mat4 getViewMatrix(const bool& legacyMethod = false) 
	{
		if (legacyMethod) 
		{
			// using the camera's transform directly
			return glm::inverse(transform.mat4());
		}

		const glm::vec3& position = transform.translation;
		const glm::vec3& rotation = transform.rotation;
		// calculate the view matrix based on rotation and position
		const float c3 = glm::cos(rotation.x);
		const float s3 = glm::sin(rotation.x);
		const float c2 = glm::cos(rotation.y);
		const float s2 = glm::sin(rotation.y);
		const float c1 = glm::cos(rotation.z);
		const float s1 = glm::sin(rotation.z);
		const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
		const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
		const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
		glm::mat4 viewMatrix = glm::mat4{ 1.f };
		viewMatrix[0][0] = u.x;
		viewMatrix[1][0] = u.y;
		viewMatrix[2][0] = u.z;
		viewMatrix[0][1] = v.x;
		viewMatrix[1][1] = v.y;
		viewMatrix[2][1] = v.z;
		viewMatrix[0][2] = w.x;
		viewMatrix[1][2] = w.y;
		viewMatrix[2][2] = w.z;
		viewMatrix[3][0] = -glm::dot(u, position);
		viewMatrix[3][1] = -glm::dot(v, position);
		viewMatrix[3][2] = -glm::dot(w, position);
		return viewMatrix;
	}

	//glm::mat4 getProjectionMatrixAlt()
	//{
	//	const float n = nearPlane;
	//	const float f = farPlane;
	//	const float ar = aspectRatio;
	//	const float vFovTanh = tan(vFOV / 2.f);
	//
	//	/*
	//	glm::mat4 pmatrix{ 0.f };
	//
	//	// x scale
	//	pmatrix[0][0] = 1.f / (ar * vFovTanh);
	//	// y scale
	//	pmatrix[1][1] = 1.f / (vFovTanh);
	//	// z scale
	//	pmatrix[2][2] = f / (f - n);
	//	// w col 2
	//	pmatrix[2][3] = 1.f;
	//	// "forward" translation
	//	pmatrix[3][2] = -(f * n) / (f - n);
	//
	//	glm::mat4 pmatrix - same as above, visualized
	//	{
	//		x,   0.f, 0.f, 0.f,
	//		0.f, y,   0.f, 0.f,
	//		0.f, 0.f, A,   1.f,
	//		0.f, 0.f, B,   0.f,
	//	};
	//	*/
	//
	//	float x = 1.f / (ar * vFovTanh);
	//	float y = 1.f / (vFovTanh);
	//	float A = f / (f - n);
	//	float B = -(f * n) / (f - n);
	//
	//	
	//
	//	glm::mat4 pmatrix
	//	{
	//		A,   0.f, 0.f, 1.f, // X
	//		0.f, -x,  0.f, 0.f, // Y
	//		0.f, 0.f, -y,  0.f, // Z
	//		B,   0.f, 0.f, 0.f, // w
	//	};
	//
	//	return pmatrix;
	//}

	// alternate projection method (flips an axis, mismatch with blender)
	//glm::mat4 getProjectionMatrixCookbook()
	//{
	//	const float n = nearPlane;
	//	const float f = farPlane;
	//	const float a = aspectRatio;
	//	float x = 1.f / tan(glm::radians(vFOV / 2.f));
	//	//	"cookbook" projection 
	//	glm::mat4 pmatrix =
	//	{
	//		x / a, 0.0f, 0.0f, 0.0f,
	//		0.0f, x, 0.0f, 0.0f, /* [1][1]: x for y=down (default), -x for y=up */
	//		0.0f, 0.0f, f / (n - f), -1.0f,
	//		0.0f, 0.0f, -(f * n) / (f - n), 1.0f 
	//	};
	//	return pmatrix;
	//}

	void moveInPlaneXY(const Vector2D<double>& lookInput, const float& moveFwd, const float& moveRight, 
					const float& moveUp, const bool& extraSpeed, const float& deltaTime)
	{
		float lookSpeed = 6.8f;
		float moveSpeed = 30.f;
		if (extraSpeed) { moveSpeed *= 80.f; }

		float yawInput = lookInput.x != 0 ? lookInput.x / abs(lookInput.x) : 0.f; 
		float pitchInput = lookInput.y != 0 ? lookInput.y / abs(lookInput.y) : 0.f;
		auto rotV = Vec{ 0.f, pitchInput, -yawInput };

		//if (Vec::dot(rotV, rotV) > std::numeric_limits<float>::epsilon())
		//{ transform.rotation += rotV.getNormalized() * lookSpeed * deltaTime; }
		
		// limit pitch values to exactly 85 degrees
		//transform.rotation.y = glm::clamp(transform.rotation.y, 
		//		(float)Transform::degToRad(-85.f), (float)Transform::degToRad(85.f));
		// prevent overflow from continous yawing
		//transform.rotation.z = glm::mod(transform.rotation.z, glm::two_pi<float>());

		const Vec forwardDir = transform.getForwardVector();
		const Vec rightDir = Vec{ forwardDir.y, -forwardDir.x, 0.f };
		const Vec upDir{ 0.f, 0.f, 1.f };

		Vec moveDir{ 0.f };
		if (moveFwd > 0.f) { moveDir += forwardDir; }
		else if (moveFwd < 0.f) { moveDir -= forwardDir; }

		if (moveRight > 0.f) { moveDir += rightDir; }
		else if (moveRight < 0.f) { moveDir -= rightDir; }

		if (moveUp > 0.f) { moveDir += upDir; }
		else if (moveUp < 0.f) { moveDir -= upDir; }

		transform.translation += moveDir * moveSpeed * deltaTime;
		std::cout << "\n" << "x:" << transform.translation.x << "y:" << transform.translation.y << "z:" << transform.translation.z;
	}

	void moveInPlaneXYN(const Vector2D<double>& lookInput, const float& moveFwd, const float& moveRight,
					const float& moveUp, const bool& extraSpeed, const float& deltaTime)
	{
		//(GLFWwindow * window, float dt, LveGameObject & gameObject)
		float lookSpeed = 6.f;
		glm::vec3 rotate{ 0.f, lookInput.y, lookInput.x };

		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) 
		{
			//transform.rotation += lookSpeed * deltaTime * glm::normalize(rotate);
		}

		// limit pitch values between about +/- 85ish degrees
		//transform.rotation.y = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
		//transform.rotation.z = glm::mod(transform.rotation.y, glm::two_pi<float>());

		float yaw = transform.rotation.y;
		const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
		const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
		const glm::vec3 upDir{ 0.f, -1.f, 0.f };

		//glm::vec3 moveDir{ 0.f };
		//if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
		//if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
		//if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
		//if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
		//if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
		//if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;
		//
		//if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) 
		//{
		//	gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
		//}
	}

	static glm::mat4 getWorldBasisMatrix() 
	{
		/*	identity matrix rearranged to swap axes 
		*	resulting in a right-handed z-up/x-forward basis:
		*	-y (up) becomes z, x (right) becomes -y, z (forward) becomes x */
		glm::mat4 basis{ 0.f };
		basis[1][0] = -1.f; // x moved to y (row 2)
		basis[2][1] = -1.f; // y moved to z (row 3)
		basis[0][2] = -1.f; // z moved to x (row 1)
		basis[3][3] = 1.f; // always 1
		return basis;
	}

	

};
