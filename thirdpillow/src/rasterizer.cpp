/*
 * rasterizer.cpp
 *
 *  Created on: Apr 29, 2014
 *      Author: jwpilly
 */

#include "rasterizer.h"

rasterizer::rasterizer(color* c) {
	this->default_color = *c;
}

color* rasterizer::get_default_color() {
	return &this->default_color;
}

void rasterizer::set_default_color(color* c) {
	this->default_color = *c;
}

void rasterizer::draw_span(screen* s, span* a, image* texture, int y) {
	int x_diff = (int)a->get_x_2() - (int)a->get_x_1();
	if (x_diff == 0) {
		return;
	}
	vector2* t_diff = a->get_uv_b()->clone();
	t_diff->subtract(a->get_uv_a());
	float factor = (float)0;
	float factor_step = (float)1 / (float)x_diff;
	for (int x = (int)a->get_x_1(); x < (int)a->get_x_2(); x++) {
		float z_depth = putils::linear_interpolate(a->get_z_1(), a->get_z_2(), (float)x / (a->get_x_2() - a->get_x_1()));
		if (z_depth < s->get_z(x, y) || true) {
			vector2* t = a->get_uv_a()->clone();
			vector2* t_1 = t_diff->clone();
			t_1->multiply(factor);
			t->add(t_1);
			float uv_x = t->get_x() * (float)texture->get_width();
			float uv_y = t->get_y() * (float)texture->get_height();
			color* i = texture->get_color(uv_x, uv_y);
			s->set_pixel(x, y, i);
			factor += factor_step;
			delete t;
			delete t_1;
		}
	}
	delete t_diff;
}

void rasterizer::draw_edge_span(screen* s, edge* a, edge* b, image* texture) {
	float y_diff_1 = (float)(a->get_b()->get_y() - a->get_a()->get_y());
	if (y_diff_1 == (float)0) {
		return;
	}
	float y_diff_2 = (float)(b->get_b()->get_y() - b->get_a()->get_y());
	if (y_diff_2 == (float)0) {
		return;
	}
	float x_diff_1 = (float)(a->get_b()->get_x() - a->get_a()->get_x());
	float x_diff_2 = (float)(b->get_b()->get_x() - b->get_a()->get_x());
	vector2* e_1 = a->get_uv_b()->clone();
	e_1->subtract(a->get_uv_a());
	vector2* e_2 = b->get_uv_b()->clone();
	e_2->subtract(b->get_uv_a());
	float factor_1 = (float)(b->get_a()->get_y() - a->get_a()->get_y()) / y_diff_1;
	float factor_step_1 = (float)1 / y_diff_1;
	float factor_2 = (float)0;
	float factor_step_2 = (float)1 / y_diff_2;
	for (int y = (int)b->get_a()->get_y(); y < (int)b->get_b()->get_y(); y++) {
		vector2* c = a->get_uv_a()->clone();
		vector2* c_1 = e_1->clone();
		c_1->multiply(factor_1);
		c->add(c_1);
		int c_x_1 = (int)a->get_a()->get_x() + (int)(x_diff_1 * factor_1);
		vector2* d = b->get_uv_a()->clone();
		vector2* d_1 = e_2->clone();
		d_1->multiply(factor_2);
		d->add(d_1);
		int c_x_2 = (int)b->get_a()->get_x() + (int)(x_diff_2 * factor_2);
		float z_1 = putils::linear_interpolate(a->get_z_a(), b->get_z_a(), (float)y/(b->get_a()->get_y() - b->get_a()->get_y()));
		float z_2 = putils::linear_interpolate(a->get_z_b(), b->get_z_b(), (float)y/(b->get_a()->get_y() - b->get_a()->get_y()));
		span* sp = new span(*c, z_1, c_x_1, *d, z_2, c_x_2);
		draw_span(s, sp, texture, y);
		delete c;
		delete c_1;
		delete d;
		delete d_1;
		delete sp;
		factor_1 += factor_step_1;
		factor_2 += factor_step_2;
	}
	delete e_1;
	delete e_2;
}

void rasterizer::draw_triangle_wire_color(screen* s, vector2* a, color* a_color, vector2* b, color* b_color, vector2* c, color* c_color) {
	draw_line_color(s, a, a_color, b, b_color);
	draw_line_color(s, b, b_color, c, c_color);
	draw_line_color(s, c, c_color, a, a_color);
}

void rasterizer::draw_triangle_wire(screen* s, vector2* a, vector2* b, vector2* c) {
	draw_line_color(s, a, b, &this->default_color);
	draw_line_color(s, b, c, &this->default_color);
	draw_line_color(s, c, a, &this->default_color);
}

void rasterizer::draw_triangle_wire(screen* s, triangle2* t) {
	vector2* v_t = t->get_vertices();
	draw_triangle_wire(s, &v_t[0], &v_t[1], &v_t[2]);
	free(v_t);
}

void rasterizer::draw_line_color(screen* s, vector2* a, vector2* b, color* c) {
	float x_diff = b->get_x() - a->get_x();
	float y_diff = b->get_y() - a->get_y();
	if (x_diff == 0 && y_diff == 0) {
		s->set_pixel(a->get_x(), a->get_y(), c);
		return;
	}
	if (fabs(x_diff) > fabs(y_diff)) {
		float x_min, x_max;
		if (a->get_x() < b->get_x()) {
			x_min = a->get_x();
			x_max = b->get_x();
		}
		else {
			x_min = b->get_x();
			x_max = a->get_x();
		}
		float slope = y_diff / x_diff;
		for (float x = x_min; x <= x_max; x += (float)1) {
			float y = a->get_y() + ((x - a->get_x()) * slope);
			s->set_pixel((int)x, (int)y, c);
		}
	}
	else {
		float y_min, y_max;
		if (a->get_y() < b->get_y()) {
			y_min = a->get_y();
			y_max = b->get_y();
		}
		else {
			y_min = b->get_y();
			y_max = a->get_y();
		}
		float slope = x_diff / y_diff;
		for (float y = y_min; y <= y_max; y += (float)1) {
			float x = a->get_x() + ((y - a->get_y()) * slope);
			s->set_pixel((int)x, (int)y, c);
		}
	}
}

void rasterizer::draw_line_color(screen* s, vector2* a, color* a_color, vector2* b, color* b_color) {
	float x_diff = b->get_x() - a->get_x();
	float y_diff = b->get_y() - a->get_y();
	if (x_diff == 0 && y_diff == 0) {
		s->set_pixel(a->get_x(), a->get_y(), a_color);
		return;
	}
	if (fabs(x_diff) > fabs(y_diff)) {
		float x_min, x_max;
		if (a->get_x() < b->get_x()) {
			x_min = a->get_x();
			x_max = b->get_x();
		}
		else {
			x_min = b->get_x();
			x_max = a->get_x();
		}
		float slope = y_diff / x_diff;
		for (float x = x_min; x <= x_max; x += (float)1) {
			float y = a->get_y() + ((x - a->get_x()) * slope);
			color* c = a_color->clone();
			float c_s = (x - a->get_x()) / x_diff;
			color* d = b_color->clone();
			d->subtract(a_color);
			d->multiply(c_s);
			c->add(d);
			s->set_pixel((int)x, (int)y, c);
			delete c;
			delete d;
		}
	}
	else {
		float y_min, y_max;
		if (a->get_y() < b->get_y()) {
			y_min = a->get_y();
			y_max = b->get_y();
		}
		else {
			y_min = b->get_y();
			y_max = a->get_y();
		}
		float slope = x_diff / y_diff;
		for (float y = y_min; y <= y_max; y += (float)1) {
			float x = a->get_x() + ((y - a->get_y()) * slope);
			color* c = a_color->clone();
			float c_s = ((y - a->get_y()) / y_diff);
			color* d = b_color->clone();
			d->subtract(a_color);
			d->multiply(c_s);
			c->add(d);
			s->set_pixel((int)x, (int)y, c);
			delete c;
			delete d;
		}
	}
}

void rasterizer::draw_triangle3_wire(screen* s, triangle3* t3, matrix4* mt) {
	triangle2* flat = t3->flatten(mt);
	this->draw_triangle_wire(s, flat);
	delete flat;
}

void rasterizer::draw_face_textured(screen* s, face* f, image* texture, matrix4* mt) {
	triangle2* flat = f->get_triangle()->flatten(mt);
	vector2* vertices = flat->get_vertices();
	vector2* uvs = f->get_uvs();
	edge* edges[3];
	edges[0] = new edge(vertices[0], uvs[0], f->get_triangle()->get_vertices()[0].get_z(), vertices[1], uvs[1], f->get_triangle()->get_vertices()[1].get_z());
	edges[1] = new edge(vertices[1], uvs[1], f->get_triangle()->get_vertices()[1].get_z(), vertices[2], uvs[2], f->get_triangle()->get_vertices()[2].get_z());
	edges[2] = new edge(vertices[2], uvs[2], f->get_triangle()->get_vertices()[2].get_z(), vertices[0], uvs[0], f->get_triangle()->get_vertices()[0].get_z());
	float max_length = (float)0;
	int long_edge = 0;
	for (int i = 0; i < 3; i++) {
		float length = edges[i]->get_b()->get_y() - edges[i]->get_a()->get_y();
		if (length > max_length) {
			max_length = length;
			long_edge = i;
		}
	}
	int short_1 = (long_edge + 1) % 3;
	int short_2 = (long_edge + 2) % 3;
	draw_edge_span(s, edges[long_edge], edges[short_1], texture);
	draw_edge_span(s, edges[long_edge], edges[short_2], texture);
	for (int k = 0; k < 3; k++) {
		delete edges[k];
	}
	delete flat;
}

void rasterizer::draw_mesh_wire(screen* s, mesh* m, matrix4* mt) {
	for (int i = 0; i < m->faces.size(); i++) {
		draw_triangle3_wire(s, m->faces[i].get_triangle(), mt);
	}
}

void rasterizer::draw_mesh_normals(screen* s, mesh* m, matrix4* mt) {
	for (int i = 0; i < m->faces.size(); i++) {
		vector3* local_normal = m->faces[i].get_triangle()->get_normal();
		vector3* barycenter = m->faces[i].get_triangle()->get_center();
		vector3* normal = barycenter->clone();
		normal->add(local_normal);
		vector4* a4 = new vector4(barycenter);
		vector4* b4 = new vector4(normal);
		a4->multiply_first(mt);
		b4->multiply_first(mt);
		a4->set_x(a4->get_x() / a4->get_w());
		a4->set_y(a4->get_y() / a4->get_w());
		a4->set_z(a4->get_z() / a4->get_w());
		a4->set_x(a4->get_x() / a4->get_z());
		a4->set_y(a4->get_y() / a4->get_z());
		b4->set_x(b4->get_x() / b4->get_w());
		b4->set_y(b4->get_y() / b4->get_w());
		b4->set_z(b4->get_z() / b4->get_w());
		b4->set_x(b4->get_x() / b4->get_z());
		b4->set_y(b4->get_y() / b4->get_z());
		float p_x_a = ((a4->get_x() / (float)2) + (float)0.5) * (float)transform::get_camera()->get_render_width();
		float p_y_a = ((a4->get_y() / (float)2) + (float)0.5) * (float)transform::get_camera()->get_render_height();
		float p_x_b = ((b4->get_x() / (float)2) + (float)0.5) * (float)transform::get_camera()->get_render_width();
		float p_y_b = ((b4->get_y() / (float)2) + (float)0.5) * (float)transform::get_camera()->get_render_height();
		vector2* a = new vector2(p_x_a, p_y_a);
		vector2* b = new vector2(p_x_b, p_y_b);
		delete a4;
		delete b4;
		delete barycenter;
		delete normal;
		color* white = new color(1, 1, 1, 1);
		draw_line_color(s, a, b, white);
		delete a;
		delete b;
		delete white;
	}
}

void rasterizer::draw_mesh_textured(screen *s, mesh* m, image* texture, matrix4* mt) {
	for (int i = 0; i < m->faces.size(); i++) {
		draw_face_textured(s, &m->faces[i], texture, mt);
	}
}

void rasterizer::draw_mesh_wire_cull(screen* s, mesh* m, matrix4* mt) {
	for (int i = 0; i < m->faces.size(); i++) {
		float dot = m->faces[i].get_triangle()->get_normal()->dot_product(transform::get_camera()->get_forward());
		if (dot <= 0.8) {
			draw_triangle3_wire(s, m->faces[i].get_triangle(), mt);
		}
	}
}

void rasterizer::set_pixel(screen* s, vector2* p, color* c) {
	s->set_pixel((int)p->get_x(), (int)p->get_y(), c);
}

rasterizer::~rasterizer() {
}

