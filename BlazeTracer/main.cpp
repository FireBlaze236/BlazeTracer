// BlazeTracer.cpp : Defines the entry point for the application.
//

#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <map>
#include <sstream>

#include "rtweekend.h"

#include "color.h"
#include "hittable_list.h"
#include "sphere.h"

#include "camera.h"

#include "material.h"

#include "moving_sphere.h"

#include "bvh.h"

hittable_list random_scene() {
	hittable_list world;

	auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
	world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));

	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			auto choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9) {
				shared_ptr<material> sphere_material;

				if (choose_mat < 0.8) {
					// diffuse
					auto albedo = color::random() * color::random();
					sphere_material = make_shared<lambertian>(albedo);
					auto center2 = center + vec3(0, random_double(0, .5), 0);
					world.add(make_shared<moving_sphere>(
						center, center2, 0.0, 1.0, 0.2, sphere_material));

				}
				else if (choose_mat < 0.95) {
					// metal
					auto albedo = color::random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = make_shared<metal>(albedo, fuzz);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else {
					// glass
					sphere_material = make_shared<dielectric>(1.5);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}

	auto material1 = make_shared<dielectric>(1.5);
	world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

	auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
	world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

	auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

	return world;
}



color ray_color(const ray& r, const hittable& world, int depth) {
	hit_record rec;

	if (depth <= 0) {
		return color(0, 0, 0);
	}
	
	if (world.hit(r, 0.001, infinity, rec)) {
		ray scattered;
		color attenuation;

		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			return attenuation * ray_color(scattered, world, depth - 1);
		}

		return color(0, 0, 0);
	}
	vec3 unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}

color ray_bvh(const ray& r, const bvh_node& world, int depth) {
	hit_record rec;

	if (depth <= 0) {
		return color(0, 0, 0);
	}

	if (world.hit(r, 0.001, infinity, rec)) {
		ray scattered;
		color attenuation;

		if (rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			return attenuation * ray_color(scattered, world, depth - 1);
		}

		return color(0, 0, 0);
	}
	vec3 unit_direction = unit_vector(r.direction());
	auto t = 0.5 * (unit_direction.y() + 1.0);
	return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}


int threadsDone = 0;
std::map<std::thread::id, double> threadProgress;

void thread_trace(std::vector<std::vector<color>>& colors, bvh_node& world, camera cam,
	int max_depth, int start_line, int end_line, int image_height, int image_width, int samples_per_pixel, std::chrono::steady_clock::time_point start) {
	for (int j = end_line; j >= start_line; --j) {
		//std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i) {
			color pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; ++s) {
				auto u = double(i + random_double()) / (image_width - 1);
				auto v = double(j + random_double()) / (image_height - 1);

				ray r = cam.get_ray(u, v);


				pixel_color += ray_bvh(r, world, max_depth);

				threadProgress[std::this_thread::get_id()] = (double)(end_line - j) / (end_line - start_line);
			}
			colors[j][i] = pixel_color;
		}
	}

	

	threadsDone++;
}


int main()
{
	//Image
	const auto aspect_ratio = 16.0 / 9.0;
	const int image_width = 1280;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int samples_per_pixel = 100;
	const int max_depth = 50;
	//World
	auto R = cos(pi / 4);
	auto world = random_scene();

	auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
	auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
	auto material_left = make_shared<dielectric>(1.5);
	auto material_right = make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);

	//world.add(make_shared<sphere>(point3(0.0, -100.5, -1.0), 100.0, material_ground));
	//world.add(make_shared<sphere>(point3(0.0, 0.0, -1.0), 0.5, material_center));
	//world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), 0.5, material_left));
	//world.add(make_shared<sphere>(point3(-1.0, 0.0, -1.0), -0.45, material_left));
	//world.add(make_shared<sphere>(point3(1.0, 0.0, -1.0), 0.5, material_right));
	//Camera
	point3 lookfrom(13, 2, 3);
	point3 lookat(0, 0, 0);
	vec3 vup(0, 1, 0);
	auto dist_to_focus = 10.0;
	auto aperture = 0.1;
	camera cam(lookfrom, lookat, vup, 20, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);
	
	//Render
	std::fstream imageFile("out.ppm");
	//imageFile.open("out.ppm");

	if (!imageFile.is_open()) {
		LOG(LOG_TYPE::ERROR, "Error writing ppm image!");
	}
	imageFile << "P3\n" << image_width << ' ' << image_height << "\n255\n";

	std::vector<std::vector<color>> colors(image_height, std::vector<color>(image_width));

	auto start = std::chrono::high_resolution_clock::now();
	/*
	for (int j = image_height - 1; j >= 0; --j) {
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i) {
			color pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; ++s) {
				auto u = double(i + random_double()) / (image_width - 1);
				auto v = double(j + random_double()) / (image_height - 1);

				ray r = cam.get_ray(u, v);


				pixel_color += ray_color(r, world, max_depth);
			}
			colors[j][i] = pixel_color;
		}
	}
	*/

	bvh_node scene(world.objects, 0, world.objects.size(), 0, 0);

	std::vector<std::thread> threads;
	int thread_count = 12;
	int inc = image_height / thread_count;
	int st = 0;
	int end = inc;
	for (int i = 0; i < thread_count; i++) {

		std::thread t(thread_trace, std::ref(colors), std::ref(scene), cam, max_depth, st , end-1,
			image_height, image_width, samples_per_pixel, start);

		st += inc;
		end += inc;

		threadProgress[t.get_id()] = 0;
		threads.push_back(std::move(t));
	}
	
	while (threadsDone < thread_count) {
		std::stringstream ss;
		

		int count = 1;
		for (auto& t : threads) {
			ss << "Thread " <<  count << " :"
				<< threadProgress[t.get_id()] * 100.0 << '%' << '\n';
			count++;
		}
		ss << "Threads remaining: " << thread_count - threadsDone << '\n';
		ss << '\r';
		std::cerr << ss.str();
		std::cerr.flush();
		std::this_thread::sleep_for(std::chrono::microseconds(500));
		


	}

	for (auto& t : threads) {
		t.join();
	}

	auto dur = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start);
	std::cout << "\nTime: " << dur << '\n';

	

	
	//thread_trace(colors, world, cam, max_depth, image_height, image_width, samples_per_pixel);

	

	for (int i = colors.size() - 1; i >= 0; --i) {
		auto cc = colors[i];
		for (auto c : cc) {
			write_color(imageFile, c, samples_per_pixel);
		}
	}

	imageFile.close();
	std::cerr << "\nDone.\n";
	LOG(LOG_TYPE::INFO, "Wrote ppm image");
	
	return 0;
}
