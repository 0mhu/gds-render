#include <catch.hpp>
#include <limits>

extern "C" {
#include <gds-render/geometric/vector-operations.h>
}

TEST_CASE("geometric/vector-operations/vector_2d_add", "[GEOMETRIC]")
{
	struct vector_2d res;
	struct vector_2d a;
	struct vector_2d b;

	a.x = 1;
	a.y = 2;

	b.x = 2;
	b.y = 6;

	vector_2d_add(&res, &a, &b);

	REQUIRE(res.x == Approx(a.x + b.x));
	REQUIRE(res.y == Approx(a.y + b.y));
}

TEST_CASE("geometric/vector-operations/vector_2d_calculate_angle_between", "[GEOMETRIC]")
{
	double angle;
	struct vector_2d a;
	struct vector_2d b;

	a.x = 1;
	a.y = 0;

	b.x = 0;
	b.y = 1;

	angle = vector_2d_calculate_angle_between(&a, &a);
	REQUIRE(angle == Approx(0.0));

	angle = vector_2d_calculate_angle_between(&a, &b);
	REQUIRE(angle == Approx(90.0 / 180.0 * M_PI));
}

TEST_CASE("geometric/vector-operations/vector_2d_subtract", "[GEOMETRIC]")
{
	struct vector_2d res;
	struct vector_2d a;
	struct vector_2d b;

	a.x = 1;
	a.y = 2;

	b.x = 2;
	b.y = 6;

	vector_2d_subtract(&res, &a, &b);

	REQUIRE(res.x == Approx(a.x - b.x));
	REQUIRE(res.y == Approx(a.y - b.y));
}

TEST_CASE("geometric/vector-operations/vector_2d_abs", "[GEOMETRIC]")
{
	struct vector_2d c;
	struct vector_2d a;
	struct vector_2d b;
	double a_len, b_len, c_len;

	a.x = 1;
	a.y = 0;

	b.x = 0;
	b.y = 2;

	c.x = 3;
	c.y = 4;

	a_len = vector_2d_abs(&a);
	b_len = vector_2d_abs(&b);
	c_len = vector_2d_abs(&c);

	REQUIRE(a_len == Approx(1.0));
	REQUIRE(b_len == Approx(2.0));
	REQUIRE(c_len == Approx(5.0));
}

TEST_CASE("geometric/vector-operations/vector_2d_scalar_multipy", "[GEOMETRIC]")
{
	struct vector_2d c;
	struct vector_2d a;
	struct vector_2d b;
	double mult;

	a.x = 1;
	a.y = 0;
	b.x = 0;
	b.y = 2;
	mult = vector_2d_scalar_multipy(&a, &b);
	REQUIRE(mult == Approx(0.0));
	
	a.x = 1;
	a.y = 1;
	b.x = 1;
	b.y = 1;
	mult = vector_2d_scalar_multipy(&a, &b);
	REQUIRE(mult == Approx(2.0));	
}

TEST_CASE("geometric/vector-operations/vector_2d_normalize", "[GEOMETRIC]")
{
	struct vector_2d a;

	a.x = 1;
	a.y = 0;
	vector_2d_normalize(&a);
	REQUIRE(a.x == Approx(1.0));
	REQUIRE(a.y == Approx(0.0));

	a.x = 1;
	a.y = -1;
	vector_2d_normalize(&a);
	REQUIRE(a.x == Approx(1.0/sqrt(2)));
	REQUIRE(a.y == Approx(-1.0/sqrt(2)));
}

TEST_CASE("geometric/vector-operations/vector_2d_rotate", "[GEOMETRIC]")
{
	struct vector_2d a;

	a.x = 1;
	a.y = 0;
	vector_2d_rotate(&a, M_PI/2);
	REQUIRE(a.x == Approx(0.0).scale(0.001));
	REQUIRE(a.y == Approx(1.0));

	a.x = 0;
	a.y = 1;
	vector_2d_rotate(&a, -M_PI/2);
	vector_2d_rotate(&a, M_PI);
	REQUIRE(a.x == Approx(-1.0));
	REQUIRE(a.y == Approx(0.0).scale(0.001));
}

TEST_CASE("geometric/vector-operations/vector_2d_scale", "[GEOMETRIC]")
{
	struct vector_2d a;

	a.x = 1;
	a.y = 0;
	vector_2d_scale(&a, 2.0);
	REQUIRE(a.x == Approx(2.0));
	REQUIRE(a.y == Approx(0.0));

	a.x = 1;
	a.y = -3;
	vector_2d_scale(&a, 0.5);
	REQUIRE(a.x == Approx(0.5));
	REQUIRE(a.y == Approx(-1.5));
}
