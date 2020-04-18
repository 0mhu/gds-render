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
