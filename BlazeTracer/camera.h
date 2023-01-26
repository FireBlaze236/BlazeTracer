#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"

class camera {
public:
	point3 origin;
	point3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
	
	vec3 forward, right, up;
	double lens_radius;
public:

	camera(point3 lookfrom, point3 lookat, vec3 vup,
		double vfov, double aspect_ratio,
		double aperture,
		double focus_dist) {
		auto theta = degrees_to_radians(vfov);
		auto h = tan(theta / 2);
		
		auto viewport_height = 2.0 * h;
		auto viewport_width = aspect_ratio * viewport_height;
		

		forward = unit_vector(lookfrom - lookat);
		right = unit_vector(cross(vup, forward));
		up = cross(forward, right);

		origin = lookfrom;
		horizontal = focus_dist * viewport_width * right;
		vertical = focus_dist * viewport_height * up;

		lower_left_corner = origin - horizontal / 2 - vertical / 2 - focus_dist * forward;

		lens_radius = aperture / 2;
	}

	ray get_ray(double u, double v) const {

		vec3 rd = lens_radius * random_in_unit_disk();
		vec3 offset = right * rd.x() + up * rd.y();

		return ray(origin + offset,
			lower_left_corner + u * horizontal + v * vertical - origin - offset);
	}
};

#endif