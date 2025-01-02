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

struct SymParams {
	
	float dt;
	float mass;
	float c1;
	//float c2;
	float k;

	SymParams(float dt, float mass, float c1, float k) {
		this->dt = dt;
		this->mass = mass;
		this->c1 = c1;
		//this->c2 = c2;
		this->k = k;
	}
};

glm::vec3 calculateAcceleration(std::vector<glm::vec3> pos, int start, int end, float idealDist, SymParams params) {
	float dist = glm::distance(pos[start], pos[end]);
	float l = dist - idealDist;
	float force = -params.c1 * l;
	glm::vec3 acceleration = force * - glm::normalize(pos[end] - pos[start]) / params.mass;
	return acceleration;
}

struct SymMemory {
	SymParams params;
	SymData data; 
	std::unordered_map<int, std::vector<std::tuple<int, float>>> neighbours;

	std::mutex mutex;
	std::atomic<bool> stopThread;
	std::atomic<float> sleep_debt;

	SymMemory(float dt, float mass, float c1, float k, std::vector<glm::vec3> positions) 
		: params(dt, mass, c1, k), data(positions), stopThread(false), sleep_debt(0.f) {
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

		for (const auto& [key, value] : memory->neighbours) {
			glm::vec3 acceleration(-memory->params.k * memory->data.velocities[key] / memory->params.mass);

			for (const auto& val : value) {
				acceleration += calculateAcceleration(positions, key, std::get<0>(val), std::get<1>(val), memory->params);
			}

			memory->data.velocities[key] += acceleration * dt;
			memory->data.positions[key] += memory->data.velocities[key] * dt;
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