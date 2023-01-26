#ifndef IMAGE_H
#define IMAGE_H

#include <fstream>

#include "util.h"
#include "color.h"

void write_ppm(int image_width, int image_height, const std::string& filename, std::vector<color> &colors) {

	std::fstream imageFile(filename);
	
	if (!imageFile.is_open()) {
		LOG(LOG_TYPE::ERROR, "Error writing ppm image!");
	}
	imageFile << "P3\n" << image_width << ' ' << image_height << "\n255\n";
	

	for (int j = image_height - 1; j >= 0; --j) {
		std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;
		for (int i = 0; i < image_width; ++i) {
			auto r = double(i) / (image_width - 1);
			auto g = double(j) / (image_height - 1);
			auto b = 0.25;

			color pixel_color(r, g, b);
			write_color(imageFile, pixel_color);
		}
	}

	imageFile.close();
	std::cerr << "\nDone.\n";
	LOG(LOG_TYPE::INFO, "Wrote ppm image at " + filename);
	
}

#endif