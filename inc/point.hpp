//
// Created by miguelyermo on 1/3/20.
//

#pragma once

inline constexpr float SENSEPSILON = 0.001; // Sensible epsilon

#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include <armadillo>
#include "cheesemap/cheesemap.hpp"

using Vector = class Point; // Point and Vector are exactly the same entity, but this is done
                            // to preserve mathematical correctness

class Point : public chs::Point
{
	protected:
	unsigned int id_{}; // Id of the point (in order of reading)

	public:
	Point() = default;
	explicit Point(unsigned int id) : id_(id) {}

	// 2D Geometric constructor ( Z = 0.0 )
	Point(double x, double y) : chs::Point({x, y}) {}

	// 3D Geometric constructor
	Point(double x, double y, double z) : chs::Point({x, y, z}) {}

	// 3D Geometric constructor with ID
	Point(unsigned int id, double x, double y, double z) : id_(id), chs::Point({x, y, z}) {}

	// Point Methods
	/* 2D distance between two points */
	[[nodiscard]] inline double distance2D(const Point& p) const
	{
		double diffX = this->at(0) - p.getX();
		double diffY = this->at(1) - p.getY();

		return sqrt(diffX * diffX + diffY * diffY);
	}

	/* 2D distance between two points as coded in C version */
	[[nodiscard]] inline double distance2D100Rounded(const Point& p) const
	{
		double diffX = std::round(this->at(0) * 100) - std::round(p.getX() * 100);
		double diffY = std::round(this->at(1) * 100) - std::round(p.getY() * 100);

		return sqrt(diffX * diffX + diffY * diffY);
	}

	[[nodiscard]] inline double distance2Dsquared(const Point& p) const
	{
		const double diffX = this->at(0) - p.getX();
		const double diffY = this->at(1) - p.getY();

		return diffX * diffX + diffY * diffY;
	}

	/* 3D distance between two points */
	[[nodiscard]] inline double distance3D(const Point& p) const
	{
		double diffX = this->at(0) - p.getX();
		double diffY = this->at(1) - p.getY();
		double diffZ = this->at(2) - p.getZ();

		return sqrt(diffX * diffX + diffY * diffY + diffZ * diffZ);
	}

	[[nodiscard]] inline double distanceToLine(const Point& l1, const Point& l2) const
	/**
	 * Computes the distance of this point to the line defined by two points
	 * @param l1 Origin of the line
	 * @param l2 End of the line
	 * @return
	 */
	{
		double y2_y1 = l2.getY() - l1.getY();
		double x2_x1 = l2.getX() - l1.getX();

		return fabs(y2_y1 * this->at(0) - x2_x1 * this->at(1) + l2.getX() * l1.getY() - l2.getY() * l1.getX()) /
		       sqrt(y2_y1 * y2_y1 + x2_x1 * x2_x1);
	}

	[[nodiscard]] inline double distanceToLine2(const Point& p, const Vector& d) const
	/**
	 * Computes the distance from this point to the line passing through p with direction d (unitary).
	 *
	 * @param p starting point of the line
	 * @param d unitary direction of the line
	 * @return distance to line
	 */
	{
		const Vector pt = *this - p;
		// return (pt - d * (pt.dotProduct(d))).norm3D();
		return arma::norm(pt - Vector(d) * (pt.dotProduct(d)));
	}

	[[nodiscard]] inline Point getDest(Vector& v, double distance) const
	/**
	 * Get the destination point of moving a point form a vector a given distance
	 */
	{
		v.normalize2D();
		return { this->at(0) + v.getX() * distance, this->at(1) + v.getY() * distance, this->at(2) };
	}

	// Vector Methods
	/* Gets the perpendicular vector (XY plane) */
	[[nodiscard]] inline Vector perpenVector() const { return { -this->at(1), this->at(0), this->at(2) }; }

	[[nodiscard]] inline bool isZero() const
	{
		return std::abs(this->at(0)) < SENSEPSILON && std::abs(this->at(1)) < SENSEPSILON && std::abs(this->at(2)) < SENSEPSILON;
	}

	/* Returns the 2D norm of the vector */
	[[nodiscard]] inline double norm2D() const { return std::sqrt(this->at(0) * this->at(0) + this->at(1) * this->at(1)); }

	/* Returns the 3D norm of the vector */
	[[nodiscard]] inline double norm3D() const { return this->at(0) * this->at(0) + this->at(1) * this->at(1) + this->at(2) * this->at(2); }

	/* Normalization of the vector in two dimensions */
	void normalize2D()
	{
		const double norm = norm2D();
		if (norm < SENSEPSILON) { std::cerr << "Warning: \"normalize2D\" called on a zero-ish vector\n"; }
		*this /= norm;
	}

	/* Normalization of the vector in three dimensions */
	void normalize3D()
	{
		const double norm = norm3D();
		if (norm2D() < SENSEPSILON) { std::cerr << "Warning: \"normalize3D\" called on a zero-ish vector\n"; }
		*this /= norm;
	}

	/* Project onto XY plane */
	void projectXY() { this->setZ(0.0); }

	/* Dot product in 2D */
	[[nodiscard]] inline double dotProduct2D(const Vector& vec) const { return this->at(0) * vec.getX() + this->at(1) * vec.getY(); }

	/* Dot product in 3D */
	[[nodiscard]] inline double dotProduct(const Vector& vec) const
	{
		return this->at(0) * vec.getX() + this->at(1) * vec.getY() + this->at(2) * vec.getZ();
	}

	/* Cross product */
	[[nodiscard]] inline Vector crossProduct(const Vector& vec) const
	{
		return { this->at(1) * vec.getZ() - this->at(2) * vec.getY(), this->at(2) * vec.getX() - this->at(0) * vec.getZ(), this->at(0) * vec.getY() - this->at(1) * vec.getX() };
	}

	/* Angle between two points */
	[[nodiscard]] inline double angle(const Point& p) const
	{
		return acos(this->dotProduct(p) / (this->norm3D() * p.norm3D()));
	}


	/* Returns the angle formed by each coordinate with its corresponding edge */
	[[nodiscard]] inline Vector vectorAngles() const
	{
		double denom = norm3D();

		if (denom != 0)
		{
			return { (acos(this->at(0) / denom) * 180.0 / M_PI), (acos(this->at(1) / denom) * 180.0 / M_PI),
				       (acos(this->at(2) / denom) * 180.0 / M_PI) };
		}

		return {};
	}


	// Overload << operator for Point.
	friend std::ostream& operator<<(std::ostream& out, const Point& p)
	{
		out << p.id_ << " " << p(0) << " " << p(1) << " " << p(2);
		return out;
	}


	// Overload << operator for Point*
	friend std::ostream& operator<<(std::ostream& out, const Point* p)
	{
		out << *p;
		return out;
	}

	friend Point abs(const Point& p) { return { std::abs(p(0)), std::abs(p(1)), std::abs(p(2)) }; }

	// Operator overloading (pointer version done with dereferencing in place)
	Point  operator+(const Point& v) const { return { this->at(0) + v.getX(), this->at(1) + v.getY(), this->at(2) + v.getZ() }; }
	Point  operator-(const Point& v) const { return { this->at(0) - v.getX(), this->at(1) - v.getY(), this->at(2) - v.getZ() }; }
	Point& operator+=(const Point& p)
	{
		this->at(0) += p.getX();
		this->at(1) += p.getY();
		this->at(2) += p.getZ();

		return *this;
	}
	Point& operator-=(const Point& p)
	{
		this->at(0) -= p.getX();
		this->at(1) -= p.getY();
		this->at(2) -= p.getZ();

		return *this;
	}

	bool operator==(const Point& p) const
	{
		return std::abs(this->at(0) - p.getX()) < SENSEPSILON && std::abs(this->at(1) - p.getY()) < SENSEPSILON &&
		       std::abs(this->at(2) - p.getZ()) < SENSEPSILON;
	}

	bool operator!=(const Point& p) const { return !(*this == p); }

	Point& operator/=(const double val)
	{
		this->at(0) /= val;
		this->at(1) /= val;
		this->at(2) /= val;

		return *this;
	}

	friend Point operator-(const Point& lhs, const double val) { return { lhs(0) - val, lhs(1) - val, lhs(2) - val }; }

	friend Point operator+(const Point& lhs, const double val) { return { lhs(0) + val, lhs(1) + val, lhs(2) + val }; }

	friend Point operator-(const double val, const Point& rhs) { return { rhs(0) - val, rhs(1) - val, rhs(2) - val }; }

	friend Point operator+(const double val, const Point& rhs) { return { rhs(0) + val, rhs(1) + val, rhs(2) + val }; }

	// Multiplication of vector and a scalar
	template<typename T>
	Vector operator*(const T scalar) const
	{
		return Vector(this->at(0) * scalar, this->at(1) * scalar, this->at(2) * scalar);
	}

	// Division of vector and a scalar
	template<typename T>
	Vector operator/(const T scalar) const
	{
		return Vector(this->at(0) / scalar, this->at(1) / scalar, this->at(2) / scalar);
	}

	template<typename T>
	void operator/=(const T scalar)
	{
		this->at(0) /= scalar, this->at(1) /= scalar, this->at(2) /= scalar;
	}


	// Getters and setters
	[[nodiscard]] inline unsigned int id() const { return id_; }
	inline void                       id(unsigned int id) { id_ = id; }
	[[nodiscard]] inline double       getX() const { return this->at(0); }
	inline void                       setX(double x) { this->at(0) = x; }
	[[nodiscard]] inline double       getY() const { return this->at(1); }
	inline void                       setY(double y) { this->at(1) = y; }
	[[nodiscard]] inline double       getZ() const { return this->at(2); }
	inline void                       setZ(double z) { this->at(2) = z; }
};