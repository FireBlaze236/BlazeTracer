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
#include "texture.h"
#include "material.h"
#include "moving_sphere.h"
#include "bvh.h"
#include "aarect.h"

hittable_list random_scene() {
	hittable_list world;

	auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));
	auto ground_material = make_shared<lambertian>(checker);
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

hittable_list two_spheres() {
	hittable_list objects;

	auto checker = make_shared<checker_texture>(color(0.2, 0.3, 0.1), color(0.9, 0.9, 0.9));

	objects.add(make_shared<sphere>(point3(0, -10, 0), 10, make_shared<lambertian>(checker)));
	objects.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

	return objects;
}

hittable_list two_perlin_spheres() {
	hittable_list objects;

	auto pertext = make_shared<noise_texture>(4.0);
	objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
	objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));

	return objects;
}

hittable_list earth() {
	auto earth_texture = make_shared<image_texture>("earthmap.jpg");
	auto earth_surface = make_shared<diffuse_light>(earth_texture);
	auto globe = make_shared<sphere>(point3(0, 0, 0), 2, earth_surface);

	return hittable_list(globe);
}

hittable_list simple_light() {
	hittable_list objects;

	auto pertext = make_shared<noise_texture>(4);
	objects.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(pertext)));
	objects.add(make_shared<sphere>(point3(0, 2, 0), 2, make_shared<lambertian>(pertext)));
	objects.add(make_shared<sphere>(point3(0, 5, 0), 1, make_shared<diffuse_light>(color(5,5,5))));


	auto difflight = make_shared<diffuse_light>(color(4, 4, 4));
	objects.add(make_shared<xy_rect>(3, 5, 1, 3, -2, difflight));

	return objects;
}


//TRACING
color ray_color(const ray& r, const color& background, const hittable& world, int depth) {
	hit_record rec;
	

	if (depth <= 0) {
		return color(0, 0, 0);
	}
	if (!world.hit(r, 0.001, infinity, rec))
		return background;

	ray scattered;
	color attenuation;
	color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);


	if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
		return emitted;
	}

	return emitted + attenuation * ray_color(scattered, background, world, depth - 1);

	//vec3 unit_direction = unit_vector(r.direction());
	//auto t = 0.5 * (unit_direction.y() + 1.0);
	//return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
}



int threadsDone = 0;
std::map<std::thread::id, double> threadProgress;

void thread_trace(std::vector<std::vector<color>>& colors, color& bg, hittable& world, camera cam,
	int max_depth, int start_line, int end_line, int image_height, int image_width, int samples_per_pixel, std::chrono::steady_clock::time_point start) {
	for (int j = end_line; j >= start_line; --j) {
		//std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i) {
			color pixel_color(0, 0, 0);
			for (int s = 0; s < samples_per_pixel; ++s) {
				auto u = double(i + random_double()) / (image_width - 1);
				auto v = double(j + random_double()) / (image_height - 1);

				ray r = cam.get_ray(u, v);


				pixel_color += ray_color(r, bg, world, max_depth);

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
	const int image_width = 512;
	int samples_per_pixel = 40;
	const int max_depth = 50;
	//World
	auto R = cos(pi / 4);
	hittable_list world;

	point3 lookfrom;
	point3 lookat;
	auto vfov = 40.0;
	auto aperture = 0.0;
	color background(0, 0, 0);

	switch (5) {
	case 1:
		world = random_scene();
		background = color(0.70, 0.80, 1.00);
		lookfrom = point3(13, 2, 3);
		lookat = point3(0, 0, 0);
		vfov = 20.0;
		aperture = 0.1;
		break;

	
	case 2:
		world = two_spheres();
		background = color(0.70, 0.80, 1.00);
		lookfrom = point3(13, 2, 3);
		lookat = point3(0, 0, 0);
		vfov = 20.0;
		break;

	default:
	case 3:
		world = two_perlin_spheres();
		background = color(0.70, 0.80, 1.00);
		lookfrom = point3(13, 2, 3);
		lookat = point3(0, 0, 0);
		vfov = 20.0;
		break;
	case 4:
		world = earth();
		background = color(0.70, 0.80, 1.00);
		lookfrom = point3(13, 2, 3);
		lookat = point3(0, 0, 0);
		vfov = 20.0;
		break;
	case 5:
		world = simple_light();
		samples_per_pixel = 400;
		background = color(0.0, 0.0, 0.0);
		lookfrom = point3(26, 3, 6);
		lookat = point3(0, 2, 0);
		vfov = 20.0;
		break;

	}

	// Camera

	vec3 vup(0, 1, 0);
	auto dist_to_focus = 10.0;
	int image_height = static_cast<int>(image_width / aspect_ratio);

	camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

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
	int thread_count = 4;
	int inc = image_height / thread_count;
	int st = 0;
	int end = inc;
	for (int i = 0; i < thread_count; i++) {

		std::thread t(thread_trace, std::ref(colors), std::ref(background), std::ref(scene), cam, max_depth, st , end-1,
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
