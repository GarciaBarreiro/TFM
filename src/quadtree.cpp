#include "quadtree.h"
#include "Box.hpp"

Quadtree::Quadtree() = default;

Quadtree::Quadtree(std::vector<Lpoint>& points)
{
    // given how mbb is implemented, could happen that radius returns Z
    // for our point clouds this shouldn't be a problem, but something to keep in mind
    center_ = mbb(points, radius_);
    quadrants_.reserve(QUADRANTS_PER_NODE);
    buildQuadtree(points);
}

Quadtree::Quadtree(std::vector<Lpoint>& points, std::pair<Point, Point> minmax) : min_(minmax.first), max_(minmax.second)
{
    center_ = midpoint(min_, max_);
    Box box(minmax);
    radius_ = std::max({ box.radii().getX(), box.radii().getY() });
    quadrants_.reserve(QUADRANTS_PER_NODE);
    insertPoints(points);
}

Quadtree::Quadtree(std::vector<Lpoint>& points, std::pair<Point, Point> minmax, unsigned int maxpoints) :
  min_(minmax.first), max_(minmax.second), MAX_POINTS(maxpoints)
{
    center_ = midpoint(min_, max_);
    Box box(minmax);
    radius_ = std::max({ box.radii().getX(), box.radii().getY() });
    quadrants_.reserve(QUADRANTS_PER_NODE);
    insertPoints(points);
}

Quadtree::Quadtree(const Vector& center, const float radius) : center_(center), radius_(radius)
{
	quadrants_.reserve(QUADRANTS_PER_NODE);
    computeQuadtreeLimits();
}

Quadtree::Quadtree(const Vector& center, const float radius, unsigned int maxpoints) :
  center_(center), radius_(radius), MAX_POINTS(maxpoints)
{
	quadrants_.reserve(QUADRANTS_PER_NODE);
    computeQuadtreeLimits();
}

void Quadtree::computeQuadtreeLimits()
{
    min_ = center_ - radius_;
    max_ = center_ + radius_;
}

void Quadtree::insertPoints(std::vector<Lpoint>& points)
{
    for (Lpoint& p : points)
    {
        insertPoint(&p);
    }
}

void Quadtree::insertPoint(Lpoint* p)
{
    unsigned int idx = 0;
    if (isLeaf())
    {
        if (isEmpty()) { points_.emplace_back(p); }
        else
        {
            if (points_.size() > MAX_POINTS && radius_ >= MIN_QUADRANT_RADIUS)
            {
                createQuadrants();
                fillQuadrants();
                idx = quadrantIdx(p);
                quadrants_[idx].insertPoint(p);
            }
            else { points_.emplace_back(p); }
        }
    }
    else
    {
        idx = quadrantIdx(p);
        quadrants_[idx].insertPoint(p);
    }
}

void Quadtree::createQuadrants()
{
    for (size_t i = 0; i < QUADRANTS_PER_NODE; i++)
    {
        auto newCenter = center_;
        newCenter.setX(newCenter.getX() + radius_ * ((i & 2U) != 0U ? 0.5F : -0.5F));
		newCenter.setY(newCenter.getY() + radius_ * ((i & 1U) != 0U ? 0.5F : -0.5F));
        quadrants_.emplace_back(newCenter, 0.5F * radius_, MAX_POINTS);
    }
}

void Quadtree::fillQuadrants()
{
    for (Lpoint* p : points_)
    {
        const auto idx = quadrantIdx(p);
        quadrants_[idx].insertPoint(p);
    }

    points_.clear();
}

size_t Quadtree::quadrantIdx(const Lpoint* p) const
{
    size_t child = 0;

    if (p->getX() >= center_.getX()) { child |= 2U; }
	if (p->getY() >= center_.getY()) { child |= 1U; }

	return child;
}

void Quadtree::buildQuadtree(std::vector<Lpoint>& points)
{
    computeQuadtreeLimits();
    insertPoints(points);
}

std::vector<std::pair<Point, Point>> Quadtree::findLeafs()
{
    std::vector<std::pair<Point, Point>> quads;
    if (isLeaf())
    {
        if (!isEmpty()) { quads.emplace_back(min_, max_); }
        return quads;
    }
    for (auto& q : quadrants_)
    {
        auto ret = q.findLeafs();
        if (!ret.empty()) quads.insert(quads.end(), ret.begin(), ret.end());
    }
    return quads;
}

std::vector<std::pair<Point, Point>> Quadtree::getQuadrants()
{
    std::vector<std::pair<Point, Point>> quads;
    for (auto& q : quadrants_)
    {
        auto ret = q.findLeafs();
        if (!ret.empty()) quads.insert(quads.end(), ret.begin(), ret.end());
    }
    return quads;
}