#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"

#include "hittable.h"

class material {
public:
	virtual bool scatter(const ray& r, const hit_record& rec, color& attenuation,
		ray& scattered) const = 0;
};

class lambertian : public material {
public:
	color albedo;
public:
	lambertian(const color& a) : albedo(a) {}
	virtual bool scatter(const ray& r, const hit_record& rec, color& attenuation,
		ray& scattered) const override {
		auto scatter_direction = rec.normal + random_unit_vector();

		//Catch degenerate scatter direction
		if (scatter_direction.near_zero()) {
			scatter_direction = rec.normal;
		}
		scattered = ray(rec.p, scatter_direction);
		attenuation = albedo;

		return true;
	}
};

class metal : public material {
public:
	color albedo;
	double fuzz;
public:
	metal(const color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}
	virtual bool scatter(const ray& r, const hit_record& rec, color& attenuation,
		ray& scattered) const override {
		auto reflected = reflect(r.direction(), rec.normal);

		scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());

		attenuation = albedo;

		//Catch degenerate scatter directio
		return dot(scattered.direction(), rec.normal) > 0.0;
	}
};

#endif