#pragma once

#include <thread> 
#include <mutex>
#include <atomic> 
#include <chrono>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <tuple>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const float DISTANCE = 1.f / 3.f;
const float CROSS_DISTANCE = sqrt(DISTANCE * DISTANCE * 2.f);
const float G = 9.81f;
const std::unordered_map<int, int> CONTROL_CUBE_MAPPING = {
	{0, 0},
	{1, 3},
	{2, 15},
	{3, 12},
	{4, 48},
	{5, 51},
	{6, 63},
	{7, 60}
};

glm::vec3 drawNormalizedDirection() {
	float theta = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 2.0f * M_PI;
	float phi = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * M_PI;

	float x = sin(phi) * cos(theta);
	float y = sin(phi) * sin(theta);
	float z = cos(phi);

	return glm::normalize(glm::vec3(x, y, z));
};

struct SymData {
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> velocities;
	float time;

	SymData(std::vector<glm::vec3> pos) : time(0.f) {
		for (auto& pos : pos) {
			positions.push_back(pos);
			velocities.push_back(glm::vec3(0.f));
		}
	}
};

struct RestrainingStruct {
	float xMin;
	float xMax;
	float yMin;
	float yMax;
	float zMin;
	float zMax;

	RestrainingStruct(std::vector<glm::vec3> restrainingCube) {
		xMin = std::numeric_limits<float>::max();
		xMax = std::numeric_limits<float>::min();
		yMin = std::numeric_limits<float>::max();
		yMax = std::numeric_limits<float>::min();
		zMin = std::numeric_limits<float>::max();
		zMax = std::numeric_limits<float>::min();
		for (auto& pos : restrainingCube) {
			if (pos.x < xMin) xMin = pos.x;
			if (pos.x > xMax) xMax = pos.x;
			if (pos.y < yMin) yMin = pos.y;
			if (pos.y > yMax) yMax = pos.y;
			if (pos.z < zMin) zMin = pos.z;
			if (pos.z > zMax) zMax = pos.z;
		}
	}
};

struct SymParams {
	
	float dt;
	float mass;
	float c1;
	float c2;
	float k;
	std::vector<glm::vec3> controlCube;
	float mu;
	RestrainingStruct restraints;
	bool wholeVectorReflection;
	float gravity;

	SymParams(float dt, float mass, float c1, float c2, float k, std::vector<glm::vec3> controlCube, float mu, std::vector<glm::vec3> restrainingCube, bool wholeVectorReflection, float gravity) 
		: restraints(restrainingCube) {
		this->dt = dt;
		this->mass = mass;
		this->c1 = c1;
		this->c2 = c2;
		this->k = k;
		this->controlCube = controlCube;
		this->mu = mu;
		this->wholeVectorReflection = wholeVectorReflection;
		this->gravity = gravity;
	}
};

glm::vec3 calculateAcceleration(glm::vec3 startPos, glm::vec3 endPos, float idealDist, float c, float mass) {
	float dist = glm::distance(startPos, endPos);
	float l = dist - idealDist;
	float force = -c * l;
	glm::vec3 acceleration = force * - (dist == 0 ? glm::vec3(0.f) : glm::normalize(endPos - startPos)) / mass;
	return acceleration;
}

void AdjustForCollisions(glm::vec3& position, glm::vec3& velocity, float mu, float dt, RestrainingStruct restraints, bool wholeVectorReflection) {
	glm::vec3 newPos = position + velocity * dt;
	bool collision = false;

	if (newPos.x < restraints.xMin) {
		collision = true;
		float t = (restraints.xMin - position.x) / velocity.x;
		position += velocity * t;
		velocity.x *= -mu;
		if (wholeVectorReflection) {
			velocity.y *= mu;
			velocity.z *= mu;
		}
		AdjustForCollisions(position, velocity, mu, dt - t, restraints, wholeVectorReflection);
	}
	if (newPos.x > restraints.xMax) {
		collision = true;
		float t = (restraints.xMax - position.x) / velocity.x;
		position += velocity * t;
		velocity.x *= -mu;
		if (wholeVectorReflection) {
			velocity.y *= mu;
			velocity.z *= mu;
		}
		AdjustForCollisions(position, velocity, mu, dt - t, restraints, wholeVectorReflection);
	}
	if (newPos.y < restraints.yMin) {
		collision = true;
		float t = (restraints.yMin - position.y) / velocity.y;
		position += velocity * t;
		velocity.y *= -mu;
		if (wholeVectorReflection) {
			velocity.x *= mu;
			velocity.z *= mu;
		}
		AdjustForCollisions(position, velocity, mu, dt - t, restraints, wholeVectorReflection);
	}
	if (newPos.y > restraints.yMax) {
		collision = true;
		float t = (restraints.yMax - position.y) / velocity.y;
		position += velocity * t;
		velocity.y *= -mu;
		if (wholeVectorReflection) {
			velocity.x *= mu;
			velocity.z *= mu;
		}
		AdjustForCollisions(position, velocity, mu, dt - t, restraints, wholeVectorReflection);
	}
	if (newPos.z < restraints.zMin) {
		collision = true;
		float t = (restraints.zMin - position.z) / velocity.z;
		position += velocity * t;
		velocity.z *= -mu;
		if (wholeVectorReflection) {
			velocity.x *= mu;
			velocity.y *= mu;
		}
		AdjustForCollisions(position, velocity, mu, dt - t, restraints, wholeVectorReflection);
	}
	if (newPos.z > restraints.zMax) {
		collision = true;
		float t = (restraints.zMax - position.z) / velocity.z;
		position += velocity * t;
		velocity.z *= -mu;
		if (wholeVectorReflection) {
			velocity.x *= mu;
			velocity.y *= mu;
		}
		AdjustForCollisions(position, velocity, mu, dt - t, restraints, wholeVectorReflection);
	}
	if (!collision)
		position = newPos;
}

struct SymMemory {
	SymParams params;
	SymData data; 
	std::unordered_map<int, std::vector<std::tuple<int, float>>> neighbours;

	std::mutex mutex;
	std::atomic<bool> stopThread;
	std::atomic<float> sleep_debt;

	SymMemory(float dt, float mass, float c1, float c2, float k, std::vector<glm::vec3> controlCube, float mu, std::vector<glm::vec3> restrainingCube, bool wholeVectorReflection, float gravity, std::vector<glm::vec3> positions)
		: params(dt, mass, c1, c2, k, controlCube, mu, restrainingCube, wholeVectorReflection, gravity), data(positions), stopThread(false), sleep_debt(0.f) {
		CalculateNeighbours(positions.size());
	}

	~SymMemory() {
		mutex.lock();
		stopThread = true;
		mutex.unlock();
	}

	void Distrupt(float maxDisturbance) {
		mutex.lock();
		for (auto& velocity : data.velocities) {
			glm::vec3 direction = drawNormalizedDirection();
			float disturbance = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * maxDisturbance;
			velocity += direction * disturbance;
		}
		mutex.unlock();
	}

	void CalculateNeighbours(int count) {
		this->neighbours.clear();

		for (int i = 0; i < count; i++) {
			std::vector<std::tuple<int, float>> temp;

			bool mostFront = i < 16;
			bool mostBack = i >= 48;
			bool mostLeft = i % 4 == 0;
			bool mostRight = i % 4 == 3;
			bool mostBottom = i % 16 < 4;
			bool mostTop = i % 16 >= 12;

			// ORDER: front->top->back->bottom
			if (!mostFront)
				temp.push_back({ i - 16, DISTANCE });
			if (!mostFront && !mostTop)
				temp.push_back({ i - 12, CROSS_DISTANCE });
			if (!mostTop)
				temp.push_back({ i + 4, DISTANCE });
			if (!mostTop && !mostBack)
				temp.push_back({ i + 20, CROSS_DISTANCE });
			if (!mostBack)
				temp.push_back({ i + 16, DISTANCE });
			if (!mostBack && !mostBottom)
				temp.push_back({ i + 12, CROSS_DISTANCE });
			if (!mostBottom)
				temp.push_back({ i - 4, DISTANCE });
			if (!mostBottom && !mostFront)
				temp.push_back({ i - 20, CROSS_DISTANCE });

			// ORDER: left->top->right->bottom
			if (!mostLeft)
				temp.push_back({ i - 1, DISTANCE });
			if (!mostLeft && !mostTop)
				temp.push_back({ i + 3, CROSS_DISTANCE });
			//if (!mostTop)
			//  temp.push_back({ i + 4, DISTANCE });
			if (!mostTop && !mostRight)
				temp.push_back({ i + 5, CROSS_DISTANCE });
			if (!mostRight)
				temp.push_back({ i + 1, DISTANCE });
			if (!mostRight && !mostBottom)
				temp.push_back({ i - 3, CROSS_DISTANCE });
			//if (!mostBottom)
			//  temp.push_back({ i - 4, DISTANCE });
			if (!mostBottom && !mostLeft)
				temp.push_back({ i - 5, CROSS_DISTANCE });

			// ORDER: top->right-back->bottom->left-front
			//if (!mostTop)
			//  temp.push_back({ i + 4, DISTANCE });
			//if (!mostTop && !mostBack && !mostRight)
			//  temp.push_back({ i + 21, 3D_DISTANCE });
			if (!mostBack && !mostRight)
				temp.push_back({ i + 17, CROSS_DISTANCE });
			//if (!mostBack && !mostRight && !mostBottom)
			//  temp.push_back({ i + 13, 3D_DISTANCE });
			//if (!mostBottom)
			//  temp.push_back({ i - 4, DISTANCE });
			//if (!mostBottom && !mostFront && !mostLeft)
			//  temp.push_back({ i - 21, 3D_DISTANCE });
			if (!mostFront && !mostLeft)
				temp.push_back({ i - 17, CROSS_DISTANCE });
			//if (!mostFront && !mostLeft && !mostTop)
			//  temp.push_back({ i - 13, 3D_DISTANCE });

			// ORDER: top->left-back->bottom->right-front
			//if (!mostTop)
			//  temp.push_back({ i + 4, DISTANCE });
			//if (!mostTop && !mostBack && !mostLeft)
			//  temp.push_back({ i + 19, 3D_DISTANCE });
			if (!mostBack && !mostLeft)
				temp.push_back({ i + 15, CROSS_DISTANCE });
			//if (!mostBack && !mostLeft && !mostBottom)
			//  temp.push_back({ i + 11, 3D_DISTANCE });
			//if (!mostBottom)
			//  temp.push_back({ i - 4, DISTANCE });
			//if (!mostBottom && !mostFront && !mostRight)
			//  temp.push_back({ i - 19, 3D_DISTANCE });
			if (!mostFront && !mostRight)
				temp.push_back({ i - 15, CROSS_DISTANCE });
			//if (!mostFront && !mostRight && !mostTop)
			//  temp.push_back({ i - 11, 3D_DISTANCE });

			this->neighbours[i] = temp;
		}
	}
};

void calculationThread(SymMemory* memory) {
	std::chrono::high_resolution_clock::time_point calc_start, calc_end, wait_start;

	while (!memory->stopThread) {
		calc_start = std::chrono::high_resolution_clock::now();

		memory->mutex.lock();

		float dt = memory->params.dt / 1000.f;
		memory->data.time += dt;

		// create copy of positions
		std::vector<glm::vec3> positions = memory->data.positions;

		// springs within the cube
		for (const auto& [key, value] : memory->neighbours) {
			glm::vec3 acceleration(-memory->params.k * memory->data.velocities[key] / memory->params.mass);

			for (const auto& val : value) {
				acceleration += calculateAcceleration(positions[key], positions[std::get<0>(val)], std::get<1>(val), memory->params.c1, memory->params.mass);
			}

			memory->data.velocities[key] += acceleration * dt;
		}

		// springs between the cube and the control cube
		for (const auto& [key, value] : CONTROL_CUBE_MAPPING) {
			glm::vec3 acceleration = calculateAcceleration(positions[value], memory->params.controlCube[key], 0, memory->params.c2, memory->params.mass);
			memory->data.velocities[value] += acceleration * dt;
		}

		// gravity
		for (auto& velocity : memory->data.velocities) {
			float acceleration = -G / memory->params.mass * memory->params.gravity;
			velocity.y += acceleration * dt;
		}

		for (int i = 0; i < memory->data.positions.size(); i++) {
			AdjustForCollisions(memory->data.positions[i], memory->data.velocities[i], memory->params.mu, dt, memory->params.restraints, memory->params.wholeVectorReflection);
		}

		memory->mutex.unlock();

		calc_end = std::chrono::high_resolution_clock::now();

		float time2sleep = memory->params.dt * 1000000.f - std::chrono::duration_cast<std::chrono::nanoseconds>(calc_end - calc_start).count() - memory->sleep_debt;

		wait_start = std::chrono::high_resolution_clock::now();

		if (time2sleep > 0) {
			while (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - wait_start).count() < time2sleep) {
				// busy waiting because std::this_thread::sleep_for sucks ass
			}
		}

		memory->sleep_debt = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - wait_start).count() - time2sleep;
	}
}