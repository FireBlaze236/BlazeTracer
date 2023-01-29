#ifndef TEXTURE_H
#define TEXTURE_H

#include "rtweekend.h"

#include "perlin.h"

class texture {
public:
	virtual color value(double u, double v, const point3& p) const = 0;
};

class solid_color : public texture {
private:
	color color_value;
public:
	solid_color() {}
	solid_color(color c) : color_value(c) {}

	solid_color(double red, double green, double blue) 
		: solid_color(color(red, green, blue)) {}

	virtual color value(double u, double v, const point3& p) const override {
		return color_value;
	}
};


class checker_texture : public texture {
public:
	shared_ptr<texture> odd;
	shared_ptr<texture> even;
public:
	checker_texture() {}
	checker_texture(shared_ptr<texture> _even, shared_ptr<texture> _odd)
		: odd(_odd), even(_even) {}

	checker_texture(color a, color b)
		: odd(make_shared<solid_color>(a)), even(make_shared<solid_color>(b))
	{}

	virtual color value(double u, double v, const point3& p) const override {
		auto sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());

		if (sines < 0) {
			return odd->value(u, v, p);
		}
		else {
			return even->value(u, v, p);
		}
	}
};


class noise_texture : public texture {
public:
	perlin noise;
	double scale;
public:
	noise_texture() : scale(1) {}
	noise_texture(double sc) : scale(sc) {}

	virtual color value(double u, double v, const point3& p) const override {
		return color(1, 1, 1) * noise.noise(scale * p);
	}



};


#endif