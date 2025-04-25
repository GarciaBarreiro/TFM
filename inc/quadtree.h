// quick quadtree class without too much things inside for now used just for quadPart in partitions.hpp

#pragma once

#include "Lpoint.hpp"
#include "point.hpp"

class Quadtree
{
    private:
    unsigned int                  MAX_POINTS          = 100;
    static constexpr float        MIN_QUADRANT_RADIUS = 0.1;
    static constexpr short        QUADRANTS_PER_NODE  = 4;

    std::vector<Quadtree> quadrants_{};
    Point                 center_{};
    Point                 min_{};
    Point                 max_{};
    std::vector<Lpoint*>  points_{};
    float                 radius_{};

    public:
    Quadtree();

    explicit Quadtree(std::vector<Lpoint>& points);
    explicit Quadtree(std::vector<Lpoint>& points, std::pair<Point, Point> minmax);
    explicit Quadtree(std::vector<Lpoint>& points, std::pair<Point, Point> minmax, unsigned int maxpoints);
    explicit Quadtree(const Vector& center, const float radius);
    explicit Quadtree(const Vector& center, const float radius, unsigned int maxpoints);

    inline void setMin(const Point& min) { min_ = min; }
	inline void setMax(const Point& max) { max_ = max; }

    void computeQuadtreeLimits();

    void insertPoints(std::vector<Lpoint>& points);
    void insertPoint(Lpoint* p);
    void createQuadrants();
	void fillQuadrants();
	size_t quadrantIdx(const Lpoint* p) const;

	void buildQuadtree(std::vector<Lpoint>& points);

    [[nodiscard]] inline bool isLeaf() const { return quadrants_.empty(); }
    [[nodiscard]] inline bool isEmpty() const { return this->points_.empty(); };

    [[nodiscard]] std::vector<std::pair<Point, Point>> findLeafs();
    [[nodiscard]] std::vector<std::pair<Point, Point>> getQuadrants();
};